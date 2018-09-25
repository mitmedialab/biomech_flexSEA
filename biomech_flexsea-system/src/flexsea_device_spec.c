#ifdef __cplusplus
extern "C" {
#endif

#include "flexsea_device_spec.h"
#include "flexsea_dataformats.h"
#include "flexsea_sys_def.h"
#include "stdlib.h"
#include "string.h"
#include "flexsea_user_structs.h"
#include "flexsea_board.h"
#include "user-mn-MIT-DLeg.h"
#include "user-mn-MIT-EMG.h"

#define IS_FIELD_HIGH(i, map) ( (map)[(i)/32] & (1 << ((i)%32)) )
#define SET_MAP_HIGH(i, map) ( (map)[(i)/32] |= (1 << ((i)%32)) )
#define SET_MAP_LOW(i, map) ( (map)[(i)/32] &= (~(1 << ((i)%32))) )

/** Interface specs */
FlexseaDeviceSpec fx_none_spec = {
		.numFields = 0,
		.fieldLabels = NULL,
		.fieldTypes = NULL
};

// STM 32 UUID location from manual
// https://ee-programming-notepad.blogspot.com/2017/06/reading-stm32f4-unique-device-id-from.html
// http://www.st.com/content/ccc/resource/technical/document/reference_manual/3d/6d/5a/66/b4/99/40/d4/DM00031020.pdf/files/DM00031020.pdf/jcr:content/translations/en.DM00031020.pdf



// FX_RIGID spec starts here
// -------------------------

#ifdef DEPHY
#define _rigid_numFields 36
#elif (defined DLEG_MULTIPACKET)
#define _rigid_numFields 43
#else
#define _rigid_numFields 28
#endif // DEPHY
																																	//NUMFIELDS RUNNING_TOTAL
const char* _rigid_fieldlabels[_rigid_numFields] = 		{"board id", 			"devId",											// METADATA			2 2
														"state_time",																// STATE TIME		1 3
														"accelx", 	"accely", 	"accelz", 	"gyrox", 	"gyroy", 	"gyroz",		// IMU				6 9

														"mot_ang", 		"mot_vel", 	"mot_acc", 										// MOTOR KIN		3 12
														"mot_cur", 		"mot_volt",													// MOTOR ELEC		2 14
														"batt_volt", 	"batt_curr",												// BATTERY			2 16
														"temp", 		"status",													// STATS			2 18
														"genvar_0", "genvar_1", "genvar_2", "genvar_3", "genvar_4", 				// GEN VARS			5 23
														"genvar_5", "genvar_6", "genvar_7", "genvar_8", "genvar_9", 				// GEN VARS			5 28

#ifdef DEPHY
														"ank_ang", "ank_vel", "ank_from_mot", "ank_torque",							// ANKLE			4 32
														"cur_stpt",	"step_energy", "walking_state", "gait_state" 					// CONTROLLER		4 36

#elif (defined DLEG_MULTIPACKET)
														"intJointAngleDegrees", "intJointVelDegrees", "intJointTorque",				// INT ACTUATOR		3 31
														"safetyFlag", "desiredJointAngleDeg", "desiredJointK", "desiredJointB"		// ACTUATOR			4 35
														"emg_0", "emg_1", "emg_2", "emg_3", "emg_4",								// EMG				5 40
														"emg_5", "emg_6", "emg_7"													// EMG				3 42

#endif
};

const uint8_t _rigid_field_formats[_rigid_numFields] =	{FORMAT_8U, 	FORMAT_8U,													// METADATA			2 2
														FORMAT_32U,																	// STATE TIME		1 3
														FORMAT_16S, FORMAT_16S, FORMAT_16S, FORMAT_16S, FORMAT_16S, FORMAT_16S ,	// IMU				6 9

														FORMAT_32S, FORMAT_32S, FORMAT_32S,											// MOTOR KIN		3 12
														FORMAT_32S, FORMAT_32S,														// MOTOR ELEC		2 14
														FORMAT_16U, FORMAT_16S,														// BATTERY			2 16
														FORMAT_8S, 	FORMAT_16U,														// STATS			2 18
														FORMAT_16S, FORMAT_16S, FORMAT_16S, FORMAT_16S, FORMAT_16S,					// GEN VARS			5 23
														FORMAT_16S, FORMAT_16S, FORMAT_16S, FORMAT_16S, FORMAT_16S,					// GEN VARS			5 28

#ifdef DEPHY
														FORMAT_16S, FORMAT_16S, FORMAT_16S,	FORMAT_16S,								// ANKLE			4 32
														FORMAT_32S, FORMAT_16S, FORMAT_8S, FORMAT_8S,								// CONTROLLER		4 36

#elif (defined DLEG_MULTIPACKET)

														FORMAT_16S, FORMAT_16S, FORMAT_16S,											// INT ACTUATOR		3 31
														FORMAT_8S, FORMAT_16S, FORMAT_16U, FORMAT_16U,								// ACTUATOR			4 35
														FORMAT_16S, FORMAT_16S, FORMAT_16S, FORMAT_16S, FORMAT_16S,					// EMG				5 40
														FORMAT_16S, FORMAT_16S, FORMAT_16S											// EMG				3 43
#endif
};

#if(defined BOARD_TYPE_FLEXSEA_MANAGE  && defined BOARD_SUBTYPE_RIGID)


#define	_dev_numFields _rigid_numFields

#define PTR2(x) (uint8_t*)&(x)
#define UIDPTR(o) (uint8_t*)(0x1FFF7A10U + o)
// only defined on boards not on plan

uint8_t* _rigid_field_pointers[_rigid_numFields] =	{	(uint8_t*) &board_id,	(uint8_t*) &((uint8_t*)(UID_BASE))[2],													// METADATA			2 2
														PTR2(rigid1.ctrl.timestamp),																// STATE TIME		1 3
														(uint8_t*)&rigid1.mn.accel.x, (uint8_t*)&rigid1.mn.accel.y, (uint8_t*)&rigid1.mn.accel.z,	// IMU				3 6
														(uint8_t*)&rigid1.mn.gyro.x, (uint8_t*)&rigid1.mn.gyro.y, (uint8_t*)&rigid1.mn.gyro.z,		// IMU 				3 9
														PTR2(rigid1.ex._enc_ang_), PTR2(rigid1.ex._enc_ang_vel_), PTR2(rigid1.ex.mot_acc), 			// MOTOR KIN		3 12
														PTR2(rigid1.ex.mot_current), PTR2(rigid1.ex.mot_volt),	 						 			// MOTOR ELEC		2 14
														PTR2(rigid1.re.vb), PTR2(rigid1.re.current), 												// BATTERY			2 16
														PTR2(rigid1.re.temp),  PTR2(rigid1.re.status),												// STATS			2 18
														(uint8_t*)(rigid1.mn.genVar),	(uint8_t*)(rigid1.mn.genVar+1),								// GEN VARS
														(uint8_t*)(rigid1.mn.genVar+2), (uint8_t*)(rigid1.mn.genVar+3),								// GEN VARS
														(uint8_t*)(rigid1.mn.genVar+4), (uint8_t*)(rigid1.mn.genVar+5),								// GEN VARS
														(uint8_t*)(rigid1.mn.genVar+6), (uint8_t*)(rigid1.mn.genVar+7),								// GEN VARS
														(uint8_t*)(rigid1.mn.genVar+8), (uint8_t*)(rigid1.mn.genVar+9),								// GEN VARS			10 28

#ifdef DEPHY
														PTR2(rigid1.ctrl._ank_ang_deg_), PTR2(rigid1.ctrl._ank_vel_), 								// ANKLE			2 30
														PTR2(rigid1.ctrl._ank_ang_from_mot_),  0,													// ANKLE			2 32
														PTR2(rigid1.ex.ctrl.current.setpoint_val), PTR2(rigid1.ctrl.step_energy),					// CONTROLLER		2 34
														PTR2(rigid1.ctrl.walkingState), PTR2(rigid1.ctrl.gaitState),								// CONTROLLER		2 36
#elif (defined DLEG_MULTIPACKET)
														PTR2(act1.intJointAngleDegrees), PTR2(act1.intJointVelDegrees), PTR2(act1.intJointTorque),	// INT ACTUATOR		3 31
														PTR2(act1.safetyFlag), PTR2(act1.desiredJointAngleDeg), PTR2(act1.desiredJointK) , PTR2(act1.desiredJointB),	// ACTUATOR			4 35
														PTR2(emg_data[0]), PTR2(emg_data[1]), PTR2(emg_data[2]), PTR2(emg_data[3]), PTR2(emg_data[4]),	// EMG				5 40
														PTR2(emg_data[5]), PTR2(emg_data[6]), PTR2(emg_data[7])										// EMG				3 42
#endif
};

#undef PTR2
#endif // BOARD_TYPE_FLEXSEA_MANAGE

FlexseaDeviceSpec fx_rigid_spec = {
		.numFields = _rigid_numFields,
		.fieldLabels = _rigid_fieldlabels,
		.fieldTypes = _rigid_field_formats
};
// -------------------------
// FX_RIGID spec ends here



	
// FX_EXECUTE spec starts here
// -------------------------
#define _execute_numFields 5								

															// type
const char* _execute_fieldlabels[_execute_numFields] = 		{"execute", 		"id", 	"accelx", 	"accely", 	"accelz"};
const uint8_t _execute_field_formats[_execute_numFields] =	{FORMAT_8U, 	FORMAT_16U, FORMAT_16S, FORMAT_16S, FORMAT_16S };

#if ( defined BOARD_TYPE_FLEXSEA_EXECUTE)

#define	_dev_numFields _execute_numFields
// only defined on boards not on plan
uint8_t* _execute_field_pointers[_execute_numFields] =	{	(uint8_t*)0, (uint8_t*)0, \
														(uint8_t*)&exec1.accel.x, (uint8_t*)&exec1.accel.y, (uint8_t*)&exec1.accel.z };

#endif // BOARD_TYPE_FLEXSEA_EXECUTE

FlexseaDeviceSpec fx_execute_spec = {
		.numFields = _execute_numFields,
		.fieldLabels = _execute_fieldlabels,
		.fieldTypes = _execute_field_formats
};
// -------------------------
// FX_EXECUTE spec ends here


// FX_MANAGE spec starts here
// -------------------------


	
#define _manage_numFields 4									// type				
const char* _manage_fieldlabels[_manage_numFields] = 		{"manage", 		"id", 		"accelx", 	"accely"};
const uint8_t _manage_field_formats[_manage_numFields] =	{FORMAT_8U, 	FORMAT_16U, FORMAT_16S, FORMAT_16S };

#if( defined BOARD_TYPE_FLEXSEA_MANAGE && !(defined BOARD_SUBTYPE_RIGID))

#define	_dev_numFields _manage_numFields
// only defined on boards not on plan
uint8_t* _manage_field_pointers[_manage_numFields] =	{	(uint8_t*)0, (uint8_t*)0, \
														(uint8_t*)&manag1.accel.x, (uint8_t*)&manag1.accel.y };

#endif // BOARD_TYPE_FLEXSEA_MANAGE

FlexseaDeviceSpec fx_manage_spec = {
		.numFields = _manage_numFields,
		.fieldLabels = _manage_fieldlabels,
		.fieldTypes = _manage_field_formats
};
// -------------------------
// FX_MANAGE spec ends here



#if(defined BOARD_TYPE_FLEXSEA_MANAGE)

uint32_t *fx_dev_timestamp = &rigid1.ctrl.timestamp;
const FlexseaDeviceSpec *fx_this_device_spec = &fx_rigid_spec;
const uint8_t ** _dev_data_pointers = (const uint8_t **) _rigid_field_pointers;
const uint8_t * _dev_field_formats = _rigid_field_formats;

#elif(defined BOARD_TYPE_FLEXSEA_PLAN)
const FlexseaDeviceSpec *fx_this_device_spec = &fx_none_spec;
uint32_t fx_empty_timestamp;
uint32_t *fx_dev_timestamp = &fx_empty_timestamp;
const uint8_t** _dev_data_pointers = NULL;
	
#elif(defined BOARD_TYPE_FLEXSEA_EXECUTE)
	
const FlexseaDeviceSpec *fx_this_device_spec = &fx_execute_spec;
const uint8_t** _dev_data_pointers = (const uint8_t**)_execute_field_pointers;
const uint8_t * _dev_field_formats = _execute_field_formats;

#endif

// initialization goes in payload_ptr initialization which is a hack :(
FlexseaDeviceSpec deviceSpecs[NUM_DEVICE_TYPES];

#ifndef BOARD_TYPE_FLEXSEA_PLAN

uint32_t fx_active_bitmap[FX_BITMAP_WIDTH_C];
uint16_t fx_num_fields_active = 0;
const uint8_t *_device_active_field_pointers[_dev_numFields];
uint8_t _device_active_field_lengths[_dev_numFields];

const uint16_t const* read_num_fields_active = &fx_num_fields_active;
const uint8_t const* const* read_device_active_field_pointers = (const uint8_t const* const*)_device_active_field_pointers;
const uint8_t const* read_device_active_field_lengths =_device_active_field_lengths;

void setActiveFieldsByMap(uint32_t *map)
{
	int i, j=0;
	for(i=0;i<_dev_numFields; ++i)
	{
		if(IS_FIELD_HIGH(i, map))
		{
			_device_active_field_pointers[j] = _dev_data_pointers[i];
			_device_active_field_lengths[j] = FORMAT_SIZE_MAP[_dev_field_formats[i]];
			++j;
		}
	}

	fx_num_fields_active = j;
}

#endif

#ifdef BOARD_TYPE_FLEXSEA_PLAN
FlexseaDeviceSpec connectedDeviceSpecs[MAX_CONNECTED_DEVICES];
uint8_t fx_spec_numConnectedDevices = 0;
uint8_t* deviceData[MAX_CONNECTED_DEVICES];

void addConnectedDevice(uint8_t devType, uint16_t devId)
{
	if(fx_spec_numConnectedDevices == MAX_CONNECTED_DEVICES)
	{
		return;
	}

	FlexseaDeviceSpec *ds = &connectedDeviceSpecs[fx_spec_numConnectedDevices];
	switch(devType) {
	case(FX_RIGID):
		*ds = fx_rigid_spec;
		break;
	case(FX_EXECUTE):
		*ds = fx_execute_spec;
		break;
	case(FX_MANAGE):
		*ds = fx_manage_spec;
		break;

	}

	uint16_t i, totalSpaceNeeded = 0;
	for(i=0; i<ds->numFields;i++)
		totalSpaceNeeded += FORMAT_SIZE_MAP[ds->fieldTypes[i]];

	if(totalSpaceNeeded)
	{
		if(deviceData[fx_spec_numConnectedDevices])
			deviceData[fx_spec_numConnectedDevices] = realloc(deviceData[fx_spec_numConnectedDevices], totalSpaceNeeded);
		else
			deviceData[fx_spec_numConnectedDevices] = malloc(totalSpaceNeeded);

		if(deviceData[fx_spec_numConnectedDevices])
			memset(deviceData[fx_spec_numConnectedDevices], 0, totalSpaceNeeded);
	}

	if(deviceData[fx_spec_numConnectedDevices])
	{
		// set the device id and type in the data vector
		deviceData[fx_spec_numConnectedDevices][0] = devType;
		uint16_t *devIdPtr = (uint16_t*)(deviceData[fx_spec_numConnectedDevices]+1);
		(*devIdPtr) = devId;

		fx_spec_numConnectedDevices++;
	}
}

void initializeDeviceSpecs()
{
	deviceSpecs[FX_NONE] = fx_none_spec;
	deviceSpecs[FX_RIGID] = fx_rigid_spec;
	deviceSpecs[FX_EXECUTE] = fx_execute_spec;
	deviceSpecs[FX_MANAGE] = fx_manage_spec;

	int i;

	for(i=0;i<MAX_CONNECTED_DEVICES;i++)
	{
		deviceData[i] = 0;
	}
}

#else
void addConnectedDevice(uint8_t devType, uint16_t devId) {(void)devType; (void) devId;}
void initializeDeviceSpecs() {}
#endif

#ifdef __cplusplus
}
#endif
