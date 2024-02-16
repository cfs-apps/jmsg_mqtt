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
**   Manage MQTT Manager's table
**
** Notes:
**   1. Each supported MQTT topic is listed in a JSON file and each
**      topic has a JSON file that defines the topics content.
**
*/

/*
** Include Files:
*/

#include <string.h>
#include "mqtt_gw_topic_plugin.h"


/***********************/
/** Macro Definitions **/
/***********************/

#define JSON_MAX_TOPIC_LEN  MQTT_GW_MAX_MQTT_TOPIC_LEN  // Shorter & more ledgable


/********************************** **/
/** Local File Function Prototypes **/
/************************************/

static bool LoadJsonData(size_t JsonFileLen);


/**********************/
/** Global File Data **/
/**********************/

static MQTT_TOPIC_TBL_Class_t* MqttTopicTbl = NULL;

static MQTT_TOPIC_TBL_Data_t TblData; /* Working buffer for loads */

static CJSON_Obj_t JsonTblObjs[] = 
{

   /* Table                     Data                                            core-json             length of query       */
   /* Data Address,             Len,                Updated, Data Type,  Float,  query string,         string(exclude '\0')  */
   
   { &TblData.Topic[0].Mqtt,    JSON_MAX_TOPIC_LEN, false,   JSONString, false, { "topic[0].mqtt",     (sizeof("topic[0].mqtt")-1)}},
   { &TblData.Topic[0].Cfe,     2,                  false,   JSONNumber, false, { "topic[0].cfe",      (sizeof("topic[0].cfe")-1)}},
   { &TblData.Topic[0].SbStr,   JSON_MAX_KW_LEN,    false,   JSONString, false, { "topic[0].sb-role",  (sizeof("topic[0].sb-role")-1)}},
   { &TblData.Topic[0].EnaStr,  JSON_MAX_KW_LEN,    false,   JSONString, false, { "topic[0].enabled",  (sizeof("topic[0].enabled")-1)}},

   { &TblData.Topic[1].Mqtt,    JSON_MAX_TOPIC_LEN, false,   JSONString, false, { "topic[1].mqtt",     (sizeof("topic[1].mqtt")-1)}},
   { &TblData.Topic[1].Cfe,     2,                  false,   JSONNumber, false, { "topic[1].cfe",      (sizeof("topic[1].cfe")-1)}},
   { &TblData.Topic[1].SbStr,   JSON_MAX_KW_LEN,    false,   JSONString, false, { "topic[1].sb-role",  (sizeof("topic[1].sb-role")-1)}},
   { &TblData.Topic[1].EnaStr,  JSON_MAX_KW_LEN,    false,   JSONString, false, { "topic[1].enabled",  (sizeof("topic[1].enabled")-1)}},

   { &TblData.Topic[2].Mqtt,    JSON_MAX_TOPIC_LEN, false,   JSONString, false, { "topic[2].mqtt",     (sizeof("topic[2].mqtt")-1)}},
   { &TblData.Topic[2].Cfe,     2,                  false,   JSONNumber, false, { "topic[2].cfe",      (sizeof("topic[2].cfe")-1)}},
   { &TblData.Topic[2].SbStr,   JSON_MAX_KW_LEN,    false,   JSONString, false, { "topic[2].sb-role",  (sizeof("topic[2].sb-role")-1)}},
   { &TblData.Topic[2].EnaStr,  JSON_MAX_KW_LEN,    false,   JSONString, false, { "topic[2].enabled",  (sizeof("topic[2].enabled")-1)}},

   { &TblData.Topic[3].Mqtt,    JSON_MAX_TOPIC_LEN, false,   JSONString, false, { "topic[3].mqtt",     (sizeof("topic[3].mqtt")-1)}},
   { &TblData.Topic[3].Cfe,     2,                  false,   JSONNumber, false, { "topic[3].cfe",      (sizeof("topic[3].cfe")-1)}},
   { &TblData.Topic[3].SbStr,   JSON_MAX_KW_LEN,    false,   JSONString, false, { "topic[3].sb-role",  (sizeof("topic[3].sb-role")-1)}},
   { &TblData.Topic[3].EnaStr,  JSON_MAX_KW_LEN,    false,   JSONString, false, { "topic[3].enabled",  (sizeof("topic[3].enabled")-1)}},

   { &TblData.Topic[4].Mqtt,    JSON_MAX_TOPIC_LEN, false,   JSONString, false, { "topic[4].mqtt",     (sizeof("topic[4].mqtt")-1)}},
   { &TblData.Topic[4].Cfe,     2,                  false,   JSONNumber, false, { "topic[4].cfe",      (sizeof("topic[4].cfe")-1)}},
   { &TblData.Topic[4].SbStr,   JSON_MAX_KW_LEN,    false,   JSONString, false, { "topic[4].sb-role",  (sizeof("topic[4].sb-role")-1)}},
   { &TblData.Topic[4].EnaStr,  JSON_MAX_KW_LEN,    false,   JSONString, false, { "topic[4].enabled",  (sizeof("topic[4].enabled")-1)}},

   { &TblData.Topic[5].Mqtt,    JSON_MAX_TOPIC_LEN, false,   JSONString, false, { "topic[5].mqtt",     (sizeof("topic[5].mqtt")-1)}},
   { &TblData.Topic[5].Cfe,     2,                  false,   JSONNumber, false, { "topic[5].cfe",      (sizeof("topic[5].cfe")-1)}},
   { &TblData.Topic[5].SbStr,   JSON_MAX_KW_LEN,    false,   JSONString, false, { "topic[5].sb-role",  (sizeof("topic[5].sb-role")-1)}},
   { &TblData.Topic[5].EnaStr,  JSON_MAX_KW_LEN,    false,   JSONString, false, { "topic[5].enabled",  (sizeof("topic[5].enabled")-1)}},

   { &TblData.Topic[6].Mqtt,    JSON_MAX_TOPIC_LEN, false,   JSONString, false, { "topic[6].mqtt",     (sizeof("topic[6].mqtt")-1)}},
   { &TblData.Topic[6].Cfe,     2,                  false,   JSONNumber, false, { "topic[6].cfe",      (sizeof("topic[6].cfe")-1)}},
   { &TblData.Topic[6].SbStr,   JSON_MAX_KW_LEN,    false,   JSONString, false, { "topic[6].sb-role",  (sizeof("topic[6].sb-role")-1)}},
   { &TblData.Topic[6].EnaStr,  JSON_MAX_KW_LEN,    false,   JSONString, false, { "topic[6].enabled",  (sizeof("topic[6].enabled")-1)}},

   { &TblData.Topic[7].Mqtt,    JSON_MAX_TOPIC_LEN, false,   JSONString, false, { "topic[7].mqtt",     (sizeof("topic[7].mqtt")-1)}},
   { &TblData.Topic[7].Cfe,     2,                  false,   JSONNumber, false, { "topic[7].cfe",      (sizeof("topic[7].cfe")-1)}},
   { &TblData.Topic[7].SbStr,   JSON_MAX_KW_LEN,    false,   JSONString, false, { "topic[7].sb-role",  (sizeof("topic[7].sb-role")-1)}},
   { &TblData.Topic[7].EnaStr,  JSON_MAX_KW_LEN,    false,   JSONString, false, { "topic[7].enabled",  (sizeof("topic[7].enabled")-1)}}
   
};


// Table is populated by topic plugin constructors
static MQTT_TOPIC_TBL_PluginFuncTbl_t PluginFuncTbl[MQTT_GW_TopicPlugin_Enum_t_MAX];

/******************************************************************************
** Function: MQTT_TOPIC_TBL_Constructor
**
** Notes:
**    1. This must be called prior to any other functions
**
*/
void MQTT_TOPIC_TBL_Constructor(MQTT_TOPIC_TBL_Class_t *MqttTopicTblPtr,
                                uint32 DiscreteTlmTopicId)
{

   enum MQTT_GW_TopicPlugin i;
   MqttTopicTbl = MqttTopicTblPtr;

   CFE_PSP_MemSet(MqttTopicTbl, 0, sizeof(MQTT_TOPIC_TBL_Class_t));

   MqttTopicTbl->DiscreteTlmTopicId = DiscreteTlmTopicId;
   MqttTopicTbl->JsonObjCnt = (sizeof(JsonTblObjs)/sizeof(CJSON_Obj_t));
   
   for (i=0; i < MQTT_GW_TopicPlugin_Enum_t_MAX; i++)
   {
      TblData.Topic[i].Enabled = false;
      MqttTopicTbl->Data.Topic[i].Enabled = false;
      MqttTopicTbl->Data.Topic[i].SbRole = MQTT_GW_TopicSbRole_UNDEF;
   }
   
   // Plugin stubs loaded for disabled entries
   MQTT_GW_TOPIC_PLUGIN_Constructor(&MqttTopicTbl->Data, PluginFuncTbl,
                                    MqttTopicTbl->DiscreteTlmTopicId);
                                    

} /* End MQTT_TOPIC_TBL_Constructor() */


/******************************************************************************
** Function: MQTT_TOPIC_TBL_DisablePlugin
**
** Disable a topic plugin
** 
** Notes:
**   1. This function operates at the table scope so it is only concerned with
**      validating table parameters. If this returns true then the calling
**      function can take action such as managing subscriptions.
**
*/
bool MQTT_TOPIC_TBL_DisablePlugin(enum MQTT_GW_TopicPlugin TopicPlugin)
{
   
   bool RetStatus = false;
   
   if (TopicPlugin < MQTT_GW_TopicPlugin_Enum_t_MAX)
   {
      if (MqttTopicTbl->Data.Topic[TopicPlugin].Enabled)
      {
         MqttTopicTbl->Data.Topic[TopicPlugin].Enabled = false;
         RetStatus = true;
      }
      else
      {
         CFE_EVS_SendEvent(MQTT_TOPIC_TBL_DIS_PLUGIN_EID, CFE_EVS_EventType_ERROR, 
                           "Attempt to disable topic plugin ID %d(index %d) that is already disabled",
                           (TopicPlugin+1), TopicPlugin);
      }
   }
   else
   {
      CFE_EVS_SendEvent(MQTT_TOPIC_TBL_DIS_PLUGIN_EID, CFE_EVS_EventType_ERROR, 
                        "Attempt to disable a topic plugin with an invalid topic plugin ID %d(index %d)",
                        (TopicPlugin+1), TopicPlugin);             
   }
   
   return RetStatus;
   
} /* MQTT_TOPIC_TBL_DisablePlugin() */


/******************************************************************************
** Function: MQTT_TOPIC_TBL_DumpCmd
**
** Notes:
**  1. Function signature must match TBLMGR_DumpTblFuncPtr_t.
**  2. File is formatted so it can be used as a load file.
*/
bool MQTT_TOPIC_TBL_DumpCmd(osal_id_t  FileHandle)
{

   uint8  i;
   char   DumpRecord[256];


   sprintf(DumpRecord,"   \"topic\": [\n");
   OS_write(FileHandle,DumpRecord,strlen(DumpRecord));

   for (i=0; i < MQTT_GW_TopicPlugin_Enum_t_MAX; i++)
   {
      if (MqttTopicTbl->Data.Topic[i].Enabled)
      {
         if (i > 0)
         {
            sprintf(DumpRecord,",\n");
            OS_write(FileHandle,DumpRecord,strlen(DumpRecord));      
         }
         sprintf(DumpRecord,"   {\n         \"mqtt\": \"%s\",\n         \"cfe\": %d,\n         \"sb-role\": \"%s\",\n         \"enabled\": \"%s\"\n      }",
                 MqttTopicTbl->Data.Topic[i].Mqtt, MqttTopicTbl->Data.Topic[i].Cfe, MqttTopicTbl->Data.Topic[i].SbStr, MqttTopicTbl->Data.Topic[i].EnaStr);
         OS_write(FileHandle,DumpRecord,strlen(DumpRecord));
      }
   }

   sprintf(DumpRecord,"   ]\n");
   OS_write(FileHandle,DumpRecord,strlen(DumpRecord));

   return true;
   
} /* End of MQTT_TOPIC_TBL_DumpCmd() */


/******************************************************************************
** Function: MQTT_TOPIC_TBL_EnablePlugin
**
** Enable a topic plugin
** 
** Notes:
**   1. This function operates at the table scope so it is only concerned with
**      validating table parameters. If this returns true then the calling
**      function can take action such as managing subscriptions. 
**
*/
bool MQTT_TOPIC_TBL_EnablePlugin(enum MQTT_GW_TopicPlugin TopicPlugin)
{
   
   bool RetStatus = false;
   
   if (TopicPlugin < MQTT_GW_TopicPlugin_Enum_t_MAX)
   {      
      if (MqttTopicTbl->Data.Topic[TopicPlugin].Enabled)
      {
         CFE_EVS_SendEvent(MQTT_TOPIC_TBL_ENA_PLUGIN_EID, CFE_EVS_EventType_ERROR, 
                           "Attempt to enable topic plugin %d(index %d) that is already enabled",
                           (TopicPlugin+1), TopicPlugin);
      }
      else
      {
         if ((MqttTopicTbl->Data.Topic[TopicPlugin].SbRole == MQTT_GW_TopicSbRole_PUBLISH) ||
             (MqttTopicTbl->Data.Topic[TopicPlugin].SbRole == MQTT_GW_TopicSbRole_SUBSCRIBE))
         {
            MqttTopicTbl->Data.Topic[TopicPlugin].Enabled = true;
            RetStatus = true;   
         }
         else
         {
            CFE_EVS_SendEvent(MQTT_TOPIC_TBL_ENA_PLUGIN_EID, CFE_EVS_EventType_ERROR, 
                              "Attempt to enable topic plugin ID %d(index %d) with an invalid sb-role definition",
                              (TopicPlugin+1), TopicPlugin);             
         }
      }
   }
   else
   {
      CFE_EVS_SendEvent(MQTT_TOPIC_TBL_ENA_PLUGIN_EID, CFE_EVS_EventType_ERROR,
                        "Attempt to enable a topic plugin with an invalid topic plugin ID %d(index %d)",
                        (TopicPlugin+1), TopicPlugin);           
   }
   
   return RetStatus;
   
} /* MQTT_TOPIC_TBL_EnablePlugin() */


/******************************************************************************
** Function: MQTT_TOPIC_TBL_GetCfeToJson
**
** Return a pointer to the CfeToJson conversion function for 'TopicPlugin' and return a
** pointer to the JSON topic string in JsonMsgTopic.
** 
** Notes:
**   1. TopicPlugin must be less than MQTT_GW_TopicPlugin_Enum_t_MAX
**
*/
MQTT_TOPIC_TBL_CfeToJson_t MQTT_TOPIC_TBL_GetCfeToJson(enum MQTT_GW_TopicPlugin TopicPlugin, 
                                                       const char **JsonMsgTopic)
{

   MQTT_TOPIC_TBL_CfeToJson_t CfeToJsonFunc = NULL;
   
   if (MQTT_TOPIC_TBL_ValidTopicPlugin(TopicPlugin))
   {
         *JsonMsgTopic = MqttTopicTbl->Data.Topic[TopicPlugin].Mqtt;
         CfeToJsonFunc = PluginFuncTbl[TopicPlugin].CfeToJson;
   }

   return CfeToJsonFunc;
   
} /* End MQTT_TOPIC_TBL_GetCfeToJson() */


/******************************************************************************
** Function: MQTT_TOPIC_TBL_GetDisabledTopic
**
** Return a pointer to the table topic entry identified by 'TopicPlugin'.
** 
** Notes:
**   1. A special case arose for getting a disabled topic pointer. Rather than
**      add a parameter MQTT_TOPIC_TBL_GetTopic() in whcih this one case would
**      be the exception, it seemed cleaner to add another function that has
**      some duplicate functionality.
**
*/
const MQTT_TOPIC_TBL_Topic_t *MQTT_TOPIC_TBL_GetDisabledTopic(enum MQTT_GW_TopicPlugin TopicPlugin)
{

   MQTT_TOPIC_TBL_Topic_t *Topic = NULL;

   if (TopicPlugin < MQTT_GW_TopicPlugin_Enum_t_MAX)
   {
      if ((MqttTopicTbl->Data.Topic[TopicPlugin].SbRole == MQTT_GW_TopicSbRole_PUBLISH)||
          (MqttTopicTbl->Data.Topic[TopicPlugin].SbRole == MQTT_GW_TopicSbRole_SUBSCRIBE))
      {
         Topic = &MqttTopicTbl->Data.Topic[TopicPlugin];
      }
   }
   else
   {
      CFE_EVS_SendEvent(MQTT_TOPIC_TBL_INDEX_ERR_EID, CFE_EVS_EventType_ERROR, 
                        "Table topic plugin ID %d(index %d) is out of range. It must less than %d",
                        (TopicPlugin+1), TopicPlugin, MQTT_GW_TopicPlugin_Enum_t_MAX);
   }
   
   return Topic;
   
} /* End MQTT_TOPIC_TBL_GetDisabledTopic() */


/******************************************************************************
** Function: MQTT_TOPIC_TBL_GetTopic
**
** Return a pointer to the table topic entry identified by 'TopicPlugin'.
** 
** Notes:
**   1. TopicPlugin must be less than MQTT_GW_TopicPlugin_Enum_t_MAX
**
*/
const MQTT_TOPIC_TBL_Topic_t *MQTT_TOPIC_TBL_GetTopic(enum MQTT_GW_TopicPlugin TopicPlugin)
{

   MQTT_TOPIC_TBL_Topic_t *Topic = NULL;
   
   if (MQTT_TOPIC_TBL_ValidTopicPlugin(TopicPlugin))
   {
      Topic = &MqttTopicTbl->Data.Topic[TopicPlugin];
   }

   return Topic;
   
} /* End MQTT_TOPIC_TBL_GetTopic() */



/******************************************************************************
** Function: MQTT_TOPIC_TBL_GetJsonToCfe
**
** Return a pointer to the JsonToCfe conversion function for 'TopicPlugin'.
** 
** Notes:
**   1. TopicPlugin must be less than MQTT_GW_TopicPlugin_Enum_t_MAX
**
*/
MQTT_TOPIC_TBL_JsonToCfe_t MQTT_TOPIC_TBL_GetJsonToCfe(enum MQTT_GW_TopicPlugin TopicPlugin)
{

   MQTT_TOPIC_TBL_JsonToCfe_t JsonToCfeFunc = NULL;
   
   if (MQTT_TOPIC_TBL_ValidTopicPlugin(TopicPlugin))
   {
      JsonToCfeFunc = PluginFuncTbl[TopicPlugin].JsonToCfe;
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
bool MQTT_TOPIC_TBL_LoadCmd(APP_C_FW_TblLoadOptions_Enum_t LoadType, const char *Filename)
{

   bool  RetStatus = false;

   if (CJSON_ProcessFile(Filename, MqttTopicTbl->JsonBuf, MQTT_TOPIC_TBL_JSON_FILE_MAX_CHAR, LoadJsonData))
   {
      MqttTopicTbl->Loaded = true;
      RetStatus = true;
   }

   return RetStatus;
   
} /* End MQTT_TOPIC_TBL_LoadCmd() */


/******************************************************************************
** Function: MQTT_TOPIC_TBL_MsgIdToTopicPlugin
**
** Return a topic plugin ID for a message ID
** 
** Notes:
**   1. MQTT_GW_TopicPlugin_UNDEF is returned if the message ID isn't found.
**
*/
uint8 MQTT_TOPIC_TBL_MsgIdToTopicPlugin(CFE_SB_MsgId_t MsgId)
{
   
   uint8  TopicPlugin = MQTT_GW_TopicPlugin_UNDEF;
   uint32 MsgIdValue = CFE_SB_MsgIdToValue(MsgId);
  
   for (enum MQTT_GW_TopicPlugin i=0; i < MQTT_GW_TopicPlugin_Enum_t_MAX; i++)
   {
      if (MsgIdValue == MqttTopicTbl->Data.Topic[i].Cfe)
      {
         TopicPlugin = i;
         break;
      }
   }

   return TopicPlugin;
   
} /* End MQTT_TOPIC_TBL_MsgIdToTopicPlugin() */


/******************************************************************************
** Function: MQTT_TOPIC_TBL_ResetStatus
**
*/
void MQTT_TOPIC_TBL_ResetStatus(void)
{

   MqttTopicTbl->LastLoadCnt = 0;
 
} /* End MQTT_TOPIC_TBL_ResetStatus() */


/******************************************************************************
** Function: MQTT_TOPIC_TBL_RunSbMsgTest
**
** Notes:
**   1. Assumes Plugin ID enumeration value is a PluginFuncTbl index and that
**      the PluginFuncTbl array does not have any NULL pointers 
**
*/
void MQTT_TOPIC_TBL_RunSbMsgTest(enum MQTT_GW_TopicPlugin TopicPlugin, 
                                 bool Init, int16 Param)
{

   (PluginFuncTbl[TopicPlugin].SbMsgTest)(Init, Param);

} /* End MQTT_TOPIC_TBL_RunSbMsgTest() */


/******************************************************************************
** Function: MQTT_TOPIC_TBL_ValidId
**
** In addition to being in range, valid means that the ID has been defined.
*/
bool MQTT_TOPIC_TBL_ValidTopicPlugin(enum MQTT_GW_TopicPlugin TopicPlugin)
{

   bool RetStatus = false;
   
   if (TopicPlugin < MQTT_GW_TopicPlugin_Enum_t_MAX)
   {
      if (MqttTopicTbl->Data.Topic[TopicPlugin].Enabled &&
         ((MqttTopicTbl->Data.Topic[TopicPlugin].SbRole == MQTT_GW_TopicSbRole_PUBLISH)||
          (MqttTopicTbl->Data.Topic[TopicPlugin].SbRole == MQTT_GW_TopicSbRole_SUBSCRIBE)))
      {
         RetStatus = true;
      }
   }
   else
   {
      CFE_EVS_SendEvent(MQTT_TOPIC_TBL_INDEX_ERR_EID, CFE_EVS_EventType_ERROR, 
                        "Table topic plugin ID %d(index %d) is out of range. ID must be less than %d",
                        (TopicPlugin+1), TopicPlugin, (MQTT_GW_TopicPlugin_Enum_t_MAX+1));
   }

   return RetStatus;
   

} /* End MQTT_TOPIC_TBL_ValidTopicPlugin() */


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

      for (int i=0; i < MQTT_GW_TopicPlugin_Enum_t_MAX; i++)
      {
         TblData.Topic[i].Enabled = (strcmp(TblData.Topic[i].EnaStr, "true") == 0);
         if (strcmp(TblData.Topic[i].SbStr,"pub") == 0)
         {         
             TblData.Topic[i].SbRole = MQTT_GW_TopicSbRole_PUBLISH;
         }
         else if (strcmp(TblData.Topic[i].SbStr,"sub") == 0)
         {
             TblData.Topic[i].SbRole = MQTT_GW_TopicSbRole_SUBSCRIBE;
         }            
         else
         {
            TblData.Topic[i].Enabled = false; 
            TblData.Topic[i].SbRole  = MQTT_GW_TopicSbRole_INVALID;
            CFE_EVS_SendEvent(MQTT_TOPIC_TBL_LOAD_ERR_EID, CFE_EVS_EventType_ERROR, 
                              "Invalid sb-role '%s'. It must be either 'pub' or 'sub'", TblData.Topic[i].SbStr);
         }
      }
      memcpy(&MqttTopicTbl->Data,&TblData, sizeof(MQTT_TOPIC_TBL_Data_t));
      MqttTopicTbl->LastLoadCnt = ObjLoadCnt;

      MQTT_GW_TOPIC_PLUGIN_Constructor(&MqttTopicTbl->Data,PluginFuncTbl,
                                       MqttTopicTbl->DiscreteTlmTopicId);

      RetStatus = true;
      
   } /* End if valid JSON object count */
   
   return RetStatus;
   
} /* End LoadJsonData() */


