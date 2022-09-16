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

#ifndef _mqtt_topic_rate_
#define _mqtt_topic_rate_

/*
** Includes
*/

#include "app_cfg.h"


/***********************/
/** Macro Definitions **/
/***********************/


/*
** Event Message IDs
*/

#define MQTT_TOPIC_RATE_INIT_SB_MSG_TEST_EID  (MQTT_TOPIC_RATE_BASE_EID + 0)
#define MQTT_TOPIC_RATE_SB_MSG_TEST_EID       (MQTT_TOPIC_RATE_BASE_EID + 1)
#define MQTT_TOPIC_RATE_LOAD_JSON_DATA_EID    (MQTT_TOPIC_RATE_BASE_EID + 2)
#define MQTT_TOPIC_RATE_JSON_TO_CCSDS_ERR_EID (MQTT_TOPIC_RATE_BASE_EID + 3)

/**********************/
/** Type Definitions **/
/**********************/

typedef enum 
{

  MQTT_TOPIC_RATE_TEST_AXIS_X = 1,
  MQTT_TOPIC_RATE_TEST_AXIS_Y = 2,
  MQTT_TOPIC_RATE_TEST_AXIS_Z = 3
  
} MQTT_TOPIC_RATE_TestAxis_t;

/******************************************************************************
** Telemetry
** 
** MQTT_GW_RateTlm_t & MQTT_GW_RateTlm_Payload_t defined in EDS
*/



typedef struct
{

   /*
   ** Rate Telemetry
   */
   
   MQTT_GW_RateTlm_t  TlmMsg;
   char               JsonMsgTopic[MQTT_TOPIC_TBL_MAX_TOPIC_LEN];
   char               JsonMsgPayload[1024];

   /*
   ** SB test puts rate on a single axis for N cycles
   */
   
   MQTT_TOPIC_RATE_TestAxis_t  TestAxis;
   uint16                      TestAxisCycleCnt;
   uint16                      TestAxisCycleLim;
   float                       TestAxisDefRate;
   float                       TestAxisRate;
   
   /*
   ** Subset of the standard CJSON table data because this isn't using the OSK
   ** table manager service, but is using core-json in the same way as an OSK
   ** table.
   */
   size_t  JsonObjCnt;

   uint32  CfeToJsonCnt;
   uint32  JsonToCfeCnt;
   
   
} MQTT_TOPIC_RATE_Class_t;


/************************/
/** Exported Functions **/
/************************/


/******************************************************************************
** Function: MQTT_TOPIC_RATE_Constructor
**
** Initialize the MQTT rate topic
**
** Notes:
**   None
**
*/
void MQTT_TOPIC_RATE_Constructor(MQTT_TOPIC_RATE_Class_t *MqttTopicRatePtr,
                                 CFE_SB_MsgId_t TlmMsgMid, const char *Topic);


/******************************************************************************
** Function: MQTT_TOPIC_RATE_CfeToJson
**
** Convert a cFE rate message to a JSON topic message 
**
** Notes:
**   1.  Signature must match MQTT_TOPIC_TBL_CfeToJson_t
*/
bool MQTT_TOPIC_RATE_CfeToJson(const char **JsonMsgTopic, const char **JsonMsgPayload,
                               const CFE_MSG_Message_t *CfeMsg);


/******************************************************************************
** Function: MQTT_TOPIC_RATE_JsonToCfe
**
** Convert a JSON rate topic message to a cFE rate message 
**
** Notes:
**   1.  Signature must match MQTT_TOPIC_TBL_JsonToCfe_t
*/
bool MQTT_TOPIC_RATE_JsonToCfe(CFE_MSG_Message_t **CfeMsg, 
                               const char *JsonMsgPayload, uint16 PayloadLen);

/******************************************************************************
** Function: MQTT_TOPIC_RATE_SbMsgTest
**
** Generate and send SB rate topic messages on SB that are read back by MQTT_GW
** and cause MQTT messages to be generated from the SB messages.  
**
*/
void MQTT_TOPIC_RATE_SbMsgTest(bool Init, int16 Param);


#endif /* _mqtt_topic_rate_ */
