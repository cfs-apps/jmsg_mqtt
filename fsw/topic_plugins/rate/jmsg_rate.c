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
**   Manage MQTT rate message
**
** Notes:
**   None
**
*/

/*
** Includes
*/

#include "mqtt_gw_topic_rate.h"

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

static MQTT_GW_TOPIC_RATE_Class_t* MqttGwTopicRate = NULL;

static MQTT_GW_RatePluginTlm_Payload_t  RateData; /* Working buffer for loads */

static CJSON_Obj_t JsonTblObjs[] = 
{

   /* Table           Table                                       core-json       length of query       */
   /* Data Address,   Data Length,  Updated,  Data Type,  Float,  query string,   string(exclude '\0')  */
   
   { &RateData.X,     4,            false,   JSONNumber, true,   { "rate.x",    (sizeof("rate.x")-1)} },
   { &RateData.Y,     4,            false,   JSONNumber, true,   { "rate.y",    (sizeof("rate.y")-1)} },
   { &RateData.Z,     4,            false,   JSONNumber, true,   { "rate.z",    (sizeof("rate.z")-1)} }
   
};

static const char *NullRateMsg = "{\"rate\":{\"x\": 0.0,\"y\": 0.0,\"z\": 0.0}}";


/******************************************************************************
** Function: MQTT_GW_TOPIC_RATE_Constructor
**
** Initialize the MQTT rate topic
**
** Notes:
**   None
**
*/
#define DEG_PER_SEC_IN_RADIANS 0.0174533
void MQTT_GW_TOPIC_RATE_Constructor(MQTT_GW_TOPIC_RATE_Class_t *MqttGwTopicRatePtr, 
                                     MQTT_TOPIC_TBL_PluginFuncTbl_t *PluginFuncTbl,
                                     CFE_SB_MsgId_t TlmMsgMid)
{

   MqttGwTopicRate = MqttGwTopicRatePtr;
   memset(MqttGwTopicRate, 0, sizeof(MQTT_GW_TOPIC_RATE_Class_t));

   PluginFuncTbl->CfeToJson = CfeToJson;
   PluginFuncTbl->JsonToCfe = JsonToCfe;  
   PluginFuncTbl->SbMsgTest = SbMsgTest;
   
   MqttGwTopicRate->JsonObjCnt = (sizeof(JsonTblObjs)/sizeof(CJSON_Obj_t));
   
   CFE_MSG_Init(CFE_MSG_PTR(MqttGwTopicRate->TlmMsg), TlmMsgMid, sizeof(MQTT_GW_RatePluginTlm_t));
   
   /*
   ** Cycle counts are used by sim to switch axis
   ** Need to coordinate with topic message SB pend time
   */
   MqttGwTopicRate->TestAxis         = MQTT_GW_TOPIC_RATE_TEST_AXIS_X;
   MqttGwTopicRate->TestAxisCycleCnt = 0;
   MqttGwTopicRate->TestAxisDefRate  = 0.0087265;  /* 0.5 deg/sec in radians with 250ms pend */
   MqttGwTopicRate->TestAxisDefRate  = 0.0174533;  /* 1.0 deg/sec in radians with 250ms pend */
   MqttGwTopicRate->TestAxisDefRate  *= 7.5;       /* 7.5 deg/s so 24 cycles for 90 deg */
   MqttGwTopicRate->TestAxisRate     = MqttGwTopicRate->TestAxisDefRate;
   MqttGwTopicRate->TestAxisCycleLim = 12*4;       /* Rotate 90 deg on each axis */
   
} /* End MQTT_GW_TOPIC_RATE_Constructor() */


/******************************************************************************
** Function: CfeToJson
**
** Convert a cFE rate message to a JSON topic message 
**
** Notes:
**   1.  Signature must match MQTT_TOPIC_TBL_CfeToJson_t
*/
static bool CfeToJson(const char **JsonMsgPayload, const CFE_MSG_Message_t *CfeMsg)
{

   bool  RetStatus = false;
   int   PayloadLen; 
   const MQTT_GW_RatePluginTlm_Payload_t *RateMsg = CMDMGR_PAYLOAD_PTR(CfeMsg, MQTT_GW_RatePluginTlm_t);

   *JsonMsgPayload = NullRateMsg;
   
   PayloadLen = sprintf(MqttGwTopicRate->JsonMsgPayload,
                "{\"rate\":{\"x\": %0.6f,\"y\": %0.6f,\"z\": %0.6f}}",
                RateMsg->X, RateMsg->Y, RateMsg->Z);

   if (PayloadLen > 0)
   {
      *JsonMsgPayload = MqttGwTopicRate->JsonMsgPayload;
      MqttGwTopicRate->CfeToJsonCnt++;
      RetStatus = true;
   }
   
   return RetStatus;
   
} /* End CfeToJson() */


/******************************************************************************
** Function: JsonToCfe
**
** Convert a JSON rate topic message to a cFE rate message 
**
** Notes:
**   1. Signature must match MQTT_TOPIC_TBL_JsonToCfe_t
**   2. Messages that can be pasted into MQTT broker for testing
**      {"rate":{"x": 1.1, "y": 2.2, "z": 3.3}}
*/
static bool JsonToCfe(CFE_MSG_Message_t **CfeMsg, const char *JsonMsgPayload, uint16 PayloadLen)
{
   
   bool RetStatus = false;
   
   *CfeMsg = NULL;
   
   if (LoadJsonData(JsonMsgPayload, PayloadLen))
   {
      *CfeMsg = (CFE_MSG_Message_t *)&MqttGwTopicRate->TlmMsg;

      ++MqttGwTopicRate->JsonToCfeCnt;
      RetStatus = true;
   }

   return RetStatus;
   
} /* End JsonToCfe() */


/******************************************************************************
** Function: SbMsgTest
**
** Generate and send SB rate topic messages on SB that are read back by MQTT_GW
** and cause MQTT messages to be generated from the SB messages. 
**
** Notes:
**   1. Param is used to scale the default test rate and change the sign
**      Increase rate:   2 <= Param <= 10
**      Deccrease rate: 12 <= Param <= 20
**
*/
static void SbMsgTest(bool Init, int16 Param)
{

   if (Init)
   {

      MqttGwTopicRate->TestAxisRate = MqttGwTopicRate->TestAxisDefRate;
      
      if (Param >= 2 && Param <= 10)
      {
         MqttGwTopicRate->TestAxisRate *= (float)Param;
      }
      else if (Param >= 12 && Param <= 20)
      {         
         MqttGwTopicRate->TestAxisRate /= ((float)Param - 10.0);
      }
      MqttGwTopicRate->TlmMsg.Payload.X = MqttGwTopicRate->TestAxisRate;
      MqttGwTopicRate->TlmMsg.Payload.Y = 0.0;
      MqttGwTopicRate->TlmMsg.Payload.Z = 0.0;
      MqttGwTopicRate->TestAxis         = MQTT_GW_TOPIC_RATE_TEST_AXIS_X;
      MqttGwTopicRate->TestAxisCycleCnt = 0;

      CFE_EVS_SendEvent(MQTT_GW_TOPIC_RATE_INIT_SB_MSG_TEST_EID, CFE_EVS_EventType_INFORMATION,
                        "Rate topic test started with axis rate %6.2f", MqttGwTopicRate->TestAxisRate);

   }
   else
   {
      switch (MqttGwTopicRate->TestAxis)
      {
         case MQTT_GW_TOPIC_RATE_TEST_AXIS_X:
            if (++MqttGwTopicRate->TestAxisCycleCnt > MqttGwTopicRate->TestAxisCycleLim)
            {
               MqttGwTopicRate->TestAxisCycleCnt = 0;
               MqttGwTopicRate->TlmMsg.Payload.X = 0.0;
               MqttGwTopicRate->TlmMsg.Payload.Y = MqttGwTopicRate->TestAxisRate;
               MqttGwTopicRate->TestAxis         = MQTT_GW_TOPIC_RATE_TEST_AXIS_Y;
            }
            break;
         case MQTT_GW_TOPIC_RATE_TEST_AXIS_Y:
            if (++MqttGwTopicRate->TestAxisCycleCnt > MqttGwTopicRate->TestAxisCycleLim)
            {
               MqttGwTopicRate->TestAxisCycleCnt = 0;
               MqttGwTopicRate->TlmMsg.Payload.Y = 0.0;
               MqttGwTopicRate->TlmMsg.Payload.Z = MqttGwTopicRate->TestAxisRate;
               MqttGwTopicRate->TestAxis         = MQTT_GW_TOPIC_RATE_TEST_AXIS_Z;
            }
            break;
         case MQTT_GW_TOPIC_RATE_TEST_AXIS_Z:
            if (++MqttGwTopicRate->TestAxisCycleCnt > MqttGwTopicRate->TestAxisCycleLim)
            {
               MqttGwTopicRate->TestAxisCycleCnt = 0;
               MqttGwTopicRate->TlmMsg.Payload.Z = 0.0;
               MqttGwTopicRate->TlmMsg.Payload.X = MqttGwTopicRate->TestAxisRate;
               MqttGwTopicRate->TestAxis         = MQTT_GW_TOPIC_RATE_TEST_AXIS_X;
            }
            break;
         default:
            MqttGwTopicRate->TestAxisCycleCnt = 0;
            MqttGwTopicRate->TlmMsg.Payload.X = MqttGwTopicRate->TestAxisRate;
            MqttGwTopicRate->TlmMsg.Payload.Y = 0.0;
            MqttGwTopicRate->TlmMsg.Payload.Z = 0.0;
            MqttGwTopicRate->TestAxis         = MQTT_GW_TOPIC_RATE_TEST_AXIS_X;
            break;
         
      } /* End axis switch */
   }
   
   CFE_SB_TimeStampMsg(CFE_MSG_PTR(MqttGwTopicRate->TlmMsg.TelemetryHeader));
   CFE_SB_TransmitMsg(CFE_MSG_PTR(MqttGwTopicRate->TlmMsg.TelemetryHeader), true);
   
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

   memset(&MqttGwTopicRate->TlmMsg.Payload, 0, sizeof(MQTT_GW_RatePluginTlm_Payload_t));
   ObjLoadCnt = CJSON_LoadObjArray(JsonTblObjs, MqttGwTopicRate->JsonObjCnt, 
                                  JsonMsgPayload, PayloadLen);
   CFE_EVS_SendEvent(MQTT_GW_TOPIC_RATE_LOAD_JSON_DATA_EID, CFE_EVS_EventType_DEBUG,
                     "Rate LoadJsonData() process %d JSON objects", (uint16)ObjLoadCnt);

   if (ObjLoadCnt == MqttGwTopicRate->JsonObjCnt)
   {
      memcpy(&MqttGwTopicRate->TlmMsg.Payload, &RateData, sizeof(MQTT_GW_RatePluginTlm_Payload_t));      
      RetStatus = true;
   }
   else
   {
      CFE_EVS_SendEvent(MQTT_GW_TOPIC_RATE_JSON_TO_CCSDS_ERR_EID, CFE_EVS_EventType_ERROR, 
                        "Error processing rate message, payload contained %d of %d data objects",
                        (unsigned int)ObjLoadCnt, (unsigned int)MqttGwTopicRate->JsonObjCnt);
   }
   
   return RetStatus;
   
} /* End LoadJsonData() */

