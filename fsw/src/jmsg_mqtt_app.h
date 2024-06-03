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
**   Manage an MQTT gateway between the cFS Software Bus and an MQTT
**   Broker.
**
** Notes:
**   1. This app does not provide a generic message translation capability. It
**      is expected to be used in constrained environments so a trade for
**      simplicity and extendability was made over complexity and genericity.
**      A separate JSON table is defined for each supported MQTT topic combined
**      with a custom function to manage the translation.
**
*/
#ifndef _jmsg_mqtt_app_
#define _jmsg_mqtt_app_

/*
** Includes
*/

#include "app_cfg.h"
#include "mqtt_mgr.h"

/***********************/
/** Macro Definitions **/
/***********************/

/*
** Events
*/

#define JMSG_MQTT_APP_INIT_APP_EID      (JMSG_MQTT_APP_BASE_EID + 0)
#define JMSG_MQTT_APP_NOOP_EID          (JMSG_MQTT_APP_BASE_EID + 1)
#define JMSG_MQTT_APP_EXIT_EID          (JMSG_MQTT_APP_BASE_EID + 2)
#define JMSG_MQTT_APP_INVALID_MID_EID   (JMSG_MQTT_APP_BASE_EID + 3)


/**********************/
/** Type Definitions **/
/**********************/


/******************************************************************************
** Command & Telemetry Packets
*/
/* See EDS */


/******************************************************************************
** MQTT_APP_Class
*/
typedef struct 
{

   /* 
   ** App Framework
   */ 

   INITBL_Class_t    IniTbl; 
   CFE_SB_PipeId_t   CmdPipe;
   CMDMGR_Class_t    CmdMgr;
   TBLMGR_Class_t    TblMgr;
   CHILDMGR_Class_t  ChildMgr;
      
   /*
   ** Telemetry Packets
   */
   
   JMSG_MQTT_StatusTlm_t   StatusTlm;

   
   /*
   ** JMSG_MQTT State & Contained Objects
   */ 
   
   uint32 PollCmdInterval;
   uint32 PollCmdCnt;
   uint32 PerfId;

   CFE_SB_MsgId_t  JMsgLibCmdMid;   
   CFE_SB_MsgId_t  CmdMid;
   CFE_SB_MsgId_t  ExecuteMid;
   CFE_SB_MsgId_t  SendStatusMid;
       
   MQTT_MGR_Class_t  MqttMgr;
 
} JMSG_MQTT_APP_Class_t;


/*******************/
/** Exported Data **/
/*******************/

extern JMSG_MQTT_APP_Class_t  JmsgMqttApp;


/************************/
/** Exported Functions **/
/************************/


/******************************************************************************
** Function: MQTT_AppMain
**
*/
void JMSG_MQTT_AppMain(void);


#endif /* _jmsg_mqtt_app_ */
