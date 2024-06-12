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
**    Configure JMSG topic plugins for the JMSG_MQTT app
**
**  Notes:
**    1. This file encapsulates all of the topic plugin configurations required
**       by the JMSG_MQTT. It should be the only file that is changed to add and
**       topic plugins used by JMSG_MQTT.
**    2. Each JMSG_MQTT plugin requires the following steps:
**       A. In the "Topic Plugin" section include the plugin header and define
**          data
**       B. In the JMSG_MQTT_TOPIC_PLUGIN_Constructor(), construct the plugin
**          and subscribe to the message  
**    3. JMSG_LIB plugins used by JMSG_MQTT only need to be subscribed to because
**       they've already been constructed.
*/

/*
** Include Files:
*/

#include "jmsg_mqtt_topic_plugin.h"

/*******************/
/** Topic Plugins **/
/*******************/

#include "mqtt_topic_discrete.h"
static MQTT_TOPIC_DISCRETE_Class_t  MqttTopicDiscrete;

#include "mqtt_topic_rate.h"
static MQTT_TOPIC_RATE_Class_t  MqttTopicRate;


/******************************************************************************
** Function: JMSG_MQTT_TOPIC_PLUGIN_Constructor
**
*/
void JMSG_MQTT_TOPIC_PLUGIN_Constructor(JMSG_TOPIC_TBL_ConfigSubscription_t ConfigSubscription)
{

   // The configure subscription callback must be registered prior to topic message subscriptions
   JMSG_TOPIC_TBL_RegisterConfigSubscriptionCallback(ConfigSubscription);

   // Construct JMSG_MQTT plugins
   MQTT_TOPIC_DISCRETE_Constructor(&MqttTopicDiscrete,JMSG_USR_TopicPlugin_USR_1);
   MQTT_TOPIC_RATE_Constructor(&MqttTopicRate, JMSG_USR_TopicPlugin_USR_2);

   // Register all plugins used by JMSG_MQTT
   JMSG_TOPIC_TBL_SubscribeToTopicMsg(JMSG_USR_TopicPlugin_CMD,JMSG_TOPIC_TBL_SUB_TO_ROLE);
   JMSG_TOPIC_TBL_SubscribeToTopicMsg(JMSG_USR_TopicPlugin_TLM,JMSG_TOPIC_TBL_SUB_TO_ROLE);
   JMSG_TOPIC_TBL_SubscribeToTopicMsg(JMSG_USR_TopicPlugin_TEST,JMSG_TOPIC_TBL_SUB_TO_ROLE);
  
   JMSG_TOPIC_TBL_SubscribeToTopicMsg(JMSG_MQTT_PLUGIN_TopicPlugin_Discrete,JMSG_TOPIC_TBL_SUB_TO_ROLE);
   JMSG_TOPIC_TBL_SubscribeToTopicMsg(JMSG_MQTT_PLUGIN_TopicPlugin_Rate,JMSG_TOPIC_TBL_SUB_TO_ROLE);


} /* JMSG_TOPIC_PLUGIN_Constructor() */
