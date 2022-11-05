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
**   1. OpenSatKit Object-based Application Developer's Guide
**   2. cFS Application Developer's Guide
**
*/

#ifndef _mqtt_topic_sbmsg_
#define _mqtt_topic_sbmsg_

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

#define MQTT_TOPIC_SBMSG_INIT_SB_MSG_TEST_EID  (MQTT_TOPIC_SBMSG_BASE_EID + 0)
#define MQTT_TOPIC_SBMSG_HEX_DECODE_EID        (MQTT_TOPIC_SBMSG_BASE_EID + 1)

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

   uint16 DiscreteTlmMsgLen;
   KIT_TO_WrappedSbMsgTlm_t  MqttToSbWrapTlmMsg;
   MQTT_GW_DiscreteTlm_t     DiscreteTlmMsg;

   /*
   ** MQTT message data
   */
   
   char  MqttMsgTopic[MQTT_TOPIC_TBL_MAX_TOPIC_LEN];
   char  MqttMsgPayload[MQTT_TOPIC_SB_MSG_MAX_LEN*2]; /* Endcoded hex is twice as long */

   uint32  CfeToMqttCnt;
   uint32  MqttToCfeCnt;
   
   CFE_SB_MsgId_t  KitToSbWrapTlmMid;

   /*
   ** Use a discrete message for built in test
   */
   
   uint32 SbTestCnt;
   
} MQTT_TOPIC_SBMSG_Class_t;


/************************/
/** Exported Functions **/
/************************/


/******************************************************************************
** Function: MQTT_TOPIC_SBMSG_Constructor
**
** Initialize the MQTT SBMSG topic
**
** Notes:
**   1. The discrete telemetry message is used for the built in test.
**   2. The first topic is assumed to be defined as the SB messages wrapped
**      in an DB message. These messages are sent to an MQTT broker.
**   2. DiscreteMidOffset is the offset from the topic base MID for the
**      discrete MQTT topic that is used in SbMsg's test.
**
*/
void MQTT_TOPIC_SBMSG_Constructor(MQTT_TOPIC_SBMSG_Class_t *MqttTopicSbMsgPtr,
                                 CFE_SB_MsgId_t TopicBaseMid, 
                                 CFE_SB_MsgId_t DiscreteTlmMsgMid,
                                 CFE_SB_MsgId_t WrapSbMsgMid,
                                 const char *Topic);


/******************************************************************************
** Function: MQTT_TOPIC_SBMSG_CfeToMqtt
**
** Copy a cFE SB message to an MQTT topic's payload 
**
** Notes:
**   1. Signature must match MQTT_TOPIC_TBL_CfeToJson_t
**   2. This doesn't use JSON
*/
bool MQTT_TOPIC_SBMSG_CfeToMqtt(const char **JsonMsgTopic, const char **JsonMsgPayload,
                                const CFE_MSG_Message_t *CfeMsg);


/******************************************************************************
** Function: MQTT_TOPIC_SBMSG_MqttToCfe
**
** Copy a MQTT topic's payload to cFE SB message 
**
** Notes:
**   1.  Signature must match MQTT_TOPIC_TBL_JsonToCfe_t
*/
bool MQTT_TOPIC_SBMSG_MqttToCfe(CFE_MSG_Message_t **CfeMsg, 
                                const char *JsonMsgPayload, uint16 PayloadLen);


/******************************************************************************
** Function: MQTT_TOPIC_SBMSG_SbMsgTest
**
** Generate and send MQTT_GW discrete topic messages on SB that are read back
** by MQTT_GW  and cause MQTT messages to be generated from the SB messages.  
**
*/
void MQTT_TOPIC_SBMSG_SbMsgTest(bool Init, int16 Param);


#endif /* _mqtt_topic_sbmsg_ */
