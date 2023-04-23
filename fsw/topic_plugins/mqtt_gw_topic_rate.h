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
**   Manage MQTT 3-axis rate topic
**
** Notes:
**   None
**
** References:
**   1. cFS Basecamp Object-based Application Developer's Guide
**   2. cFS Application Developer's Guide
**
*/

#ifndef _mqtt_gw_topic_rate_
#define _mqtt_gw_topic_rate_

/*
** Includes
*/

#include "app_cfg.h"
#include "mqtt_gw_topic_plugin.h"

/***********************/
/** Macro Definitions **/
/***********************/


/*
** Event Message IDs
*/

#define MQTT_GW_TOPIC_RATE_INIT_SB_MSG_TEST_EID  (MQTT_GW_TOPIC_PLUGIN_3_BASE_EID + 0)
#define MQTT_GW_TOPIC_RATE_SB_MSG_TEST_EID       (MQTT_GW_TOPIC_PLUGIN_3_BASE_EID + 1)
#define MQTT_GW_TOPIC_RATE_LOAD_JSON_DATA_EID    (MQTT_GW_TOPIC_PLUGIN_3_BASE_EID + 2)
#define MQTT_GW_TOPIC_RATE_JSON_TO_CCSDS_ERR_EID (MQTT_GW_TOPIC_PLUGIN_3_BASE_EID + 3)

/**********************/
/** Type Definitions **/
/**********************/

typedef enum 
{

  MQTT_GW_TOPIC_RATE_TEST_AXIS_X = 1,
  MQTT_GW_TOPIC_RATE_TEST_AXIS_Y = 2,
  MQTT_GW_TOPIC_RATE_TEST_AXIS_Z = 3
  
} MQTT_GW_TOPIC_RATE_TestAxis_t;

/******************************************************************************
** Telemetry
** 
** MQTT_GW_RatePluginTlm_t & MQTT_GW_RatePluginTlm_Payload_t defined in EDS
*/

typedef struct
{

   /*
   ** Rate Telemetry
   */
   
   MQTT_GW_RatePluginTlm_t  TlmMsg;
   char                     JsonMsgPayload[1024];

   /*
   ** SB test puts rate on a single axis for N cycles
   */
   
   MQTT_GW_TOPIC_RATE_TestAxis_t  TestAxis;
   uint16   TestAxisCycleCnt;
   uint16   TestAxisCycleLim;
   float    TestAxisDefRate;
   float    TestAxisRate;
   
   /*
   ** Subset of the standard CJSON table data because this isn't using the
   ** app_c_fw table manager service, but is using core-json in the same way
   ** as an app_c_fw managed table.
   */
   size_t  JsonObjCnt;

   uint32  CfeToJsonCnt;
   uint32  JsonToCfeCnt;
   
   
} MQTT_GW_TOPIC_RATE_Class_t;


/************************/
/** Exported Functions **/
/************************/


/******************************************************************************
** Function: MQTT_GW_TOPIC_RATE_Constructor
**
** Initialize the MQTT rate topic
**
** Notes:
**   None
**
*/
void MQTT_GW_TOPIC_RATE_Constructor(MQTT_GW_TOPIC_RATE_Class_t *MqttGwTopicRatePtr,
                                    MQTT_TOPIC_TBL_PluginFuncTbl_t *PluginFuncTbl,
                                    CFE_SB_MsgId_t TlmMsgMid);


#endif /* _mqtt_gw_topic_rate_ */
