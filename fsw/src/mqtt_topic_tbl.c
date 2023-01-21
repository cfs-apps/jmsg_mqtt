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
**   Manage MQTT Manager''s table
**
** Notes:
**   1. Each supported MQTT topic is listed in a JSON file and each
**      topic has a JSON file that defines the topics content.
**
** References:
**   1. OpenSatKit Object-based Application Developer's Guide
**   2. cFS Application Developer's Guide
**
*/

/*
** Include Files:
*/

#include <string.h>
#include "mqtt_topic_tbl.h"

/************************************/
/** Local File Function Prototypes **/
/************************************/

static bool LoadJsonData(size_t JsonFileLen);
static bool StubCfeToJson(const char **JsonMsgPayload, const CFE_MSG_Message_t *CfeMsg);
static bool StubJsonToCfe(CFE_MSG_Message_t **CfeMsg, const char *JsonMsgPayload, uint16 PayloadLen);
static void StubSbMsgTest(bool Init, int16 Param);


/**********************/
/** Global File Data **/
/**********************/

static MQTT_TOPIC_TBL_Class_t* MqttTopicTbl = NULL;

static MQTT_TOPIC_TBL_Data_t TblData; /* Working buffer for loads */

static CJSON_Obj_t JsonTblObjs[] = 
{

   /* Table Data Address         Table Data Length  Updated, Data Type,  Float  core-json query string, length of query string(exclude '\0') */
   
   { &TblData.Entry[0].Name,     OS_MAX_PATH_LEN,   false,   JSONString, false, { "topic[0].name",       (sizeof("topic[0].name")-1)}    },
   { &TblData.Entry[0].Payload,  2,                 false,   JSONNumber, false, { "topic[0].payload",    (sizeof("topic[0].payload")-1)} },
   { &TblData.Entry[0].SbRole,   OS_MAX_PATH_LEN,   false,   JSONString, false, { "topic[0].sb-role",    (sizeof("topic[0].sb-role")-1)} },
   { &TblData.Entry[1].Name,     OS_MAX_PATH_LEN,   false,   JSONString, false, { "topic[1].name",       (sizeof("topic[1].name")-1)}    },
   { &TblData.Entry[1].Payload,  2,                 false,   JSONNumber, false, { "topic[1].payload",    (sizeof("topic[1].payload")-1)} },
   { &TblData.Entry[1].SbRole,   OS_MAX_PATH_LEN,   false,   JSONString, false, { "topic[1].sb-role",    (sizeof("topic[1].sb-role")-1)} },
   { &TblData.Entry[2].Name,     OS_MAX_PATH_LEN,   false,   JSONString, false, { "topic[2].name",       (sizeof("topic[2].name")-1)}    },
   { &TblData.Entry[2].Payload,  2,                 false,   JSONNumber, false, { "topic[2].payload",    (sizeof("topic[2].payload")-1)} },
   { &TblData.Entry[2].SbRole,   OS_MAX_PATH_LEN,   false,   JSONString, false, { "topic[2].sb-role",    (sizeof("topic[2].sb-role")-1)} },
   { &TblData.Entry[3].Name,     OS_MAX_PATH_LEN,   false,   JSONString, false, { "topic[3].name",       (sizeof("topic[3].name")-1)}    },
   { &TblData.Entry[3].Payload,  2,                 false,   JSONNumber, false, { "topic[3].payload",    (sizeof("topic[3].payload")-1)} },
   { &TblData.Entry[3].SbRole,   OS_MAX_PATH_LEN,   false,   JSONString, false, { "topic[3].sb-role",    (sizeof("topic[3].sb-role")-1)} },
   { &TblData.Entry[4].Name,     OS_MAX_PATH_LEN,   false,   JSONString, false, { "topic[4].name",       (sizeof("topic[4].name")-1)}    },
   { &TblData.Entry[4].Payload,  2,                 false,   JSONNumber, false, { "topic[4].payload",    (sizeof("topic[4].payload")-1)} },
   { &TblData.Entry[4].SbRole,   OS_MAX_PATH_LEN,   false,   JSONString, false, { "topic[4].sb-role",    (sizeof("topic[4].sb-role")-1)} }
   
};


static MQTT_TOPIC_TBL_PayloadFunc_t PayloadFunc[] =
{   
   { MQTT_TOPIC_SBMSG_CfeToMqtt,    MQTT_TOPIC_SBMSG_MqttToCfe,    MQTT_TOPIC_SBMSG_SbMsgTest   }, // MQTT_TOPIC_TBL_PAYLOAD_SB_MSG
   { MQTT_TOPIC_INTEGER_CfeToJson,  MQTT_TOPIC_INTEGER_JsonToCfe,  MQTT_TOPIC_INTEGER_SbMsgTest }, // MQTT_TOPIC_TBL_PAYLOAD_INTEGER
   { MQTT_TOPIC_COORD_CfeToJson,    MQTT_TOPIC_COORD_JsonToCfe,    MQTT_TOPIC_COORD_SbMsgTest   }, // MQTT_TOPIC_TBL_PAYLOAD_COORD
   { StubCfeToJson,                 StubJsonToCfe,                 StubSbMsgTest                }, // MQTT_TOPIC_TBL_PAYLOAD_NULL
   { StubCfeToJson,                 StubJsonToCfe,                 StubSbMsgTest                }  // MQTT_TOPIC_TBL_PAYLOAD_NULL
   
};

/******************************************************************************
** Function: MQTT_TOPIC_TBL_Constructor
**
** Notes:
**    1. This must be called prior to any other functions
**
*/
void MQTT_TOPIC_TBL_Constructor(MQTT_TOPIC_TBL_Class_t *MqttTopicTblPtr, 
                               const char *AppName, uint32 TopicBaseMid,
                               uint32 WrapSbMsgMid)
{

   uint8 i;
   MqttTopicTbl = MqttTopicTblPtr;

   CFE_PSP_MemSet(MqttTopicTbl, 0, sizeof(MQTT_TOPIC_TBL_Class_t));

   MqttTopicTbl->AppName = AppName;
   MqttTopicTbl->JsonObjCnt = (sizeof(JsonTblObjs)/sizeof(CJSON_Obj_t));
   
   for (i=0; i < MQTT_TOPIC_TBL_MAX_TOPICS; i++)
   {
      MqttTopicTbl->Data.Entry[i].Payload = MQTT_TOPIC_TBL_PAYLOAD_NULL;
   }
   
   MQTT_TOPIC_SBMSG_Constructor(&MqttTopicTbl->SbMsg, 
                                CFE_SB_ValueToMsgId(TopicBaseMid),  
                                CFE_SB_ValueToMsgId(TopicBaseMid+1), // Discrete MID 
                                CFE_SB_ValueToMsgId(WrapSbMsgMid));
                                
   MQTT_TOPIC_INTEGER_Constructor(&MqttTopicTbl->Integer, 
                                  CFE_SB_ValueToMsgId(TopicBaseMid+1));
                                   
   MQTT_TOPIC_COORD_Constructor(&MqttTopicTbl->Coord, 
                                CFE_SB_ValueToMsgId(TopicBaseMid+2));
   
} /* End MQTT_TOPIC_TBL_Constructor() */


/******************************************************************************
** Function: MQTT_TOPIC_TBL_DumpCmd
**
** Notes:
**  1. Function signature must match TBLMGR_DumpTblFuncPtr_t.
**  2. Can assume valid table filename because this is a callback from 
**     the app framework table manager that has verified the file.
**  3. DumpType is unused.
**  4. File is formatted so it can be used as a load file. It does not follow
**     the cFE table file format. 
**  5. Creates a new dump file, overwriting anything that may have existed
**     previously
*/
bool MQTT_TOPIC_TBL_DumpCmd(TBLMGR_Tbl_t* Tbl, uint8 DumpType, const char* Filename)
{

   uint8      i;
   bool       RetStatus = false;
   int32      SysStatus;
   osal_id_t  FileHandle;
   os_err_name_t OsErrStr;
   char DumpRecord[256];
   char SysTimeStr[128];

   
   SysStatus = OS_OpenCreate(&FileHandle, Filename, OS_FILE_FLAG_CREATE, OS_READ_WRITE);

   if (SysStatus == OS_SUCCESS)
   {
 
      sprintf(DumpRecord,"{\n   \"app-name\": \"%s\",\n   \"tbl-name\": \"Message Log\",\n",MqttTopicTbl->AppName);
      OS_write(FileHandle,DumpRecord,strlen(DumpRecord));

      CFE_TIME_Print(SysTimeStr, CFE_TIME_GetTime());
      sprintf(DumpRecord,"   \"description\": \"Table dumped at %s\",\n",SysTimeStr);
      OS_write(FileHandle,DumpRecord,strlen(DumpRecord));

      sprintf(DumpRecord,"   \"topics\": [\n");
      OS_write(FileHandle,DumpRecord,strlen(DumpRecord));

      for (i=0; i < MQTT_TOPIC_TBL_MAX_TOPICS; i++)
      {
         if (MqttTopicTbl->Data.Entry[i].Payload != MQTT_TOPIC_TBL_PAYLOAD_NULL)
         {
            if (i > 0)
            {
               sprintf(DumpRecord,",\n");
               OS_write(FileHandle,DumpRecord,strlen(DumpRecord));      
            }
            sprintf(DumpRecord,"   {\n         \"name\": \"%s\",\n         \"payload\": %d\n      }",
                    MqttTopicTbl->Data.Entry[i].Name, MqttTopicTbl->Data.Entry[i].Payload);
            OS_write(FileHandle,DumpRecord,strlen(DumpRecord));
         }
      }

      sprintf(DumpRecord,"   ]\n}\n");
      OS_write(FileHandle,DumpRecord,strlen(DumpRecord));

      OS_close(FileHandle);

      RetStatus = true;

   } /* End if file create */
   else
   {
      OS_GetErrorName(SysStatus, &OsErrStr);
      CFE_EVS_SendEvent(MQTT_TOPIC_TBL_DUMP_ERR_EID, CFE_EVS_EventType_ERROR,
                        "Error creating dump file '%s', status=%s",
                        Filename, OsErrStr);
   
   } /* End if file create error */

   return RetStatus;
   
} /* End of MQTT_TOPIC_TBL_DumpCmd() */


/******************************************************************************
** Function: MQTT_TOPIC_TBL_GetCfeToJson
**
** Return a pointer to the CfeToJson conversion function for 'Idx' and return a
** pointer to the JSON topic string in JsonMsgTopic.
** 
** Notes:
**   1. Idx must be less than MQTT_TOPIC_TBL_MAX_TOPICS
**
*/
MQTT_TOPIC_TBL_CfeToJson_t MQTT_TOPIC_TBL_GetCfeToJson(uint8 Idx,
                                                      const char **JsonMsgTopic)
{
   uint16 PayloadId;
   MQTT_TOPIC_TBL_CfeToJson_t CfeToJsonFunc = NULL;
   
   if (MQTT_TOPIC_TBL_ValidId(Idx))
   {
         *JsonMsgTopic = TblData.Entry[Idx].Name;
         PayloadId     = TblData.Entry[Idx].Payload;
         CfeToJsonFunc = PayloadFunc[PayloadId].CfeToJson;
   }

   return CfeToJsonFunc;
   
} /* End MQTT_TOPIC_TBL_GetCfeToJson() */


/******************************************************************************
** Function: MQTT_TOPIC_TBL_GetEntry
**
** Return a pointer to the table entry identified by 'Idx'.
** 
** Notes:
**   1. Idx must be less than MQTT_TOPIC_TBL_MAX_TOPICS
**
*/
const MQTT_TOPIC_TBL_Entry_t *MQTT_TOPIC_TBL_GetEntry(uint8 Idx)
{

   MQTT_TOPIC_TBL_Entry_t *Entry = NULL;
   
   if (MQTT_TOPIC_TBL_ValidId(Idx))
   {
      Entry = &MqttTopicTbl->Data.Entry[Idx];
   }

   return Entry;
   
} /* End MQTT_TOPIC_TBL_GetEntry() */


/******************************************************************************
** Function: MQTT_TOPIC_TBL_GetJsonToCfe
**
** Return a pointer to the JsonToCfe conversion function for 'Idx'.
** 
** Notes:
**   1. Idx must be less than MQTT_TOPIC_TBL_MAX_TOPICS
**
*/
MQTT_TOPIC_TBL_JsonToCfe_t MQTT_TOPIC_TBL_GetJsonToCfe(uint8 Idx)
{

   uint16 PayloadId;
   MQTT_TOPIC_TBL_JsonToCfe_t JsonToCfeFunc = NULL;
   
   if (MQTT_TOPIC_TBL_ValidId(Idx))
   {
      PayloadId     = TblData.Entry[Idx].Payload;
      JsonToCfeFunc = PayloadFunc[PayloadId].JsonToCfe;
   }

   return JsonToCfeFunc;
   
} /* End MQTT_TOPIC_TBL_GetJsonToCfe() */


/******************************************************************************
** Function: MQTT_TOPIC_TBL_LoadCmd
**
** Notes:
**  1. Function signature must match TBLMGR_LoadTblFuncPtr_t.
**  2. This could migrate into table manager but I think I'll keep it here so
**     user's can add table processing code if needed.
*/
bool MQTT_TOPIC_TBL_LoadCmd(TBLMGR_Tbl_t* Tbl, uint8 LoadType, const char* Filename)
{

   bool  RetStatus = false;

   if (CJSON_ProcessFile(Filename, MqttTopicTbl->JsonBuf, MQTT_TOPIC_TBL_JSON_FILE_MAX_CHAR, LoadJsonData))
   {
      
      MqttTopicTbl->Loaded = true;
      MqttTopicTbl->LastLoadStatus = TBLMGR_STATUS_VALID;
      RetStatus = true;
   
   }
   else
   {

      MqttTopicTbl->LastLoadStatus = TBLMGR_STATUS_INVALID;

   }

   return RetStatus;
   
} /* End MQTT_TOPIC_TBL_LoadCmd() */


/******************************************************************************
** Function: MQTT_TOPIC_TBL_ResetStatus
**
*/
void MQTT_TOPIC_TBL_ResetStatus(void)
{

   MqttTopicTbl->LastLoadStatus = TBLMGR_STATUS_UNDEF;
   MqttTopicTbl->LastLoadCnt = 0;
 
} /* End MQTT_TOPIC_TBL_ResetStatus() */


/******************************************************************************
** Function: MQTT_TOPIC_TBL_RunSbMsgTest
**
** Notes:
**   1. Assumes Idx has been verified and that the PayloadFunc array does not
**      have any NULL pointers 
**
*/
void MQTT_TOPIC_TBL_RunSbMsgTest(uint8 Idx, bool Init, int16 Param)
{

   (PayloadFunc[Idx].SbMsgTest)(Init, Param);

} /* End MQTT_TOPIC_TBL_RunSbMsgTest() */


/******************************************************************************
** Function: MQTT_TOPIC_TBL_ValidId
**
** In addition to being in range, valid means that the ID has been defined.
*/
bool MQTT_TOPIC_TBL_ValidId(uint8 Idx)
{

   bool RetStatus = false;
   
   if (Idx < MQTT_TOPIC_TBL_MAX_TOPICS)
   {
      if (MqttTopicTbl->Data.Entry[Idx].Payload != MQTT_TOPIC_TBL_PAYLOAD_NULL)
      {
         RetStatus = true;
      }
   }
   else
   {
      CFE_EVS_SendEvent(MQTT_TOPIC_TBL_INDEX_ERR_EID, CFE_EVS_EventType_ERROR, 
                        "Table index %d is out of range. It must less than %d",
                        Idx, MQTT_TOPIC_TBL_MAX_TOPICS);
   }

   return RetStatus;
   

} /* End MQTT_TOPIC_TBL_ValidId() */


/******************************************************************************
** Function: LoadJsonData
**
** Notes:
**   None
*/
static bool LoadJsonData(size_t JsonFileLen)
{

   bool      RetStatus = false;
   size_t    ObjLoadCnt;


   MqttTopicTbl->JsonFileLen = JsonFileLen;

   /* 
   ** 1. Copy table owner data into local table buffer
   ** 2. Process JSON file which updates local table buffer with JSON supplied values
   ** 3. If valid, copy local buffer over owner's data 
   */
   
   memcpy(&TblData, &MqttTopicTbl->Data, sizeof(MQTT_TOPIC_TBL_Data_t));
   
   ObjLoadCnt = CJSON_LoadObjArray(JsonTblObjs, MqttTopicTbl->JsonObjCnt, MqttTopicTbl->JsonBuf, MqttTopicTbl->JsonFileLen);

   if (!MqttTopicTbl->Loaded && (ObjLoadCnt != MqttTopicTbl->JsonObjCnt))
   {

      CFE_EVS_SendEvent(MQTT_TOPIC_TBL_LOAD_ERR_EID, CFE_EVS_EventType_ERROR, 
                        "Table has never been loaded and new table only contains %d of %d data objects",
                        (unsigned int)ObjLoadCnt, (unsigned int)MqttTopicTbl->JsonObjCnt);
   
   }
   else
   {
   
      memcpy(&MqttTopicTbl->Data,&TblData, sizeof(MQTT_TOPIC_TBL_Data_t));
      MqttTopicTbl->LastLoadCnt = ObjLoadCnt;
      RetStatus = true;
      
   }
   
   return RetStatus;
   
} /* End LoadJsonData() */


/******************************************************************************
** Function: StubCfeToJson
**
** Provide a CfeToJson stub function to be used as a non-NULL pointer in the
** PayloadfFunc default values.
**
*/
static bool StubCfeToJson(const char **JsonMsgPayload, 
                          const CFE_MSG_Message_t *CfeMsg)
{

   CFE_EVS_SendEvent(MQTT_TOPIC_TBL_STUB_EID, CFE_EVS_EventType_INFORMATION, 
                     "CfeToJson stub");

   return false;
   
} /* End StubCfeToJson() */


/******************************************************************************
** Function: StubJsonToCfe
**
** Provide a CfeToJson stub function to be used as a non-NULL pointer in the
** PayloadFunc default values.
**
*/
static bool StubJsonToCfe(CFE_MSG_Message_t **CfeMsg, 
                          const char *JsonMsgPayload, uint16 PayloadLen)
{
   
   CFE_EVS_SendEvent(MQTT_TOPIC_TBL_STUB_EID, CFE_EVS_EventType_INFORMATION, 
                     "JsonToCfe stub");

   return false;
   
} /* End StubJsonToCfe() */


/******************************************************************************
** Function: StubSbMsgTest
**
** Provide a CfeToJson stub function to be used as a non-NULL pointer in the
** PayloadFunc default values.
**
*/
static void StubSbMsgTest(bool Init, int16 Param)
{

   CFE_EVS_SendEvent(MQTT_TOPIC_TBL_STUB_EID, CFE_EVS_EventType_INFORMATION, 
                     "SbMsgTest stub");
   
} /* End StubSbMsgTest() */


