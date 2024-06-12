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
**   2. The Basecamp JSON table coding idiom is to use a separate object to manage 
**      the table. Since MQTT manager has very little functionality beyond
**      processing the table, a single object is used for management functions
**      and table processing.
**
*/

/*
** Include Files:
*/

#include <string.h>

#include "mqmsg_trans.h"

/**********************/
/** Global File Data **/
/**********************/

static MQMSG_TRANS_Class_t *MqMsgTrans = NULL;


/******************************************************************************
** Function: MQMSG_TRANS_Constructor
**
*/
void MQMSG_TRANS_Constructor(MQMSG_TRANS_Class_t *MqMsgTransPtr,
                           const INITBL_Class_t  *IniTbl)
{
 
   MqMsgTrans = MqMsgTransPtr;

   CFE_PSP_MemSet((void*)MqMsgTransPtr, 0, sizeof(MQMSG_TRANS_Class_t));
   
} /* End MQMSG_TRANS_Constructor() */


/******************************************************************************
** Function: MQMSG_TRANS_ProcessMqttMsg
**
** Notes:
**   1. Signature must match MQTT_CLIENT_MsgCallback_t
**   2. MQTT has no delimeter between the topic and payload
**
*/
void MQMSG_TRANS_ProcessMqttMsg(MessageData *MsgData)
{
   
   MQTTMessage* MsgPtr = MsgData->message;

   enum JMSG_USR_TopicPlugin topic = JMSG_USR_TopicPlugin_Enum_t_MIN;
   uint16  TopicLen;
   char    TopicStr[JMSG_USR_MAX_TOPIC_NAME_LEN];
   bool    MsgFound = false;
   const JMSG_TOPIC_TBL_Topic_t *Topic;
   JMSG_TOPIC_TBL_JsonToCfe_t JsonToCfe;
   CFE_MSG_Message_t *CfeMsg;
   CFE_SB_MsgId_t    MsgId = CFE_SB_INVALID_MSG_ID;
   CFE_MSG_Size_t    MsgSize;
   CFE_MSG_Type_t    MsgType;
   
   CFE_EVS_SendEvent(MQMSG_TRANS_PROCESS_MQTT_MSG_EID, CFE_EVS_EventType_DEBUG,
                     "MQMSG_TRANS_ProcessMsg: Received topic %s", MsgData->topicName->lenstring.data);
                    
   if(MsgPtr->payloadlen)
   {
      
      CFE_EVS_SendEvent(MQMSG_TRANS_PROCESS_MQTT_MSG_EID, CFE_EVS_EventType_DEBUG,
                        "MsgPtr->payloadlen=%d", (int)MsgPtr->payloadlen);

      while (!MsgFound && topic < JMSG_USR_TopicPlugin_Enum_t_MAX)
      {

         Topic = JMSG_TOPIC_TBL_GetTopic(topic);
         if (Topic != NULL)
         {
            
            TopicLen = strlen(Topic->Name);
            
            if (TopicLen < JMSG_USR_MAX_TOPIC_NAME_LEN)
            {
               CFE_EVS_SendEvent(MQMSG_TRANS_PROCESS_MQTT_MSG_EID, CFE_EVS_EventType_DEBUG,
                                 "Table topic name=%s, length=%d", Topic->Name, TopicLen);
               if (strncmp(Topic->Name, MsgData->topicName->lenstring.data, TopicLen) == 0)
               {
                  MsgFound = true;
                  memcpy(TopicStr, MsgData->topicName->lenstring.data, TopicLen);
                  TopicStr[TopicLen] = '\0';
                  CFE_EVS_SendEvent(MQMSG_TRANS_PROCESS_MQTT_MSG_INFO_EID, CFE_EVS_EventType_INFORMATION,
                                   "MQMSG_TRANS_ProcessMqttMsg: Topic=%s, Payload=%s", 
                                    TopicStr, &MsgData->topicName->lenstring.data[TopicLen]);
               }
            } /* End if valid length */
            else
            {
               CFE_EVS_SendEvent(MQMSG_TRANS_PROCESS_MQTT_MSG_EID, CFE_EVS_EventType_ERROR,
                                 "Table topic name %s with length %d exceeds maximum length %d", 
                                 Topic->Name, TopicLen, JMSG_USR_MAX_TOPIC_NAME_LEN);               
            }
         }
         if (!MsgFound)
         {
            topic++;
         }
      } /* End while loop */

      if (MsgFound)
      {
            
         CFE_EVS_SendEvent(MQMSG_TRANS_PROCESS_MQTT_MSG_EID, CFE_EVS_EventType_DEBUG,
                           "MQMSG_TRANS_ProcessMqttMsg: Found message at index %d", topic); 
                              
         JsonToCfe = JMSG_TOPIC_TBL_GetJsonToCfe(topic);    
       
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
            
            CFE_EVS_SendEvent(MQMSG_TRANS_PROCESS_MQTT_MSG_INFO_EID, CFE_EVS_EventType_INFORMATION,
                              "MQMSG_TRANS_ProcessMqttMsg: Sending SB message 0x%04X(%d), len %d, type %d", 
                              CFE_SB_MsgIdToValue(MsgId), CFE_SB_MsgIdToValue(MsgId), (int)MsgSize, (int)MsgType); 
            CFE_SB_TransmitMsg(CFE_MSG_PTR(*CfeMsg), true);               
            MqMsgTrans->ValidMqttMsgCnt++;
            
         }
         else
         {
            CFE_EVS_SendEvent(MQMSG_TRANS_PROCESS_MQTT_MSG_EID, CFE_EVS_EventType_ERROR,
                              "MQMSG_TRANS_ProcessMqttMsg: Error creating SB message from JSON topic %s, Id %d",
                               TopicStr, topic); 
         }
         
      } /* End if message found */
      else 
      {      
         CFE_EVS_SendEvent(MQMSG_TRANS_PROCESS_MQTT_MSG_EID, CFE_EVS_EventType_ERROR, 
                           "MQMSG_TRANS_ProcessMqttMsg: Could not find a topic match for %s", 
                           MsgData->topicName->lenstring.data);      
      }
   
   } /* End null message len */
   else {
      
      CFE_EVS_SendEvent(MQMSG_TRANS_PROCESS_MQTT_MSG_EID, CFE_EVS_EventType_ERROR,
                        "Null MQTT message data length for %s", MsgData->topicName->lenstring.data);
   }
   
   fflush(stdout);

} /* End MQMSG_TRANS_ProcessMqttMsg() */


/******************************************************************************
** Function: MQMSG_TRANS_ProcessSbMsg
**
** Notes:
**   None
**
*/
bool MQMSG_TRANS_ProcessSbMsg(const CFE_MSG_Message_t *CfeMsgPtr,
                              const char **Topic, const char **Payload)
{
   
   bool RetStatus = false;
   int32 TopicIndex;
   int32 SbStatus;
   CFE_SB_MsgId_t  MsgId = CFE_SB_INVALID_MSG_ID;
   JMSG_TOPIC_TBL_CfeToJson_t CfeToJson;
   const char *JsonMsgTopic;
   const char *JsonMsgPayload;

   *Topic   = NULL; 
   *Payload = NULL;
   SbStatus = CFE_MSG_GetMsgId(CfeMsgPtr, &MsgId);
   if (SbStatus == CFE_SUCCESS)
   {
   
      CFE_EVS_SendEvent(MQMSG_TRANS_PROCESS_SB_MSG_EID, CFE_EVS_EventType_DEBUG, 
                        "MQMSG_TRANS_ProcessSbMsg: Received SB message ID 0x%04X(%d)", 
                        CFE_SB_MsgIdToValue(MsgId), CFE_SB_MsgIdToValue(MsgId)); 
      
      if ((TopicIndex = JMSG_TOPIC_TBL_MsgIdToTopicPlugin(MsgId)) != JMSG_USR_TOPIC_PLUGIN_UNDEF)
      {
         
         CfeToJson = JMSG_TOPIC_TBL_GetCfeToJson(TopicIndex, &JsonMsgTopic);    
         
         if (CfeToJson(&JsonMsgPayload, CfeMsgPtr))
         {
            *Topic   = JsonMsgTopic; 
            *Payload = JsonMsgPayload;
            RetStatus = true;
            CFE_EVS_SendEvent(MQMSG_TRANS_PROCESS_SB_MSG_INFO_EID, CFE_EVS_EventType_INFORMATION,
                              "MQMSG_TRANS_ProcessMqttMsg: Created MQTT topic %s message %s",
                              JsonMsgTopic, JsonMsgPayload);             
            MqMsgTrans->ValidSbMsgCnt++;

         }
         else
         {
            CFE_EVS_SendEvent(MQMSG_TRANS_PROCESS_SB_MSG_EID, CFE_EVS_EventType_ERROR,
                              "MQMSG_TRANS_ProcessMqttMsg: Error creating JSON message from SB for topic index %d", TopicIndex); 
         
         }        
      }
      else
      {
         CFE_EVS_SendEvent(MQMSG_TRANS_PROCESS_SB_MSG_EID, CFE_EVS_EventType_ERROR, 
                           "MQMSG_TRANS_ProcessMsg: Unable to locate SB message 0x%04X(%d) in MQTT topic table", 
                           CFE_SB_MsgIdToValue(MsgId), CFE_SB_MsgIdToValue(MsgId));
      }

   } /* End message Id */
   else
   {
      CFE_EVS_SendEvent(MQMSG_TRANS_PROCESS_SB_MSG_EID, CFE_EVS_EventType_ERROR, 
                        "Error reading SB message, return status = 0x%04X", SbStatus); 
   }

   return RetStatus;
   
} /* End MQMSG_TRANS_ProcessSbMsg() */


/******************************************************************************
** Function: MQMSG_TRANS_ResetStatus
**
** Reset counters and status flags to a known reset state.
**
*/
void MQMSG_TRANS_ResetStatus(void)
{

   MqMsgTrans->ValidMqttMsgCnt   = 0;
   MqMsgTrans->InvalidMqttMsgCnt = 0;
   MqMsgTrans->ValidSbMsgCnt     = 0;
   MqMsgTrans->InvalidSbMsgCnt   = 0;

} /* MQMSG_TRANS_ResetStatus() */
