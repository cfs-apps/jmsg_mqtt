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
**   Manage JMSG MQTT discrete plugin topic
**
** Notes:
**   None
**
*/

#ifndef _mqtt_topic_discrete_
#define _mqtt_topic_discrete_

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

#define MQTT_TOPIC_DISCRETE_INIT_PLUGIN_TEST_EID  (JMSG_MQTT_PLUGIN_TopicPluginBaseEid_Discrete + 0)
#define MQTT_TOPIC_DISCRETE_PLUGIN_TEST_EID       (JMSG_MQTT_PLUGIN_TopicPluginBaseEid_Discrete + 1)
#define MQTT_TOPIC_DISCRETE_LOAD_JSON_DATA_EID    (JMSG_MQTT_PLUGIN_TopicPluginBaseEid_Discrete + 2)
#define MQTT_TOPIC_DISCRETE_JSON_TO_CCSDS_ERR_EID (JMSG_MQTT_PLUGIN_TopicPluginBaseEid_Discrete + 3)

/**********************/
/** Type Definitions **/
/**********************/


/******************************************************************************
** Telemetry
** 
** JMSG_MQTT_PLUGIN_DiscreteTlmMsg_t & JMSG_MQTT_PLUGIN_DiscreteTlmMsg_Payload_t are defined in EDS
*/

typedef struct
{

   /*
   ** Discrete Telemetry
   */
   
   char            JsonMsgPayload[1024];
   CFE_SB_MsgId_t  SbMsgId;
   JMSG_MQTT_PLUGIN_DiscreteTlmMsg_t  TlmMsg;

   /*
   ** The plugin test treats the 4 integers as a 4-bit integer that is
   ** incremented from 0..15 
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
   
   
} MQTT_TOPIC_DISCRETE_Class_t;


/************************/
/** Exported Functions **/
/************************/


/******************************************************************************
** Function: MQTT_TOPIC_DISCRETE_Constructor
**
** Initialize the JMSG MQTT discrete plugin topic

** Notes:
**   None
**
*/
void MQTT_TOPIC_DISCRETE_Constructor(MQTT_TOPIC_DISCRETE_Class_t *MqttTopicDiscretePtr,
                                     JMSG_USR_TopicPlugin_Enum_t TopicPlugin);


#endif /* _mqtt_topic_discrete_ */
