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
**   Manage MQTT interface
**
** Notes:
**   None
**
** References:
**   1. cFS Basecamp Object-based Application Developer's Guide
**   2. cFS Application Developer's Guide
**
*/

/*
** Includes
*/

#include "app_cfg.h"
#include "mqtt_mgr.h"


/*******************************/
/** Local Function Prototypes **/
/*******************************/

static void ProcessSbTopicMsgs(uint32 PerfId);
static void SubscribeToTopicMessages(void);

/*****************/
/** Global Data **/
/*****************/

static MQTT_MGR_Class_t* MqttMgr;


/******************************************************************************
** Function: MQTT_MGR_Constructor
**
** Initialize the MQTT Manager object
**
** Notes:
**   1. This must be called prior to any other member functions.
**   2. The Subscription calls must be made after MQTT_CLIENT and MSG_TRANS
**      have been initialized.
**
*/
void MQTT_MGR_Constructor(MQTT_MGR_Class_t *MqttMgrPtr,
                          const INITBL_Class_t *IniTbl, TBLMGR_Class_t *TblMgr)
{

   MqttMgr = MqttMgrPtr;
   
   memset(MqttMgr, 0, sizeof( MQTT_MGR_Class_t));
   
   MqttMgr->IniTbl = IniTbl;
   MqttMgr->MqttYieldTime = INITBL_GetIntConfig(IniTbl, CFG_MQTT_CLIENT_YIELD_TIME);
   MqttMgr->SbPendTime    = INITBL_GetIntConfig(IniTbl, CFG_TOPIC_PIPE_PEND_TIME);
   
   CFE_SB_CreatePipe(&MqttMgr->TopicPipe, INITBL_GetIntConfig(IniTbl, CFG_TOPIC_PIPE_DEPTH),
                     INITBL_GetStrConfig(IniTbl, CFG_TOPIC_PIPE_NAME));
   
   MQTT_CLIENT_Constructor(&MqttMgr->MqttClient, IniTbl);

   MSG_TRANS_Constructor(&MqttMgr->MsgTrans, IniTbl, TblMgr);

   SubscribeToTopicMessages();
      
} /* End MQTT_MGR_Constructor() */



/******************************************************************************
** Function: MQTT_MGR_ChildTaskCallback
**
*/
bool MQTT_MGR_ChildTaskCallback(CHILDMGR_Class_t *ChildMgr)
{

   MQTT_CLIENT_Yield(MqttMgr->MqttYieldTime);

   return true;
   
} /* End MQTT_MGR_ChildTaskCallback() */


/******************************************************************************
** Function: MQTT_MGR_ConfigSbTopicTestCmd
**
** Notes:
**   None
*/
bool MQTT_MGR_ConfigSbTopicTestCmd(void* DataObjPtr, const CFE_MSG_Message_t *MsgPtr)
{

   const MQTT_GW_ConfigSbTopicTest_Payload_t *ConfigSbTopicTestCmd = CMDMGR_PAYLOAD_PTR(MsgPtr, MQTT_GW_ConfigSbTopicTest_t);
   bool RetStatus = false;

   if (MQTT_TOPIC_TBL_ValidId(ConfigSbTopicTestCmd->Id))
   {
      if (ConfigSbTopicTestCmd->Action == MQTT_GW_TestAction_Start)
      {
         MqttMgr->SbTopicTestId     = ConfigSbTopicTestCmd->Id;
         MqttMgr->SbTopicTestParam  = ConfigSbTopicTestCmd->Param;
         MqttMgr->SbTopicTestActive = true;
         RetStatus = true;
         CFE_EVS_SendEvent(MQTT_MGR_CONFIG_TEST_EID, CFE_EVS_EventType_INFORMATION, 
                           "Started SB test for topic ID %d(table index %d)",
                           (ConfigSbTopicTestCmd->Id+1),ConfigSbTopicTestCmd->Id);
         MQTT_TOPIC_TBL_RunSbMsgTest(MqttMgr->SbTopicTestId, true, ConfigSbTopicTestCmd->Param);
      }
      else if (ConfigSbTopicTestCmd->Action == MQTT_GW_TestAction_Stop)
      {
         MqttMgr->SbTopicTestId = ConfigSbTopicTestCmd->Id;
         MqttMgr->SbTopicTestActive = false;
         RetStatus = true;
         CFE_EVS_SendEvent(MQTT_MGR_CONFIG_TEST_EID, CFE_EVS_EventType_INFORMATION, 
                           "Stopped SB test for topic ID %d(table index %d)",
                           (ConfigSbTopicTestCmd->Id+1),ConfigSbTopicTestCmd->Id);
      }
      else
      {
         CFE_EVS_SendEvent(MQTT_MGR_CONFIG_TEST_ERR_EID, CFE_EVS_EventType_ERROR, 
                           "Configured SB topic test command rejected. Invalid start/stop parameter %d", 
                           ConfigSbTopicTestCmd->Action);
      
      }
   }
   else
   {
      CFE_EVS_SendEvent(MQTT_MGR_CONFIG_TEST_ERR_EID, CFE_EVS_EventType_ERROR, 
                        "Configured SB topic test command rejected. Id %d(table index %d) either invalid or not loaded", 
                        (ConfigSbTopicTestCmd->Id+1),ConfigSbTopicTestCmd->Id);

   }
  
   return RetStatus;
   
} /* End MQTT_MGR_ConfigSbTopicTest() */


/******************************************************************************
** Function: MQTT_MGR_ConnectToMqttBrokerCmd
**
*/
bool MQTT_MGR_ConnectToMqttBrokerCmd(void* DataObjPtr, const CFE_MSG_Message_t *MsgPtr)
{
   const MQTT_GW_ConnectToMqttBroker_Payload_t *ConnectToMqttBrokerCmd = 
                                               CMDMGR_PAYLOAD_PTR(MsgPtr, MQTT_GW_ConnectToMqttBroker_t);
   bool RetStatus = true;
   const char *BrokerAddress;
   uint32     BrokerPort;
   const char *ClientName;

   if (ConnectToMqttBrokerCmd->BrokerAddress[0] == '\0')
   {
      BrokerAddress = INITBL_GetStrConfig(MqttMgr->IniTbl, CFG_MQTT_BROKER_ADDRESS);
   }
   else
   {
      BrokerAddress = ConnectToMqttBrokerCmd->BrokerAddress;
   }
   
   if (ConnectToMqttBrokerCmd->BrokerPort == 0)
   {
      BrokerPort = INITBL_GetIntConfig(MqttMgr->IniTbl, CFG_MQTT_BROKER_PORT);
   }
   else
   {
      BrokerPort = ConnectToMqttBrokerCmd->BrokerPort;
   }
   
   if (ConnectToMqttBrokerCmd->ClientName[0] == '\0')
   {
      ClientName = INITBL_GetStrConfig(MqttMgr->IniTbl, CFG_MQTT_CLIENT_NAME);
   }
   else
   {
      ClientName = ConnectToMqttBrokerCmd->ClientName;
   }

   MQTT_CLIENT_Connect(ClientName, BrokerAddress, BrokerPort); /* Sends event messages */

   return RetStatus;
   
} /* End MQTT_MGR_ConnectToMqttBrokerCmd() */


/******************************************************************************
** Function: MQTT_MGR_Execute
**
** Perform management functions that need to be performed on a periodic basis.
**
*/
void MQTT_MGR_Execute(uint32 PerfId)
{

   ProcessSbTopicMsgs(PerfId);
   
   if (MqttMgr->SbTopicTestActive)
   {
      MQTT_TOPIC_TBL_RunSbMsgTest(MqttMgr->SbTopicTestId, false, MqttMgr->SbTopicTestParam);
   }

} /* End MQTT_MGR_Execute() */


/******************************************************************************
** Function: MQTT_MGR_ResetStatus
**
** Reset counters and status flags to a known reset state.
**
*/
void MQTT_MGR_ResetStatus(void)
{

   MQTT_CLIENT_ResetStatus();
   MSG_TRANS_ResetStatus();

} /* End MQTT_MGR_ResetStatus() */


/******************************************************************************
** Function: ProcessSbTopicMsgs
**
** Notes:
**   1. MSG_TRANS_ProcessSbMsg() and MQTT_CLIENT_Publish() send error events
**      so no need to send any events here.
*/
static void ProcessSbTopicMsgs(uint32 PerfId)
{

   int32  SbStatus;
   CFE_SB_Buffer_t  *SbBufPtr;
   const char *Topic;
   const char *Payload;

   do 
   {
      CFE_ES_PerfLogExit(PerfId);
      SbStatus = CFE_SB_ReceiveBuffer(&SbBufPtr, MqttMgr->TopicPipe, MqttMgr->SbPendTime);
      CFE_ES_PerfLogEntry(PerfId);
   
      if (SbStatus == CFE_SUCCESS)
      {
         if (MSG_TRANS_ProcessSbMsg(&SbBufPtr->Msg, &Topic, &Payload))
         {
            MQTT_CLIENT_Publish(Topic, Payload);
         }
      }
      
   } while(SbStatus == CFE_SUCCESS);
   
} /* End ProcessSbTopicMsgs() */


/******************************************************************************
** Function: SubscribeToMessages
**
** Subscribe to topic messages on the SB and MQTT_CLIENT based on a topics
** definition in the topic table.
**
*/
static void SubscribeToTopicMessages(void)
{

   uint16 i;
   uint16 SbSubscribeCnt = 0;
   uint16 MqttSubscribeCnt = 0;
   uint16 SubscribeErr = 0;
   
   const MQTT_TOPIC_TBL_Topic_t *Topic;
   
   for (i=0; i < MQTT_GW_PluginTopic_Enum_t_MAX; i++)
   {

      Topic = MQTT_TOPIC_TBL_GetTopic(i);
      if (Topic != NULL)   
      {
         if (Topic->Enabled)
         {

            if (strcmp(Topic->SbRole,"sub") == 0)
            {
               ++SbSubscribeCnt;
               CFE_SB_Subscribe(CFE_SB_ValueToMsgId(Topic->Cfe), MqttMgr->TopicPipe);
               CFE_EVS_SendEvent(MQTT_MGR_SUBSCRIBE_EID, CFE_EVS_EventType_INFORMATION, 
                       "Subscribed to SB for topic 0x%04X(%d)", Topic->Cfe, Topic->Cfe);
            }
            else
            {
               /* MQTTlib does not store a copy of topic so it must be in persistent memory */
               if (MQTT_CLIENT_Subscribe(Topic->Mqtt, MQTT_CLIENT_QOS2, MSG_TRANS_ProcessMqttMsg))
               {
                  ++MqttSubscribeCnt;
                  CFE_EVS_SendEvent(MQTT_MGR_SUBSCRIBE_EID, CFE_EVS_EventType_INFORMATION, 
                          "Subscribed to MQTT client for topic %s", Topic->Mqtt);
               }
               else
               {
                  ++SubscribeErr;
                  CFE_EVS_SendEvent(MQTT_MGR_SUBSCRIBE_ERR_EID, CFE_EVS_EventType_ERROR, 
                          "Error subscribing to MQTT client for topic %s", Topic->Mqtt);
               }
            }
         } /* End if not NULL */
      } /* End if topic in use */
  
   } /* End topic loop */
    
   CFE_EVS_SendEvent(MQTT_MGR_SUBSCRIBE_EID, CFE_EVS_EventType_INFORMATION, 
                     "Topic subscriptions: SB %d, MQTT %d, Errors %d",
                     SbSubscribeCnt, MqttSubscribeCnt, SubscribeErr);
 
} /* End SubscribeToMessages() */


