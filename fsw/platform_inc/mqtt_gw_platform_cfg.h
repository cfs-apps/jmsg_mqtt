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
**    Define platform configurations for the OSK MQTT Gateway application
**
**  Notes:
**    None
**
**  References:
**    1. OpenSatKit Object-based Application Developer's Guide.
**    2. cFS Application Developer's Guide.
**
*/

#ifndef _mqtt_gw_platform_cfg_
#define _mqtt_gw_platform_cfg_

/*
** Includes
*/

#include "mqtt_gw_mission_cfg.h"

/******************************************************************************
** Platform Deployment Configurations
*/

#define MQTT_GW_PLATFORM_REV   0
#define MQTT_GW_INI_FILENAME   "/cf/mqtt_gw_ini.json"


/******************************************************************************
** These will be in a spec file and the toolchain will create these
** definitions.
*/

/*
** mqtt topic string length
*/
#define MQTT_GW_TOPIC_LEN          100


#endif /* _mqtt_gw_platform_cfg_ */
