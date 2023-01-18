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
**   1. The JSON topic table defines each supported MQTT topic. A topic
**      is either published or subcribed to by the MQTT_GW app. A block
**      of SB message IDs is used and the ID in the topic table is an
**      offset from the base message ID define in MQTT_GW's JSON init
**      table. The number of messages in the block is defined by
**      MQTT_TOPIC_TBL_MAX_TOPICS for the C code but must be manually
**      configured when the EDS topic IDs are used.
**   2. Steps to create a mqtt_topic_xxx translator:
**      1. Create the translator object mqtt_topic_xxx
**         See mqtt_topic_rate.h/c for an example
**         Define CCSDS packet in mqtt_gw.xml EDS file
**         Include a test function to generate CCSDS packets
**      2. mqtt_topic_tbl.h:
**         - Include the translator object header
**         - Add the class/object declaration to MQTT_TOPIC_TBL_Class_t
**      3. mqtt_topic_tbl.c:
**         - Add the conversion functions to ConvertFunc[]
**         - In the constructor, add a call to the translator object's constructor 
**      4. mqtt_gw.xml: 
**         - Add topic definition
**      5. cpu1_mqtt_topic.json: 
**         - Add topic definition
**      6. Create/modify apps that generate CCSDS topic packets to
**         use the EDS definition
**      7. Ensure topic identfiiers are consistent 
**         - mqtt_topic_tbl constructor must match the EDS definition order
**
** References:
**   1. OpenSatKit Object-based Application Developer's Guide
**   2. cFS Application Developer's Guide
**
*/

#ifndef _mqtt_topic_tbl_
#define _mqtt_topic_tbl_

/*
** Includes
*/

#include "app_cfg.h"
#include "mqtt_topic_sbmsg.h"
#include "mqtt_topic_rate.h"
#include "mqtt_topic_discrete.h"


/***********************/
/** Macro Definitions **/
/***********************/

#define MQTT_TOPIC_TBL_UNUSED_ID 99

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

   char  Name[OS_MAX_PATH_LEN];
   uint8 Id;
   char  SbRole[OS_MAX_PATH_LEN];

} MQTT_TOPIC_TBL_Entry_t;

typedef struct
{

  MQTT_TOPIC_TBL_Entry_t Entry[MQTT_TOPIC_TBL_MAX_TOPICS];
   
} MQTT_TOPIC_TBL_Data_t;


/* Return pointer to owner's table data */
typedef MQTT_TOPIC_TBL_Data_t* (*MQTT_TOPIC_TBL_GetDataPtr_t)(void);

/******************************************************************************
** Topic 'virtual' function signatures
** - Using separate MQTT_TOPIC_xxx files for each topic type and this table
**   below is mimicing an abstract base class with inheritance design
** - Naming it MQTT_TOPIC_TBL_VirtualFunc_t complies with the app_c_fw naming 
**   standard, but it's a little misleading because the MQTT_TOPIC_xxx objects
**   are not designed as subclasses of MQTT_TOPIC_TBL.
*/

typedef bool (*MQTT_TOPIC_TBL_JsonToCfe_t)(CFE_MSG_Message_t **CfeMsg, const char *JsonMsgPayload, uint16 PayloadLen);
typedef bool (*MQTT_TOPIC_TBL_CfeToJson_t)(const char **JsonMsgTopic, const char **JsonMsgPayload, const CFE_MSG_Message_t *CfeMsg);
typedef void (*MQTT_TOPIC_TBL_SbMsgTest_t)(bool Init, int16 Param);

typedef struct
{

   MQTT_TOPIC_TBL_CfeToJson_t  CfeToJson;
   MQTT_TOPIC_TBL_JsonToCfe_t  JsonToCfe;  
   MQTT_TOPIC_TBL_SbMsgTest_t  SbMsgTest;

} MQTT_TOPIC_TBL_VirtualFunc_t; 


/******************************************************************************
** Class
*/

typedef struct
{

   /*
   ** Topic Table
   */
   
   MQTT_TOPIC_TBL_Data_t       Data;
   MQTT_TOPIC_SBMSG_Class_t    SbMsg;
   MQTT_TOPIC_DISCRETE_Class_t Discrete;
   MQTT_TOPIC_RATE_Class_t     Rate;
   
   /*
   ** Standard CJSON table data
   */
   
   const char*  AppName;
   bool         Loaded;   /* Has entire table been loaded? */
   uint8        LastLoadStatus;
   uint16       LastLoadCnt;
   
   size_t       JsonObjCnt;
   char         JsonBuf[MQTT_TOPIC_TBL_JSON_FILE_MAX_CHAR];   
   size_t       JsonFileLen;
   
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
void MQTT_TOPIC_TBL_Constructor(MQTT_TOPIC_TBL_Class_t *TopicMgrPtr,
                                const char *AppName, uint32 TopicBaseMid,
                                uint32 WrapSbMsgMid);


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
** Return a pointer to the CfeToJson conversion function for 'Idx'.
** 
** Notes:
**   1. Idx must be less than MQTT_TOPIC_TBL_MAX_TOPICS
**
*/
MQTT_TOPIC_TBL_CfeToJson_t MQTT_TOPIC_TBL_GetCfeToJson(uint8 Idx);


/******************************************************************************
** Function: MQTT_TOPIC_TBL_GetEntry
**
** Return a pointer to the table entry identified by 'Idx'.
** 
** Notes:
**   1. Idx must be less than MQTT_TOPIC_TBL_MAX_TOPICS
**
*/
const MQTT_TOPIC_TBL_Entry_t *MQTT_TOPIC_TBL_GetEntry(uint8 Idx);


/******************************************************************************
** Function: MQTT_TOPIC_TBL_GetJsonToCfe
**
** Return a pointer to the JsonToCfe conversion function for 'Idx'.
** 
** Notes:
**   1. Idx must be less than MQTT_TOPIC_TBL_MAX_TOPICS
**
*/
MQTT_TOPIC_TBL_JsonToCfe_t MQTT_TOPIC_TBL_GetJsonToCfe(uint8 Idx);


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
**   1. Idx must be less than MQTT_TOPIC_TBL_MAX_TOPICS
**
*/
void MQTT_TOPIC_TBL_RunSbMsgTest(uint8 Idx, bool Init, int16 Param);


/******************************************************************************
** Function: ValidId
**
** In addition to being in range, valid means that the ID has been defined.
*/
bool MQTT_TOPIC_TBL_ValidId(uint8 Idx);



#endif /* _mqtt_topic_tbl_ */
