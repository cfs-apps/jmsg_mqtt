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
**    Define platform configurations for the JMSG MQTT Gateway application
**
**  Notes:
**    1. The MQTT_GW_TOPIC_LEN definition is based on KIT_TO's definition
**       of the maximum SB message that can be wrapped. The wrapped messages
**       are expected to come from MQTT_GW.
**
*/

#ifndef _jmsg_mqtt_platform_cfg_
#define _jmsg_mqtt_platform_cfg_

/*
** Includes
*/

#include "jmsg_mqtt_mission_cfg.h"

/******************************************************************************
** Platform Deployment Configurations
*/

#define JMSG_MQTT_APP_PLATFORM_REV   0
#define JMSG_MQTT_APP_INI_FILENAME   "/cf/jmsg_mqtt_ini.json"


#endif /* _jmsg_mqtt_platform_cfg_ */
