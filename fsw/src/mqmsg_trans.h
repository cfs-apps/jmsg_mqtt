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
**   Translate between MQTT message topics and SB messages
**
** Notes:
**   1. Each supported MQTT topic is listed in a JSON file and each
**      topic has a JSON file that defines the topic's content.
**   2. The Basecamp JSON table coding idiom is to use a separate object to manage 
**      the table. Since MQTT manager has very little functionality beyond
**      processing the table, a single object is used for management functions
**      and table processing.
**
*/
#ifndef _msg_trans_
#define _msg_trans_

/*
** Includes
*/

#include "MQTTLinux.h"  /* Must be included prior to "MQTTClient.h" */
#include "MQTTClient.h"

#include "app_cfg.h"
#include "jmsg_topic_tbl.h"

/***********************/
/** Macro Definitions **/
/***********************/


/*
** Events
*/

#define MQMSG_TRANS_PROCESS_MQTT_MSG_EID      (MQMSG_TRANS_BASE_EID + 0)
#define MQMSG_TRANS_PROCESS_MQTT_MSG_INFO_EID (MQMSG_TRANS_BASE_EID + 1)
#define MQMSG_TRANS_PROCESS_SB_MSG_EID        (MQMSG_TRANS_BASE_EID + 2)
#define MQMSG_TRANS_PROCESS_SB_MSG_INFO_EID   (MQMSG_TRANS_BASE_EID + 3)


/**********************/
/** Type Definitions **/
/**********************/

/*
** JSON Message
*/

typedef struct
{
   
   char   Data[MQTT_CLIENT_READ_BUF_LEN];

}  JSON_MSG_Payload_t;

typedef struct
{
   
   CFE_MSG_TelemetryHeader_t TelemetryHeader;
   JSON_MSG_Payload_t        Payload;

}  JSON_MSG_Pkt_t;


/*
** Class Definition
*/

typedef struct 
{

   uint32  ValidMqttMsgCnt;
   uint32  InvalidMqttMsgCnt;
   uint32  ValidSbMsgCnt;
   uint32  InvalidSbMsgCnt;
   
   /*
   ** Telemetry Messages
   */
   
   JSON_MSG_Pkt_t  JsonMsgPkt;
      
} MQMSG_TRANS_Class_t;


/************************/
/** Exported Functions **/
/************************/


/******************************************************************************
** Function: MQMSG_TRANS_Constructor
**
** Notes:
**    1. This function must be called prior to any other functions being
**       called using the same cmdmgr instance.
*/
void MQMSG_TRANS_Constructor(MQMSG_TRANS_Class_t *MsgTransPtr,
                             const INITBL_Class_t *IniTbl);


/******************************************************************************
** Function: MQMSG_TRANS_ProcessMqttMsg
**
** Notes:
**   1. Signature must mach MQTT_CLIENT_MsgCallback
**
*/
void MQMSG_TRANS_ProcessMqttMsg(MessageData *MsgData);


/******************************************************************************
** Function: MQMSG_TRANS_ProcessSbMsg
**
** Notes:
**   None
**
*/
bool MQMSG_TRANS_ProcessSbMsg(const CFE_MSG_Message_t *CfeMsgPt,
                            const char **Topic, const char **Payload);


/******************************************************************************
** Function: MQMSG_TRANS_ResetStatus
**
** Reset counters and status flags to a known reset state.
**
*/
void MQMSG_TRANS_ResetStatus(void);

#endif /* _msg_trans_ */
