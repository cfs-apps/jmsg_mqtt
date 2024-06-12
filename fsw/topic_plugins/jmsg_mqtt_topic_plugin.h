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
**  Purpose:
**    Define public interface for jmsg_mqtt_topic_plugin.c
**
**  Notes:
**    1. This module should only export a single constructor function and not
**       define any data because jmsg_mqtt_topic_plugin.c encapsulates all 
**       topic plugins for the jmsg_mqtt app. See jmsg_mqtt_topic_plugin.c file
**       prologue for details.
**
*/

#ifndef _jmsg_mqtt_topic_plugins_
#define _jmsg_mqtt_topic_plugins_


/*
** Include Files:
*/

#include "jmsg_topic_tbl.h"


/************************/
/** Exported Functions **/
/************************/


/******************************************************************************
** Function: JMSG_MQTT_TOPIC_PLUGIN_Constructor
**
** Construct and register topic plugins for the JMSG_MQTT app
**
*/
void JMSG_MQTT_TOPIC_PLUGIN_Constructor(JMSG_TOPIC_TBL_ConfigSubscription_t ConfigSubscription);


#endif /* _jmsg_mqtt_topic_plugins_ */
