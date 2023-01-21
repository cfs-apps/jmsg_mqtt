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
**   Manage MQTT discreete topic
**
** Notes:
**   None
**
** References:
**   1. OpenSatKit Object-based Application Developer's Guide
**   2. cFS Application Developer's Guide
**
*/

/*
** Includes
*/

#include "mqtt_topic_integer.h"

/************************************/
/** Local File Function Prototypes **/
/************************************/

static bool LoadJsonData(const char *JsonMsgPayload, uint16 PayloadLen);


/**********************/
/** Global File Data **/
/**********************/

static MQTT_TOPIC_INTEGER_Class_t* MqttTopicInteger = NULL;

static MQTT_GW_IntegerTlm_Payload_t  Integer; /* Working buffer for loads */

static CJSON_Obj_t JsonTblObjs[] = 
{

   /* Table           Table                                      core-json            length of query      */
   /* Data Address,   Data Length,  Updated, Data Type,  Float,  query string,        string(exclude '\0') */
   
   { &Integer.Item_1,     2,       false,   JSONNumber, false,  { "integer.item-1", (sizeof("integer.item-1")-1)} },
   { &Integer.Item_2,     2,       false,   JSONNumber, false,  { "integer.item-2", (sizeof("integer.item-2")-1)} },
   { &Integer.Item_3,     2,       false,   JSONNumber, false,  { "integer.item-3", (sizeof("integer.item-3")-1)} },
   { &Integer.Item_4,     2,       false,   JSONNumber, false,  { "integer.item-4", (sizeof("integer.item-4")-1)} }
   
};

static const char *NullIntegerMsg = "{\"integer\":{\"item-1\": 0,\"item-2\": 0,\"item-3\": 0,\"item-4\": 0}}";

/******************************************************************************
** Function: MQTT_TOPIC_INTEGER_Constructor
**
** Initialize the MQTT integer topic
**
** Notes:
**   None
**
*/
void MQTT_TOPIC_INTEGER_Constructor(MQTT_TOPIC_INTEGER_Class_t *MqttTopicIntegerPtr, 
                                     CFE_SB_MsgId_t TlmMsgMid)
{

   MqttTopicInteger = MqttTopicIntegerPtr;
   memset(MqttTopicInteger, 0, sizeof(MQTT_TOPIC_INTEGER_Class_t));

   MqttTopicInteger->JsonObjCnt = (sizeof(JsonTblObjs)/sizeof(CJSON_Obj_t));
   
   CFE_MSG_Init(CFE_MSG_PTR(MqttTopicInteger->TlmMsg), TlmMsgMid, sizeof(MQTT_GW_IntegerTlm_t));
      
} /* End MQTT_TOPIC_INTEGER_Constructor() */


/******************************************************************************
** Function: MQTT_TOPIC_INTEGER_CfeToJson
**
** Convert a cFE integer message to a JSON topic message 
**
*/
bool MQTT_TOPIC_INTEGER_CfeToJson(const char **JsonMsgPayload,
                                  const CFE_MSG_Message_t *CfeMsg)
{

   bool  RetStatus = false;
   int   PayloadLen; 
   const MQTT_GW_IntegerTlm_Payload_t *IntegerMsg = CMDMGR_PAYLOAD_PTR(CfeMsg, MQTT_GW_IntegerTlm_t);

   *JsonMsgPayload = NullIntegerMsg;
   
   PayloadLen = sprintf(MqttTopicInteger->JsonMsgPayload,
                "{\"integer\":{\"item-1\": %1d,\"item-2\": %1d,\"item-3\": %1d,\"item-4\": %1d}}",
                IntegerMsg->Item_1, IntegerMsg->Item_2, IntegerMsg->Item_3, IntegerMsg->Item_4);

   if (PayloadLen > 0)
   {
      *JsonMsgPayload = MqttTopicInteger->JsonMsgPayload;
   
      ++MqttTopicInteger->CfeToJsonCnt;
      RetStatus = true;
   }
   
   return RetStatus;
   
} /* End MQTT_TOPIC_INTEGER_CfeToJson() */


/******************************************************************************
** Function: MQTT_TOPIC_INTEGER_JsonToCfe
**
** Convert a JSON integer topic message to a cFE integer message 
**
*/
bool MQTT_TOPIC_INTEGER_JsonToCfe(CFE_MSG_Message_t **CfeMsg, 
                               const char *JsonMsgPayload, uint16 PayloadLen)
{
   
   bool RetStatus = false;
   
   *CfeMsg = NULL;
   
   if (LoadJsonData(JsonMsgPayload, PayloadLen))
   {
      *CfeMsg = (CFE_MSG_Message_t *)&MqttTopicInteger->TlmMsg;

      ++MqttTopicInteger->JsonToCfeCnt;
      RetStatus = true;
   }

   return RetStatus;
   
} /* End MQTT_TOPIC_INTEGER_JsonToCfe() */


/******************************************************************************
** Function: MQTT_TOPIC_INTEGER_SbMsgTest
**
** Convert a JSON integer topic message to a cFE integer message
**
** Notes:
**   1. Param is not used
**
*/
void MQTT_TOPIC_INTEGER_SbMsgTest(bool Init, int16 Param)
{

   if (Init)
   {
      MqttTopicInteger->TestData = 0;  
      CFE_EVS_SendEvent(MQTT_TOPIC_INTEGER_INIT_SB_MSG_TEST_EID, CFE_EVS_EventType_INFORMATION,
                        "Integer topic test started");

   }
   else
   {
   
      CFE_EVS_SendEvent(MQTT_TOPIC_INTEGER_SB_MSG_TEST_EID, CFE_EVS_EventType_DEBUG,
                        "Integer topic test sending 0x%04X", MqttTopicInteger->TestData);
                        
      MqttTopicInteger->TlmMsg.Payload.Item_1 =  (MqttTopicInteger->TestData & 0x01);
      MqttTopicInteger->TlmMsg.Payload.Item_2 = ((MqttTopicInteger->TestData & 0x02) >> 1);
      MqttTopicInteger->TlmMsg.Payload.Item_3 = ((MqttTopicInteger->TestData & 0x04) >> 2);
      MqttTopicInteger->TlmMsg.Payload.Item_4 = ((MqttTopicInteger->TestData & 0x08) >> 3);
      
      if (++MqttTopicInteger->TestData > 15)
      {
         MqttTopicInteger->TestData = 0;
      }
      
   }
   
   CFE_SB_TimeStampMsg(CFE_MSG_PTR(MqttTopicInteger->TlmMsg.TelemetryHeader));
   CFE_SB_TransmitMsg(CFE_MSG_PTR(MqttTopicInteger->TlmMsg.TelemetryHeader), true);
   
} /* End MQTT_TOPIC_INTEGER_SbMsgTest() */


/******************************************************************************
** Function: LoadJsonData
**
** Notes:
**  1. See file prologue for full/partial table load scenarios
*/
static bool LoadJsonData(const char *JsonMsgPayload, uint16 PayloadLen)
{

   bool      RetStatus = false;
   size_t    ObjLoadCnt;

   memset(&MqttTopicInteger->TlmMsg.Payload, 0, sizeof(MQTT_GW_IntegerTlm_Payload_t));
   ObjLoadCnt = CJSON_LoadObjArray(JsonTblObjs, MqttTopicInteger->JsonObjCnt, 
                                   JsonMsgPayload, PayloadLen);
   CFE_EVS_SendEvent(MQTT_TOPIC_INTEGER_LOAD_JSON_DATA_EID, CFE_EVS_EventType_DEBUG,
                     "Integer LoadJsonData() process %d JSON objects", (uint16)ObjLoadCnt);

   if (ObjLoadCnt == MqttTopicInteger->JsonObjCnt)
   {
      memcpy(&MqttTopicInteger->TlmMsg.Payload, &Integer, sizeof(MQTT_GW_IntegerTlm_Payload_t));      
      RetStatus = true;
   }
   else
   {
      CFE_EVS_SendEvent(MQTT_TOPIC_INTEGER_JSON_TO_CCSDS_ERR_EID, CFE_EVS_EventType_ERROR, 
                        "Error processing integer topic, payload contained %d of %d data objects",
                        (unsigned int)ObjLoadCnt, (unsigned int)MqttTopicInteger->JsonObjCnt);
   }
   
   return RetStatus;
   
} /* End LoadJsonData() */

