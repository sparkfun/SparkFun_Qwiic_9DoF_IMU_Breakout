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

/** @defgroup VSensorListener VSensorListener
 *  @brief    VSensorListener object defintion
 *
 *  VSensorListener, standing for Virtual Sensor Listener, is the object that
 *  allows controlling of a VSensor and receiving data from it.
 *
 *  Data and other events will be received by means of an event callback,
 *  in which the VSensorListener can process-it.
 *
 *	@ingroup VSensorFwk
 *  @{
 */

#ifndef _V_SENSOR_LISTNER_H_
#define _V_SENSOR_LISTNER_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#include "Invn/EmbUtils/InvList.h"

/*
 * Forward declaration
 */
struct VSensor;
struct VSensorListener;

/** @brief Event handler callback for VSensorListener
 *
 *  Called by the VSensor the listener is attach to and if active.
 *
 *  @param[in] listener listener that received the event
 *  @param[in] event    event type received
 *  @param[in] data     data associated with the event if any
 *  @return    none
 */
typedef void (*VSensorListenerHandler)(struct VSensorListener * listener,
		int event, const void * data);

/** @brief VSensorListener states
 */
typedef struct VSensorListener {
	InvList                next;    /**< pointer to next listener */
	VSensorListenerHandler handler; /**< event handler */
	struct VSensor *       vsensor; /**< reference to VSensor a listener is attach to */
	uint32_t               ri;      /**< requested report interval */
	uint32_t               mrl;     /**< requested minimum report latency */
	void *                 arg;     /**< some cookie */
} VSensorListener;

/** @brief Getter to retrieve the VSensor reference a listener is attach to
 *  @param[in] listener the VSensorListener
 *  @return    VSensor handle
 */
static inline struct VSensor *VSensorListener_getVSensor(VSensorListener * listener)
{
	return listener->vsensor;
}

/** @brief Attach a VSensorListener to a VSensor
 *  @param[in] listener the VSensorListener
 *  @param[in] vsensor reference to the VSensor the listener connects to
 *  @param[in] handler handler to be called upon new event send by the VSensor
 *  @param[in] arg     some cookie
 *  @return	   0 on success, negative value on error (eg: invalid VSensor handle)
 */
int VSensorListener_attach(VSensorListener * listener, struct VSensor * vsensor,
		VSensorListenerHandler handler, void * arg);

/** @brief Update listener requested RI
 *
 *  If the listener is currently subscribed to the VSensor, a NEW_REQUESTED_RI event
 *  will be sent to VSensor (with a value corresponding to the minimum of
 *  all requested RI of all active listener) and a NEW_EFFECTIVE_RI event will
 *  be sent to the listener.
 *
 *  @param[in] listener the VSensorListener
 *	@param[in] ri       requested report interval for DATA events
 *	@return none
 */
void VSensorListener_setRi(VSensorListener *listener, uint32_t ri);

/** @brief Updates a listener MRL field and requests the publisher it subscribed to to reconfigure itself
 *
 *  If the listener is currently subscribed to the VSensor, a NEW_REQUESTED_MRL event
 *  will be sent to VSensor (with a value corresponding to the minimum of
 *  all requested MRL of all active listener) and a NEW_EFFECTIVE_MRL event will
 *  be sent to the listener.
 *
 *  @param[in] listener the VSensorListener
 *  @param[in] mrl      request MRL for NEW_DATA events
 *  @return none
 */
void VSensorListener_setMrl(VSensorListener *listener, uint32_t mrl);

/** @brief Subscribe to the VSensor
 *
 *  Start receiving events from a VSensor
 *
 *  @param[in] listener the VSensorListener
 *	@return none
 */
void VSensorListener_enable(VSensorListener *listener);

/** @brief Unsubscribe from he VSensor
 *
 *  Stop receiving event from a VSensor
 *
 *  @return none
 */
void VSensorListener_disable(VSensorListener *listener);

#ifdef __cplusplus
}
#endif

#endif /* _V_SENSOR_LISTNER_H_ */

/** @} **/
