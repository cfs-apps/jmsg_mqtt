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
**   Manage MQTT integer topic
**
** Notes:
**   None
**
** References:
**   1. OpenSatKit Object-based Application Developer's Guide
**   2. cFS Application Developer's Guide
**
*/

#ifndef _mqtt_topic_integer_
#define _mqtt_topic_integer_

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

#define MQTT_TOPIC_INTEGER_INIT_SB_MSG_TEST_EID  (MQTT_TOPIC_INTEGER_BASE_EID + 0)
#define MQTT_TOPIC_INTEGER_SB_MSG_TEST_EID       (MQTT_TOPIC_INTEGER_BASE_EID + 1)
#define MQTT_TOPIC_INTEGER_LOAD_JSON_DATA_EID    (MQTT_TOPIC_INTEGER_BASE_EID + 2)
#define MQTT_TOPIC_INTEGER_JSON_TO_CCSDS_ERR_EID (MQTT_TOPIC_INTEGER_BASE_EID + 3)

/**********************/
/** Type Definitions **/
/**********************/


/******************************************************************************
** Telemetry
** 
** MQTT_GW_IntegerTlm_t & MQTT_GW_IntegerTlm_Payload_t defined in EDS
*/



typedef struct
{

   /*
   ** Integer Telemetry
   */
   
   MQTT_GW_IntegerTlm_t  TlmMsg;
   char                  JsonMsgPayload[1024];

   /*
   ** SB test treats the 4 integers as a 4-bit integer that is incremented 
   ** from 0..15 
   */
   
   uint16  TestData;
   
   /*
   ** Subset of the standard CJSON table data because this isn't using the
   ** app_c_fw table manager service, but is using core-json in the same way
   ** as an app_c_fw managed table.
   */
   size_t  JsonObjCnt;

   uint32  CfeToJsonCnt;
   uint32  JsonToCfeCnt;
   
   
} MQTT_TOPIC_INTEGER_Class_t;


/************************/
/** Exported Functions **/
/************************/


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
                                    CFE_SB_MsgId_t TlmMsgMid);


/******************************************************************************
** Function: MQTT_TOPIC_INTEGER_CfeToJson
**
** Convert a cFE integer message to a JSON topic message 
**
** Notes:
**   1.  Signature must match MQTT_TOPIC_TBL_CfeToJson_t
*/
bool MQTT_TOPIC_INTEGER_CfeToJson(const char **JsonMsgPayload,
                                  const CFE_MSG_Message_t *CfeMsg);


/******************************************************************************
** Function: MQTT_TOPIC_INTEGER_JsonToCfe
**
** Convert a JSON integer topic message to a cFE integer message 
**
** Notes:
**   1.  Signature must match MQTT_TOPIC_TBL_JsonToCfe_t
*/
bool MQTT_TOPIC_INTEGER_JsonToCfe(CFE_MSG_Message_t **CfeMsg, 
                                  const char *JsonMsgPayload, uint16 PayloadLen);

/******************************************************************************
** Function: MQTT_TOPIC_INTEGER_SbMsgTest
**
** Generate and send SB integer topic messages on SB that are read back by MQTT_GW
** and cause MQTT messages to be generated from the SB messages.  
**
*/
void MQTT_TOPIC_INTEGER_SbMsgTest(bool Init, int16 Param);


#endif /* _mqtt_topic_integer_ */
