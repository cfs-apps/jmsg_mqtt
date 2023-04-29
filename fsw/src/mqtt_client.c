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
**   Manage the MQTT Client and the supported messages
**
** Notes:
**   1. Each supported MQTT topic is listed in a JSON file and each
**      topic has a JSON file that defines the topic's content.
**   2. The Basecamp JSON table coding ideom is use a separate object to manage 
**      the table. Since MQTT manager has very little functionality beyond
**      processing the table, a single object is used for management functions
**      and table processing.
**
** References:
**   1. cFS Basecamp Object-based Application Developer's Guide
**   2. cFS Application Developer's Guide
**
*/

/*
** Include Files:
*/

#include <string.h>
#include <time.h>
#include <stdlib.h>

#include "mqtt_client.h"


/*****************/
/** Global Data **/
/*****************/

static MQTT_CLIENT_Class_t* MqttClient;
static char TestPayload[] = "Test Payload";

/******************************************************************************
** Function: MQTT_CLIENT_Constructor
**
** Notes:
**    1. This function must be called prior to any other functions being
**       called using the same MQTT_CLIENT instance.
*/
void MQTT_CLIENT_Constructor(MQTT_CLIENT_Class_t *MqttClientPtr,
                             const INITBL_Class_t *IniTbl)
{

   MqttClient = MqttClientPtr;
   CFE_PSP_MemSet((void*)MqttClient, 0, sizeof(MQTT_CLIENT_Class_t));
   
   srand(time(NULL));
   strncpy(MqttClient->BrokerAddress,INITBL_GetStrConfig(IniTbl, CFG_MQTT_BROKER_ADDRESS),MAX_CLIENT_PARAM_STR_LEN);
   MqttClient->BrokerPort = INITBL_GetIntConfig(IniTbl, CFG_MQTT_BROKER_PORT);
   sprintf(MqttClient->ClientName,"%s-%d", INITBL_GetStrConfig(IniTbl, CFG_MQTT_CLIENT_NAME), (rand() % 10000));

   MqttClient->PubMsg.qos = MQTT_CLIENT_QOS0;
   MqttClient->PubMsg.retained = 0;
   MqttClient->PubMsg.dup = 0;
   MqttClient->PubMsg.id = 0;
   MqttClient->PubMsg.payload = TestPayload;
   MqttClient->PubMsg.id = strlen(TestPayload);

   MQTT_CLIENT_Connect(MqttClient->ClientName, MqttClient->BrokerAddress, MqttClient->BrokerPort);

} /* End MQTT_CLIENT_Constructor() */
   

/******************************************************************************
** Function: MQTT_CLIENT_Connect
**
** Notes:
**    None
**
*/
bool MQTT_CLIENT_Connect(const char *ClientName, const char *BrokerAddress,
                         uint32 BrokerPort)
{

   bool RetStatus = false;
   int  RetCode;
   
   /* Initial to a local variable to avoid a compiler error */
   MQTTPacket_connectData DefConnectOptions = MQTTPacket_connectData_initializer;
   memcpy(&MqttClient->ConnectData, &DefConnectOptions, sizeof(MQTTPacket_connectData));

   /*
   ** Init and connect to network
   */
   
   NetworkInit(&MqttClient->Network);
   RetCode = NetworkConnect(&MqttClient->Network, (char *)BrokerAddress, BrokerPort);
   if (RetCode == 0) 
   {

      /*
      ** Init MQTT client
      */
      MQTTClientInit(&MqttClient->Client, 
                     &MqttClient->Network, MQTT_CLIENT_TIMEOUT_MS,
                     MqttClient->SendBuf,  MQTT_CLIENT_SEND_BUF_LEN,
                     MqttClient->ReadBuf,  MQTT_CLIENT_READ_BUF_LEN); 

      MqttClient->ConnectData.willFlag = 0;
      MqttClient->ConnectData.MQTTVersion = 3;
      MqttClient->ConnectData.clientID.cstring = (char *)ClientName;
      MqttClient->ConnectData.username.cstring = NULL;
      MqttClient->ConnectData.password.cstring = NULL;

      MqttClient->ConnectData.keepAliveInterval = 10;
      MqttClient->ConnectData.cleansession = 1;

      /*
      ** Connect to MQTT server
      */
      
      RetCode = MQTTConnect(&MqttClient->Client, &MqttClient->ConnectData);

      if (RetCode == SUCCESS)
      {
      
         CFE_EVS_SendEvent(MQTT_CLIENT_CONNECT_EID, CFE_EVS_EventType_INFORMATION, 
                           "Successfully connected to MQTT broker %s:%d as client %s", BrokerAddress, BrokerPort, ClientName);
         MqttClient->Connected = true;
         RetStatus = true;
         
      }
      else
      {
         CFE_EVS_SendEvent(MQTT_CLIENT_CONNECT_ERR_EID, CFE_EVS_EventType_ERROR, 
                           "Error initializing %s:%d MQTT broker client %s. Status=%d",
                           BrokerAddress, BrokerPort, ClientName, RetCode);
      }
      
   } /* End if successful NetworkConnect */
   else
   {
      CFE_EVS_SendEvent(MQTT_CLIENT_CONNECT_ERR_EID, CFE_EVS_EventType_ERROR, 
                        "Error creating MQTT network connection to %s:%d. Status=%d",
                        BrokerAddress, BrokerPort, RetCode);
   }
   
   return RetStatus;

} /* End MQTT_CLIENT_Connect() */


/******************************************************************************
** Function: MQTT_CLIENT_Disconnect
**
** Notes:
**    None
**
*/
void MQTT_CLIENT_Disconnect(void)
{
   
   MQTTDisconnect(&MqttClient->Client);
   NetworkDisconnect(&MqttClient->Network);

} /* End MQTT_CLIENT_Disconnect() */


/******************************************************************************
** Function: MQTT_CLIENT_Publish
**
** Notes:
**    1. QOS needs to be converted to MQTT library constants
*/
bool MQTT_CLIENT_Publish(const char *Topic, const char *Payload)
{
   
   bool RetStatus = false;
   

   MqttClient->PubMsg.payload = (void *)Payload;
   MqttClient->PubMsg.payloadlen = strlen(Payload);
   
   if (MQTTPublish(&MqttClient->Client, Topic, &MqttClient->PubMsg) == SUCCESS)
   {
      RetStatus = true;
      CFE_EVS_SendEvent(MQTT_CLIENT_PUBLISH_EID, CFE_EVS_EventType_INFORMATION, 
                       "Successfully published topic %s with payload %s",
                       Topic, Payload);
   }
   else
   {
      CFE_EVS_SendEvent(MQTT_CLIENT_PUBLISH_ERR_EID, CFE_EVS_EventType_ERROR, 
                       "Error publishing topic %s with payload %s",
                       Topic, Payload);   
   }

   return RetStatus;

} /* End MQTT_CLIENT_Publish() */


/******************************************************************************
** Function: MQTT_CLIENT_Reconnect
**
** Reconnect to an MQTT broker using current parameters
**
*/
bool MQTT_CLIENT_Reconnect(void)
{
   
   return MQTT_CLIENT_Connect(MqttClient->ClientName, MqttClient->BrokerAddress, MqttClient->BrokerPort);
   
} /* MQTT_CLIENT_Reconnect() */


/******************************************************************************
** Function: MQTT_CLIENT_ResetStatus
**
** Reset counters and status flags to a known reset state.
**
*/
void MQTT_CLIENT_ResetStatus(void)
{

   /* Nothing to do */

} /* End MQTT_CLIENT_ResetStatus() */


/******************************************************************************
** Function: MQTT_CLIENT_Subscribe
**
** Notes:
**    1. QOS needs to be converted to MQTT library constants
*/

bool MQTT_CLIENT_Subscribe(const char *Topic, int Qos, 
                           MQTT_CLIENT_MsgCallback_t MsgCallbackFunc)
{
   
   bool RetStatus = false;
   
   if (MqttClient->Connected)
   {
      RetStatus = (MQTTSubscribe(&MqttClient->Client, Topic, Qos, MsgCallbackFunc) == SUCCESS);
   }
   
   return RetStatus;
   
} /* End MQTT_CLIENT_Subscribe() */


/******************************************************************************
** Function: MQTT_CLIENT_Unsubscribe
**
*/
bool MQTT_CLIENT_Unsubscribe(const char *Topic)
{
   
   bool RetStatus = false;
   
   RetStatus = (MQTTUnsubscribe(&MqttClient->Client, Topic) == SUCCESS);

   return RetStatus;
   
} /* End MQTT_CLIENT_Unsubscribe() */


/******************************************************************************
** Function: MQTT_CLIENT_Yield
**
** Notes:
**    1. If yield fails, enforce a timeout to avoid CPU hogging
**
*/
bool MQTT_CLIENT_Yield(uint32 YieldTime)
{
   
   bool RetStatus = false;

   if (MqttClient->Connected)
   {
      
      /* Return code doesn't have additional information, only returns SUCCESS/FAILURE */ 
      if (MQTTYield(&MqttClient->Client, YieldTime) == SUCCESS)
      {
         RetStatus = true;
      }
      else
      {
         
         CFE_EVS_SendEvent(MQTT_CLIENT_YIELD_ERR_EID, CFE_EVS_EventType_ERROR, "MQTT client yield error");
         OS_TaskDelay(YieldTime);

      }
   }
   else
   {
      OS_TaskDelay(YieldTime);
   }
    
   return RetStatus;
    
    
} /* End MQTT_CLIENT_Yield() */


