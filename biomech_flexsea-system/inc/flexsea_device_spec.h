/*
 * flexsea_device_spec.h
 *
 *  Created on: Apr 24, 2018
 *      Author: Dephy Inc
 */

#ifndef FLEXSEA_SYSTEM_INC_FLEXSEA_DEVICE_SPEC_H_
#define FLEXSEA_SYSTEM_INC_FLEXSEA_DEVICE_SPEC_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "flexsea_dataformats.h"
#include "flexsea_sys_def.h"

/* Flexsea Device C Interface defined here - FlexseaDeviceSpec */
typedef struct FlexseaDeviceSpec_s {
	char* name;
	uint8_t numFields;

	const char** fieldLabels;

	const uint8_t* fieldTypes;
} FlexseaDeviceSpec;

/* Specs exist for the following devices */
#define NUM_DEVICE_TYPES 4



/* this array contains all the device specs */
extern FlexseaDeviceSpec deviceSpecs[NUM_DEVICE_TYPES];

/* defines and externs needed by plan only */
#ifdef BOARD_TYPE_FLEXSEA_PLAN
	#define MAX_CONNECTED_DEVICES 10
	extern FlexseaDeviceSpec connectedDeviceSpecs[MAX_CONNECTED_DEVICES];
	extern uint8_t* deviceData[MAX_CONNECTED_DEVICES];
	extern uint8_t fx_spec_numConnectedDevices;

	// convenience define for accessing deviceData
	// only works on little endian systems
	// TODO: actually implement a big_endian/little_endian define so this isn't a bug waiting to happen
	#ifdef BIG_ENDIAN
	#else
		#define DEVICESPEC_TYPE_BY_IDX(i) ( *(deviceData[i]) )
		#define DEVICESPEC_UUID_BY_IDX(i) ( *((uint16_t*)(deviceData[i] + 1)) )
	#endif
#else

	extern const uint16_t const* read_num_fields_active;
	extern const uint8_t const* const* read_device_active_field_pointers;
	extern const uint8_t const* read_device_active_field_lengths;
	void setActiveFieldsByMap(uint32_t *map);

#endif //BOARD_TYPE_FLEXSEA_PLAN

void addConnectedDevice(uint8_t devType, uint16_t devId);
void initializeDeviceSpecs();

extern const FlexseaDeviceSpec *fx_this_device_spec;
extern const uint8_t** _dev_data_pointers;
extern uint32_t *fx_dev_timestamp;

/* Related to max number of fields, should probably call it max num fields.. */
#define FX_BITMAP_WIDTH_C 3

extern uint32_t fx_active_bitmap[FX_BITMAP_WIDTH_C];

#ifdef __cplusplus
}
#endif

#endif /* FLEXSEA_SYSTEM_INC_FLEXSEA_DEVICE_SPEC_H_ */
