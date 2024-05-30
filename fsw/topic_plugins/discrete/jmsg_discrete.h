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
**   Manage JMSG discrete topic
**
** Notes:
**   None
**
*/

#ifndef _jmsg_topic_discrete_
#define _jmsg_topic_discrete_

/*
** Includes
*/

#include "app_cfg.h"
#include "jmsg_topic_plugin.h"

/***********************/
/** Macro Definitions **/
/***********************/


/*
** Event Message IDs
*/

#define JMSG_TOPIC_DISCRETE_INIT_SB_MSG_TEST_EID  (JMSG_TOPIC_PLUGIN_1_BASE_EID + 0)
#define JMSG_TOPIC_DISCRETE_SB_MSG_TEST_EID       (JMSG_TOPIC_PLUGIN_1_BASE_EID + 1)
#define JMSG_TOPIC_DISCRETE_LOAD_JSON_DATA_EID    (JMSG_TOPIC_PLUGIN_1_BASE_EID + 2)
#define JMSG_TOPIC_DISCRETE_JSON_TO_CCSDS_ERR_EID (JMSG_TOPIC_PLUGIN_1_BASE_EID + 3)

/**********************/
/** Type Definitions **/
/**********************/


/******************************************************************************
** Telemetry
** 
** JMSG_DiscretePluginTlm_t & JMSG_DiscretePluginTlm_Payload_t are defined in EDS
*/

typedef struct
{

   /*
   ** Discrete Telemetry
   */
   
   JMSG_DiscretePluginTlm_t  TlmMsg;
   char                      JsonMsgPayload[1024];

   /*
   ** SB test treats the 4 integers as a 4-bit integer that is incremented 
   ** from 0..15 
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
   
   
} JMSG_TOPIC_DISCRETE_Class_t;


/************************/
/** Exported Functions **/
/************************/


/******************************************************************************
** Function: JMSG_TOPIC_DISCRETE_Constructor
**
** Initialize the JMSG discrete topic

** Notes:
**   None
**
*/
void JMSG_TOPIC_DISCRETE_Constructor(JMSG_TOPIC_DISCRETE_Class_t *JMsgTopicDiscretePtr,
                                     JMSG_TOPIC_TBL_PluginFuncTbl_t *PluginFuncTbl,
                                     CFE_SB_MsgId_t TlmMsgMid);


#endif /* _jmsg_topic_discrete_ */
