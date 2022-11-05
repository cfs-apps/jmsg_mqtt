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

/*
** Includes
*/

#include "mqtt_topic_sbmsg.h"

/************************************/
/** Local File Function Prototypes **/
/************************************/


/**********************/
/** Global File Data **/
/**********************/

static MQTT_TOPIC_SBMSG_Class_t* MqttTopicSbMsg = NULL;


/******************************************************************************
** Function: MQTT_TOPIC_SBMSG_Constructor
**
** Initialize the MQTT SB Message topic
**
** Notes:
**   1. The discrete telemetry message is used for the built in test.
**
*/
void MQTT_TOPIC_SBMSG_Constructor(MQTT_TOPIC_SBMSG_Class_t *MqttTopicSbMsgPtr,
                                  CFE_SB_MsgId_t TopicBaseMid, 
                                  CFE_SB_MsgId_t DiscreteTlmMsgMid,
                                  CFE_SB_MsgId_t WrapSbMsgMid,
                                  const char *Topic)
{

   MqttTopicSbMsg = MqttTopicSbMsgPtr;
   memset(MqttTopicSbMsg, 0, sizeof(MQTT_TOPIC_SBMSG_Class_t));

   strncpy(MqttTopicSbMsg->MqttMsgTopic, Topic, MQTT_TOPIC_TBL_MAX_TOPIC_LEN);
   
   MqttTopicSbMsg->DiscreteTlmMsgLen = sizeof(MQTT_GW_DiscreteTlm_t);

   CFE_MSG_Init(CFE_MSG_PTR(MqttTopicSbMsg->MqttToSbWrapTlmMsg), WrapSbMsgMid,   sizeof(KIT_TO_WrappedSbMsgTlm_t));
   CFE_MSG_Init(CFE_MSG_PTR(MqttTopicSbMsg->DiscreteTlmMsg),     DiscreteTlmMsgMid, MqttTopicSbMsg->DiscreteTlmMsgLen);
      
} /* End MQTT_TOPIC_SBMSG_Constructor() */


/******************************************************************************
** Function: MQTT_TOPIC_SBMSG_CfeToMqtt
**
** Normally this function would convert a cFE message to a JSON topic message.
** In this case the SB message is the MQTT payload.
**
*/
bool MQTT_TOPIC_SBMSG_CfeToMqtt(const char **JsonMsgTopic, const char **JsonMsgPayload,
                                const CFE_MSG_Message_t *CfeMsg)
{

   bool  RetStatus = false;
   CFE_Status_t   CfeStatus;
   CFE_MSG_Size_t MsgSize;
   const KIT_TO_WrappedSbMsgTlm_Payload_t *PayloadSbMsg = CMDMGR_PAYLOAD_PTR(CfeMsg, KIT_TO_WrappedSbMsgTlm_t);
   
   *JsonMsgTopic   = MqttTopicSbMsg->MqttMsgTopic;
   *JsonMsgPayload = MqttTopicSbMsg->MqttMsgPayload;

   CfeStatus = CFE_MSG_GetSize((CFE_MSG_Message_t *)PayloadSbMsg, &MsgSize);
   if (CfeStatus == CFE_SUCCESS)
   {
       if (MsgSize < MQTT_TOPIC_SB_MSG_MAX_LEN)
       {
           PktUtil_HexEncode(MqttTopicSbMsg->MqttMsgPayload, (uint8 *)PayloadSbMsg, MsgSize);
           MqttTopicSbMsg->CfeToMqttCnt++;
           RetStatus = true;
       }
   }
      
   return RetStatus;
   
} /* End MQTT_TOPIC_SBMSG_CfeToMqtt() */


/******************************************************************************
** Function: MQTT_TOPIC_SBMSG_MqttToCfe
**
** Normally this function would convert a JSON topic message to a cFS SB
** message. In this case the MQTT payload is a SB message. The SB message
** is decoded and copied into an SB message and sent to TO that expects a
** wrapped message.
**
** Encoded Discrete message that can be pasted into HiveMQ:
**    0001690003000000110000007C460F007472000000000001
**    0001690003000000110000004F460F008759000000000100
**    0001690003000000110000004F460F003B19000000010000
**    0001690003000000110000004F460F003B19000001000000
*/
bool MQTT_TOPIC_SBMSG_MqttToCfe(CFE_MSG_Message_t **CfeMsg, 
                               const char *JsonMsgPayload, uint16 PayloadLen)
{
   
   bool RetStatus = false;
   KIT_TO_WrappedSbMsgTlm_Payload_t *SbMsgPayload = &(MqttTopicSbMsg->MqttToSbWrapTlmMsg.Payload);
   size_t DecodedLen;
   
   DecodedLen = PktUtil_HexDecode((uint8 *)SbMsgPayload, JsonMsgPayload, PayloadLen);
   if (DecodedLen > 0)
   {
      CFE_EVS_SendEvent(MQTT_TOPIC_SBMSG_HEX_DECODE_EID, CFE_EVS_EventType_ERROR,
                        "MQTT message successfully decoded. MQTT len = %d, Decoded len = %d",
                        (uint16)PayloadLen, (uint16)DecodedLen);
      *CfeMsg = CFE_MSG_PTR(MqttTopicSbMsg->MqttToSbWrapTlmMsg);
      MqttTopicSbMsg->MqttToCfeCnt++;
      RetStatus = true;
   }
   else
   {
      CFE_EVS_SendEvent(MQTT_TOPIC_SBMSG_HEX_DECODE_EID, CFE_EVS_EventType_ERROR,
                        "MQTT message decode failed. MQTT len = %d, Decoded len = %d",
                        (uint16)PayloadLen, (uint16)DecodedLen);
   }
   
   return RetStatus;
   
} /* End MQTT_TOPIC_SBMSG_MqttToCfe() */


/******************************************************************************
** Function: MQTT_TOPIC_SBMSG_SbMsgTest
**
** Send a MQTT_GW discrere SB message
**
** Notes:
**   1. A walking bit pattern is used to help validation.
**
*/
void MQTT_TOPIC_SBMSG_SbMsgTest(bool Init, int16 Param)
{
   
   MQTT_GW_DiscreteTlm_Payload_t *Payload = &MqttTopicSbMsg->DiscreteTlmMsg.Payload;
   uint8 DiscreteItem;
    
   memset(Payload, 0, sizeof(MQTT_GW_DiscreteTlm_Payload_t));

   if (Init)
   {
      MqttTopicSbMsg->SbTestCnt = 0;
      Payload->Item_1 = 1;
      
      CFE_EVS_SendEvent(MQTT_TOPIC_SBMSG_INIT_SB_MSG_TEST_EID, CFE_EVS_EventType_INFORMATION,
                        "SB message topic test started");
   }
   else
   {
   
      MqttTopicSbMsg->SbTestCnt++;
      
      DiscreteItem = MqttTopicSbMsg->SbTestCnt % 4;
      switch (DiscreteItem)
      {
         case 0:
            Payload->Item_1 = 1;
            break;
         case 1:
            Payload->Item_2 = 1;
            break;
         case 2:
            Payload->Item_3 = 1;
            break;
         case 3:
            Payload->Item_4 = 1;
            break;
         default:
            break;
         
      } /* End axis switch */
   }
   
   // Wrap discrete message in an SbMsg
   CFE_SB_TimeStampMsg(CFE_MSG_PTR(MqttTopicSbMsg->DiscreteTlmMsg.TelemetryHeader));   
   memcpy(&MqttTopicSbMsg->MqttToSbWrapTlmMsg.Payload, &MqttTopicSbMsg->DiscreteTlmMsg, MqttTopicSbMsg->DiscreteTlmMsgLen);
   CFE_SB_TimeStampMsg(CFE_MSG_PTR(MqttTopicSbMsg->MqttToSbWrapTlmMsg.TelemetryHeader));
   CFE_SB_TransmitMsg(CFE_MSG_PTR(MqttTopicSbMsg->MqttToSbWrapTlmMsg.TelemetryHeader), true);
   
} /* End MQTT_TOPIC_SBMSG_SbMsgTest() */


