/*
** Copyright 2022 bitValence, Inc.
** All Rights Reserved.
**
** This program is free software; you can modify and/or redistribute it
** under the terms of the GNU Affero General Public License
** as published by the Free Software Foundation; version 3 with
** attribution addendums as found in the LICENSE.txt.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU Affero General Public License for more details.
**
** Purpose:
**   Define platform configurations for the MQTT Gateway application
**
** Notes:
**   1. These macros can only be build with the application and can't
**      have a platform scope because the same app_cfg.h file name is used for
**      all applications following the object-based application design.
**
*/

#ifndef _app_cfg_
#define _app_cfg_

/*
** Includes
*/

#include "mqtt_gw_eds_typedefs.h"
#include "mqtt_gw_platform_cfg.h"
#include "app_c_fw.h"


/******************************************************************************
** Application Macros
*/

/*
** Versions:
**
** 1.0 - Initial refactoring of Alan's MQTT
** 1.4 - refactored sbmsg plugin into seperate cmd/tlm plugins
** 1.5 - Update to Basecamp v1.12's app_c_fw TBLMGR API
*/

#define  MQTT_GW_MAJOR_VER      1
#define  MQTT_GW_MINOR_VER      6


/******************************************************************************
** Init File declarations create:
**
**  typedef enum {
**     CMD_PIPE_DEPTH,
**     CMD_PIPE_NAME
**  } INITBL_ConfigEnum;
**    
**  typedef struct {
**     CMD_PIPE_DEPTH,
**     CMD_PIPE_NAME
**  } INITBL_ConfigStruct;
**
**   const char *GetConfigStr(value);
**   ConfigEnum GetConfigVal(const char *str);
**
** XX(name,type)
*/

#define CFG_APP_CFE_NAME                    APP_CFE_NAME

#define CFG_APP_MAIN_PERF_ID                APP_MAIN_PERF_ID
#define CFG_CHILD_TASK_PERF_ID              CHILD_TASK_PERF_ID

#define CFG_MQTT_GW_CMD_TOPICID             MQTT_GW_CMD_TOPICID
#define CFG_SEND_HK_TLM_TOPICID             BC_SCH_2_SEC_TOPICID
#define CFG_MQTT_GW_HK_TLM_TOPICID          MQTT_GW_HK_TLM_TOPICID
#define CFG_MQTT_GW_DISCRETE_PLUGIN_TOPICID MQTT_GW_DISCRETE_PLUGIN_TOPICID
#define CFG_KIT_TO_PUB_WRAPPED_TLM_TOPICID  KIT_TO_PUB_WRAPPED_TLM_TOPICID

#define CFG_CMD_PIPE_NAME            CMD_PIPE_NAME
#define CFG_CMD_PIPE_DEPTH           CMD_PIPE_DEPTH

#define CFG_TOPIC_PIPE_NAME          TOPIC_PIPE_NAME
#define CFG_TOPIC_PIPE_DEPTH         TOPIC_PIPE_DEPTH
#define CFG_TOPIC_PIPE_PEND_TIME     TOPIC_PIPE_PEND_TIME

#define CFG_MQTT_BROKER_PORT         MQTT_BROKER_PORT
#define CFG_MQTT_BROKER_ADDRESS      MQTT_BROKER_ADDRESS
#define CFG_MQTT_BROKER_USERNAME     MQTT_BROKER_USERNAME
#define CFG_MQTT_BROKER_PASSWORD     MQTT_BROKER_PASSWORD
#define CFG_MQTT_ENABLE_RECONNECT    MQTT_ENABLE_RECONNECT
#define CFG_MQTT_RECONNECT_PERIOD    MQTT_RECONNECT_PERIOD

#define CFG_MQTT_CLIENT_NAME         MQTT_CLIENT_NAME
#define CFG_MQTT_CLIENT_YIELD_TIME   MQTT_CLIENT_YIELD_TIME
#define CFG_MQTT_TOPIC_TBL_DEF_FILE  MQTT_TOPIC_TBL_DEF_FILE

#define CFG_CHILD_NAME               CHILD_NAME
#define CFG_CHILD_STACK_SIZE         CHILD_STACK_SIZE
#define CFG_CHILD_PRIORITY           CHILD_PRIORITY


#define APP_CONFIG(XX) \
   XX(APP_CFE_NAME,char*) \
   XX(APP_MAIN_PERF_ID,uint32) \
   XX(CHILD_TASK_PERF_ID,uint32) \
   XX(MQTT_GW_CMD_TOPICID,uint32) \
   XX(BC_SCH_2_SEC_TOPICID,uint32) \
   XX(MQTT_GW_HK_TLM_TOPICID,uint32) \
   XX(MQTT_GW_DISCRETE_PLUGIN_TOPICID,uint32) \
   XX(KIT_TO_PUB_WRAPPED_TLM_TOPICID,uint32) \
   XX(CMD_PIPE_NAME,char*) \
   XX(CMD_PIPE_DEPTH,uint32) \
   XX(TOPIC_PIPE_NAME,char*) \
   XX(TOPIC_PIPE_DEPTH,uint32) \
   XX(TOPIC_PIPE_PEND_TIME,uint32) \
   XX(MQTT_BROKER_PORT,uint32) \
   XX(MQTT_BROKER_ADDRESS,char*) \
   XX(MQTT_BROKER_USERNAME,char*) \
   XX(MQTT_BROKER_PASSWORD,char*) \
   XX(MQTT_ENABLE_RECONNECT,uint32) \
   XX(MQTT_RECONNECT_PERIOD,uint32) \
   XX(MQTT_CLIENT_NAME,char*) \
   XX(MQTT_CLIENT_YIELD_TIME,uint32) \
   XX(MQTT_TOPIC_TBL_DEF_FILE,char*) \
   XX(CHILD_NAME,char*) \
   XX(CHILD_STACK_SIZE,uint32) \
   XX(CHILD_PRIORITY,uint32)

DECLARE_ENUM(Config,APP_CONFIG)


/******************************************************************************
** Command Macros
** - Commands implmented by child task are annotated with a comment
** - Load/dump table definitions are placeholders for JSON table
*/

#define MQTT_GW_TBL_LOAD_CMD_FC   (CMDMGR_APP_START_FC +  0)
#define MQTT_GW_TBL_DUMP_CMD_FC   (CMDMGR_APP_START_FC +  1)


/******************************************************************************
** Event Macros
**
** Define the base event message IDs used by each object/component used by the
** application. There are no automated checks to ensure an ID range is not
** exceeded so it is the developer's responsibility to verify the ranges. 
*/

#define MQTT_GW_BASE_EID              (APP_C_FW_APP_BASE_EID +  0)
#define MQTT_MGR_BASE_EID             (APP_C_FW_APP_BASE_EID + 20)
#define MQTT_CLIENT_BASE_EID          (APP_C_FW_APP_BASE_EID + 40)
#define MSG_TRANS_BASE_EID            (APP_C_FW_APP_BASE_EID + 60)
#define MQTT_TOPIC_TBL_BASE_EID       (APP_C_FW_APP_BASE_EID + 80)

// Topic plugin macros are defined in mqtt_gw_topic_plugin.h and start at (APP_C_FW_APP_BASE_EID + 200)

/******************************************************************************
** MQTT Client
**
*/

#define MQTT_CLIENT_READ_BUF_LEN  1000 
#define MQTT_CLIENT_SEND_BUF_LEN  1000 
#define MQTT_CLIENT_TIMEOUT_MS    2000 

/******************************************************************************
** MQTT Topic CCSDS
**
** The CCSDS topic contains a CCSDS message as its payload so this length must
** be large enough to accomodate the largest CCSDS packet that will be sent as
** a MQTT payload.
*/

#define MQTT_TOPIC_SB_MSG_MAX_LEN  4096  //TODO - Replace with EDS definition

/******************************************************************************
** MQTT Topic Table
**
*/

#define MQTT_TOPIC_TBL_JSON_FILE_MAX_CHAR  5000
#define MQTT_TOPIC_TBL_NAME                "MQTT Topics"

#endif /* _app_cfg_ */
