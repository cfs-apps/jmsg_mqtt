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
**   Manage the conversion of command messages
**
** Notes:
**   1. This only supports a remote commanding use case which is 
**      converting an MQTT command message into a SB message.
**   2. This plugin performs the sane EDS processing as CI_LAB.
**
*/

#ifndef _mqtt_gw_topic_cmd_
#define _mqtt_gw_topic_cmd_

/*
** Includes
*/

#include "cfe_hdr_eds_typedefs.h"

#include "app_cfg.h"
#include "mqtt_gw_topic_plugin.h"

/***********************/
/** Macro Definitions **/
/***********************/


/*
** Event Message IDs
*/

#define MQTT_GW_TOPIC_CMD_INIT_SB_MSG_TEST_EID  (MQTT_GW_TOPIC_PLUGIN_1_BASE_EID + 0)
#define MQTT_GW_TOPIC_CMD_CFE2JSON_EID          (MQTT_GW_TOPIC_PLUGIN_1_BASE_EID + 1)
#define MQTT_GW_TOPIC_CMD_JSON2CFE_EID          (MQTT_GW_TOPIC_PLUGIN_1_BASE_EID + 2)

/**********************/
/** Type Definitions **/
/**********************/

typedef struct
{

   /*
   ** MQTT message data
   */
   
   CFE_SB_Buffer_t                 *SbBufPtr;
   CFE_HDR_Message_PackedBuffer_t  MqttMsgBuf;

   uint32  CfeToMqttCnt;
   uint32  MqttToCfeCnt;
   
   /*
   ** Built in test
   */
   
   uint32 SbTestCnt;
   
} MQTT_GW_TOPIC_CMD_Class_t;


/************************/
/** Exported Functions **/
/************************/


/******************************************************************************
** Function: MQTT_GW_TOPIC_CMD_Constructor
**
** Initialize the MQTT Command plugin
**
** Notes:
**   1. No message ID's are required because this only supports MQTT commands
**      converted to SB messages and the MQTT message payload contains the
**      message ID.
**
*/
void MQTT_GW_TOPIC_CMD_Constructor(MQTT_GW_TOPIC_CMD_Class_t *MqttGwTopicCmdPtr,
                                   MQTT_TOPIC_TBL_PluginFuncTbl_t *PluginFuncTbl);

#endif /* _mqtt_gw_topic_cmd_ */
