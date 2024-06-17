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
**   Manage JMSG MQTT rate plugin topic
**
** Notes:
**   None
**
*/

#ifndef _mqtt_topic_rate_
#define _mqtt_topic_rate_

/*
** Includes
*/

#include "app_cfg.h"
#include "jmsg_topic_plugin.h"
#include "jmsg_mqtt_plugin_eds_typedefs.h"


/***********************/
/** Macro Definitions **/
/***********************/


/*
** Event Message IDs
*/

#define MQTT_TOPIC_RATE_INIT_SB_MSG_TEST_EID  (JMSG_MQTT_PLUGIN_TopicPluginBaseEid_Rate + 0)
#define MQTT_TOPIC_RATE_SB_MSG_TEST_EID       (JMSG_MQTT_PLUGIN_TopicPluginBaseEid_Rate + 1)
#define MQTT_TOPIC_RATE_LOAD_JSON_DATA_EID    (JMSG_MQTT_PLUGIN_TopicPluginBaseEid_Rate + 2)
#define MQTT_TOPIC_RATE_JSON_TO_CCSDS_ERR_EID (JMSG_MQTT_PLUGIN_TopicPluginBaseEid_Rate + 3)


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
** JMSG_MQTT_PLUGIN_RatePluginTlm_t & JMSG_MQTT_PLUGIN_RatePluginTlm_Payload_t defined in EDS
*/

typedef struct
{

   /*
   ** Rate Telemetry
   */
   
   char  JsonMsgPayload[1024];
   CFE_SB_MsgId_t  SbMsgId;
   JMSG_MQTT_PLUGIN_RateTlmMsg_t  TlmMsg;

   /*
   ** SB test puts rate on a single axis for N cycles
   */
   
   MQTT_TOPIC_RATE_TestAxis_t  TestAxis;
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
                                 JMSG_USR_TopicPlugin_Enum_t TopicPlugin);


#endif /* _mqtt_topic_rate_ */
