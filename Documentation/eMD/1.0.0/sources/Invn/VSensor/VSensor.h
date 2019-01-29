/*
 * ________________________________________________________________________________________________________
 * Copyright (c) 2015-2016 InvenSense Inc. All rights reserved.
 *
 * This software, related documentation and any modifications thereto (collectively “Software”) is subject
 * to InvenSense and its licensors' intellectual property rights under U.S. and international copyright
 * and other intellectual property rights laws.
 *
 * InvenSense and its licensors retain all intellectual property and proprietary rights in and to the Software
 * and any use, reproduction, disclosure or distribution of the Software without an express license agreement
 * from InvenSense is strictly prohibited.
 *
 * EXCEPT AS OTHERWISE PROVIDED IN A LICENSE AGREEMENT BETWEEN THE PARTIES, THE SOFTWARE IS
 * PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED
 * TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.
 * EXCEPT AS OTHERWISE PROVIDED IN A LICENSE AGREEMENT BETWEEN THE PARTIES, IN NO EVENT SHALL
 * INVENSENSE BE LIABLE FOR ANY DIRECT, SPECIAL, INDIRECT, INCIDENTAL, OR CONSEQUENTIAL DAMAGES, OR ANY
 * DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT,
 * NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE
 * OF THE SOFTWARE.
 * ________________________________________________________________________________________________________
 */

/** @defgroup VSensor VSensor
 *  @brief    VSensor object definition
 *	
 *  VSensor object, standing for Virtual Sensor, is a virtual object representing
 *  a data provider, or publisher. It provides data to VSensorListener object.
 *
 *  VSensor’s implementation can publish data coming from an HW resource
 *  (HW sensors) or from an algorithm.
 *  VSensor’s implementation can instantiate its own set of VSensorListeners to
 *  retrieve input data for its own purpose.
 *
 *	@ingroup VSensorFwk
 *  @{
 */

#ifndef _V_SENSOR_H_
#define _V_SENSOR_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#include "Invn/EmbUtils/InvList.h"

#include "Invn/VSensor/VSensorType.h"
#include "Invn/VSensor/VSensorSmartListener.h"
#include "Invn/VSensor/VSensorData.h"
#include "Invn/VSensor/VSensorConfig.h"
#include "Invn/VSensor/VSensorEvent.h"

/** @brief VSensor attributes definition
 */
typedef struct VSensorAttr {
	uint32_t    min_ri;     /**< minimum report interval in us */
	/*
	 * Don't use it for now...
	 */
	// uint32_t    max_ri;  /**< maximum report interval in us */
} VSensorAttr;

#ifndef VSENSOR_ATTR_MIN_RI_DEFAULT
  /** @brief Default value for minimum RI attribute
   */
  #define VSENSOR_ATTR_MIN_RI_DEFAULT 1000 /* 1 ms */
#endif

#ifndef VSENSOR_ATTR_MAX_RI_DEFAULT
  /** @brief Default value for maximum RI attribute
   */
  #define VSENSOR_ATTR_MAX_RI_DEFAULT 1000000 /* 1s */
#endif

/** @brief VSensor status flags
 */
enum VSensorStatusFlag {
	VSENSOR_STATUS_FLAG_SETUP_OK 	= 0x01, /**< flag indicating sucessful setup */
	VSENSOR_STATUS_FLAG_SETUP_ERROR = 0x02  /**< flag indicating setup faillure */
};

/** @brief Virtual method in charge of handling VSensor events
 *
 *  Automatically called when:
 *  - A listener subscribe/unsubscribe to/from the VSensor
 *  - Requested Report Interval (RI) (aka output data period) of published data event has changed
 *  - Requested Maximum Report Latency (MRL) (aka Batch Mode timeout) allowed to report data has changed
 *  - ...
 *
 * @param[in] vsensor VSensor that receives the event
 * @param[in] event   event type
 * @param[in] data    data associated if any
 * return     0 on sucess, negative value on error
 */
typedef int (*VSensorUpdateCb)(struct VSensor * vsensor, int event, void * data);

/** @brief VSensor states
 */
typedef struct VSensor {
	VSensorUpdateCb update; /**< virtual update function */
	InvList 	    list;   /**< pointer to list of subscribers to current publisher*/
	int16_t         type;   /**< type of data published by the sensor */
	uint16_t        sdata;  /**< size of data published by the sensor */
	uint32_t 	    eri;    /**< effective report interval */
	uint32_t 	    emrl;   /**< effective minimum report latency */
	uint32_t	    status; /**< status flag (to track possible error state of VSensor */
	VSensorAttr     attr;   /**< VSensor attributes */
	void *          arg;    /**< user argument specific to one subscriber filled in at init time */
} VSensor;

/** @brief Helper macro to statically initialize VSensor states
 */
#define VSENSOR_STATIC_INIT(update, type, sdata, arg) \
		{ update, {0}, type, sdata, 0, 0, 0, \
			{VSENSOR_ATTR_MIN_RI_DEFAULT/*, VSENSOR_ATTR_MAX_RI_DEFAULT*/ }, \
			arg }

/** @brief Initialize VSensor states
 *  @param[in] vsensor VSensor to initialize
 *  @param[in] update  VSensor update method
 *  @param[in] type    Type of the VSensor (ie: type of data published)
 *  @param[in] sdata   Size of the data published by the VSensor
 *  @param[in] attr    VSensor's attribue (if 0, default value are used)
 *  @param[in] arg     A cookie pointer
 *  @return    none
 */
void VSensor_init(VSensor * vsensor, VSensorUpdateCb update, int type,
		unsigned sdata, const VSensorAttr * attr, void * arg);

/** @brief Getter function to retrieve the size of the data published by the VSensor
 *  @param[in] vsensor the VSensor
 *  @return size of VSensor's data
 */
static inline unsigned VSensor_getDataSize(VSensor * vsensor)
{
	return vsensor->sdata;
}

/** @brief Getter function to retrieve the type of the data published by the VSensor
 *  @param[in] vsensor the VSensor
 *  @return type of VSensor's data
 */
static inline int VSensor_getType(VSensor * vsensor)
{
	return vsensor->type;
}

/** @brief Set attributes for the VSensor
 *  @param[in] vsensor the VSensor
 *  @param[in] attr   attributes
 *  @return    none
 */
static inline void VSensor_setAttr(VSensor * vsensor, const VSensorAttr * attr)
{
	vsensor->attr = *attr;
}

/** @brief Get attributes of a VSensor
 *  @param[in] vsensor the VSensor
 *  @return reference to VSensor's attributes
 */
static inline const VSensorAttr * VSensor_getAttr(const VSensor * vsensor)
{
	return &vsensor->attr;
}


/** @brief Helper method to call virtual update() method for a VSensor
 *
 *  Should not be called except you know what you're doing.
 *
 *  @param[in] vsensor the VSensor
 *  @param[in] event   event type
 *  @param[in] data    event data
 *  @return    0 on sucess, negative value otherwise
 */
static inline int VSensor_update(VSensor * vsensor, int event, void * data)
{
	return vsensor->update(vsensor, event, data);
}

/** @brief Notify active listeners of a VSensor of a new event
 *  @param[in] vsensor the VSensor
 *  @param[in] event   event type
 *  @param[in] data    event data
 *  @return    none
 */
void VSensor_notifyEvent(VSensor * vsensor, int event, const void * data);

/** @brief Notify active listeners of a VSensor of a new data availibitly
 *  @param[in] vsensor the VSensor
 *  @param[in] data    new VSensor data
 *  @return    none
 */
static inline void VSensor_notifyData(VSensor * vsensor, const void * data)
{
	VSensor_notifyEvent(vsensor, VSENSOR_EVENT_NEW_DATA, data);
}

/** @brief Retrieve last data from a VSsensor
 *  @param[in]  vsensor the VSensor
 *  @param[out] data    last VSensor data
 *                      data must point to a buffer of the size of the VSensor data
 *  @return     0 on sucess and output correctly set, negative value otherwise
 */
static inline int VSensor_getData(VSensor * vsensor, void * data)
{
	return VSensor_update(vsensor, VSENSOR_EVENT_GET_DATA, data);
}

/** @brief Send config data a VSsensor
 *  @param[in] vsensor the VSensor
 *  @param[in] data    config data for the VSensor
 *                     data must point to a VSensorConfig object
 *  @return    0 on sucess, negative value otherwise
 */
static inline int VSensor_setConfig(VSensor * vsensor, const void * data)
{
	return VSensor_update(vsensor, VSENSOR_EVENT_SET_CONFIG, (void *)data);
}

/** @brief Reetrive config data from VSsensor
 *  @param[in]  vsensor the VSensor
 *  @param[out] data    config data
 *                      data must point to a VSensorConfig object
 *  @return     0 on sucess and output correctly set, negative value otherwise
 */
static inline int VSensor_getConfig(VSensor * vsensor, void * data)
{
	return VSensor_update(vsensor, VSENSOR_EVENT_GET_CONFIG, data);
}

/** @brief Notify active listeners of a VSensor updated configuration
 *  @param[in]  vsensor the VSensor
 *  @param[out] data    config data (expected to point to a VSensorConfig object)
 *  @return none
 */
static inline void VSensor_notifyConfig(VSensor * vsensor, const void * data)
{
	VSensor_notifyEvent(vsensor, VSENSOR_EVENT_NEW_CONFIG, data);
}

#ifdef __cplusplus
}
#endif

#endif /* _V_SENSOR_H_ */

/** @} **/
