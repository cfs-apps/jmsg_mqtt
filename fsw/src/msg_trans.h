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
**   2. The OSK JSON table coding ideom is use a separate object to manage 
**      the table. Since MQTT manager has very little functionality beyond
**      processing the table, a single object is used for management functions
**      and table processing.
**
** References:
**   1. OpenSatKit Object-based Application Developer's Guide
**   2. cFS Application Developer's Guide
**
*/
#ifndef _msg_trans_
#define _msg_trans_

/*
** Includes
*/

#include "app_cfg.h"
#include "mqtt_topic_tbl.h"


/***********************/
/** Macro Definitions **/
/***********************/


/*
** Events
*/

#define MSG_TRANS_PROCESS_MQTT_MSG_EID  (MSG_TRANS_BASE_EID + 0)
#define MSG_TRANS_PROCESS_SB_MSG_EID    (MSG_TRANS_BASE_EID + 1)


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

   uint32  TopicBaseMid;
   
   /*
   ** Telemetry Messages
   */
   
   JSON_MSG_Pkt_t  JsonMsgPkt;
   
   /*
   ** Contained Objects
   */

   MQTT_TOPIC_TBL_Class_t TopicTbl;
   
} MSG_TRANS_Class_t;


/************************/
/** Exported Functions **/
/************************/


/******************************************************************************
** Function: MSG_TRANS_Constructor
**
** Notes:
**    1. This function must be called prior to any other functions being
**       called using the same cmdmgr instance.
*/
void MSG_TRANS_Constructor(MSG_TRANS_Class_t *MsgTransPtr,
                           const INITBL_Class_t *IniTbl,
                           TBLMGR_Class_t *TblMgr);


/******************************************************************************
** Function: MSG_TRANS_ProcessMqttMsg
**
** Notes:
**   1. Signature must mach MQTT_CLIENT_MsgCallback
**
*/
void MSG_TRANS_ProcessMqttMsg(MessageData* MsgData);


/******************************************************************************
** Function: MSG_TRANS_ProcessSbMsg
**
** Notes:
**   None
**
*/
bool MSG_TRANS_ProcessSbMsg(const CFE_MSG_Message_t *MsgPt,
                            const char **Topic, const char **Payload);


/******************************************************************************
** Function: MSG_TRANS_ResetStatus
**
** Reset counters and status flags to a known reset state.
**
*/
void MSG_TRANS_ResetStatus(void);

#endif /* _msg_trans_ */
