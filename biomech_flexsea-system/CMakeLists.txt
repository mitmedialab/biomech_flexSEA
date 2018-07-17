set(SRCS
	${SRCS}
	${CMAKE_CURRENT_SOURCE_DIR}/src/flexsea_system.c 
	${CMAKE_CURRENT_SOURCE_DIR}/src/flexsea_global_structs.c 
	${CMAKE_CURRENT_SOURCE_DIR}/src/flexsea_cmd_data.c 
	${CMAKE_CURRENT_SOURCE_DIR}/src/flexsea_cmd_external.c 
	${CMAKE_CURRENT_SOURCE_DIR}/src/flexsea_cmd_sensors.c 
	${CMAKE_CURRENT_SOURCE_DIR}/src/flexsea_cmd_calibration.c 
	${CMAKE_CURRENT_SOURCE_DIR}/src/flexsea_cmd_control_1.c 
	${CMAKE_CURRENT_SOURCE_DIR}/src/flexsea_cmd_control_2.c 
	${CMAKE_CURRENT_SOURCE_DIR}/src/flexsea_cmd_tools.c 
	${CMAKE_CURRENT_SOURCE_DIR}/src/flexsea_cmd_in_control.c 
	${CMAKE_CURRENT_SOURCE_DIR}/src/flexsea_cmd_stream.c
	${CMAKE_CURRENT_SOURCE_DIR}/src/flexsea_device_spec.c
	${CMAKE_CURRENT_SOURCE_DIR}/src/flexsea_cmd_sysdata.c
	PARENT_SCOPE
)

