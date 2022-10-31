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
**   Manage MQTT rate topic
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

#include "mqtt_topic_rate.h"

/************************************/
/** Local File Function Prototypes **/
/************************************/

static bool LoadJsonData(const char *JsonMsgPayload, uint16 PayloadLen);


/**********************/
/** Global File Data **/
/**********************/

static MQTT_TOPIC_RATE_Class_t* MqttTopicRate = NULL;

static MQTT_GW_RateTlm_Payload_t  RateData; /* Working buffer for loads */

static CJSON_Obj_t JsonTblObjs[] = 
{

   /* Table           Table                                      core-json      length of query      */
   /* Data Address,   Data Length,  Updated, Data Type,  Float,  query string,  string(exclude '\0') */
   
   { &RateData.X,     4,            false,   JSONNumber, true,   { "rate.x",    (sizeof("rate.x")-1)} },
   { &RateData.Y,     4,            false,   JSONNumber, true,   { "rate.y",    (sizeof("rate.y")-1)} },
   { &RateData.Z,     4,            false,   JSONNumber, true,   { "rate.z",    (sizeof("rate.z")-1)} }
   
};

static const char *NullRateMsg = "{\"rate\":{\"x\": 0.0,\"y\": 0.0,\"z\": 0.0}}";

/******************************************************************************
** Function: MQTT_TOPIC_RATE_Constructor
**
** Initialize the MQTT rate topic
**
** Notes:
**   None
**
*/
#define DEG_PER_SEC_IN_RADIANS 0.0174533
void MQTT_TOPIC_RATE_Constructor(MQTT_TOPIC_RATE_Class_t *MqttTopicRatePtr, 
                                 CFE_SB_MsgId_t TlmMsgMid, const char *Topic)
{

   MqttTopicRate = MqttTopicRatePtr;
   memset(MqttTopicRate, 0, sizeof(MQTT_TOPIC_RATE_Class_t));

   strncpy(MqttTopicRate->JsonMsgTopic, Topic, MQTT_TOPIC_TBL_MAX_TOPIC_LEN);
   MqttTopicRate->JsonObjCnt = (sizeof(JsonTblObjs)/sizeof(CJSON_Obj_t));
   
   CFE_MSG_Init(CFE_MSG_PTR(MqttTopicRate->TlmMsg), TlmMsgMid, sizeof(MQTT_GW_RateTlm_t));
   
   /*
   ** Cycle counts are used by sim to switch axis
   ** Need to coordinate with topic message SB pend time
   */
   MqttTopicRate->TestAxis         = MQTT_TOPIC_RATE_TEST_AXIS_X;
   MqttTopicRate->TestAxisCycleCnt = 0;
   MqttTopicRate->TestAxisDefRate  = 0.0087265;  /* 0.5 deg/sec in radians with 250ms pend */
   MqttTopicRate->TestAxisDefRate  = 0.0174533;  /* 1.0 deg/sec in radians with 250ms pend */
   MqttTopicRate->TestAxisDefRate  *= 7.5;        /* 7.5 deg/s so 24 cycles for 90 deg */
   MqttTopicRate->TestAxisRate     = MqttTopicRate->TestAxisDefRate;
   MqttTopicRate->TestAxisCycleLim = 12*4;       /* Rotate 90 deg on each axis */
   
} /* End MQTT_TOPIC_RATE_Constructor() */


/******************************************************************************
** Function: MQTT_TOPIC_RATE_CfeToJson
**
** Convert a cFE rate message to a JSON topic message 
**
*/
bool MQTT_TOPIC_RATE_CfeToJson(const char **JsonMsgTopic, const char **JsonMsgPayload,
                               const CFE_MSG_Message_t *CfeMsg)
{

   bool  RetStatus = false;
   int   PayloadLen; 
   const MQTT_GW_RateTlm_Payload_t *RateMsg = CMDMGR_PAYLOAD_PTR(CfeMsg, MQTT_GW_RateTlm_t);

   *JsonMsgTopic   = MqttTopicRate->JsonMsgTopic;
   *JsonMsgPayload = NullRateMsg;
   
   PayloadLen = sprintf(MqttTopicRate->JsonMsgPayload,
                "{\"rate\":{\"x\": %0.6f,\"y\": %0.6f,\"z\": %0.6f}}",
                RateMsg->X, RateMsg->Y, RateMsg->Z);

   if (PayloadLen > 0)
   {
      *JsonMsgPayload = MqttTopicRate->JsonMsgPayload;
   
      ++MqttTopicRate->CfeToJsonCnt;
      RetStatus = true;
   }
   
   return RetStatus;
   
} /* End MQTT_TOPIC_RATE_CfeToJson() */


/******************************************************************************
** Function: MQTT_TOPIC_RATE_JsonToCfe
**
** Send a cFE rate message on the SB causing MQTT_TOPIC_RATE_CfeToJson()
** to be called which the converts the SB message to a MQTT JSON rate topic. 
** The topic can be observed
**
*/
bool MQTT_TOPIC_RATE_JsonToCfe(CFE_MSG_Message_t **CfeMsg, 
                               const char *JsonMsgPayload, uint16 PayloadLen)
{
   
   bool RetStatus = false;
   
   *CfeMsg = NULL;
   
   if (LoadJsonData(JsonMsgPayload, PayloadLen))
   {
      *CfeMsg = (CFE_MSG_Message_t *)&MqttTopicRate->TlmMsg;

      ++MqttTopicRate->JsonToCfeCnt;
      RetStatus = true;
   }

   return RetStatus;
   
} /* End MQTT_TOPIC_RATE_JsonToCfe() */


/******************************************************************************
** Function: MQTT_TOPIC_RATE_SbMsgTest
**
** Convert a JSON rate topic message to a cFE rate message
**
** Notes:
**   1. Param is used to scale the default test rate and change the sign
**      Increase rate:   2 <= Param <= 10
**      Deccrease rate: 12 <= Param <= 20
**
*/
void MQTT_TOPIC_RATE_SbMsgTest(bool Init, int16 Param)
{

   if (Init)
   {

      MqttTopicRate->TestAxisRate = MqttTopicRate->TestAxisDefRate;
      
      if (Param >= 2 && Param <= 10)
      {
         MqttTopicRate->TestAxisRate *= (float)Param;
      }
      else if (Param >= 12 && Param <= 20)
      {         
         MqttTopicRate->TestAxisRate /= ((float)Param - 10.0);
      }
      MqttTopicRate->TlmMsg.Payload.X = MqttTopicRate->TestAxisRate;
      MqttTopicRate->TlmMsg.Payload.Y = 0.0;
      MqttTopicRate->TlmMsg.Payload.Z = 0.0;
      MqttTopicRate->TestAxis         = MQTT_TOPIC_RATE_TEST_AXIS_X;
      MqttTopicRate->TestAxisCycleCnt = 0;

      CFE_EVS_SendEvent(MQTT_TOPIC_RATE_INIT_SB_MSG_TEST_EID, CFE_EVS_EventType_INFORMATION,
                        "Rate topic test started with axis rate %6.2f", MqttTopicRate->TestAxisRate);

   }
   else
   {
      switch (MqttTopicRate->TestAxis)
      {
         case MQTT_TOPIC_RATE_TEST_AXIS_X:
            if (++MqttTopicRate->TestAxisCycleCnt > MqttTopicRate->TestAxisCycleLim)
            {
               MqttTopicRate->TestAxisCycleCnt = 0;
               MqttTopicRate->TlmMsg.Payload.X = 0.0;
               MqttTopicRate->TlmMsg.Payload.Y = MqttTopicRate->TestAxisRate;
               MqttTopicRate->TestAxis         = MQTT_TOPIC_RATE_TEST_AXIS_Y;
            }
            break;
         case MQTT_TOPIC_RATE_TEST_AXIS_Y:
            if (++MqttTopicRate->TestAxisCycleCnt > MqttTopicRate->TestAxisCycleLim)
            {
               MqttTopicRate->TestAxisCycleCnt = 0;
               MqttTopicRate->TlmMsg.Payload.Y = 0.0;
               MqttTopicRate->TlmMsg.Payload.Z = MqttTopicRate->TestAxisRate;
               MqttTopicRate->TestAxis         = MQTT_TOPIC_RATE_TEST_AXIS_Z;
            }
            break;
         case MQTT_TOPIC_RATE_TEST_AXIS_Z:
            if (++MqttTopicRate->TestAxisCycleCnt > MqttTopicRate->TestAxisCycleLim)
            {
               MqttTopicRate->TestAxisCycleCnt = 0;
               MqttTopicRate->TlmMsg.Payload.Z = 0.0;
               MqttTopicRate->TlmMsg.Payload.X = MqttTopicRate->TestAxisRate;
               MqttTopicRate->TestAxis         = MQTT_TOPIC_RATE_TEST_AXIS_X;
            }
            break;
         default:
            MqttTopicRate->TestAxisCycleCnt = 0;
            MqttTopicRate->TlmMsg.Payload.X = MqttTopicRate->TestAxisRate;
            MqttTopicRate->TlmMsg.Payload.Y = 0.0;
            MqttTopicRate->TlmMsg.Payload.Z = 0.0;
            MqttTopicRate->TestAxis         = MQTT_TOPIC_RATE_TEST_AXIS_X;
            break;
         
      } /* End axis switch */
   }
   
   CFE_SB_TimeStampMsg(CFE_MSG_PTR(MqttTopicRate->TlmMsg.TelemetryHeader));
   CFE_SB_TransmitMsg(CFE_MSG_PTR(MqttTopicRate->TlmMsg.TelemetryHeader), true);
   
} /* End MQTT_TOPIC_RATE_SbMsgTest() */


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

   memset(&MqttTopicRate->TlmMsg.Payload, 0, sizeof(MQTT_GW_RateTlm_Payload_t));
   ObjLoadCnt = CJSON_LoadObjArray(JsonTblObjs, MqttTopicRate->JsonObjCnt, 
                                  JsonMsgPayload, PayloadLen);
   CFE_EVS_SendEvent(MQTT_TOPIC_RATE_LOAD_JSON_DATA_EID, CFE_EVS_EventType_DEBUG,
                     "Rate LoadJsonData() process %d JSON objects", (uint16)ObjLoadCnt);

   if (ObjLoadCnt == MqttTopicRate->JsonObjCnt)
   {
      memcpy(&MqttTopicRate->TlmMsg.Payload, &RateData, sizeof(MQTT_GW_RateTlm_Payload_t));      
      RetStatus = true;
   }
   else
   {
      CFE_EVS_SendEvent(MQTT_TOPIC_RATE_JSON_TO_CCSDS_ERR_EID, CFE_EVS_EventType_ERROR, 
                        "Error processing rate topic, payload contained %d of %d data objects",
                        (unsigned int)ObjLoadCnt, (unsigned int)MqttTopicRate->JsonObjCnt);
   }
   
   return RetStatus;
   
} /* End LoadJsonData() */

