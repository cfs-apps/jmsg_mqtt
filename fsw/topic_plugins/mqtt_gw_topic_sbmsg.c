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
**   1. cFS Basecamp Object-based Application Developer's Guide
**   2. cFS Application Developer's Guide
**
*/

/*
** Includes
*/

#include "mqtt_gw_topic_sbmsg.h"

/************************************/
/** Local File Function Prototypes **/
/************************************/

static bool CfeToJson(const char **JsonMsgPayload, const CFE_MSG_Message_t *CfeMsg);
static bool JsonToCfe(CFE_MSG_Message_t **CfeMsg, const char *JsonMsgPayload, uint16 PayloadLen);
static void SbMsgTest(bool Init, int16 Param);

/**********************/
/** Global File Data **/
/**********************/

static MQTT_GW_TOPIC_SBMSG_Class_t* MqttGwTopicSbMsg = NULL;


/******************************************************************************
** Function: MQTT_GW_TOPIC_SBMSG_Constructor
**
** Initialize the MQTT SB Message topic
**
** Notes:
**   1. The integer telemetry message is used for the built in test.
**
*/
void MQTT_GW_TOPIC_SBMSG_Constructor(MQTT_GW_TOPIC_SBMSG_Class_t *MqttGwTopicSbMsgPtr,
                                     MQTT_TOPIC_TBL_PluginFuncTbl_t *PluginFuncTbl,
                                     CFE_SB_MsgId_t DiscretePluginTlmMid,
                                     CFE_SB_MsgId_t WrapSbTlmMid, CFE_SB_MsgId_t TunnelTlmMid)
{

   MqttGwTopicSbMsg = MqttGwTopicSbMsgPtr;
   memset(MqttGwTopicSbMsg, 0, sizeof(MQTT_GW_TOPIC_SBMSG_Class_t));

   PluginFuncTbl->CfeToJson = CfeToJson;
   PluginFuncTbl->JsonToCfe = JsonToCfe;  
   PluginFuncTbl->SbMsgTest = SbMsgTest;
   
   MqttGwTopicSbMsg->DiscretePluginTlmMsgLen = sizeof(MQTT_GW_DiscretePluginTlm_t);
   CFE_MSG_Init(CFE_MSG_PTR(MqttGwTopicSbMsg->MqttToSbWrapTlmMsg), WrapSbTlmMid,     sizeof(KIT_TO_WrappedSbMsgTlm_t));
   CFE_MSG_Init(CFE_MSG_PTR(MqttGwTopicSbMsg->DiscretePluginTlmMsg),      DiscretePluginTlmMid, MqttGwTopicSbMsg->DiscretePluginTlmMsgLen);
   CFE_MSG_Init(CFE_MSG_PTR(MqttGwTopicSbMsg->TunnelTlm), TunnelTlmMid, sizeof(KIT_TO_WrappedSbMsgTlm_t));
         
      
} /* End MQTT_GW_TOPIC_SBMSG_Constructor() */


/******************************************************************************
** Function: CfeToJson
**
** Create a text encoded MQTT JSON message from the payload of the SB message.
**
** Notes:
**   1. Signature must match MQTT_TOPIC_TBL_CfeToJson_t
**   2. The SB message's payload is another complete SB message (includes headers)
**
*/
static bool CfeToJson(const char **JsonMsgPayload, const CFE_MSG_Message_t *CfeMsg)
{

   bool  RetStatus = false;
   CFE_Status_t   CfeStatus;
   CFE_MSG_Size_t MsgSize;
   const KIT_TO_WrappedSbMsgTlm_Payload_t *PayloadSbMsg = CMDMGR_PAYLOAD_PTR(CfeMsg, KIT_TO_WrappedSbMsgTlm_t);
   
   *JsonMsgPayload = MqttGwTopicSbMsg->MqttMsgPayload;

   CfeStatus = CFE_MSG_GetSize((CFE_MSG_Message_t *)PayloadSbMsg, &MsgSize);
   if (CfeStatus == CFE_SUCCESS)
   {
       if (MsgSize < MQTT_TOPIC_SB_MSG_MAX_LEN)
       {
           PktUtil_HexEncode(MqttGwTopicSbMsg->MqttMsgPayload, (uint8 *)PayloadSbMsg, MsgSize, true);
           MqttGwTopicSbMsg->CfeToMqttCnt++;
           RetStatus = true;
       }
   }
      
   return RetStatus;
   
} /* End CfeToJson() */


/******************************************************************************
** Function: JsonToCfe
**
** Normally this function would convert a JSON topic message to a cFS SB
** message. In this case the MQTT payload is a SB message. The SB message
** is decoded and copied into an SB message and sent to TO that expects a
** wrapped message.
**
** Notes:
**   1. Signature must match MQTT_TOPIC_TBL_JsonToCfe_t
**   2. Encoded discrete message that can be pasted into MQTT broker for testing:
**      00016B00030049001D0000005A890F00FB99000001000000000000000000000000000000
**      00016B00030048001D0000005A890F007D59000000000000000000000000000001000000
**      00016B00030047001D0000005A890F002A19000000000000000000000100000000000000
**      00016B00030046001D00000059890F001A9A000000000000010000000000000000000000
**
**      00016B00030000001D000000000000000000000001000000000000000000000000000000
**      00016B00030000001D000000000000000000000000000000010000000000000000000000
**      00016B00030000001D000000000000000000000000000000000000000100000000000000
**      00016B00030000001D000000000000000000000000000000000000000000000001000000
*/
static bool JsonToCfe(CFE_MSG_Message_t **CfeMsg, const char *JsonMsgPayload, uint16 PayloadLen)
{
   
   bool RetStatus = false;
   KIT_TO_WrappedSbMsgTlm_Payload_t *SbMsgPayload = &(MqttGwTopicSbMsg->MqttToSbWrapTlmMsg.Payload);
   size_t DecodedLen;
   
   DecodedLen = PktUtil_HexDecode((uint8 *)SbMsgPayload, JsonMsgPayload, PayloadLen);
   if (DecodedLen > 0)
   {
      CFE_EVS_SendEvent(MQTT_GW_TOPIC_SBMSG_HEX_DECODE_EID, CFE_EVS_EventType_ERROR,
                        "MQTT message successfully decoded. MQTT len = %d, Decoded len = %d",
                        (uint16)PayloadLen, (uint16)DecodedLen);
      *CfeMsg = CFE_MSG_PTR(MqttGwTopicSbMsg->MqttToSbWrapTlmMsg);
      MqttGwTopicSbMsg->MqttToCfeCnt++;
      RetStatus = true;
   }
   else
   {
      CFE_EVS_SendEvent(MQTT_GW_TOPIC_SBMSG_HEX_DECODE_EID, CFE_EVS_EventType_ERROR,
                        "MQTT message decode failed. MQTT len = %d, Decoded len = %d",
                        (uint16)PayloadLen, (uint16)DecodedLen);
   }
   
   return RetStatus;
   
} /* End JsonToCfe() */


/******************************************************************************
** Function: SbMsgTest
**
** Send a MQTT_GW discrete SB message
**
** Notes:
**   1. Kit-to's packet table entry for MQTT_GW_DISCRETE_PLUGIN_TOPICID must have
**      the forward attribute set to true.
**   2. A walking bit pattern is used to help validation.
**
*/
static void SbMsgTest(bool Init, int16 Param)
{
   
   MQTT_GW_DiscretePluginTlm_Payload_t *Payload = &MqttGwTopicSbMsg->DiscretePluginTlmMsg.Payload;
   uint8 DiscreteItem;
       
   memset(Payload, 0, sizeof(MQTT_GW_DiscretePluginTlm_Payload_t));

   if (Init)
   {
         
      MqttGwTopicSbMsg->SbTestCnt = 0;
      Payload->Item_1 = 1;
      
      CFE_EVS_SendEvent(MQTT_GW_TOPIC_SBMSG_INIT_SB_MSG_TEST_EID, CFE_EVS_EventType_INFORMATION,
                        "SB message topic test started");
   }
   else
   {
   
      MqttGwTopicSbMsg->SbTestCnt++;
      
      DiscreteItem = MqttGwTopicSbMsg->SbTestCnt % 4;
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
   
   CFE_SB_TimeStampMsg(CFE_MSG_PTR(MqttGwTopicSbMsg->DiscretePluginTlmMsg.TelemetryHeader));   
   memcpy(&(MqttGwTopicSbMsg->TunnelTlm.Payload), &MqttGwTopicSbMsg->DiscretePluginTlmMsg, MqttGwTopicSbMsg->DiscretePluginTlmMsgLen);
   CFE_SB_TimeStampMsg(CFE_MSG_PTR(MqttGwTopicSbMsg->TunnelTlm.TelemetryHeader));
   CFE_SB_TransmitMsg(CFE_MSG_PTR(MqttGwTopicSbMsg->TunnelTlm.TelemetryHeader), true);

   //CFE_SB_TransmitMsg(CFE_MSG_PTR(MqttGwTopicSbMsg->DiscretePluginTlmMsg.TelemetryHeader), true);
   
} /* End SbMsgTest() */


