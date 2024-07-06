project(CFS_JMSG_MQTT C)

include_directories(fsw/mission_inc)
include_directories(fsw/platform_inc)
include_directories(fsw/src)
include_directories(${app_c_fw_MISSION_DIR}/fsw/app_inc)
include_directories(${app_c_fw_MISSION_DIR}/fsw/platform_inc)
include_directories(${app_c_fw_MISSION_DIR}/fsw/mission_inc)
include_directories(${jmsg_lib_MISSION_DIR}/fsw/platform_inc)
include_directories(${jmsg_lib_MISSION_DIR}/fsw/mission_inc)
include_directories(${jmsg_lib_MISSION_DIR}/fsw/src)
include_directories(${mqtt_lib_MISSION_DIR}/fsw/public_inc)

aux_source_directory(fsw/src APP_SRC_FILES)

# Create the app module
add_cfe_app(jmsg_mqtt ${APP_SRC_FILES})

