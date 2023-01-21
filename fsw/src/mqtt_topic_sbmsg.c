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
**   1. The integer telemetry message is used for the built in test.
**
*/
void MQTT_TOPIC_SBMSG_Constructor(MQTT_TOPIC_SBMSG_Class_t *MqttTopicSbMsgPtr,
                                  CFE_SB_MsgId_t TopicBaseMid, 
                                  CFE_SB_MsgId_t IntegerTlmMsgMid,
                                  CFE_SB_MsgId_t WrapSbMsgMid)
{

   MqttTopicSbMsg = MqttTopicSbMsgPtr;
   memset(MqttTopicSbMsg, 0, sizeof(MQTT_TOPIC_SBMSG_Class_t));

   MqttTopicSbMsg->IntegerTlmMsgLen = sizeof(MQTT_GW_IntegerTlm_t);
OS_printf("WrapSbMsgMid %d\n",CFE_SB_MsgIdToValue(WrapSbMsgMid));
   CFE_MSG_Init(CFE_MSG_PTR(MqttTopicSbMsg->MqttToSbWrapTlmMsg), WrapSbMsgMid,     sizeof(KIT_TO_WrappedSbMsgTlm_t));
   CFE_MSG_Init(CFE_MSG_PTR(MqttTopicSbMsg->IntegerTlmMsg),      IntegerTlmMsgMid, MqttTopicSbMsg->IntegerTlmMsgLen);
      
} /* End MQTT_TOPIC_SBMSG_Constructor() */


/******************************************************************************
** Function: MQTT_TOPIC_SBMSG_CfeToMqtt
**
** Normally this function would convert a cFE message to a JSON topic message.
** In this case the SB message is the MQTT payload.
**
*/
bool MQTT_TOPIC_SBMSG_CfeToMqtt(const char **JsonMsgPayload,
                                const CFE_MSG_Message_t *CfeMsg)
{

   bool  RetStatus = false;
   CFE_Status_t   CfeStatus;
   CFE_MSG_Size_t MsgSize;
   const KIT_TO_WrappedSbMsgTlm_Payload_t *PayloadSbMsg = CMDMGR_PAYLOAD_PTR(CfeMsg, KIT_TO_WrappedSbMsgTlm_t);
   
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
** Encoded Integer message that can be pasted into HiveMQ:
**    00016B00030049001D0000005A890F00FB99000001000000000000000000000000000000
**    00016B00030048001D0000005A890F007D59000000000000000000000000000001000000
**    00016B00030047001D0000005A890F002A19000000000000000000000100000000000000
**    00016B00030046001D00000059890F001A9A000000000000010000000000000000000000
**
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
**   1. Kit-to's packet table entry for MQTT_GW_TOPIC_2_TLM_TOPICID must have
**      the forward attribute set to true.
**   2. A walking bit pattern is used to help validation.
**
*/
void MQTT_TOPIC_SBMSG_SbMsgTest(bool Init, int16 Param)
{
   
   MQTT_GW_IntegerTlm_Payload_t *Payload = &MqttTopicSbMsg->IntegerTlmMsg.Payload;
   uint8 IntegerItem;
    
   memset(Payload, 0, sizeof(MQTT_GW_IntegerTlm_Payload_t));

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
      
      IntegerItem = MqttTopicSbMsg->SbTestCnt % 4;
      switch (IntegerItem)
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
   
   // Wrap integer message in an SbMsg
   CFE_SB_TimeStampMsg(CFE_MSG_PTR(MqttTopicSbMsg->IntegerTlmMsg.TelemetryHeader));   
   CFE_SB_TransmitMsg(CFE_MSG_PTR(MqttTopicSbMsg->IntegerTlmMsg.TelemetryHeader), true);
   
} /* End MQTT_TOPIC_SBMSG_SbMsgTest() */


