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

#include "mqtt_topic_discrete.h"

/************************************/
/** Local File Function Prototypes **/
/************************************/

static bool LoadJsonData(const char *JsonMsgPayload, uint16 PayloadLen);


/**********************/
/** Global File Data **/
/**********************/

static MQTT_TOPIC_DISCRETE_Class_t* MqttTopicDiscrete = NULL;

static MQTT_GW_DiscreteTlm_Payload_t  Discrete; /* Working buffer for loads */

static CJSON_Obj_t JsonTblObjs[] = 
{

   /* Table           Table                                      core-json            length of query      */
   /* Data Address,   Data Length,  Updated, Data Type,  Float,  query string,        string(exclude '\0') */
   
   { &Discrete.Item_1,     2,       false,   JSONNumber, false,  { "discrete.item-1", (sizeof("discrete.item-1")-1)} },
   { &Discrete.Item_2,     2,       false,   JSONNumber, false,  { "discrete.item-2", (sizeof("discrete.item-2")-1)} },
   { &Discrete.Item_3,     2,       false,   JSONNumber, false,  { "discrete.item-3", (sizeof("discrete.item-3")-1)} },
   { &Discrete.Item_4,     2,       false,   JSONNumber, false,  { "discrete.item-4", (sizeof("discrete.item-4")-1)} }
   
};

static const char *NullDiscreteMsg = "{\"discrete\":{\"item-1\": 0,\"item-2\": 0,\"item-3\": 0,\"item-4\": 0}}";

/******************************************************************************
** Function: MQTT_TOPIC_DISCRETE_Constructor
**
** Initialize the MQTT discrete topic
**
** Notes:
**   None
**
*/
void MQTT_TOPIC_DISCRETE_Constructor(MQTT_TOPIC_DISCRETE_Class_t *MqttTopicDiscretePtr, 
                                     CFE_SB_MsgId_t TlmMsgMid, const char *Topic)
{

   MqttTopicDiscrete = MqttTopicDiscretePtr;
   memset(MqttTopicDiscrete, 0, sizeof(MQTT_TOPIC_DISCRETE_Class_t));

   strncpy(MqttTopicDiscrete->JsonMsgTopic, Topic, MQTT_TOPIC_TBL_MAX_TOPIC_LEN);
   MqttTopicDiscrete->JsonObjCnt = (sizeof(JsonTblObjs)/sizeof(CJSON_Obj_t));
   
   CFE_MSG_Init(CFE_MSG_PTR(MqttTopicDiscrete->TlmMsg), TlmMsgMid, sizeof(MQTT_GW_DiscreteTlm_t));
      
} /* End MQTT_TOPIC_DISCRETE_Constructor() */


/******************************************************************************
** Function: MQTT_TOPIC_DISCRETE_CfeToJson
**
** Convert a cFE discrete message to a JSON topic message 
**
*/
bool MQTT_TOPIC_DISCRETE_CfeToJson(const char **JsonMsgTopic, const char **JsonMsgPayload,
                                   const CFE_MSG_Message_t *CfeMsg)
{

   bool  RetStatus = false;
   int   PayloadLen; 
   const MQTT_GW_DiscreteTlm_Payload_t *DiscreteMsg = CMDMGR_PAYLOAD_PTR(CfeMsg, MQTT_GW_DiscreteTlm_t);

   *JsonMsgTopic   = MqttTopicDiscrete->JsonMsgTopic;
   *JsonMsgPayload = NullDiscreteMsg;
   
   PayloadLen = sprintf(MqttTopicDiscrete->JsonMsgPayload,
                "{\"discrete\":{\"item-1\": %1d,\"item-2\": %1d,\"item-3\": %1d,\"item-4\": %1d}}",
                DiscreteMsg->Item_1, DiscreteMsg->Item_2, DiscreteMsg->Item_3, DiscreteMsg->Item_4);

   if (PayloadLen > 0)
   {
      *JsonMsgPayload = MqttTopicDiscrete->JsonMsgPayload;
   
      ++MqttTopicDiscrete->CfeToJsonCnt;
      RetStatus = true;
   }
   
   return RetStatus;
   
} /* End MQTT_TOPIC_DISCRETE_CfeToJson() */


/******************************************************************************
** Function: MQTT_TOPIC_DISCRETE_JsonToCfe
**
** Convert a JSON discrete topic message to a cFE discrete message 
**
*/
bool MQTT_TOPIC_DISCRETE_JsonToCfe(CFE_MSG_Message_t **CfeMsg, 
                               const char *JsonMsgPayload, uint16 PayloadLen)
{
   
   bool RetStatus = false;
   
   *CfeMsg = NULL;
   
   if (LoadJsonData(JsonMsgPayload, PayloadLen))
   {
      *CfeMsg = (CFE_MSG_Message_t *)&MqttTopicDiscrete->TlmMsg;

      ++MqttTopicDiscrete->JsonToCfeCnt;
      RetStatus = true;
   }

   return RetStatus;
   
} /* End MQTT_TOPIC_DISCRETE_JsonToCfe() */


/******************************************************************************
** Function: MQTT_TOPIC_DISCRETE_SbMsgTest
**
** Convert a JSON discrete topic message to a cFE discrete message
**
** Notes:
**   1. Param is not used
**
*/
void MQTT_TOPIC_DISCRETE_SbMsgTest(bool Init, int16 Param)
{

   if (Init)
   {
      MqttTopicDiscrete->TestData = 0;  
      CFE_EVS_SendEvent(MQTT_TOPIC_DISCRETE_INIT_SB_MSG_TEST_EID, CFE_EVS_EventType_INFORMATION,
                        "Discrete topic test started");

   }
   else
   {
   
      CFE_EVS_SendEvent(MQTT_TOPIC_DISCRETE_SB_MSG_TEST_EID, CFE_EVS_EventType_DEBUG,
                        "Discrete topic test sending 0x%04X", MqttTopicDiscrete->TestData);
                        
      MqttTopicDiscrete->TlmMsg.Payload.Item_1 =  (MqttTopicDiscrete->TestData & 0x01);
      MqttTopicDiscrete->TlmMsg.Payload.Item_2 = ((MqttTopicDiscrete->TestData & 0x02) >> 1);
      MqttTopicDiscrete->TlmMsg.Payload.Item_3 = ((MqttTopicDiscrete->TestData & 0x04) >> 2);
      MqttTopicDiscrete->TlmMsg.Payload.Item_4 = ((MqttTopicDiscrete->TestData & 0x08) >> 3);
      
      if (++MqttTopicDiscrete->TestData > 15)
      {
         MqttTopicDiscrete->TestData = 0;
      }
      
   }
   
   CFE_SB_TimeStampMsg(CFE_MSG_PTR(MqttTopicDiscrete->TlmMsg.TelemetryHeader));
   CFE_SB_TransmitMsg(CFE_MSG_PTR(MqttTopicDiscrete->TlmMsg.TelemetryHeader), true);
   
} /* End MQTT_TOPIC_DISCRETE_SbMsgTest() */


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

   memset(&MqttTopicDiscrete->TlmMsg.Payload, 0, sizeof(MQTT_GW_DiscreteTlm_Payload_t));
   ObjLoadCnt = CJSON_LoadObjArray(JsonTblObjs, MqttTopicDiscrete->JsonObjCnt, 
                                   JsonMsgPayload, PayloadLen);
   CFE_EVS_SendEvent(MQTT_TOPIC_DISCRETE_LOAD_JSON_DATA_EID, CFE_EVS_EventType_DEBUG,
                     "Discrete LoadJsonData() process %d JSON objects", (uint16)ObjLoadCnt);

   if (ObjLoadCnt == MqttTopicDiscrete->JsonObjCnt)
   {
      memcpy(&MqttTopicDiscrete->TlmMsg.Payload, &Discrete, sizeof(MQTT_GW_DiscreteTlm_Payload_t));      
      RetStatus = true;
   }
   else
   {
      CFE_EVS_SendEvent(MQTT_TOPIC_DISCRETE_JSON_TO_CCSDS_ERR_EID, CFE_EVS_EventType_ERROR, 
                        "Error processing discrete topic, payload contained %d of %d data objects",
                        (unsigned int)ObjLoadCnt, (unsigned int)MqttTopicDiscrete->JsonObjCnt);
   }
   
   return RetStatus;
   
} /* End LoadJsonData() */

