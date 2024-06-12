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
**   Manage MQTT interface
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

#ifndef _mqtt_mgr_
#define _mqtt_mgr_

/*
** Includes
*/

#include "app_cfg.h"
#include "mqmsg_trans.h"
#include "mqtt_client.h"


/***********************/
/** Macro Definitions **/
/***********************/


/*
** Event Message IDs
*/

#define MQTT_MGR_CONFIG_SUBSCRIPTIONS_EID  (MQTT_MGR_BASE_EID + 0)
#define MQTT_MGR_RECONNECT_EID             (MQTT_MGR_BASE_EID + 1)
#define MQTT_MGR_START_TEST_EID            (MQTT_MGR_BASE_EID + 2)
#define MQTT_MGR_STOP_TEST_EID             (MQTT_MGR_BASE_EID + 3)
#define MQTT_MGR_SEND_CONNECTION_INFO_EID  (MQTT_MGR_BASE_EID + 4)


/**********************/
/** Type Definitions **/
/**********************/


typedef struct
{
   
   bool    Enabled;
   uint32  Period;
   uint32  DelayCnt;
   uint32  Attempts;

} MQTT_MGR_Reconnect_t;


typedef struct
{

   const INITBL_Class_t  *IniTbl; 

   uint32  MqttYieldTime;
   uint32  SbPendTime;
   uint32  UnpublishedSbMsgCnt;
   
   MQTT_MGR_Reconnect_t Reconnect;
   
   CFE_SB_PipeId_t TopicPipe;
   
   CFE_ES_TaskId_t  PluginTestChildTaskId;
   bool             PluginTestActive;
   int16            PluginTestParam;
   int32            PluginTestDelay;
   JMSG_USR_TopicPlugin_Enum_t  PluginTestId;
   
   /*
   ** Contained Objects
   */
   
   MQTT_CLIENT_Class_t  MqttClient;
   MQMSG_TRANS_Class_t  MqMsgTrans;  
   
} MQTT_MGR_Class_t;


/************************/
/** Exported Functions **/
/************************/


/******************************************************************************
** Function: MQTT_MGR_Constructor
**
** Initialize the MQTT Manager object
**
** Notes:
**   1. This must be classed prior to any other member functions.
**
*/
void MQTT_MGR_Constructor(MQTT_MGR_Class_t *MqttMgrPtr, const INITBL_Class_t *IniTbl);


/******************************************************************************
** Function: MQTT_MGR_ChildTaskCallback
**
*/
bool MQTT_MGR_ChildTaskCallback(CHILDMGR_Class_t *ChildMgr);


/******************************************************************************
** Function: MQTT_MGR_ConnectToMqttBrokerCmd
**
** Start/stop a topic test. 
**
** Notes:
**   1. Signature must match CMDMGR_CmdFuncPtr_t
**
*/
bool MQTT_MGR_ConnectToMqttBrokerCmd(void* DataObjPtr, const CFE_MSG_Message_t *MsgPtr);


/******************************************************************************
** Function: MQTT_MGR_Execute
**
** Perform management functions that need to be performed on a periodic basis.
**
** Notes:
**   1. This function is designed to be continuously called from the app's main
**      loop so it pends with a timeout on the SB
**   2. In normal operations it receives topic messages from the SB and 
**      creates/sends corresponding MQTT JSON messages to the MQTT_CLIENT. It 
**      also has test modes of operation.
**
*/
void MQTT_MGR_Execute(uint32 PerfId);


/******************************************************************************
** Function: MQTT_MGR_ReconnectToMqttBrokerCmd
**
*/
bool MQTT_MGR_ReconnectToMqttBrokerCmd(void* DataObjPtr, const CFE_MSG_Message_t *MsgPtr);


/******************************************************************************
** Function: MQTT_MGR_ResetStatus
**
** Reset counters and status flags to a known reset state.
**
*/
void MQTT_MGR_ResetStatus(void);


/******************************************************************************
** Function: MQTT_MGR_StartPluginTestCmd
**
** Notes:
**   1. Signature must match CMDMGR_CmdFuncPtr_t
**   2. DataObjPtr is not used
*/
bool MQTT_MGR_StartPluginTestCmd(void* DataObjPtr, const CFE_MSG_Message_t *MsgPtr);


/******************************************************************************
** Function: MQTT_MGR_SendConnectionInfoCmd
**
** Notes:
**   1. Signature must match CMDMGR_CmdFuncPtr_t
**   2. DataObjPtr is not used
*/
bool MQTT_MGR_SendConnectionInfoCmd(void* DataObjPtr, const CFE_MSG_Message_t *MsgPtr);


/******************************************************************************
** Function: MQTT_MGR_StopPluginTestCmd
**
** Notes:
**   1. Signature must match CMDMGR_CmdFuncPtr_t
**   2. DataObjPtr is not used
*/
bool MQTT_MGR_StopPluginTestCmd(void* DataObjPtr, const CFE_MSG_Message_t *MsgPtr);


#endif /* _mqtt_mgr_ */
