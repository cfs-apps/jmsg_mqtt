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
**   Manage MQTT 3-axis coordinate topic
**
** Notes:
**   None
**
** References:
**   1. OpenSatKit Object-based Application Developer's Guide
**   2. cFS Application Developer's Guide
**
*/

#ifndef _mqtt_topic_coord_
#define _mqtt_topic_coord_

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

#define MQTT_TOPIC_COORD_INIT_SB_MSG_TEST_EID  (MQTT_TOPIC_COORD_BASE_EID + 0)
#define MQTT_TOPIC_COORD_SB_MSG_TEST_EID       (MQTT_TOPIC_COORD_BASE_EID + 1)
#define MQTT_TOPIC_COORD_LOAD_JSON_DATA_EID    (MQTT_TOPIC_COORD_BASE_EID + 2)
#define MQTT_TOPIC_COORD_JSON_TO_CCSDS_ERR_EID (MQTT_TOPIC_COORD_BASE_EID + 3)

/**********************/
/** Type Definitions **/
/**********************/

typedef enum 
{

  MQTT_TOPIC_COORD_TEST_AXIS_X = 1,
  MQTT_TOPIC_COORD_TEST_AXIS_Y = 2,
  MQTT_TOPIC_COORD_TEST_AXIS_Z = 3
  
} MQTT_TOPIC_COORD_TestAxis_t;

/******************************************************************************
** Telemetry
** 
** MQTT_GW_CoordTlm_t & MQTT_GW_CoordTlm_Payload_t defined in EDS
*/



typedef struct
{

   /*
   ** Coordinate Telemetry
   */
   
   MQTT_GW_CoordTlm_t  TlmMsg;
   char                JsonMsgPayload[1024];

   /*
   ** SB test puts rate on a single axis for N cycles
   */
   
   MQTT_TOPIC_COORD_TestAxis_t  TestAxis;
   uint16                       TestAxisCycleCnt;
   uint16                       TestAxisCycleLim;
   float                        TestAxisDefRate;
   float                        TestAxisRate;
   
   /*
   ** Subset of the standard CJSON table data because this isn't using the
   ** app_c_fw table manager service, but is using core-json in the same way
   ** as an app_c_fw managed table.
   */
   size_t  JsonObjCnt;

   uint32  CfeToJsonCnt;
   uint32  JsonToCfeCnt;
   
   
} MQTT_TOPIC_COORD_Class_t;


/************************/
/** Exported Functions **/
/************************/


/******************************************************************************
** Function: MQTT_TOPIC_COORD_Constructor
**
** Initialize the MQTT coord topic
**
** Notes:
**   None
**
*/
void MQTT_TOPIC_COORD_Constructor(MQTT_TOPIC_COORD_Class_t *MqttTopicCoordPtr,
                                  CFE_SB_MsgId_t TlmMsgMid);


/******************************************************************************
** Function: MQTT_TOPIC_COORD_CfeToJson
**
** Convert a cFE coord message to a JSON topic message 
**
** Notes:
**   1.  Signature must match MQTT_TOPIC_TBL_CfeToJson_t
*/
bool MQTT_TOPIC_COORD_CfeToJson(const char **JsonMsgPayload,
                                const CFE_MSG_Message_t *CfeMsg);


/******************************************************************************
** Function: MQTT_TOPIC_COORD_JsonToCfe
**
** Convert a JSON coord topic message to a cFE coord message 
**
** Notes:
**   1.  Signature must match MQTT_TOPIC_TBL_JsonToCfe_t
*/
bool MQTT_TOPIC_COORD_JsonToCfe(CFE_MSG_Message_t **CfeMsg, 
                                const char *JsonMsgPayload, uint16 PayloadLen);


/******************************************************************************
** Function: MQTT_TOPIC_COORD_SbMsgTest
**
** Generate and send SB coord topic messages on SB that are read back by MQTT_GW
** and cause MQTT messages to be generated from the SB messages.  
**
*/
void MQTT_TOPIC_COORD_SbMsgTest(bool Init, int16 Param);


#endif /* _mqtt_topic_coord_ */
