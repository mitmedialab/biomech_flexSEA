
set(SRCS
	${SRCS}
	${CMAKE_CURRENT_SOURCE_DIR}/src/flexsea.c
	${CMAKE_CURRENT_SOURCE_DIR}/src/flexsea_buffers.c
	${CMAKE_CURRENT_SOURCE_DIR}/src/flexsea_circular_buffer.c
	${CMAKE_CURRENT_SOURCE_DIR}/src/flexsea_comm.c
	${CMAKE_CURRENT_SOURCE_DIR}/src/flexsea_comm_multi.c
	${CMAKE_CURRENT_SOURCE_DIR}/src/flexsea_interface.c
	${CMAKE_CURRENT_SOURCE_DIR}/src/flexsea_payload.c
	PARENT_SCOPE
)
