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
**   Manage the conversion of telemetry messages
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

#include "mqtt_gw_topic_tlm.h"

/************************************/
/** Local File Function Prototypes **/
/************************************/

static bool CfeToJson(const char **JsonMsgPayload, const CFE_MSG_Message_t *CfeMsg);
static bool JsonToCfe(CFE_MSG_Message_t **CfeMsg, const char *JsonMsgPayload, uint16 PayloadLen);
static void SbMsgTest(bool Init, int16 Param);

/**********************/
/** Global File Data **/
/**********************/

static MQTT_GW_TOPIC_TLM_Class_t* MqttGwTopicTlm = NULL;


/******************************************************************************
** Function: MQTT_GW_TOPIC_TLM_Constructor
**
** Initialize the telemetry topic
**
** Notes:
**   1. The discrete telemetry message is used for the built in test.
**
*/
void MQTT_GW_TOPIC_TLM_Constructor(MQTT_GW_TOPIC_TLM_Class_t *MqttGwTopicTlmPtr,
                                     MQTT_TOPIC_TBL_PluginFuncTbl_t *PluginFuncTbl,
                                     CFE_SB_MsgId_t WrappedTlmMid, CFE_SB_MsgId_t DiscretePluginTlmMid)
{

   MqttGwTopicTlm = MqttGwTopicTlmPtr;
   memset(MqttGwTopicTlm, 0, sizeof(MQTT_GW_TOPIC_TLM_Class_t));

   PluginFuncTbl->CfeToJson = CfeToJson;
   PluginFuncTbl->JsonToCfe = JsonToCfe;  
   PluginFuncTbl->SbMsgTest = SbMsgTest;
   
   MqttGwTopicTlm->DiscretePluginTlmMsgLen = sizeof(MQTT_GW_DiscretePluginTlm_t);
   CFE_MSG_Init(CFE_MSG_PTR(MqttGwTopicTlm->WrappedTlmMsg), WrappedTlmMid, sizeof(KIT_TO_WrappedSbMsgTlm_t));
   CFE_MSG_Init(CFE_MSG_PTR(MqttGwTopicTlm->DiscretePluginTlmMsg), DiscretePluginTlmMid, MqttGwTopicTlm->DiscretePluginTlmMsgLen);
         
} /* End MQTT_GW_TOPIC_TLM_Constructor() */


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
   
   *JsonMsgPayload = MqttGwTopicTlm->MqttMsgPayload;

   CfeStatus = CFE_MSG_GetSize((CFE_MSG_Message_t *)PayloadSbMsg, &MsgSize);
   if (CfeStatus == CFE_SUCCESS)
   {
       if (MsgSize < MQTT_TOPIC_SB_MSG_MAX_LEN)
       {
           PktUtil_HexEncode(MqttGwTopicTlm->MqttMsgPayload, (uint8 *)PayloadSbMsg, MsgSize, true);
           MqttGwTopicTlm->CfeToMqttCnt++;
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
   KIT_TO_WrappedSbMsgTlm_Payload_t *SbMsgPayload = &(MqttGwTopicTlm->WrappedTlmMsg.Payload);
   size_t DecodedLen;
   
   DecodedLen = PktUtil_HexDecode((uint8 *)SbMsgPayload, JsonMsgPayload, PayloadLen);
   if (DecodedLen > 0)
   {
      CFE_EVS_SendEvent(MQTT_GW_TOPIC_TLM_HEX_DECODE_EID, CFE_EVS_EventType_DEBUG,
                        "MQTT message successfully decoded. MQTT len = %d, Decoded len = %d",
                        (uint16)PayloadLen, (uint16)DecodedLen);
      *CfeMsg = CFE_MSG_PTR(MqttGwTopicTlm->WrappedTlmMsg);
      MqttGwTopicTlm->MqttToCfeCnt++;
      RetStatus = true;
   }
   else
   {
      CFE_EVS_SendEvent(MQTT_GW_TOPIC_TLM_HEX_DECODE_EID, CFE_EVS_EventType_ERROR,
                        "MQTT message decode failed. MQTT len = %d, Decoded len = %d",
                        (uint16)PayloadLen, (uint16)DecodedLen);
   }
   
   return RetStatus;
   
} /* End JsonToCfe() */


/******************************************************************************
** Function: SbMsgTest
**
** Test plugin by converting a MQTT_GW discrete SB telemetry message to an MQTT
** JSON message 
**
** Notes:
**   1. KIT_TO's packet table entry for MQTT_GW_DISCRETE_PLUGIN_TOPICID must have
**      the forward attribute set to true.
**   2. The mqtt_gw_topics.json entry must be set to subscribe to
**      KIT_TO_PUB_WRAPPED_TLM_TOPICID
**   3. A walking bit pattern is used in the dsicrete data to help validation.
**
*/
static void SbMsgTest(bool Init, int16 Param)
{
   
   MQTT_GW_DiscretePluginTlm_Payload_t *Payload = &MqttGwTopicTlm->DiscretePluginTlmMsg.Payload;
   uint8 DiscreteItem;
       
   memset(Payload, 0, sizeof(MQTT_GW_DiscretePluginTlm_Payload_t));

   if (Init)
   {
         
      MqttGwTopicTlm->SbTestCnt = 0;
      Payload->Item_1 = 1;
      
      CFE_EVS_SendEvent(MQTT_GW_TOPIC_TLM_INIT_SB_MSG_TEST_EID, CFE_EVS_EventType_INFORMATION,
                        "Telemetry topic test started");
   }
   else
   {
   
      MqttGwTopicTlm->SbTestCnt++;
      
      DiscreteItem = MqttGwTopicTlm->SbTestCnt % 4;
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
   
   CFE_SB_TimeStampMsg(CFE_MSG_PTR(MqttGwTopicTlm->DiscretePluginTlmMsg.TelemetryHeader));   
   memcpy(&(MqttGwTopicTlm->WrappedTlmMsg.Payload), &MqttGwTopicTlm->DiscretePluginTlmMsg, MqttGwTopicTlm->DiscretePluginTlmMsgLen);
   CFE_SB_TimeStampMsg(CFE_MSG_PTR(MqttGwTopicTlm->WrappedTlmMsg.TelemetryHeader));
   CFE_SB_TransmitMsg(CFE_MSG_PTR(MqttGwTopicTlm->WrappedTlmMsg.TelemetryHeader), true);

} /* End SbMsgTest() */


