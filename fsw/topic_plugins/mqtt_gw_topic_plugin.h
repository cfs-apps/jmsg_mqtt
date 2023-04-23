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
**  Purpose:
**    Provide an interface for topic plugins.
**
**  Notes:
**    None 
**
**  References:
**    1. cFS Basecampt Object-based Application Developer's Guide.
**    2. cFS Application Developer's Guide.
**
*/

#ifndef _mqtt_gw_topic_plugins_
#define _mqtt_gw_topic_plugins_


/*
** Include Files:
*/

#include "mqtt_topic_tbl.h"

/***********************/
/** Macro Definitions **/
/***********************/

// These IDs must not overlap with MQTT_GW's app_cfg.h definitions
// The topics numbers must agree with topic IDs in MQTT_GW_TOPIC_Constructor()
#define MQTT_GW_TOPIC_PLUGIN_1_BASE_EID  (APP_C_FW_APP_BASE_EID + 200)
#define MQTT_GW_TOPIC_PLUGIN_2_BASE_EID  (APP_C_FW_APP_BASE_EID + 220)
#define MQTT_GW_TOPIC_PLUGIN_3_BASE_EID  (APP_C_FW_APP_BASE_EID + 240)
#define MQTT_GW_TOPIC_PLUGIN_4_BASE_EID  (APP_C_FW_APP_BASE_EID + 260)
#define MQTT_GW_TOPIC_PLUGIN_5_BASE_EID  (APP_C_FW_APP_BASE_EID + 280)
#define MQTT_GW_TOPIC_PLUGIN_6_BASE_EID  (APP_C_FW_APP_BASE_EID + 300)
#define MQTT_GW_TOPIC_PLUGIN_7_BASE_EID  (APP_C_FW_APP_BASE_EID + 320)
#define MQTT_GW_TOPIC_PLUGIN_8_BASE_EID  (APP_C_FW_APP_BASE_EID + 340)

#define MQTT_GW_TOPIC_PLUGIN_EID (APP_C_FW_APP_BASE_EID + 360)

/************************/
/** Exported Functions **/
/************************/


/******************************************************************************
** Function: MQTT_GW_TOPIC_PLUGIN_Constructor
**
** Call constructors for each topic plugin.
**
** Notes:
**   1. 
**
*/
void MQTT_GW_TOPIC_PLUGIN_Constructor(const MQTT_TOPIC_TBL_Data_t *TopicTbl,
                                      MQTT_TOPIC_TBL_PluginFuncTbl_t *PluginFuncTbl,
                                      uint32 DiscreteTlmTopicId, uint32 TunnelTlmTopicId);


#endif /* _mqtt_gw_topic_plugins_ */
