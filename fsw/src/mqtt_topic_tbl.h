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
**   Manage MQTT topic table
**
** Notes:
**   1. The JSON topic table defines each supported MQTT topic. Topics
**      are
**      - Read from MQTT broker, translated from JSOn to binary cFE
**        message and published on the SB
**      - Read from the SB, translated from binary cFE message to
**        JSON and sent to an MQTT broker
**   2. See mqtt_gw/fsw/topics/mqtt_gw_topics.c for topic plugin instructions 
**
** References:
**   1. cFS Basecamp Object-based Application Developer's Guide
**
*/

#ifndef _mqtt_topic_tbl_
#define _mqtt_topic_tbl_

/*
** Includes
*/

#include "app_cfg.h"
#include "mqtt_topic_tbl.h"

/***********************/
/** Macro Definitions **/
/***********************/


#define JSON_MAX_KW_LEN     10  // Maximum length of short keywords
#define JSON_MAX_TOPIC_LEN  MQTT_TOPIC_TBL_MAX_TOPIC_LEN

/*
** Event Message IDs
*/

#define MQTT_TOPIC_TBL_INDEX_ERR_EID  (MQTT_TOPIC_TBL_BASE_EID + 0)
#define MQTT_TOPIC_TBL_DUMP_ERR_EID   (MQTT_TOPIC_TBL_BASE_EID + 1)
#define MQTT_TOPIC_TBL_LOAD_ERR_EID   (MQTT_TOPIC_TBL_BASE_EID + 2)
#define MQTT_TOPIC_TBL_STUB_EID       (MQTT_TOPIC_TBL_BASE_EID + 3)

/**********************/
/** Type Definitions **/
/**********************/


/******************************************************************************
** Table - Local table copy used for table loads
** 
*/

typedef struct
{

   char    Mqtt[JSON_MAX_TOPIC_LEN];
   uint16  Cfe;
   char    SbRole[JSON_MAX_KW_LEN];
   char    EnaStr[JSON_MAX_KW_LEN];
   bool    Enabled;
   
} MQTT_TOPIC_TBL_Topic_t;

typedef struct
{

  MQTT_TOPIC_TBL_Topic_t Topic[MQTT_GW_PluginTopic_Enum_t_MAX];
   
} MQTT_TOPIC_TBL_Data_t;


/******************************************************************************
** Topic 'virtual' function signatures
** - Separate MQTT_TOPIC_xxx files are used for each topic plugin. The
**   MQTT_TOPIC_TBL_PluginFuncTbl_t is used to hold pointers to each conversion
**   function so you can think of a plugin type as an abstract base class and
**   each MQTT_TOPIC_xxx as concrete classes that provide the plugin conversion
**   methods.
*/

typedef bool (*MQTT_TOPIC_TBL_JsonToCfe_t)(CFE_MSG_Message_t **CfeMsg, const char *JsonMsgPayload, uint16 PayloadLen);
typedef bool (*MQTT_TOPIC_TBL_CfeToJson_t)(const char **JsonMsgPayload, const CFE_MSG_Message_t *CfeMsg);
typedef void (*MQTT_TOPIC_TBL_SbMsgTest_t)(bool Init, int16 Param);

typedef struct
{

   MQTT_TOPIC_TBL_CfeToJson_t  CfeToJson;
   MQTT_TOPIC_TBL_JsonToCfe_t  JsonToCfe;  
   MQTT_TOPIC_TBL_SbMsgTest_t  SbMsgTest;

} MQTT_TOPIC_TBL_PluginFuncTbl_t; 


/******************************************************************************
** Class
*/

typedef struct
{

   /*
   ** Topic Table
   */
   
   MQTT_TOPIC_TBL_Data_t  Data;
   
   /*
   ** Standard CJSON table data
   */
   
   const char  *AppName;
   uint32      DiscreteTlmTopicId;
   uint32      TunnelTlmTopicId;
   bool        Loaded;   /* Has entire table been loaded? */
   uint8       LastLoadStatus;
   uint16      LastLoadCnt;
   
   size_t      JsonObjCnt;
   char        JsonBuf[MQTT_TOPIC_TBL_JSON_FILE_MAX_CHAR];   
   size_t      JsonFileLen;
   
} MQTT_TOPIC_TBL_Class_t;


/************************/
/** Exported Functions **/
/************************/


/******************************************************************************
** Function: MQTT_TOPIC_TBL_Constructor
**
** Initialize the MQTT topic table.
**
** Notes:
**   1. The table values are not populated. This is done when the table is 
**      registered with the table manager.
**
*/
void MQTT_TOPIC_TBL_Constructor(MQTT_TOPIC_TBL_Class_t *MqttTopicTblPtr, const char *AppName,
                                uint32 DiscreteTlmTopicId, uint32 TunnelTlmTopicId);


/******************************************************************************
** Function: MQTT_TOPIC_TBL_DumpCmd
**
** Command to write the table data from memory to a JSON file.
**
** Notes:
**  1. Function signature must match TBLMGR_DumpTblFuncPtr_t.
**  2. Can assume valid table file name because this is a callback from 
**     the app framework table manager.
**
*/
bool MQTT_TOPIC_TBL_DumpCmd(TBLMGR_Tbl_t *Tbl, uint8 DumpType, const char *Filename);


/******************************************************************************
** Function: MQTT_TOPIC_TBL_GetCfeToJson
**
** Return a pointer to the CfeToJson conversion function for 'Index' and return
** a pointer to the JSON topic string in JsonMsgTopic.
** 
** Notes:
**   1. Index must be less than MQTT_GW_PluginTopic_Enum_t_MAX
**
*/
MQTT_TOPIC_TBL_CfeToJson_t MQTT_TOPIC_TBL_GetCfeToJson(uint8 Index,
                                                       const char **JsonMsgTopic);


/******************************************************************************
** Function: MQTT_TOPIC_TBL_GetTopic
**
** Return a pointer to the table entry identified by 'Index'.
** 
** Notes:
**   1. Index must be less than MQTT_GW_PluginTopic_Enum_t_MAX
**
*/
const MQTT_TOPIC_TBL_Topic_t *MQTT_TOPIC_TBL_GetTopic(uint8 Index);


/******************************************************************************
** Function: MQTT_TOPIC_TBL_GetJsonToCfe
**
** Return a pointer to the JsonToCfe conversion function for 'Index'.
** 
** Notes:
**   1. Index must be less than MQTT_GW_PluginTopic_Enum_t_MAX
**
*/
MQTT_TOPIC_TBL_JsonToCfe_t MQTT_TOPIC_TBL_GetJsonToCfe(uint8 Index);


/******************************************************************************
** Function: MQTT_TOPIC_TBL_LoadCmd
**
** Command to copy the table data from a JSON file to memory.
**
** Notes:
**  1. Function signature must match TBLMGR_LoadTblFuncPtr_t.
**  2. Can assume valid table file name because this is a callback from 
**     the app framework table manager.
**
*/
bool MQTT_TOPIC_TBL_LoadCmd(TBLMGR_Tbl_t *Tbl, uint8 LoadType, const char *Filename);


/******************************************************************************
** Function: MQTT_TOPIC_TBL_MsgIdToIndex
**
** Return an topic table index for a message ID
** 
** Notes:
**   1. MQTT_GW_PluginTopic_UNDEF is returned if the message ID isn't found.
**
*/
uint8 MQTT_TOPIC_TBL_MsgIdToIndex(CFE_SB_MsgId_t MsgId);


/******************************************************************************
** Function: MQTT_TOPIC_TBL_ResetStatus
**
** Reset counters and status flags to a known reset state.  The behavior of
** the table manager should not be impacted. The intent is to clear counters
** and flags to a known default state for telemetry.
**
*/
void MQTT_TOPIC_TBL_ResetStatus(void);


/******************************************************************************
** Function: MQTT_TOPIC_TBL_RunSbMsgTest
**
** Execute a topic's SB message test.
** 
** Notes:
**   1. Index must be less than MQTT_GW_PluginTopic_Enum_t_MAX
**
*/
void MQTT_TOPIC_TBL_RunSbMsgTest(uint8 Index, bool Init, int16 Param);


/******************************************************************************
** Function: ValidId
**
** In addition to being in range, valid means that the ID has been defined.
*/
bool MQTT_TOPIC_TBL_ValidId(uint8 Index);



#endif /* _mqtt_topic_tbl_ */
