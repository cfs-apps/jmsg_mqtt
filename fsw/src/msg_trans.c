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
**   Translate between MQTT and Software Bus messages
**
** Notes:
**   1. Each supported MQTT topic is listed in a JSON file and each
**      topic has a JSON file that defines the topic's content.
**   2. The Basecamp JSON table coding ideom is use a separate object to manage 
**      the table. Since MQTT manager has very little functionality beyond
**      processing the table, a single object is used for management functions
**      and table processing.
**
*/

/*
** Include Files:
*/

#include <string.h>

#include "msg_trans.h"

/**********************/
/** Global File Data **/
/**********************/

static MSG_TRANS_Class_t *MsgTrans = NULL;


/******************************************************************************
** Function: MSG_TRANS_Constructor
**
*/
void MSG_TRANS_Constructor(MSG_TRANS_Class_t *MsgTransPtr,
                           const INITBL_Class_t  *IniTbl,
                           TBLMGR_Class_t *TblMgr)
{
 
   MsgTrans = MsgTransPtr;

   CFE_PSP_MemSet((void*)MsgTransPtr, 0, sizeof(MSG_TRANS_Class_t));
   
   MQTT_TOPIC_TBL_Constructor(&MsgTrans->TopicTbl,
                              INITBL_GetIntConfig(IniTbl, CFG_MQTT_GW_DISCRETE_PLUGIN_TOPICID));
                              
   TBLMGR_RegisterTblWithDef(TblMgr, MQTT_TOPIC_TBL_NAME, 
                             MQTT_TOPIC_TBL_LoadCmd, MQTT_TOPIC_TBL_DumpCmd,  
                             INITBL_GetStrConfig(IniTbl, CFG_MQTT_TOPIC_TBL_DEF_FILE));                           


} /* End MSG_TRANS_Constructor() */


/******************************************************************************
** Function: MSG_TRANS_ProcessMqttMsg
**
** Notes:
**   1. Signature must match MQTT_CLIENT_MsgCallback_t
**   2. MQTT has no delimeter between the topic and payload
**
*/
void MSG_TRANS_ProcessMqttMsg(MessageData *MsgData)
{
   
   MQTTMessage* MsgPtr = MsgData->message;

   enum MQTT_GW_TopicPlugin topic = MQTT_GW_TopicPlugin_Enum_t_MIN;
   uint16  TopicLen;
   char    TopicStr[MQTT_GW_MAX_MQTT_TOPIC_LEN];
   bool    MsgFound = false;
   const MQTT_TOPIC_TBL_Topic_t *Topic;
   MQTT_TOPIC_TBL_JsonToCfe_t JsonToCfe;
   CFE_MSG_Message_t *CfeMsg;
   CFE_SB_MsgId_t    MsgId = CFE_SB_INVALID_MSG_ID;
   CFE_MSG_Size_t    MsgSize;
   CFE_MSG_Type_t    MsgType;
   
   CFE_EVS_SendEvent(MSG_TRANS_PROCESS_MQTT_MSG_EID, CFE_EVS_EventType_DEBUG,
                     "MSG_TRANS_ProcessMsg: Received topic %s", MsgData->topicName->lenstring.data);
                    
   if(MsgPtr->payloadlen)
   {
      
      CFE_EVS_SendEvent(MSG_TRANS_PROCESS_MQTT_MSG_EID, CFE_EVS_EventType_DEBUG,
                        "MsgPtr->payloadlen=%d", (int)MsgPtr->payloadlen);

      while (!MsgFound && topic < MQTT_GW_TopicPlugin_Enum_t_MAX)
      {

         Topic = MQTT_TOPIC_TBL_GetTopic(topic);
         if (Topic != NULL)
         {
            
            TopicLen = strlen(Topic->Mqtt);
            
            if (TopicLen < MQTT_GW_MAX_MQTT_TOPIC_LEN)
            {
               CFE_EVS_SendEvent(MSG_TRANS_PROCESS_MQTT_MSG_EID, CFE_EVS_EventType_DEBUG,
                                 "Table topic name=%s, length=%d", Topic->Mqtt, TopicLen);
               if (strncmp(Topic->Mqtt, MsgData->topicName->lenstring.data, TopicLen) == 0)
               {
                  MsgFound = true;
                  memcpy(TopicStr, MsgData->topicName->lenstring.data, TopicLen);
                  TopicStr[TopicLen] = '\0';
                  CFE_EVS_SendEvent(MSG_TRANS_PROCESS_MQTT_MSG_INFO_EID, CFE_EVS_EventType_INFORMATION,
                                   "MSG_TRANS_ProcessMqttMsg: Topic=%s, Payload=%s", 
                                    TopicStr, &MsgData->topicName->lenstring.data[TopicLen]);
               }
            } /* End if valid length */
            else
            {
               CFE_EVS_SendEvent(MSG_TRANS_PROCESS_MQTT_MSG_EID, CFE_EVS_EventType_ERROR,
                                 "Table topic name %s with length %d exceeds maximum length %d", 
                                 Topic->Mqtt, TopicLen, MQTT_GW_MAX_MQTT_TOPIC_LEN);               
            }
         }
         if (!MsgFound)
         {
            topic++;
         }
      } /* End while loop */

      if (MsgFound)
      {
            
         CFE_EVS_SendEvent(MSG_TRANS_PROCESS_MQTT_MSG_EID, CFE_EVS_EventType_DEBUG,
                           "MSG_TRANS_ProcessMqttMsg: Found message at index %d", topic); 
                              
         JsonToCfe = MQTT_TOPIC_TBL_GetJsonToCfe(topic);    
       
         if (JsonToCfe(&CfeMsg, &MsgData->topicName->lenstring.data[TopicLen], MsgPtr->payloadlen))
         {         
      
            CFE_MSG_GetMsgId(CfeMsg, &MsgId);
            CFE_MSG_GetSize(CfeMsg, &MsgSize);
            
            CFE_MSG_GetType(CfeMsg,&MsgType);
            CFE_MSG_GetTypeFromMsgId(MsgId, &MsgType);
            if (MsgType == CFE_MSG_Type_Cmd)
            {
               CFE_MSG_GenerateChecksum(CFE_MSG_PTR(*CfeMsg));
            }
            else
            {
               CFE_SB_TimeStampMsg(CFE_MSG_PTR(*CfeMsg));
            }
            
            CFE_EVS_SendEvent(MSG_TRANS_PROCESS_MQTT_MSG_INFO_EID, CFE_EVS_EventType_INFORMATION,
                              "MSG_TRANS_ProcessMqttMsg: Sending SB message 0x%04X(%d), len %d, type %d", 
                              CFE_SB_MsgIdToValue(MsgId), CFE_SB_MsgIdToValue(MsgId), (int)MsgSize, (int)MsgType); 
            CFE_SB_TransmitMsg(CFE_MSG_PTR(*CfeMsg), true);               
            MsgTrans->ValidMqttMsgCnt++;
            
         }
         else
         {
            CFE_EVS_SendEvent(MSG_TRANS_PROCESS_MQTT_MSG_EID, CFE_EVS_EventType_ERROR,
                              "MSG_TRANS_ProcessMqttMsg: Error creating SB message from JSON topic %s, Id %d",
                               TopicStr, topic); 
         }
         
      } /* End if message found */
      else 
      {      
         CFE_EVS_SendEvent(MSG_TRANS_PROCESS_MQTT_MSG_EID, CFE_EVS_EventType_ERROR, 
                           "MSG_TRANS_ProcessMqttMsg: Could not find a topic match for %s", 
                           MsgData->topicName->lenstring.data);      
      }
   
   } /* End null message len */
   else {
      
      CFE_EVS_SendEvent(MSG_TRANS_PROCESS_MQTT_MSG_EID, CFE_EVS_EventType_ERROR,
                        "Null MQTT message data length for %s", MsgData->topicName->lenstring.data);
   }
   
   fflush(stdout);

} /* End MSG_TRANS_ProcessMqttMsg() */


/******************************************************************************
** Function: MSG_TRANS_ProcessSbMsg
**
** Notes:
**   None
**
*/
bool MSG_TRANS_ProcessSbMsg(const CFE_MSG_Message_t *MsgPtr,
                            const char **Topic, const char **Payload)
{
   
   bool RetStatus = false;
   int32 TopicIndex;
   int32 SbStatus;
   CFE_SB_MsgId_t  MsgId = CFE_SB_INVALID_MSG_ID;
   MQTT_TOPIC_TBL_CfeToJson_t CfeToJson;
   const char *JsonMsgTopic;
   const char *JsonMsgPayload;

   *Topic   = NULL; 
   *Payload = NULL;
   SbStatus = CFE_MSG_GetMsgId(MsgPtr, &MsgId);
   if (SbStatus == CFE_SUCCESS)
   {
   
      CFE_EVS_SendEvent(MSG_TRANS_PROCESS_SB_MSG_EID, CFE_EVS_EventType_DEBUG, 
                        "MSG_TRANS_ProcessSbMsg: Received SB message ID 0x%04X(%d)", 
                        CFE_SB_MsgIdToValue(MsgId), CFE_SB_MsgIdToValue(MsgId)); 
      
      if ((TopicIndex = MQTT_TOPIC_TBL_MsgIdToTopicPlugin(MsgId)) != MQTT_GW_TopicPlugin_UNDEF)
      {
         
         CfeToJson = MQTT_TOPIC_TBL_GetCfeToJson(TopicIndex, &JsonMsgTopic);    
         
         if (CfeToJson(&JsonMsgPayload, MsgPtr))
         {
            *Topic   = JsonMsgTopic; 
            *Payload = JsonMsgPayload;
            RetStatus = true;
            CFE_EVS_SendEvent(MSG_TRANS_PROCESS_SB_MSG_INFO_EID, CFE_EVS_EventType_INFORMATION,
                              "MSG_TRANS_ProcessMqttMsg: Created MQTT topic %s message %s",
                              JsonMsgTopic, JsonMsgPayload);             
            MsgTrans->ValidSbMsgCnt++;

         }
         else
         {
            CFE_EVS_SendEvent(MSG_TRANS_PROCESS_SB_MSG_EID, CFE_EVS_EventType_ERROR,
                              "MSG_TRANS_ProcessMqttMsg: Error creating JSON message from SB for topic index %d", TopicIndex); 
         
         }        
      }
      else
      {
         CFE_EVS_SendEvent(MSG_TRANS_PROCESS_SB_MSG_EID, CFE_EVS_EventType_ERROR, 
                           "MSG_TRANS_ProcessMsg: Unable to locate SB message 0x%04X(%d) in MQTT topic table", 
                           CFE_SB_MsgIdToValue(MsgId), CFE_SB_MsgIdToValue(MsgId));
      }

   } /* End message Id */
   else
   {
      CFE_EVS_SendEvent(MSG_TRANS_PROCESS_SB_MSG_EID, CFE_EVS_EventType_ERROR, 
                        "Error reading SB message, return status = 0x%04X", SbStatus); 
   }

   return RetStatus;
   
} /* End MSG_TRANS_ProcessSbMsg() */


/******************************************************************************
** Function: MSG_TRANS_ResetStatus
**
** Reset counters and status flags to a known reset state.
**
*/
void MSG_TRANS_ResetStatus(void)
{

   MQTT_TOPIC_TBL_ResetStatus();
   MsgTrans->ValidMqttMsgCnt   = 0;
   MsgTrans->InvalidMqttMsgCnt = 0;
   MsgTrans->ValidSbMsgCnt     = 0;
   MsgTrans->InvalidSbMsgCnt   = 0;

} /* MSG_TRANS_ResetStatus() */
