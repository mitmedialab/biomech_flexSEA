/*
 * flexsea_device_spec.h
 *
 *  Created on: Apr 24, 2018
 *      Author: Dephy Inc
 */

#ifndef FLEXSEA_SYSTEM_INC_FLEXSEA_DEVICE_SPEC_DEFS_H_
#define FLEXSEA_SYSTEM_INC_FLEXSEA_DEVICE_SPEC_DEFS_H_

#ifdef __cplusplus
extern "C" {
#endif




#ifdef DLEG_MULTIPACKET
	#define _rigid_mn_numFields 45
#else
	#define _rigid_mn_numFields 30
#endif

#define	_dev_numFields _rigid_mn_numFields

#ifdef __cplusplus
}
#endif

#endif /* FLEXSEA_SYSTEM_INC_FLEXSEA_DEVICE_SPEC_DEFS_H_ */
