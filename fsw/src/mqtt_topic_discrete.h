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
** References:
**   1. OpenSatKit Object-based Application Developer's Guide
**   2. cFS Application Developer's Guide
**
*/

#ifndef _mqtt_topic_discrete_
#define _mqtt_topic_discrete_

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

#define MQTT_TOPIC_DISCRETE_INIT_SB_MSG_TEST_EID  (MQTT_TOPIC_DISCRETE_BASE_EID + 0)
#define MQTT_TOPIC_DISCRETE_SB_MSG_TEST_EID       (MQTT_TOPIC_DISCRETE_BASE_EID + 1)
#define MQTT_TOPIC_DISCRETE_LOAD_JSON_DATA_EID    (MQTT_TOPIC_DISCRETE_BASE_EID + 2)
#define MQTT_TOPIC_DISCRETE_JSON_TO_CCSDS_ERR_EID (MQTT_TOPIC_DISCRETE_BASE_EID + 3)

/**********************/
/** Type Definitions **/
/**********************/


/******************************************************************************
** Telemetry
** 
** MQTT_GW_DiscreteTlm_t & MQTT_GW_DiscreteTlm_Payload_t defined in EDS
*/



typedef struct
{

   /*
   ** Discrete Telemetry
   */
   
   MQTT_GW_DiscreteTlm_t  TlmMsg;
   char  JsonMsgTopic[MQTT_TOPIC_TBL_MAX_TOPIC_LEN];
   char  JsonMsgPayload[1024];

   /*
   ** SB test treats the 4 discretes as a 4-bit integer that is incremented 
   ** from 0..15 
   */
   
   uint16  TestData;
   
   /*
   ** Subset of the standard CJSON table data because this isn't using the OSK
   ** table manager service, but is using core-json in the same way as an OSK
   ** table.
   */
   size_t  JsonObjCnt;

   uint32  CfeToJsonCnt;
   uint32  JsonToCfeCnt;
   
   
} MQTT_TOPIC_DISCRETE_Class_t;


/************************/
/** Exported Functions **/
/************************/


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
                                     CFE_SB_MsgId_t TlmMsgMid, const char *Topic);


/******************************************************************************
** Function: MQTT_TOPIC_DISCRETE_CfeToJson
**
** Convert a cFE discrete message to a JSON topic message 
**
** Notes:
**   1.  Signature must match MQTT_TOPIC_TBL_CfeToJson_t
*/
bool MQTT_TOPIC_DISCRETE_CfeToJson(const char **JsonMsgTopic, const char **JsonMsgPayload,
                                   const CFE_MSG_Message_t *CfeMsg);


/******************************************************************************
** Function: MQTT_TOPIC_DISCRETE_JsonToCfe
**
** Convert a JSON discrete topic message to a cFE discrete message 
**
** Notes:
**   1.  Signature must match MQTT_TOPIC_TBL_JsonToCfe_t
*/
bool MQTT_TOPIC_DISCRETE_JsonToCfe(CFE_MSG_Message_t **CfeMsg, 
                                   const char *JsonMsgPayload, uint16 PayloadLen);

/******************************************************************************
** Function: MQTT_TOPIC_DISCRETE_SbMsgTest
**
** Generate and send SB discrete topic messages on SB that are read back by MQTT_GW
** and cause MQTT messages to be generated from the SB messages.  
**
*/
void MQTT_TOPIC_DISCRETE_SbMsgTest(bool Init, int16 Param);


#endif /* _mqtt_topic_discrete_ */
