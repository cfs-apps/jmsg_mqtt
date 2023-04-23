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
**   Manage topic plugins
**
** Notes:
**   1. See mqtt_gw/fsw/topics/mqtt_topic_plugin_guide.txt for 
**      plugin creation and installation instructions
** 
** References:
**   1. cFS Basecamp Object-based Application Developer's Guide
**
*/

/*
** Include Files:
*/

#include <string.h>
#include "mqtt_gw_topic_plugin.h"

/************************/
/** Topic header files **/
/************************/

#include "mqtt_gw_topic_sbmsg.h"
#include "mqtt_gw_topic_rate.h"
#include "mqtt_gw_topic_discrete.h"
#include "sc_sim_mqtt_topic_cmd.h"
#include "sc_sim_mqtt_topic_mgmt.h"
#include "sc_sim_mqtt_topic_model.h"

/************************************/
/** Local File Function Prototypes **/
/************************************/

static bool StubCfeToJson(const char **JsonMsgPayload, const CFE_MSG_Message_t *CfeMsg);
static bool StubJsonToCfe(CFE_MSG_Message_t **CfeMsg, const char *JsonMsgPayload, uint16 PayloadLen);
static void StubSbMsgTest(bool Init, int16 Param);


/**********************/
/** Global File Data **/
/**********************/

// Declare topic plugin objects

static MQTT_GW_TOPIC_SBMSG_Class_t      MqttGwTopicSbMsg;
static MQTT_GW_TOPIC_DISCRETE_Class_t   MqttGwTopicDiscrete;
static MQTT_GW_TOPIC_RATE_Class_t       MqttGwTopicRate;

static SC_SIM_MQTT_TOPIC_CMD_Class_t    ScSimTopicCmd;
static SC_SIM_MQTT_TOPIC_MGMT_Class_t   ScSimTopicMgmt;
static SC_SIM_MQTT_TOPIC_MODEL_Class_t  ScSimTopicModel;


/******************************************************************************
** Function: MQTT_GW_TOPIC_PLUGIN_Constructor
**
**
** Notes:
**   1. The design convention is for topic objects to receive a
**      CFE_SB_MsgId_t type so TopicIds defined in the JSON table
**      must be converted to message IDs. 
**
*/
void MQTT_GW_TOPIC_PLUGIN_Constructor(const MQTT_TOPIC_TBL_Data_t *TopicTbl,
                                      MQTT_TOPIC_TBL_PluginFuncTbl_t *PluginFuncTbl,
                                      uint32 DiscreteTlmTopicId, uint32 TunnelTlmTopicId)
{

   for (int i=0; i < MQTT_GW_PluginTopic_Enum_t_MAX; i++)
   {
      if (TopicTbl->Topic[i].Enabled)
      {
         switch (i)
         {
            case MQTT_GW_PluginTopic_1:
               MQTT_GW_TOPIC_SBMSG_Constructor(&MqttGwTopicSbMsg, &PluginFuncTbl[i],
                                               CFE_SB_ValueToMsgId(DiscreteTlmTopicId),
                                               CFE_SB_ValueToMsgId(TopicTbl->Topic[i].Cfe),
                                               CFE_SB_ValueToMsgId(TunnelTlmTopicId));
               break;
            case MQTT_GW_PluginTopic_2:
               MQTT_GW_TOPIC_DISCRETE_Constructor(&MqttGwTopicDiscrete, &PluginFuncTbl[i],
                                                  CFE_SB_ValueToMsgId(TopicTbl->Topic[i].Cfe));
            
               break;
            case MQTT_GW_PluginTopic_3:
               MQTT_GW_TOPIC_RATE_Constructor(&MqttGwTopicRate, &PluginFuncTbl[i],
                                               CFE_SB_ValueToMsgId(TopicTbl->Topic[i].Cfe));
               break;
            case MQTT_GW_PluginTopic_4:
               SC_SIM_MQTT_TOPIC_CMD_Constructor(&ScSimTopicCmd, &PluginFuncTbl[i],
                                                 CFE_SB_ValueToMsgId(TopicTbl->Topic[i].Cfe));
               break;
            case MQTT_GW_PluginTopic_5:
               SC_SIM_MQTT_TOPIC_MGMT_Constructor(&ScSimTopicMgmt, &PluginFuncTbl[i],
                                                  CFE_SB_ValueToMsgId(TopicTbl->Topic[i].Cfe));
               break;
            case MQTT_GW_PluginTopic_6:
               SC_SIM_MQTT_TOPIC_MODEL_Constructor(&ScSimTopicModel, &PluginFuncTbl[i],
                                                   CFE_SB_ValueToMsgId(TopicTbl->Topic[i].Cfe));
               break;
            case MQTT_GW_PluginTopic_7:
            case MQTT_GW_PluginTopic_8:
            default:
               CFE_EVS_SendEvent(MQTT_GW_TOPIC_PLUGIN_EID, CFE_EVS_EventType_ERROR, 
                                 "Plugin topic %d is enabled in the topic table, but has not had a constructor installed",
                                 (i+1));
            
               break;
                           
         } // End switch
      }
      else
      {
         PluginFuncTbl[i].CfeToJson = StubCfeToJson;
         PluginFuncTbl[i].JsonToCfe = StubJsonToCfe;
         PluginFuncTbl[i].SbMsgTest = StubSbMsgTest;
      }
   } // End plugin loop
    
} /* MQTT_GW_TOPIC_PLUGIN_Constructor() */


/******************************************************************************
** Function: StubCfeToJson
**
** Provide a CfeToJson stub function to be used as a non-NULL pointer in the
** PluginFuncTbl default values.
**
*/
static bool StubCfeToJson(const char **JsonMsgPayload, 
                          const CFE_MSG_Message_t *CfeMsg)
{

   CFE_EVS_SendEvent(MQTT_TOPIC_TBL_STUB_EID, CFE_EVS_EventType_ERROR, 
                     "CfeToJson stub");

   return false;
   
} /* End StubCfeToJson() */


/******************************************************************************
** Function: StubJsonToCfe
**
** Provide a CfeToJson stub function to be used as a non-NULL pointer in the
** PluginFuncTbl default values.
**
*/
static bool StubJsonToCfe(CFE_MSG_Message_t **CfeMsg, 
                          const char *JsonMsgPayload, uint16 PayloadLen)
{
   
   CFE_EVS_SendEvent(MQTT_TOPIC_TBL_STUB_EID, CFE_EVS_EventType_ERROR, 
                     "JsonToCfe stub");

   return false;
   
} /* End StubJsonToCfe() */


/******************************************************************************
** Function: StubSbMsgTest
**
** Provide a CfeToJson stub function to be used as a non-NULL pointer in the
** PluginFuncTbl default values.
**
*/
static void StubSbMsgTest(bool Init, int16 Param)
{

   CFE_EVS_SendEvent(MQTT_TOPIC_TBL_STUB_EID, CFE_EVS_EventType_ERROR, 
                     "SbMsgTest stub");
   
} /* End StubSbMsgTest() */



