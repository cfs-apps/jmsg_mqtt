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
**   Manage MQTT coordinate message
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

#include "mqtt_topic_coord.h"

/************************************/
/** Local File Function Prototypes **/
/************************************/

static bool LoadJsonData(const char *JsonMsgPayload, uint16 PayloadLen);


/**********************/
/** Global File Data **/
/**********************/

static MQTT_TOPIC_COORD_Class_t* MqttTopicCoord = NULL;

static MQTT_GW_CoordTlm_Payload_t  CoordData; /* Working buffer for loads */

static CJSON_Obj_t JsonTblObjs[] = 
{

   /* Table           Table                                       core-json       length of query       */
   /* Data Address,   Data Length,  Updated,  Data Type,  Float,  query string,   string(exclude '\0')  */
   
   { &CoordData.X,     4,            false,   JSONNumber, true,   { "coord.x",    (sizeof("coord.x")-1)} },
   { &CoordData.Y,     4,            false,   JSONNumber, true,   { "coord.y",    (sizeof("coord.y")-1)} },
   { &CoordData.Z,     4,            false,   JSONNumber, true,   { "coord.z",    (sizeof("coord.z")-1)} }
   
};

static const char *NullCoordMsg = "{\"coord\":{\"x\": 0.0,\"y\": 0.0,\"z\": 0.0}}";

/******************************************************************************
** Function: MQTT_TOPIC_COORD_Constructor
**
** Initialize the MQTT coord topic
**
** Notes:
**   None
**
*/
#define DEG_PER_SEC_IN_RADIANS 0.0174533
void MQTT_TOPIC_COORD_Constructor(MQTT_TOPIC_COORD_Class_t *MqttTopicCoordPtr, 
                                  CFE_SB_MsgId_t TlmMsgMid)
{

   MqttTopicCoord = MqttTopicCoordPtr;
   memset(MqttTopicCoord, 0, sizeof(MQTT_TOPIC_COORD_Class_t));

   MqttTopicCoord->JsonObjCnt = (sizeof(JsonTblObjs)/sizeof(CJSON_Obj_t));
   
   CFE_MSG_Init(CFE_MSG_PTR(MqttTopicCoord->TlmMsg), TlmMsgMid, sizeof(MQTT_GW_CoordTlm_t));
   
   /*
   ** Cycle counts are used by sim to switch axis
   ** Need to coordinate with topic message SB pend time
   */
   MqttTopicCoord->TestAxis         = MQTT_TOPIC_COORD_TEST_AXIS_X;
   MqttTopicCoord->TestAxisCycleCnt = 0;
   MqttTopicCoord->TestAxisDefRate  = 0.0087265;  /* 0.5 deg/sec in radians with 250ms pend */
   MqttTopicCoord->TestAxisDefRate  = 0.0174533;  /* 1.0 deg/sec in radians with 250ms pend */
   MqttTopicCoord->TestAxisDefRate  *= 7.5;       /* 7.5 deg/s so 24 cycles for 90 deg */
   MqttTopicCoord->TestAxisRate     = MqttTopicCoord->TestAxisDefRate;
   MqttTopicCoord->TestAxisCycleLim = 12*4;       /* Rotate 90 deg on each axis */
   
} /* End MQTT_TOPIC_COORD_Constructor() */


/******************************************************************************
** Function: MQTT_TOPIC_COORD_CfeToJson
**
** Convert a cFE coord message to a JSON topic message 
**
*/
bool MQTT_TOPIC_COORD_CfeToJson(const char **JsonMsgPayload,
                                const CFE_MSG_Message_t *CfeMsg)
{

   bool  RetStatus = false;
   int   PayloadLen; 
   const MQTT_GW_CoordTlm_Payload_t *CoordMsg = CMDMGR_PAYLOAD_PTR(CfeMsg, MQTT_GW_CoordTlm_t);

   *JsonMsgPayload = NullCoordMsg;
   
   PayloadLen = sprintf(MqttTopicCoord->JsonMsgPayload,
                "{\"coord\":{\"x\": %0.6f,\"y\": %0.6f,\"z\": %0.6f}}",
                CoordMsg->X, CoordMsg->Y, CoordMsg->Z);

   if (PayloadLen > 0)
   {
      *JsonMsgPayload = MqttTopicCoord->JsonMsgPayload;
      MqttTopicCoord->CfeToJsonCnt++;
      RetStatus = true;
   }
   
   return RetStatus;
   
} /* End MQTT_TOPIC_COORD_CfeToJson() */


/******************************************************************************
** Function: MQTT_TOPIC_COORD_JsonToCfe
**
** Convert a cFE coordinate message on the SB causing MQTT_TOPIC_COORD_CfeToJson()
** to be called which the converts the SB message to a MQTT JSON coord topic. 
** The topic can be observed
**
*/
bool MQTT_TOPIC_COORD_JsonToCfe(CFE_MSG_Message_t **CfeMsg, 
                                const char *JsonMsgPayload, uint16 PayloadLen)
{
   
   bool RetStatus = false;
   
   *CfeMsg = NULL;
   
   if (LoadJsonData(JsonMsgPayload, PayloadLen))
   {
      *CfeMsg = (CFE_MSG_Message_t *)&MqttTopicCoord->TlmMsg;

      ++MqttTopicCoord->JsonToCfeCnt;
      RetStatus = true;
   }

   return RetStatus;
   
} /* End MQTT_TOPIC_COORD_JsonToCfe() */


/******************************************************************************
** Function: MQTT_TOPIC_COORD_SbMsgTest
**
** Convert a JSON coord topic message to a cFE coord message
**
** Notes:
**   1. Param is used to scale the default test rate and change the sign
**      Increase rate:   2 <= Param <= 10
**      Deccrease rate: 12 <= Param <= 20
**
*/
void MQTT_TOPIC_COORD_SbMsgTest(bool Init, int16 Param)
{

   if (Init)
   {

      MqttTopicCoord->TestAxisRate = MqttTopicCoord->TestAxisDefRate;
      
      if (Param >= 2 && Param <= 10)
      {
         MqttTopicCoord->TestAxisRate *= (float)Param;
      }
      else if (Param >= 12 && Param <= 20)
      {         
         MqttTopicCoord->TestAxisRate /= ((float)Param - 10.0);
      }
      MqttTopicCoord->TlmMsg.Payload.X = MqttTopicCoord->TestAxisRate;
      MqttTopicCoord->TlmMsg.Payload.Y = 0.0;
      MqttTopicCoord->TlmMsg.Payload.Z = 0.0;
      MqttTopicCoord->TestAxis         = MQTT_TOPIC_COORD_TEST_AXIS_X;
      MqttTopicCoord->TestAxisCycleCnt = 0;

      CFE_EVS_SendEvent(MQTT_TOPIC_COORD_INIT_SB_MSG_TEST_EID, CFE_EVS_EventType_INFORMATION,
                        "Coord topic test started with axis rate %6.2f", MqttTopicCoord->TestAxisRate);

   }
   else
   {
      switch (MqttTopicCoord->TestAxis)
      {
         case MQTT_TOPIC_COORD_TEST_AXIS_X:
            if (++MqttTopicCoord->TestAxisCycleCnt > MqttTopicCoord->TestAxisCycleLim)
            {
               MqttTopicCoord->TestAxisCycleCnt = 0;
               MqttTopicCoord->TlmMsg.Payload.X = 0.0;
               MqttTopicCoord->TlmMsg.Payload.Y = MqttTopicCoord->TestAxisRate;
               MqttTopicCoord->TestAxis         = MQTT_TOPIC_COORD_TEST_AXIS_Y;
            }
            break;
         case MQTT_TOPIC_COORD_TEST_AXIS_Y:
            if (++MqttTopicCoord->TestAxisCycleCnt > MqttTopicCoord->TestAxisCycleLim)
            {
               MqttTopicCoord->TestAxisCycleCnt = 0;
               MqttTopicCoord->TlmMsg.Payload.Y = 0.0;
               MqttTopicCoord->TlmMsg.Payload.Z = MqttTopicCoord->TestAxisRate;
               MqttTopicCoord->TestAxis         = MQTT_TOPIC_COORD_TEST_AXIS_Z;
            }
            break;
         case MQTT_TOPIC_COORD_TEST_AXIS_Z:
            if (++MqttTopicCoord->TestAxisCycleCnt > MqttTopicCoord->TestAxisCycleLim)
            {
               MqttTopicCoord->TestAxisCycleCnt = 0;
               MqttTopicCoord->TlmMsg.Payload.Z = 0.0;
               MqttTopicCoord->TlmMsg.Payload.X = MqttTopicCoord->TestAxisRate;
               MqttTopicCoord->TestAxis         = MQTT_TOPIC_COORD_TEST_AXIS_X;
            }
            break;
         default:
            MqttTopicCoord->TestAxisCycleCnt = 0;
            MqttTopicCoord->TlmMsg.Payload.X = MqttTopicCoord->TestAxisRate;
            MqttTopicCoord->TlmMsg.Payload.Y = 0.0;
            MqttTopicCoord->TlmMsg.Payload.Z = 0.0;
            MqttTopicCoord->TestAxis         = MQTT_TOPIC_COORD_TEST_AXIS_X;
            break;
         
      } /* End axis switch */
   }
   
   CFE_SB_TimeStampMsg(CFE_MSG_PTR(MqttTopicCoord->TlmMsg.TelemetryHeader));
   CFE_SB_TransmitMsg(CFE_MSG_PTR(MqttTopicCoord->TlmMsg.TelemetryHeader), true);
   
} /* End MQTT_TOPIC_COORD_SbMsgTest() */


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

   memset(&MqttTopicCoord->TlmMsg.Payload, 0, sizeof(MQTT_GW_CoordTlm_Payload_t));
   ObjLoadCnt = CJSON_LoadObjArray(JsonTblObjs, MqttTopicCoord->JsonObjCnt, 
                                  JsonMsgPayload, PayloadLen);
   CFE_EVS_SendEvent(MQTT_TOPIC_COORD_LOAD_JSON_DATA_EID, CFE_EVS_EventType_DEBUG,
                     "Coord LoadJsonData() process %d JSON objects", (uint16)ObjLoadCnt);

   if (ObjLoadCnt == MqttTopicCoord->JsonObjCnt)
   {
      memcpy(&MqttTopicCoord->TlmMsg.Payload, &CoordData, sizeof(MQTT_GW_CoordTlm_Payload_t));      
      RetStatus = true;
   }
   else
   {
      CFE_EVS_SendEvent(MQTT_TOPIC_COORD_JSON_TO_CCSDS_ERR_EID, CFE_EVS_EventType_ERROR, 
                        "Error processing coord message, payload contained %d of %d data objects",
                        (unsigned int)ObjLoadCnt, (unsigned int)MqttTopicCoord->JsonObjCnt);
   }
   
   return RetStatus;
   
} /* End LoadJsonData() */

