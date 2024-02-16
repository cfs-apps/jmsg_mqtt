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
**   Manage the conversion of telemetry messages
**
** Notes:
**   1. Depends on KIT_TO's wrapped teleetry message definitions.
**   2. Telemetry messages are not interpretted, they are treated 
**      as payloads in wrapped messages.
**
*/

#ifndef _mqtt_gw_topic_tlm_
#define _mqtt_gw_topic_tlm_

/*
** Includes
*/

#include "app_cfg.h"
#include "mqtt_gw_topic_plugin.h"
#include "kit_to_eds_typedefs.h"

/***********************/
/** Macro Definitions **/
/***********************/


/*
** Event Message IDs
*/

#define MQTT_GW_TOPIC_TLM_INIT_SB_MSG_TEST_EID  (MQTT_GW_TOPIC_PLUGIN_1_BASE_EID + 0)
#define MQTT_GW_TOPIC_TLM_HEX_DECODE_EID        (MQTT_GW_TOPIC_PLUGIN_1_BASE_EID + 1)

/**********************/
/** Type Definitions **/
/**********************/

/******************************************************************************
** Telemetry
** 
** Since this topic is a wrapper for existing telemetry messages, no new
** telemetry definitions are required.
*/

typedef struct
{

   /*
   ** Telemetry
   */

   uint16 DiscretePluginTlmMsgLen;
   KIT_TO_WrappedSbMsgTlm_t  WrappedTlmMsg;

   /*
   ** MQTT message data
   */
   
   char  MqttMsgPayload[MQTT_TOPIC_SB_MSG_MAX_LEN*2]; /* Endcoded hex is twice as long as the binary */

   uint32  CfeToMqttCnt;
   uint32  MqttToCfeCnt;
   
   /*
   ** Built in test
   */
   
   uint32 SbTestCnt;
   MQTT_GW_DiscretePluginTlm_t  DiscretePluginTlmMsg;
   
} MQTT_GW_TOPIC_TLM_Class_t;


/************************/
/** Exported Functions **/
/************************/


/******************************************************************************
** Function: MQTT_GW_TOPIC_TLM_Constructor
**
** Initialize the MQTT Telemetry topic
**
** Notes:
**   1. The discrete telemetry message is used for the built in test
**
*/
void MQTT_GW_TOPIC_TLM_Constructor(MQTT_GW_TOPIC_TLM_Class_t *MqttGwTopicTlmPtr,
                                   MQTT_TOPIC_TBL_PluginFuncTbl_t *PluginFuncTbl,
                                   CFE_SB_MsgId_t WrappedTlmMid, CFE_SB_MsgId_t DiscretePluginTlmMid);

#endif /* _mqtt_gw_topic_tlm_ */
