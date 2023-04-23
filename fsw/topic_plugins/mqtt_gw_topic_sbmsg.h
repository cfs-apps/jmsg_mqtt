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
**   Manage MQTT Software Bus topics
**
** Notes:
**   1. SB messages are not interpretted. A single MQTT topic is used
**      to transport any SB message as its payload
**
** References:
**   1. cFS Basecamp Object-based Application Developer's Guide
**   2. cFS Application Developer's Guide
**
*/

#ifndef _mqtt_gw_topic_sbmsg_
#define _mqtt_gw_topic_sbmsg_

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

#define MQTT_GW_TOPIC_SBMSG_INIT_SB_MSG_TEST_EID  (MQTT_GW_TOPIC_PLUGIN_1_BASE_EID + 0)
#define MQTT_GW_TOPIC_SBMSG_HEX_DECODE_EID        (MQTT_GW_TOPIC_PLUGIN_1_BASE_EID + 1)

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
   KIT_TO_WrappedSbMsgTlm_t     MqttToSbWrapTlmMsg;
   MQTT_GW_DiscretePluginTlm_t  DiscretePluginTlmMsg;

   /*
   ** MQTT message data
   */
   
   char  MqttMsgPayload[MQTT_TOPIC_SB_MSG_MAX_LEN*2]; /* Endcoded hex is twice as long as the binary */

   uint32  CfeToMqttCnt;
   uint32  MqttToCfeCnt;
   
   CFE_SB_MsgId_t  KitToSbWrapTlmMid;

   /*
   ** Built in test
   */
   
   uint32 SbTestCnt;
   KIT_TO_WrappedSbMsgTlm_t  TunnelTlm;
   
} MQTT_GW_TOPIC_SBMSG_Class_t;


/************************/
/** Exported Functions **/
/************************/


/******************************************************************************
** Function: MQTT_GW_TOPIC_SBMSG_Constructor
**
** Initialize the MQTT SBMSG topic
**
** Notes:
**   1. The integer telemetry message is used for the built in test.
**   2. The first topic is assumed to be defined as the SB messages wrapped
**      in an DB message. These messages are sent to an MQTT broker.
**   3. DiscreteMidOffset is the offset from the topic base MID for the
**      integer MQTT topic that is used in SbMsg's test.
**
*/
void MQTT_GW_TOPIC_SBMSG_Constructor(MQTT_GW_TOPIC_SBMSG_Class_t *MqttGwTopicSbMsgPtr,
                                     MQTT_TOPIC_TBL_PluginFuncTbl_t *PluginFuncTbl,
                                     CFE_SB_MsgId_t DiscretePluginTlmMid,
                                     CFE_SB_MsgId_t WrapSbTlmMid, CFE_SB_MsgId_t TunnelTlmMid);

#endif /* _mqtt_gw_topic_sbmsg_ */
