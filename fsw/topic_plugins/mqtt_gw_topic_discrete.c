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
**   Manage MQTT discrete topic
**
** Notes:
**   None
**
*/

/*
** Includes
*/

#include "mqtt_gw_topic_discrete.h"

/************************************/
/** Local File Function Prototypes **/
/************************************/

static bool CfeToJson(const char **JsonMsgPayload, const CFE_MSG_Message_t *CfeMsg);
static bool JsonToCfe(CFE_MSG_Message_t **CfeMsg, const char *JsonMsgPayload, uint16 PayloadLen);
static bool LoadJsonData(const char *JsonMsgPayload, uint16 PayloadLen);
static void SbMsgTest(bool Init, int16 Param);


/**********************/
/** Global File Data **/
/**********************/

static MQTT_GW_TOPIC_DISCRETE_Class_t* MqttTopicDiscrete = NULL;

static MQTT_GW_DiscretePluginTlm_Payload_t  Discrete; /* Working buffer for loads */

static CJSON_Obj_t JsonTblObjs[] = 
{

   /* Table           Table                                      core-json            length of query      */
   /* Data Address,   Data Length,  Updated, Data Type,  Float,  query string,        string(exclude '\0') */
   
   { &Discrete.Item_1,     2,       false,   JSONNumber, false,  { "integer.item-1", (sizeof("integer.item-1")-1)} },
   { &Discrete.Item_2,     2,       false,   JSONNumber, false,  { "integer.item-2", (sizeof("integer.item-2")-1)} },
   { &Discrete.Item_3,     2,       false,   JSONNumber, false,  { "integer.item-3", (sizeof("integer.item-3")-1)} },
   { &Discrete.Item_4,     2,       false,   JSONNumber, false,  { "integer.item-4", (sizeof("integer.item-4")-1)} }
   
};

static const char *NullDiscreteMsg = "{\"integer\":{\"item-1\": 0,\"item-2\": 0,\"item-3\": 0,\"item-4\": 0}}";


/******************************************************************************
** Function: MQTT_GW_TOPIC_DISCRETE_Constructor
**
** Initialize the MQTT discrete topic
**
** Notes:
**   None
**
*/
void MQTT_GW_TOPIC_DISCRETE_Constructor(MQTT_GW_TOPIC_DISCRETE_Class_t *MqttTopicDiscretePtr, 
                                       MQTT_TOPIC_TBL_PluginFuncTbl_t *PluginFuncTbl,
                                       CFE_SB_MsgId_t TlmMsgMid)
{

   MqttTopicDiscrete = MqttTopicDiscretePtr;
   memset(MqttTopicDiscrete, 0, sizeof(MQTT_GW_TOPIC_DISCRETE_Class_t));

   PluginFuncTbl->CfeToJson = CfeToJson;
   PluginFuncTbl->JsonToCfe = JsonToCfe;  
   PluginFuncTbl->SbMsgTest = SbMsgTest;
   
   MqttTopicDiscrete->JsonObjCnt = (sizeof(JsonTblObjs)/sizeof(CJSON_Obj_t));
   
   CFE_MSG_Init(CFE_MSG_PTR(MqttTopicDiscrete->TlmMsg), TlmMsgMid, sizeof(MQTT_GW_DiscretePluginTlm_t));
      
} /* End MQTT_GW_TOPIC_DISCRETE_Constructor() */


/******************************************************************************
** Function: CfeToJson
**
** Convert a cFE discrete message to a JSON topic message 
**
** Notes:
**   1. Signature must match MQTT_TOPIC_TBL_CfeToJson_t
**   2. Messages that can be pasted into MQTT broker for testing
**      {"integer":{"item-1": 1,"item-2": 2,"item-3": 3,"item-4": 4}}
**
*/
static bool CfeToJson(const char **JsonMsgPayload, const CFE_MSG_Message_t *CfeMsg)
{

   bool  RetStatus = false;
   int   PayloadLen; 
   const MQTT_GW_DiscretePluginTlm_Payload_t *DiscreteMsg = CMDMGR_PAYLOAD_PTR(CfeMsg, MQTT_GW_DiscretePluginTlm_t);

   *JsonMsgPayload = NullDiscreteMsg;
   
   PayloadLen = sprintf(MqttTopicDiscrete->JsonMsgPayload,
                "{\"integer\":{\"item-1\": %1d,\"item-2\": %1d,\"item-3\": %1d,\"item-4\": %1d}}",
                DiscreteMsg->Item_1, DiscreteMsg->Item_2, DiscreteMsg->Item_3, DiscreteMsg->Item_4);

   if (PayloadLen > 0)
   {
      *JsonMsgPayload = MqttTopicDiscrete->JsonMsgPayload;
   
      ++MqttTopicDiscrete->CfeToJsonCnt;
      RetStatus = true;
   }
   
   return RetStatus;
   
} /* End CfeToJson() */


/******************************************************************************
** Function: JsonToCfe
**
** Convert a JSON discrete topic message to a cFE discrete message 
**
** Notes:
**   1.  Signature must match MQTT_TOPIC_TBL_JsonToCfe_t
**
*/
static bool JsonToCfe(CFE_MSG_Message_t **CfeMsg, const char *JsonMsgPayload, uint16 PayloadLen)
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
   
} /* End JsonToCfe() */


/******************************************************************************
** Function: SbMsgTest
**
** Generate and send SB discrete topic messages on SB that are read back by MQTT_GW
** and cause MQTT messages to be generated from the SB messages.  
**
** Notes:
**   1. Param is not used
**
*/
static void SbMsgTest(bool Init, int16 Param)
{

   if (Init)
   {
      MqttTopicDiscrete->TestData = 0;  
      CFE_EVS_SendEvent(MQTT_GW_TOPIC_DISCRETE_INIT_SB_MSG_TEST_EID, CFE_EVS_EventType_INFORMATION,
                        "Discrete topic test started");

   }
   else
   {
   
      CFE_EVS_SendEvent(MQTT_GW_TOPIC_DISCRETE_SB_MSG_TEST_EID, CFE_EVS_EventType_DEBUG,
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
   
} /* End SbMsgTest() */


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

   memset(&MqttTopicDiscrete->TlmMsg.Payload, 0, sizeof(MQTT_GW_DiscretePluginTlm_Payload_t));
   ObjLoadCnt = CJSON_LoadObjArray(JsonTblObjs, MqttTopicDiscrete->JsonObjCnt, 
                                   JsonMsgPayload, PayloadLen);
   CFE_EVS_SendEvent(MQTT_GW_TOPIC_DISCRETE_LOAD_JSON_DATA_EID, CFE_EVS_EventType_DEBUG,
                     "Discrete LoadJsonData() process %d JSON objects", (uint16)ObjLoadCnt);

   if (ObjLoadCnt == MqttTopicDiscrete->JsonObjCnt)
   {
      memcpy(&MqttTopicDiscrete->TlmMsg.Payload, &Discrete, sizeof(MQTT_GW_DiscretePluginTlm_Payload_t));      
      RetStatus = true;
   }
   else
   {
      CFE_EVS_SendEvent(MQTT_GW_TOPIC_DISCRETE_JSON_TO_CCSDS_ERR_EID, CFE_EVS_EventType_ERROR, 
                        "Error processing discrete topic, payload contained %d of %d data objects",
                        (unsigned int)ObjLoadCnt, (unsigned int)MqttTopicDiscrete->JsonObjCnt);
   }
   
   return RetStatus;
   
} /* End LoadJsonData() */

