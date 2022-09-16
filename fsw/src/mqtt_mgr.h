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
**   2. The Basecamp JSON table coding ideom is use a separate object to manage 
**      the table. Since MQTT manager has very little functionality beyond
**      processing the table, a single object is used for management functions
**      and table processing.
**
** References:
**   1. OpenSatKit Object-based Application Developer's Guide
**   2. cFS Application Developer's Guide
**
*/

#ifndef _mqtt_mgr_
#define _mqtt_mgr_

/*
** Includes
*/

#include "app_cfg.h"
#include "msg_trans.h"
#include "mqtt_client.h"


/***********************/
/** Macro Definitions **/
/***********************/


/*
** Event Message IDs
*/

#define MQTT_MGR_SUBSCRIBE_EID        (MQTT_MGR_BASE_EID + 0)
#define MQTT_MGR_SUBSCRIBE_ERR_EID    (MQTT_MGR_BASE_EID + 1)
#define MQTT_MGR_CONFIG_TEST_EID      (MQTT_MGR_BASE_EID + 2)
#define MQTT_MGR_CONFIG_TEST_ERR_EID  (MQTT_MGR_BASE_EID + 3)


/**********************/
/** Type Definitions **/
/**********************/


typedef struct
{

   const INITBL_Class_t  *IniTbl; 

   uint32  MqttYieldTime;
   uint32  SbPendTime;
   
   CFE_SB_PipeId_t TopicPipe;
   
   bool    SbTopicTestActive;
   uint16  SbTopicTestId;
   int16   SbTopicTestParam;
   
   /*
   ** Contained Objects
   */
   
   MQTT_CLIENT_Class_t  MqttClient;
   MSG_TRANS_Class_t    MsgTrans;  
   
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
void MQTT_MGR_Constructor(MQTT_MGR_Class_t *MqttMgrPtr,
                          const INITBL_Class_t *IniTbl, TBLMGR_Class_t *TblMgr);


/******************************************************************************
** Function: MQTT_MGR_ChildTaskCallback
**
*/
bool MQTT_MGR_ChildTaskCallback(CHILDMGR_Class_t *ChildMgr);


/******************************************************************************
** Function: MQTT_MGR_ConfigSbTopicTestCmd
**
** Start/stop a topic test. 
**
** Notes:
**   1. Topic tests verify the topic SB-to-MQTT translation path. The tests 
**      create and send SB topic messages. These messages are looped back
**      to MQTT_GW whcih causes the CfeToJson transaltion to occur. The tests
**      are continuously run by MQTT_MGR_Execute() until they are commanded
**      to stop. 
**   2. In addition to testing the tranlation process they are useful for 
**      verifying web apps that are processing the MQTT messages.
**   3. Signature must match CMDMGR_CmdFuncPtr_t
*/
bool MQTT_MGR_ConfigSbTopicTestCmd(void* DataObjPtr, const CFE_MSG_Message_t *MsgPtr);


/******************************************************************************
** Function: MQTT_MGR_ConnectToMqttBrokerCmd
**
** Start/stop a topic test. 
**
** Notes:
**   1. Signature must match CMDMGR_CmdFuncPtr_t
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
** Function: MQTT_MGR_ResetStatus
**
** Reset counters and status flags to a known reset state.
**
*/
void MQTT_MGR_ResetStatus(void);


#endif /* _mqtt_mgr_ */
