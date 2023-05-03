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


/**********************/
/** Type Definitions **/
/**********************/

typedef enum
{
   TOPIC_SUB_SB    = 1,
   TOPIC_SUB_MQTT  = 2,
   TOPIC_SUB_ERR   = 3,
   TOPIC_SUB_UNDEF = 4

} TopicSubscribeStatusEnum_t;

typedef enum
{
   TOPIC_SUB_TO_ALL  = 1,
   TOPIC_SUB_TO_MQTT = 2,
   TOPIC_SUB_TO_SB   = 3
   
} TopicSubscribeToEnum_t;


/*******************************/
/** Local Function Prototypes **/
/*******************************/

static void MqttConnectionError(void);
static void ProcessSbTopicMsgs(uint32 PerfId);
static void SubscribeToTopicTblMsgs(TopicSubscribeToEnum_t SubscribeTo);
static TopicSubscribeStatusEnum_t SubscribeToTopicMsg(enum MQTT_GW_TopicPlugin TopicPlugin, TopicSubscribeToEnum_t SubscribeTo);
static bool UnsubscribeFromTopicMsg(enum MQTT_GW_TopicPlugin TopicPlugin);


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
   
   MqttMgr->Reconnect.Enabled  = (INITBL_GetIntConfig(IniTbl, CFG_MQTT_ENABLE_RECONNECT) == 1);
   MqttMgr->Reconnect.Period   = INITBL_GetIntConfig(IniTbl, CFG_MQTT_RECONNECT_PERIOD);
   MqttMgr->Reconnect.DelayCnt = MqttMgr->Reconnect.Period;
   
   CFE_SB_CreatePipe(&MqttMgr->TopicPipe, INITBL_GetIntConfig(IniTbl, CFG_TOPIC_PIPE_DEPTH),
                     INITBL_GetStrConfig(IniTbl, CFG_TOPIC_PIPE_NAME));
   
   MQTT_CLIENT_Constructor(&MqttMgr->MqttClient, IniTbl);

   MSG_TRANS_Constructor(&MqttMgr->MsgTrans, IniTbl, TblMgr);

   if (MqttMgr->MqttClient.Connected)
   {
      SubscribeToTopicTblMsgs(TOPIC_SUB_TO_ALL);
   }
   
} /* End MQTT_MGR_Constructor() */



/******************************************************************************
** Function: MQTT_MGR_ChildTaskCallback
**
*/
bool MQTT_MGR_ChildTaskCallback(CHILDMGR_Class_t *ChildMgr)
{

   bool ClientYield;
   
   ClientYield = MQTT_CLIENT_Yield(MqttMgr->MqttYieldTime);

   if (!ClientYield)
   {
      MqttConnectionError(); 
   } 
   
   return true;
   
} /* End MQTT_MGR_ChildTaskCallback() */


/******************************************************************************
** Function: MQTT_MGR_ConfigTopicPluginCmd
**
** Enable/disable a plugin topic
**
** Notes:
**   1. Signature must match CMDMGR_CmdFuncPtr_t
**   2. The functions called send error events so this function only needs to
**      report a successful command. 
**
*/
bool MQTT_MGR_ConfigTopicPluginCmd(void* DataObjPtr, const CFE_MSG_Message_t *MsgPtr)
{

   const MQTT_GW_ConfigTopicPlugin_Payload_t *ConfigTopicPlugin = CMDMGR_PAYLOAD_PTR(MsgPtr, MQTT_GW_ConfigTopicPlugin_t);
   bool RetStatus = false;
   TopicSubscribeStatusEnum_t TopicSubscription;
   
   if (ConfigTopicPlugin->Action == APP_C_FW_ConfigEnaAction_ENABLE)
   {
      if ((RetStatus = MQTT_TOPIC_TBL_EnablePlugin(ConfigTopicPlugin->Id)))
      {
         TopicSubscription = SubscribeToTopicMsg(ConfigTopicPlugin->Id, TOPIC_SUB_TO_ALL);
         if ((TopicSubscription == TOPIC_SUB_MQTT) || (TopicSubscription == TOPIC_SUB_SB))
         {
            CFE_EVS_SendEvent(MQTT_MGR_CONFIG_TOPIC_PLUGIN_EID, CFE_EVS_EventType_INFORMATION, 
                              "Sucessfully enabled topic %d",ConfigTopicPlugin->Id);
         }
         else
         {
            MQTT_TOPIC_TBL_DisablePlugin(ConfigTopicPlugin->Id);            
         }
      }
   }
   else if (ConfigTopicPlugin->Action == APP_C_FW_ConfigEnaAction_DISABLE)
   {
      if ((RetStatus = MQTT_TOPIC_TBL_DisablePlugin(ConfigTopicPlugin->Id)))
      {
         if ((RetStatus = UnsubscribeFromTopicMsg(ConfigTopicPlugin->Id)))
         {
            CFE_EVS_SendEvent(MQTT_MGR_CONFIG_TOPIC_PLUGIN_EID, CFE_EVS_EventType_INFORMATION, 
                              "Sucessfully disabled plugin topic %d",ConfigTopicPlugin->Id);
         }
      }
   }
   else
   {
      CFE_EVS_SendEvent(MQTT_MGR_CONFIG_TOPIC_PLUGIN_EID, CFE_EVS_EventType_ERROR, 
                        "Configure plugin topic %d command rejected. Invalid action %d",
                        ConfigTopicPlugin->Id, ConfigTopicPlugin->Action);
   }
  
   return RetStatus;
    
   
} /* End MQTT_MGR_ConfigTopicPluginCmd() */


/******************************************************************************
** Function: MQTT_MGR_ConfigSbTopicTestCmd
**
** Notes:
**   None
*/
bool MQTT_MGR_ConfigSbTopicTestCmd(void* DataObjPtr, const CFE_MSG_Message_t *MsgPtr)
{

   const MQTT_GW_ConfigSbTopicTest_Payload_t *ConfigSbTopicTest = CMDMGR_PAYLOAD_PTR(MsgPtr, MQTT_GW_ConfigSbTopicTest_t);
   bool RetStatus = false;

   if (MQTT_TOPIC_TBL_ValidTopicPlugin(ConfigSbTopicTest->Id))
   {
      if (ConfigSbTopicTest->Action == APP_C_FW_ConfigExeAction_START)
      {
         MqttMgr->SbTopicTestId     = ConfigSbTopicTest->Id;
         MqttMgr->SbTopicTestParam  = ConfigSbTopicTest->Param;
         MqttMgr->SbTopicTestActive = true;
         RetStatus = true;
         CFE_EVS_SendEvent(MQTT_MGR_CONFIG_TOPIC_PLUGIN_TEST_EID, CFE_EVS_EventType_INFORMATION, 
                           "Started SB test for topic ID %d(table index %d)",
                           ConfigSbTopicTest->Id,(ConfigSbTopicTest->Id-1));
         MQTT_TOPIC_TBL_RunSbMsgTest(MqttMgr->SbTopicTestId, true, ConfigSbTopicTest->Param);
      }
      else if (ConfigSbTopicTest->Action == APP_C_FW_ConfigExeAction_STOP)
      {
         MqttMgr->SbTopicTestId = ConfigSbTopicTest->Id;
         MqttMgr->SbTopicTestActive = false;
         RetStatus = true;
         CFE_EVS_SendEvent(MQTT_MGR_CONFIG_TOPIC_PLUGIN_TEST_EID, CFE_EVS_EventType_INFORMATION, 
                           "Stopped SB test for topic ID %d(table index %d)",
                           ConfigSbTopicTest->Id,(ConfigSbTopicTest->Id-1));
      }
      else
      {
         CFE_EVS_SendEvent(MQTT_MGR_CONFIG_TOPIC_PLUGIN_TEST_EID, CFE_EVS_EventType_ERROR, 
                           "Configure SB topic test command rejected. Invalid start/stop parameter %d", 
                           ConfigSbTopicTest->Action);
      
      }
   }
   else
   {
      CFE_EVS_SendEvent(MQTT_MGR_CONFIG_TOPIC_PLUGIN_TEST_EID, CFE_EVS_EventType_ERROR, 
                        "Configure SB topic test command rejected. Id %d(table index %d) either invalid or not loaded", 
                        ConfigSbTopicTest->Id,(ConfigSbTopicTest->Id-1));

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
   bool RetStatus = false;
   const char *BrokerAddress;
   uint32     BrokerPort;
   const char *ClientName;

   if (ConnectToMqttBrokerCmd->BrokerAddress[0] == '\0')
   {
      BrokerAddress = MqttMgr->MqttClient.BrokerAddress;
   }
   else
   {
      BrokerAddress = ConnectToMqttBrokerCmd->BrokerAddress;
   }
   
   if (ConnectToMqttBrokerCmd->BrokerPort == 0)
   {
      BrokerPort = MqttMgr->MqttClient.BrokerPort;
   }
   else
   {
      BrokerPort = ConnectToMqttBrokerCmd->BrokerPort;
   }
   
   if (ConnectToMqttBrokerCmd->ClientName[0] == '\0')
   {
      ClientName = MqttMgr->MqttClient.ClientName;
   }
   else
   {
      ClientName = ConnectToMqttBrokerCmd->ClientName;
   }

   // Connect sends event messages
   if (MQTT_CLIENT_Connect(ClientName, BrokerAddress, BrokerPort))
   {
      SubscribeToTopicTblMsgs(TOPIC_SUB_TO_MQTT);
      RetStatus = true;
   }
   
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
** Function: MQTT_MGR_ReconnectToMqttBrokerCmd
**
*/
bool MQTT_MGR_ReconnectToMqttBrokerCmd(void* DataObjPtr, const CFE_MSG_Message_t *MsgPtr)
{

   bool RetStatus = false;

   if (MQTT_CLIENT_Reconnect())
   {
      SubscribeToTopicTblMsgs(TOPIC_SUB_TO_MQTT);
      RetStatus = true;
   }
   
   return RetStatus;
   
} /* End MQTT_MGR_ReconnectToMqttBrokerCmd() */


/******************************************************************************
** Function: MQTT_MGR_ResetStatus
**
** Reset counters and status flags to a known reset state.
**
*/
void MQTT_MGR_ResetStatus(void)
{

   MqttMgr->UnpublishedSbMsgCnt = 0;
   MQTT_CLIENT_ResetStatus();
   MSG_TRANS_ResetStatus();

} /* End MQTT_MGR_ResetStatus() */


/******************************************************************************
** Function: MqttConnectionError
**
** Report an error using the MQTT connection
**
** Notes:
**   1. Other than the constructor this should be the only function that
**      modifies the reconnect data structure.
*/
static void MqttConnectionError(void)
{
   
   if (MqttMgr->Reconnect.Enabled)
   {
      MqttMgr->Reconnect.DelayCnt++;
      if (MqttMgr->Reconnect.DelayCnt >= MqttMgr->Reconnect.Period)
      {
         CFE_EVS_SendEvent(MQTT_MGR_RECONNECT_EID, CFE_EVS_EventType_INFORMATION, 
                           "Attempting MQTT broker reconnect");
         
         if (MQTT_CLIENT_Reconnect())
         {
            SubscribeToTopicTblMsgs(TOPIC_SUB_TO_MQTT);
         }
         MqttMgr->Reconnect.Attempts++; 
         MqttMgr->Reconnect.DelayCnt = 0;
      }
   }
   
} /* MqttConnectionError() */


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
         if (MqttMgr->MqttClient.Connected)
         {
            if (MSG_TRANS_ProcessSbMsg(&SbBufPtr->Msg, &Topic, &Payload))
            {
               if(!MQTT_CLIENT_Publish(Topic, Payload))
               {
                  MqttConnectionError();
               }
            }
         }
         else
         {
            MqttMgr->UnpublishedSbMsgCnt++;
         }
      }
      
   } while(SbStatus == CFE_SUCCESS);
   
} /* End ProcessSbTopicMsgs() */


/******************************************************************************
** Function: SubscribeToTopicMsg
**
** Subscribe to topic message on the SB and MQTT_CLIENT based on a topic's
** definition in the topic table.
**
*/
static TopicSubscribeStatusEnum_t SubscribeToTopicMsg(enum MQTT_GW_TopicPlugin TopicPlugin,
                                                      TopicSubscribeToEnum_t SubscribeTo )
{
   
   CFE_SB_Qos_t Qos;
   const MQTT_TOPIC_TBL_Topic_t *Topic;
   TopicSubscribeStatusEnum_t TopicSubscription = TOPIC_SUB_ERR;

   Qos.Priority    = 0;
   Qos.Reliability = 0;
   Topic = MQTT_TOPIC_TBL_GetTopic(TopicPlugin);
   if (Topic != NULL)   
   {
      if (Topic->Enabled)
      {
         // Table load logic doesn't enable an invalid SbRole so don't report invalid
         if (Topic->SbRole == MQTT_GW_TopicSbRole_PUBLISH)
         {
            if (SubscribeTo == TOPIC_SUB_TO_ALL || SubscribeTo == TOPIC_SUB_TO_MQTT)
            {
               if (MQTT_CLIENT_Subscribe(Topic->Mqtt, MQTT_CLIENT_QOS2, MSG_TRANS_ProcessMqttMsg))
               {
                  TopicSubscription = TOPIC_SUB_MQTT;
                  CFE_EVS_SendEvent(MQTT_MGR_SUBSCRIBE_EID, CFE_EVS_EventType_INFORMATION, 
                          "Subscribed to MQTT client for topic %s", Topic->Mqtt);
               }
               else
               {
                  CFE_EVS_SendEvent(MQTT_MGR_SUBSCRIBE_EID, CFE_EVS_EventType_ERROR, 
                          "Error subscribing to MQTT client for topic %s", Topic->Mqtt);
               }
            }
         }
         else if (Topic->SbRole == MQTT_GW_TopicSbRole_SUBSCRIBE)
         {
            if (SubscribeTo == TOPIC_SUB_TO_ALL || SubscribeTo == TOPIC_SUB_TO_SB)
            {
               TopicSubscription = TOPIC_SUB_SB;
               CFE_SB_SubscribeEx(CFE_SB_ValueToMsgId(Topic->Cfe), MqttMgr->TopicPipe, Qos, 20);
               CFE_EVS_SendEvent(MQTT_MGR_SUBSCRIBE_EID, CFE_EVS_EventType_INFORMATION, 
                       "Subscribed to SB for topic 0x%04X(%d)", Topic->Cfe, Topic->Cfe);
            }
         }
      } /* End if enabled */
      else
      {
         TopicSubscription = TOPIC_SUB_UNDEF;
      }
   } /* End if not NULL */

   return TopicSubscription;
   
} /* End SubscribeToTopicMsg() */


/******************************************************************************
** Function: SubscribeToTopicTblMsgs
**
** Subscribe to all topics defined in the topic table. A topic's SbRole
** determines which type of subscription will be performed.
**
*/
static void SubscribeToTopicTblMsgs(TopicSubscribeToEnum_t SubscribeTo)
{

   TopicSubscribeStatusEnum_t TopicSubscription;
   
   uint16 SbSubscribeCnt = 0;
   uint16 MqttSubscribeCnt = 0;
   uint16 SubscribeErr = 0;
   
 
   // MQTT_GW_TopicPlugin_Enum_t_MAX is defined as a last enum value to represent null 
   for (enum MQTT_GW_TopicPlugin i=MQTT_GW_TopicPlugin_Enum_t_MIN; i < MQTT_GW_TopicPlugin_Enum_t_MAX; i++)
   {
      TopicSubscription = SubscribeToTopicMsg(i, SubscribeTo);
      switch (TopicSubscription)
      {
         case TOPIC_SUB_SB:
            SbSubscribeCnt++;
            break;
         case TOPIC_SUB_MQTT:
            MqttSubscribeCnt++;
            break;
         case TOPIC_SUB_ERR:
            SubscribeErr++;
            break;
         default:
            break;
      } /** End switch */
   } /* End topic loop */
    
   CFE_EVS_SendEvent(MQTT_MGR_SUBSCRIBE_EID, CFE_EVS_EventType_INFORMATION, 
                     "Topic subscriptions: SB %d, MQTT %d, Errors %d",
                     SbSubscribeCnt, MqttSubscribeCnt, SubscribeErr);
 
} /* End SubscribeToTopicTblMsgs() */


/******************************************************************************
** Function: UnsubscribeFromTopicMsg
**
** Unsubscribe to topic messages on the SB and MQTT_CLIENT based on a topics
** definition in the topic table.
**
*/
static bool UnsubscribeFromTopicMsg(enum MQTT_GW_TopicPlugin TopicPlugin)
{

   const MQTT_TOPIC_TBL_Topic_t *Topic;
   bool RetStatus = false;
   int32 SbStatus;

   Topic = MQTT_TOPIC_TBL_GetDisabledTopic(TopicPlugin);
   if (Topic != NULL)   
   {
      if (Topic->Enabled == false)
      {
         if (Topic->SbRole == MQTT_GW_TopicSbRole_PUBLISH)
         {
            RetStatus = MQTT_CLIENT_Unsubscribe(Topic->Mqtt);
         }
         else if (Topic->SbRole == MQTT_GW_TopicSbRole_SUBSCRIBE)
         {
            SbStatus = CFE_SB_Unsubscribe(CFE_SB_ValueToMsgId(Topic->Cfe), MqttMgr->TopicPipe);
            if(SbStatus == CFE_SUCCESS)
            {
               RetStatus = true;
            }
            else
            {
               CFE_EVS_SendEvent(MQTT_MGR_UNSUBSCRIBE_EID, CFE_EVS_EventType_ERROR,
                                 "Error unsubscribing plugin topic ID %d(index %d) with SB message ID 0x%04X(%d). Unsubscribe status 0x%8X",
                                 (TopicPlugin+1), TopicPlugin, Topic->Cfe, Topic->Cfe, SbStatus);
            }
         }
      } /* End if disabled */
   } /* End if not NULL */

   return RetStatus;
 
} /* End UnsubscribeFromTopicMsg() */
