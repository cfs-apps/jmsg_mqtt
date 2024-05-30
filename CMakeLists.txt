project(CFS_MQTT_GW C)

# SC_SIM is included to serve as an example topic plugin
include_directories(fsw/mission_inc)
include_directories(fsw/platform_inc)
include_directories(fsw/src)
include_directories(fsw/topic_plugins)
include_directories(${app_c_fw_MISSION_DIR}/fsw/app_inc)
include_directories(${app_c_fw_MISSION_DIR}/fsw/platform_inc)
include_directories(${app_c_fw_MISSION_DIR}/fsw/mission_inc)
include_directories(${mqtt_lib_MISSION_DIR}/fsw/public_inc)
include_directories(${sc_sim_MISSION_DIR}/fsw/mqtt)

aux_source_directory(fsw/src APP_SRC_FILES)
aux_source_directory(fsw/topic_plugins APP_SRC_FILES)
aux_source_directory(${sc_sim_MISSION_DIR}/fsw/mqtt APP_SRC_FILES)

# Create the app module
add_cfe_app(mqtt_gw ${APP_SRC_FILES})

