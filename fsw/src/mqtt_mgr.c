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
*/

/*
** Includes
*/

#include "mqtt_mgr.h"

/***********************/
/** Macro Definitions **/
/***********************/

/* Convenience macros */
#define  INITBL_OBJ   (MqttMgr->IniTbl)


/**********************/
/** Type Definitions **/
/**********************/


/*******************************/
/** Local Function Prototypes **/
/*******************************/

static bool ConfigSubscription(const JMSG_TOPIC_TBL_Topic_t *Topic, JMSG_TOPIC_TBL_SubscriptionOptEnum_t ConfigOpt);
static void MqttConnectionError(void);

/**********************/
/** Global File Data **/
/**********************/

static MQTT_MGR_Class_t *MqttMgr;


/******************************************************************************
** Function: MQTT_MGR_Constructor
**
** Initialize the MQTT Manager object
**
** Notes:
**   1. This must be called prior to any other member functions.
**   2. JMSG_TOPIC_TBL is constructed by JMSG_LIB that must be loaded prior to
**      this app. See inline comments for topic regitration constraints. 
**
*/
void MQTT_MGR_Constructor(MQTT_MGR_Class_t *MqttMgrPtr, const INITBL_Class_t *IniTbl)
{
   
   int32 SysStatus;
   
   MqttMgr = MqttMgrPtr;
   
   memset(MqttMgr, 0, sizeof( MQTT_MGR_Class_t));
   
   MqttMgr->IniTbl = IniTbl;
   MqttMgr->MqttYieldTime = INITBL_GetIntConfig(INITBL_OBJ, CFG_MQTT_CLIENT_YIELD_TIME);
   MqttMgr->SbPendTime    = INITBL_GetIntConfig(INITBL_OBJ, CFG_TOPIC_PIPE_PEND_TIME);
   
   MqttMgr->Reconnect.Enabled  = (INITBL_GetIntConfig(INITBL_OBJ, CFG_MQTT_ENABLE_RECONNECT) == 1);
   MqttMgr->Reconnect.Period   = INITBL_GetIntConfig(INITBL_OBJ, CFG_MQTT_RECONNECT_PERIOD);
   MqttMgr->Reconnect.DelayCnt = MqttMgr->Reconnect.Period;
   
   MQTT_CLIENT_Constructor(&MqttMgr->MqttClient, INITBL_OBJ);

   MQMSG_TRANS_Constructor(&MqttMgr->MqMsgTrans, INITBL_OBJ);

   SysStatus = CFE_SB_CreatePipe(&MqttMgr->TopicPipe, INITBL_GetIntConfig(INITBL_OBJ, CFG_TOPIC_PIPE_DEPTH),
                                 INITBL_GetStrConfig(INITBL_OBJ, CFG_TOPIC_PIPE_NAME));
   if (SysStatus != CFE_SUCCESS)
   {
      CFE_EVS_SendEvent(MQTT_MGR_CONSTRUCTOR_EID, CFE_EVS_EventType_ERROR, 
                        "Error creating topic pipe %s,depth %d, return status %d",
                        INITBL_GetStrConfig(INITBL_OBJ, CFG_TOPIC_PIPE_NAME),
                        INITBL_GetIntConfig(INITBL_OBJ, CFG_TOPIC_PIPE_DEPTH), SysStatus);      
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
** Function: MQTT_MGR_ConnectToMqttBrokerCmd
**
*/
bool MQTT_MGR_ConnectToMqttBrokerCmd(void* DataObjPtr, const CFE_MSG_Message_t *MsgPtr)
{
   const JMSG_MQTT_ConnectToMqttBroker_CmdPayload_t *ConnectToMqttBrokerCmd = 
                                               CMDMGR_PAYLOAD_PTR(MsgPtr, JMSG_MQTT_ConnectToMqttBroker_t);
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
      JMSG_TOPIC_TBL_SubscribeToAll(JMSG_TOPIC_TBL_SUB_JMSG);
      RetStatus = true;
   }
   
   return RetStatus;
   
} /* End MQTT_MGR_ConnectToMqttBrokerCmd() */


/******************************************************************************
** Function: MQTT_MGR_ProcessSbTopicMsgs
**
** Notes:
**   1. MQMSG_TRANS_ProcessSbMsg() and MQTT_CLIENT_Publish() send error events
**      so no need to send any events here.
**
*/
void MQTT_MGR_ProcessSbTopicMsgs(uint32 PerfId)
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
            if (MQMSG_TRANS_ProcessSbMsg(&SbBufPtr->Msg, &Topic, &Payload))
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
   
} /* End MQTT_MGR_ProcessSbTopicMsgs() */


/******************************************************************************
** Function: MQTT_MGR_ReconnectToMqttBrokerCmd
**
*/
bool MQTT_MGR_ReconnectToMqttBrokerCmd(void* DataObjPtr, const CFE_MSG_Message_t *MsgPtr)
{

   bool RetStatus = false;

   if (MQTT_CLIENT_Reconnect())
   {
      JMSG_TOPIC_TBL_SubscribeToAll(JMSG_TOPIC_TBL_SUB_JMSG);
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
   MQMSG_TRANS_ResetStatus();

} /* End MQTT_MGR_ResetStatus() */


/******************************************************************************
** Function: MQTT_MGR_SendConnectionInfoCmd
**
** Notes:
**   1. Signature must match CMDMGR_CmdFuncPtr_t
**   2. DataObjPtr is not used
*/
bool MQTT_MGR_SendConnectionInfoCmd(void* DataObjPtr, const CFE_MSG_Message_t *MsgPtr)
{
   char ConnectStatus[32];
   
   
   if (MqttMgr->MqttClient.Connected)
   {
      strcpy(ConnectStatus,"connected to");
   }
   else
   {
      strcpy(ConnectStatus,"disconnected from");      
   }
   CFE_EVS_SendEvent(MQTT_MGR_SEND_CONNECTION_INFO_EID, CFE_EVS_EventType_INFORMATION, 
                     "MQTT %s broker %s:%d", ConnectStatus,
                     INITBL_GetStrConfig(INITBL_OBJ, CFG_MQTT_BROKER_ADDRESS),
                     INITBL_GetIntConfig(INITBL_OBJ, CFG_MQTT_BROKER_PORT));
   
   return true;
   
} /* MQTT_MGR_SendConnectionInfoCmd() */


/******************************************************************************
** Function: MQTT_MGR_SubscribeToTopicPlugin
**
*/
bool MQTT_MGR_SubscribeToTopicPlugin(const CFE_MSG_Message_t *MsgPtr)
{
   const JMSG_LIB_TopicSubscribeTlm_Payload_t *TopicSubscribe = CMDMGR_PAYLOAD_PTR(MsgPtr, JMSG_LIB_TopicSubscribeTlm_t);
   bool RetStatus = true;

   if (TopicSubscribe->Protocol == JMSG_LIB_TopicProtocol_MQTT)
   {
      JMSG_TOPIC_TBL_SubscriptionOptEnum_t SubscriptionOpt;

      RetStatus = JMSG_TOPIC_TBL_RegisterConfigSubscriptionCallback(TopicSubscribe->Id, ConfigSubscription);      
     
      if (RetStatus)
      {

         const JMSG_TOPIC_TBL_Topic_t *Topic = JMSG_TOPIC_TBL_GetTopic(TopicSubscribe->Id);
               
         SubscriptionOpt = JMSG_TOPIC_TBL_SubscribeToTopicMsg(TopicSubscribe->Id, JMSG_TOPIC_TBL_SUB_TO_ROLE);
         if (SubscriptionOpt == JMSG_TOPIC_TBL_SUB_ERR)
         {
            RetStatus = false;
            CFE_EVS_SendEvent(MQTT_MGR_SUBSCRIBE_TOPIC_PLUGIN_EID, CFE_EVS_EventType_ERROR, 
                              "Error subscribing to topic Id: %d, Name: %s, cFE Msg: 0x%04X(%d)", 
                              TopicSubscribe->Id, Topic->Name, Topic->Cfe, Topic->Cfe);                              
         }
         else
         {
            CFE_EVS_SendEvent(MQTT_MGR_SUBSCRIBE_TOPIC_PLUGIN_EID, CFE_EVS_EventType_INFORMATION, 
                              "Successfully subscribed to topic Id: %d, Name: %s, cFE Msg: 0x%04X(%d)", 
                              TopicSubscribe->Id, Topic->Name, Topic->Cfe, Topic->Cfe);
         }
         
      } /* End if registered */
   
   } /* End if MQTT protocol */
   
   return RetStatus;
   
} /* End MQTT_MGR_SubscribeToTopicPlugin() */


/******************************************************************************
** Function: ConfigSubscription
**
** Callback function that is called when a topic plugin's configuration
** is changed. Perform functions that apply to the network layer.
**
*/
static bool ConfigSubscription(const JMSG_TOPIC_TBL_Topic_t *Topic, 
                               JMSG_TOPIC_TBL_SubscriptionOptEnum_t ConfigOpt)
{

   bool RetStatus = false;
   int32 SbStatus;
   CFE_SB_Qos_t Qos;
        
   switch (ConfigOpt)
   {

      case JMSG_TOPIC_TBL_SUB_SB:
         Qos.Priority    = 0;
         Qos.Reliability = 0;
         SbStatus = CFE_SB_SubscribeEx(CFE_SB_ValueToMsgId(Topic->Cfe), MqttMgr->TopicPipe, Qos, 20);
         if (SbStatus == CFE_SUCCESS)
         {
            RetStatus = true;
            CFE_EVS_SendEvent(MQTT_MGR_CONFIG_SUBSCRIPTIONS_EID, CFE_EVS_EventType_INFORMATION, 
                           "Subscribed to SB for topic 0x%04X(%d)", Topic->Cfe, Topic->Cfe);
         }
         else
         {
            CFE_EVS_SendEvent(MQTT_MGR_CONFIG_SUBSCRIPTIONS_EID, CFE_EVS_EventType_ERROR, 
                           "Error subscribing to SB for topic 0x%04X(%d)", Topic->Cfe, Topic->Cfe);
         }
         break;
         
      case JMSG_TOPIC_TBL_SUB_JMSG:
         if (MQTT_CLIENT_Subscribe(Topic->Name, MQTT_CLIENT_QOS2, MQMSG_TRANS_ProcessMqttMsg))
         {
            RetStatus = true;
            CFE_EVS_SendEvent(MQTT_MGR_CONFIG_SUBSCRIPTIONS_EID, CFE_EVS_EventType_INFORMATION, 
                              "Subscribed to MQTT client for topic %s", Topic->Name);
         }
         else
         {
            CFE_EVS_SendEvent(MQTT_MGR_CONFIG_SUBSCRIPTIONS_EID, CFE_EVS_EventType_ERROR, 
                    "Error subscribing to MQTT client for topic %s", Topic->Name);
         }
         break;
         
      case JMSG_TOPIC_TBL_UNSUB_SB:
         SbStatus = CFE_SB_Unsubscribe(CFE_SB_ValueToMsgId(Topic->Cfe), MqttMgr->TopicPipe);
         if(SbStatus == CFE_SUCCESS)
         {
            RetStatus = true;
            CFE_EVS_SendEvent(MQTT_MGR_CONFIG_SUBSCRIPTIONS_EID, CFE_EVS_EventType_INFORMATION, 
                             "Unsubscribed from SB for topic %s", Topic->Name);
         }
         else
         {
            CFE_EVS_SendEvent(MQTT_MGR_CONFIG_SUBSCRIPTIONS_EID, CFE_EVS_EventType_ERROR, 
                             "Error unsubscribing from SB for topic %s, status = %d", Topic->Name, SbStatus);            
         }
         break;
      
      case JMSG_TOPIC_TBL_UNSUB_JMSG:
         if (MQTT_CLIENT_Unsubscribe(Topic->Name))
         {
            RetStatus = true;
            CFE_EVS_SendEvent(MQTT_MGR_CONFIG_SUBSCRIPTIONS_EID, CFE_EVS_EventType_INFORMATION, 
                              "Unsubscribed from MQTT client for topic %s", Topic->Name);
         }
         else
         {
            CFE_EVS_SendEvent(MQTT_MGR_CONFIG_SUBSCRIPTIONS_EID, CFE_EVS_EventType_INFORMATION, 
                              "Error unsubscribing from MQTT client for topic %s", Topic->Name);
         }
         break;
      
      default:
         CFE_EVS_SendEvent(MQTT_MGR_CONFIG_SUBSCRIPTIONS_EID, CFE_EVS_EventType_ERROR, 
                           "Invalid subscription configuration option for topic %s", Topic->Name);
         break;

   } /** End switch */

   return RetStatus;
   
} /* End ConfigSubscription() */


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
            JMSG_TOPIC_TBL_SubscribeToAll(JMSG_TOPIC_TBL_SUB_JMSG);
         }
         MqttMgr->Reconnect.Attempts++; 
         MqttMgr->Reconnect.DelayCnt = 0;
      }
   }
   
} /* MqttConnectionError() */
