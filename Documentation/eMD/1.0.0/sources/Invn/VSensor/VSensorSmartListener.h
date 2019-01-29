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

/** @defgroup VSensorSmartListener VSensorSmartListener
 *  @brief    VSensorSmartListener object defintion
 *
 *  VSensorSmartListener arr VSensorListener that automatically perform decimation.
 *
 *	@ingroup VSensorFwk
 *  @{
 */

#ifndef _V_SENSOR_SMART_LISTNER_H_
#define _V_SENSOR_SMART_LISTNER_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>

#include "Invn/VSensor/VSensorListener.h"

/*
 * Forward declaration
 */
struct VSensorSmartListener;

/** @brief Prototype for user decimator function
 *
 *  Decimator function will be called before the registered VSensorListenerHandler.
 *  Only VSENSOR_EVENT_NEW_DATA are expected to be filtered out.
 *  The function shall return a integer >0 if the received data is expected to be passed to VSensorListenerHandler.
 *  In this case, decimator function implementation must provide a valid pointer to the data
 *  to be passed to the VSensorListenerHandler. This allows the decimator implementation to altered
 *  the received data (eg: to perform interpolation or update the timestamp value).
 *  The decimator must then provide a pointer to a static object of the same size of the of the original data.
 *  If original data don't need to be altered (eg: when performing decimation by dropping events),
 *  data pointer can simply be copied to new_data pointer.
 
 *  If returned value is 1, (updated) data is notify (user handler is called).
 *  If returned is >1, the decimator is called again with the same argument (this permits to duplicate data).
 *  If returned value is 0, data is drop.
 *
 *  @param[in] smartlistener handle to smart listener receiving the event
 *  @param[in] event         event type
 *  @param[in] data          event data
 *  @param[out] new_data     altered event data to notify (shall be a pointer to a static buffer or data pointer)
 *  @return 0 to drop the data 
 *          1 to new_data should be notify
 *          >1 to recall the decimator
 */
typedef int (*VSensorDecimatorCb)(struct VSensorSmartListener * smartlistener, int event,
		const void * data, const void ** new_data);

/** @brief VSensorSmartListener states
 */
typedef struct VSensorSmartListener {
	struct VSensorListener base;      /**< base VSensorListener object */
	VSensorListenerHandler handler;   /**< user event handler */
	VSensorDecimatorCb     decimator; /**< decimator process function */ 
} VSensorSmartListener;

/** @brief Getter to retrieve underlying VSensorListener handle
 *  @param[in] listener the VSensorSmartListener
 *  @return    VSensorListener handle
 */
static inline struct VSensorListener * VSensorSmartListener_getBase(VSensorSmartListener * listener)
{
	return &listener->base;
}

/** @brief Attach a VSensorSmartListener to a VSensor
 *  @param[in] listener         the VSensorSmartListener
 *  @param[in] vsensor          reference to the VSensor the listener connects to
 *  @param[in] handler          handler to be called upon new event send by the VSensor
 *  @param[in] arg              some cookie
 *  @param[in] decimator        decimator function
 *  @return	   0 on success, negative value on error (eg: invalid VSensor handle)
 */
int VSensorSmartListener_attach(VSensorSmartListener * listener, struct VSensor * vsensor,
		VSensorListenerHandler handler, void * arg, VSensorDecimatorCb decimator);

/*
 * Commonly used decimator
 */

/** @brief Report Interval Ratio (RIR) decimator
 *
 * This decimator compute the ration between requested and effectir RI to determine the number of sample to drop
 * The ERI/RI ratio is computed using unsigned integer. Value will be rounded down.
 *
 * The next VSENSOR_EVENT_NEW_DATA received after handling VSENSOR_EVENT_NEW_EFFECTIVE_RI will be forwarded.
 */
typedef struct VSensorSmartListenerRIR {
	struct VSensorSmartListener base; /**< base VSensorSmartListener object */
	uint32_t cnt;                     /**< internal sample counter */
	uint32_t rate;                    /**< RIR value */
} VSensorSmartListenerRIR;

/** @brief Helper function to retrieve base VSensorListener object
 *  @return handle to parent VSensorListener object
 */
static inline VSensorListener * VSensorSmartListenerRIR_getBase(VSensorSmartListenerRIR * l)
{
	return VSensorSmartListener_getBase(&l->base);
}

/** @brief Initialize RIR VSensorListener
 *  Will utltimately call VSensorListener_attach() with the same argument
 *  @param[in] listener the VSensorSmartListenerRIR
 *  @param[in] vsensor reference to the VSensor the listener connects to
 *  @param[in] handler handler to be called upon new event send by the VSensor
 *  @param[in] arg     cookie passed to listener object
 *  @return underlying VSensorListener_attach() return value
 */
int VSensorSmartListenerRIR_attach(struct VSensorSmartListenerRIR * listener, struct VSensor * vsensor,
		VSensorListenerHandler handler, void * arg);

/** @brief Decimator function called internaly by VSensorSmartListenerRIR object upon event
 *
 *  This function is exported only to reuse the internal logic in case a enhanced SmartListener need to be built.
 *  User does not need to call it explicitly when using VSensorSmartListenerRIR in a normal way.
 */
int VSensorSmartListenerRIR_decimator(struct VSensorSmartListener * smartlistener, int event,
		const void * data, const void ** new_data);

#ifdef __cplusplus
}
#endif

#endif /* _V_SENSOR_SMART_LISTNER_H_ */

/** @} **/
