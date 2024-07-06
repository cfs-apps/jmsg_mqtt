/*
** Copyright 2022 bitValence, Inc.
** All Rights Reserved.
**
** This program is free software; you can modify and/or redistribute it
** under the terms of the GNU Affero General Public License
** as published by the Free Software Foundation; version 3 with
** attribution addendums as found in the LICENSE.txt
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU Affero General Public License for more details.
**
** Purpose:
**   Manage an MQTT gateway between the cFS Software Bus and an MQTT
**   Broker.
**
** Notes:
**   1. This file only performs app-level functions. MQTT_MGR controls
**      the MQTT gateway functionality.
**   2. TODO: Remove 'send Status tlm' logic once the app app has been used
**      in enough use cases to know the current command processing
**      and send Status logic is sufficient 
**
*/

/*
** Includes
*/

#include <string.h>
#include "jmsg_mqtt_app.h"
#include "jmsg_mqtt_eds_cc.h"

/***********************/
/** Macro Definitions **/
/***********************/

/* Convenience macros */
#define  INITBL_OBJ      (&(JMsgMqttApp.IniTbl))
#define  CMDMGR_OBJ      (&(JMsgMqttApp.CmdMgr))
#define  CHILDMGR_OBJ    (&(JMsgMqttApp.ChildMgr))
#define  MQTT_MGR_OBJ    (&(JMsgMqttApp.MqttMgr))

/*******************************/
/** Local Function Prototypes **/
/*******************************/

static int32 InitApp(void);
static int32 ProcessCommands(void);
static void SendStatusPkt(void);


/**********************/
/** File Global Data **/
/**********************/

/* 
** Must match DECLARE ENUM() declaration in app_cfg.h
** Defines "static INILIB_CfgEnum IniCfgEnum"
*/
DEFINE_ENUM(Config,APP_CONFIG)  

static CFE_EVS_BinFilter_t  EventFilters[] =
{  
   /* Event ID                             Mask */
   {MQTT_CLIENT_CONNECT_ERR_EID,           CFE_EVS_FIRST_4_STOP},
   {MQTT_CLIENT_YIELD_ERR_EID,             CFE_EVS_FIRST_4_STOP},
   {MQTT_CLIENT_PUBLISH_EID,               CFE_EVS_FIRST_4_STOP},
   {MQTT_CLIENT_PUBLISH_ERR_EID,           CFE_EVS_FIRST_4_STOP},
   {MQMSG_TRANS_PROCESS_MQTT_MSG_INFO_EID, CFE_EVS_FIRST_4_STOP}, // "INFO_" tag used so other MSG_TRANS_PROCESS_MQTT_MSG_EIDs are not filtered
   {MQMSG_TRANS_PROCESS_SB_MSG_INFO_EID,   CFE_EVS_FIRST_4_STOP}, // "INFO_" tag used so other MSG_TRANS_PROCESS_SB_MSG_EIDs are not filtered 
   {MQTT_MGR_RECONNECT_EID,                CFE_EVS_FIRST_4_STOP}
};

/*****************/
/** Global Data **/
/*****************/

JMSG_MQTT_APP_Class_t   JMsgMqttApp;


/******************************************************************************
** Function: JMSG_MQTT_AppMain
**
*/
void JMSG_MQTT_AppMain(void)
{

   uint32 RunStatus = CFE_ES_RunStatus_APP_ERROR;
   
   CFE_EVS_Register(EventFilters, sizeof(EventFilters)/sizeof(CFE_EVS_BinFilter_t),
                    CFE_EVS_EventFilter_BINARY);

   if (InitApp() == CFE_SUCCESS)      /* Performs initial CFE_ES_PerfLogEntry() call */
   {
      RunStatus = CFE_ES_RunStatus_APP_RUN; 
   }
   
   /*
   ** Main process loop
   */
   while (CFE_ES_RunLoop(&RunStatus))
   {
      
      /*
      ** Since the app is data driven MQTT_MGR_Execute() pends on SB for topic messages
      ** and the command pipe is polled below at a lower rate
      */ 
      MQTT_MGR_ProcessSbTopicMsgs(JMsgMqttApp.PerfId);
      if (++JMsgMqttApp.PollCmdCnt > JMsgMqttApp.PollCmdInterval)
      {
          JMsgMqttApp.PollCmdCnt = 0;
          RunStatus = ProcessCommands();
          SendStatusPkt();
      } 
      
   } /* End CFE_ES_RunLoop */

   CFE_ES_WriteToSysLog("MQTT Gateway App terminating, run status = 0x%08X\n", RunStatus);   /* Use SysLog, events may not be working */

   CFE_EVS_SendEvent(JMSG_MQTT_APP_EXIT_EID, CFE_EVS_EventType_CRITICAL, "MQTT Gateway App terminating, run status = 0x%08X", RunStatus);

   CFE_ES_ExitApp(RunStatus);  /* Let cFE kill the task (and any child tasks) */


} /* End of JMSG_MQTT_AppMain() */



/******************************************************************************
** Function: JMSG_MQTT_APP_NoOpCmd
**
*/

bool JMSG_MQTT_APP_NoOpCmd(void* ObjDataPtr, const CFE_MSG_Message_t *MsgPtr)
{

   CFE_EVS_SendEvent (JMSG_MQTT_APP_NOOP_EID, CFE_EVS_EventType_INFORMATION,
                      "No operation command received for MQTT Gateway App version %d.%d.%d",
                      JMSG_MQTT_APP_MAJOR_VER, JMSG_MQTT_APP_MINOR_VER, JMSG_MQTT_APP_PLATFORM_REV);

   return true;


} /* End JMSG_MQTT_APP_NoOpCmd() */


/******************************************************************************
** Function: JMSG_MQTT_APP_ResetAppCmd
**
*/

bool JMSG_MQTT_APP_ResetAppCmd(void* ObjDataPtr, const CFE_MSG_Message_t *MsgPtr)
{

   CFE_EVS_ResetAllFilters();

   CMDMGR_ResetStatus(CMDMGR_OBJ);
   CHILDMGR_ResetStatus(CHILDMGR_OBJ);
   
   MQTT_MGR_ResetStatus();
	  
   return true;

} /* End JMSG_MQTT_APP_ResetAppCmd() */


/******************************************************************************
** Function: InitApp
**
*/
static int32 InitApp(void)
{

   int32 RetStatus = APP_C_FW_CFS_ERROR;
   
   CFE_SB_Qos_t SbQos;
   CHILDMGR_TaskInit_t ChildTaskInit;


   /*
   ** Read JSON INI Table & class variable defaults defined in JSON  
   */

   if (INITBL_Constructor(INITBL_OBJ, JMSG_MQTT_APP_INI_FILENAME, &IniCfgEnum))
   {
   
      /* Poll for a command every 2 seconds */
      JMsgMqttApp.PollCmdInterval = 2000 / INITBL_GetIntConfig(INITBL_OBJ, CFG_TOPIC_PIPE_PEND_TIME);
      JMsgMqttApp.PollCmdCnt = 0;
      JMsgMqttApp.PerfId = INITBL_GetIntConfig(INITBL_OBJ, CFG_APP_MAIN_PERF_ID);
      CFE_ES_PerfLogEntry(JMsgMqttApp.PerfId);

      JMsgMqttApp.CmdMid         = CFE_SB_ValueToMsgId(INITBL_GetIntConfig(INITBL_OBJ, CFG_JMSG_MQTT_CMD_TOPICID));
      JMsgMqttApp.SendStatusMid  = CFE_SB_ValueToMsgId(INITBL_GetIntConfig(INITBL_OBJ, CFG_SEND_STATUS_TLM_TOPICID));
      JMsgMqttApp.TopicSubTlmMid = CFE_SB_ValueToMsgId(INITBL_GetIntConfig(INITBL_OBJ, CFG_JMSG_LIB_TOPIC_SUBSCRIBE_TLM_TOPICID));
   
      /* Construct contained objects */
      MQTT_MGR_Constructor(MQTT_MGR_OBJ, INITBL_OBJ);

      /* Child Manager constructor sends error events */

      ChildTaskInit.TaskName  = INITBL_GetStrConfig(INITBL_OBJ, CFG_MQTT_CHILD_NAME);
      ChildTaskInit.StackSize = INITBL_GetIntConfig(INITBL_OBJ, CFG_MQTT_CHILD_STACK_SIZE);
      ChildTaskInit.Priority  = INITBL_GetIntConfig(INITBL_OBJ, CFG_MQTT_CHILD_PRIORITY);
      ChildTaskInit.PerfId    = INITBL_GetIntConfig(INITBL_OBJ, CFG_CHILD_TASK_PERF_ID);
      RetStatus = CHILDMGR_Constructor(CHILDMGR_OBJ, ChildMgr_TaskMainCallback,
                                       MQTT_MGR_ChildTaskCallback, &ChildTaskInit); 

      /*
      ** Initialize app level interfaces
      */
 
      CFE_SB_CreatePipe(&JMsgMqttApp.CmdPipe, INITBL_GetIntConfig(INITBL_OBJ, CFG_CMD_PIPE_DEPTH), INITBL_GetStrConfig(INITBL_OBJ, CFG_CMD_PIPE_NAME));  
      CFE_SB_Subscribe(JMsgMqttApp.CmdMid, JMsgMqttApp.CmdPipe);
      
      SbQos.Priority    = 0;
      SbQos.Reliability = 0;
      CFE_SB_SubscribeEx(JMsgMqttApp.TopicSubTlmMid, JMsgMqttApp.CmdPipe, SbQos, JMSG_PLATFORM_TOPIC_PLUGIN_MAX);
      
      //TODO: See file prologue. CFE_SB_Subscribe(JMsgMqttApp.SendStatusMid, JMsgMqttApp.CmdPipe);

      CMDMGR_Constructor(CMDMGR_OBJ);
      CMDMGR_RegisterFunc(CMDMGR_OBJ, JMSG_MQTT_NOOP_CC,   NULL, JMSG_MQTT_APP_NoOpCmd,     0);
      CMDMGR_RegisterFunc(CMDMGR_OBJ, JMSG_MQTT_RESET_CC,  NULL, JMSG_MQTT_APP_ResetAppCmd, 0);

      CMDMGR_RegisterFunc(CMDMGR_OBJ, JMSG_MQTT_CONNECT_TO_MQTT_BROKER_CC,    MQTT_MGR_OBJ, MQTT_MGR_ConnectToMqttBrokerCmd,    sizeof(JMSG_MQTT_ConnectToMqttBroker_CmdPayload_t));
      CMDMGR_RegisterFunc(CMDMGR_OBJ, JMSG_MQTT_RECONNECT_TO_MQTT_BROKER_CC,  MQTT_MGR_OBJ, MQTT_MGR_ReconnectToMqttBrokerCmd,  0);
      CMDMGR_RegisterFunc(CMDMGR_OBJ, JMSG_MQTT_SEND_CONNECTION_INFO_CC,      MQTT_MGR_OBJ, MQTT_MGR_SendConnectionInfoCmd,     0);
      
      CFE_MSG_Init(CFE_MSG_PTR(JMsgMqttApp.StatusTlm.TelemetryHeader), CFE_SB_ValueToMsgId(INITBL_GetIntConfig(INITBL_OBJ, CFG_JMSG_MQTT_STATUS_TLM_TOPICID)), sizeof(JMSG_MQTT_StatusTlm_t));

      /*
      ** Application startup event message
      */
      CFE_EVS_SendEvent(JMSG_MQTT_APP_INIT_APP_EID, CFE_EVS_EventType_INFORMATION,
                        "MQTT Gateway App Initialized. Version %d.%d.%d",
                        JMSG_MQTT_APP_MAJOR_VER, JMSG_MQTT_APP_MINOR_VER, JMSG_MQTT_APP_PLATFORM_REV);
                        
   } /* End if INITBL Constructed */
   
   return RetStatus;

} /* End of InitApp() */


/******************************************************************************
** Function: ProcessCommands
**
** 
*/
static int32 ProcessCommands(void)
{
   
   int32  RetStatus = CFE_ES_RunStatus_APP_RUN;
   int32  SysStatus;

   CFE_SB_Buffer_t  *SbBufPtr;
   CFE_SB_MsgId_t   MsgId = CFE_SB_INVALID_MSG_ID;


   CFE_ES_PerfLogExit(JMsgMqttApp.PerfId);
   SysStatus = CFE_SB_ReceiveBuffer(&SbBufPtr, JMsgMqttApp.CmdPipe, CFE_SB_POLL);
   CFE_ES_PerfLogEntry(JMsgMqttApp.PerfId);

   if (SysStatus == CFE_SUCCESS)
   {
      SysStatus = CFE_MSG_GetMsgId(&SbBufPtr->Msg, &MsgId);

      if (SysStatus == CFE_SUCCESS)
      {

         if (CFE_SB_MsgId_Equal(MsgId, JMsgMqttApp.CmdMid))
         {
            CMDMGR_DispatchFunc(CMDMGR_OBJ, &SbBufPtr->Msg);
         } 
         else if (CFE_SB_MsgId_Equal(MsgId, JMsgMqttApp.SendStatusMid))
         {   
            SendStatusPkt();
         }
         else if (CFE_SB_MsgId_Equal(MsgId, JMsgMqttApp.TopicSubTlmMid))
         {   
            MQTT_MGR_SubscribeToTopicPlugin(&SbBufPtr->Msg);
         }
         else
         {   
            CFE_EVS_SendEvent(JMSG_MQTT_APP_INVALID_MID_EID, CFE_EVS_EventType_ERROR,
                              "Received invalid command packet, MID = 0x%04X(%d)", 
                              CFE_SB_MsgIdToValue(MsgId), CFE_SB_MsgIdToValue(MsgId));
         }

      } /* End if got message ID */
   } /* End if received buffer */
   else
   {
      if (SysStatus != CFE_SB_NO_MESSAGE)
      {
         RetStatus = CFE_ES_RunStatus_APP_ERROR;
      }
   } 

   return RetStatus;
   
} /* End ProcessCommands() */


/******************************************************************************
** Function: SendStatusPkt
**
*/
void SendStatusPkt(void)
{

   JMSG_MQTT_StatusTlm_Payload_t *Payload = &JMsgMqttApp.StatusTlm.Payload;

   /*
   ** Framework Data
   */

   Payload->ValidCmdCnt    = JMsgMqttApp.CmdMgr.ValidCmdCnt;
   Payload->InvalidCmdCnt  = JMsgMqttApp.CmdMgr.InvalidCmdCnt;

   Payload->ChildValidCmdCnt    = JMsgMqttApp.ChildMgr.ValidCmdCnt;
   Payload->ChildInvalidCmdCnt  = JMsgMqttApp.ChildMgr.InvalidCmdCnt;

   
   /*
   ** MQTT Data
   */

   Payload->MqttYieldTime     = JMsgMqttApp.MqttMgr.MqttYieldTime;
   Payload->SbPendTime        = JMsgMqttApp.MqttMgr.SbPendTime;

   Payload->MqttConnected     = JMsgMqttApp.MqttMgr.MqttClient.Connected;
   Payload->ReconnectAttempts = JMsgMqttApp.MqttMgr.Reconnect.Attempts;

   Payload->ValidMqttMsgCnt   = JMsgMqttApp.MqttMgr.MqMsgTrans.ValidMqttMsgCnt;
   Payload->InvalidMqttMsgCnt = JMsgMqttApp.MqttMgr.MqMsgTrans.InvalidMqttMsgCnt;
   Payload->ValidSbMsgCnt     = JMsgMqttApp.MqttMgr.MqMsgTrans.ValidSbMsgCnt;
   Payload->InvalidSbMsgCnt   = JMsgMqttApp.MqttMgr.MqMsgTrans.InvalidSbMsgCnt;

   Payload->UnpublishedSbMsgCnt = JMsgMqttApp.MqttMgr.UnpublishedSbMsgCnt;

   CFE_SB_TimeStampMsg(CFE_MSG_PTR(JMsgMqttApp.StatusTlm.TelemetryHeader));
   CFE_SB_TransmitMsg(CFE_MSG_PTR(JMsgMqttApp.StatusTlm.TelemetryHeader), true);

} /* End SendStatusPkt() */

