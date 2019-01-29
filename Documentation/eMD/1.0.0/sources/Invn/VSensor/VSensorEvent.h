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

/** @defgroup 	VSensorEvent	VSensorEvent
 *  @brief 		Events definition for VSensor and VSensorListener
 *	@ingroup VSensorFwk
 *  @{
 */

#ifndef _V_SENSOR_EVENT_H_
#define _V_SENSOR_EVENT_H_

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Standard events to be handled by a VSensor
 */

/** @brief Raised once upon first attach of a listener to a VSensor
 *
 *  Expected event data: none
 *
 *  When receiving this event, a VSensor is expected to:
 *   - Reset its internal states
 *   - Attach to other VSensor through listeners (if any)
 *   - Setup HW ressources (if needed)
 *
 *   VSensor musst return 0 on sucess, negative value otherwise
 */
#define VSENSOR_EVENT_SETUP             0

/** @brief Raised when the frist listener enables a VSensor
 *
 *  Expected event data: none
 *
 *  When receiving this event, a VSensor is expected to:
 *   - Enable listeners (if any)
 *   - Start HW ressources / data acquisition (if needed)
 *   - Start algorithms (if any)
 */
#define VSENSOR_EVENT_SUBSCRIBE         2

/** @brief Raised when the last listener disables a VSensor
 *
 *  Expected event data: none
 *
 *  When receiving this event, a VSensor is expected to:
 *   - Disable listeners (if any)
 *   - Stop HW ressources / data acquisition (if needed)
 *   - Stop algorithms (if any)
 */
#define	VSENSOR_EVENT_UNSUBSCRIBE       3

/** @brief Raised when requested RI has changed
 *
 *  Expected event data: new requested RI value (passed as a uint32_t *)
 *
 *  When receiving this event, a VSensor is expected to:
 *   - Reconfigure listeners RI (if any)
 *   - Reconfigure HW ressources (timers, ODRs, ...) (if needed)
 *   - Reconfigure algorithms (if any)
 *   - Overwwrite its .ri field with effective applied RI
 */
#define	VSENSOR_EVENT_NEW_REQUESTED_RI  4

/** @brief Raised when requested MRL has changed
 *
 *  Expected event data: new requested MRL value (passed as a uint32_t *)
 *
 *  When receiving this event, a VSensor is expected to:
 *   - Reconfigure listeners RI (if any)
 *   - Reconfigure HW ressources (timers, FIFOs, ...) (if needed)
 *   - Reconfigure algorithms (if any)
 *   - Overwwrite its .mrl field with effective applied MRL
 *
 *   VSensor should return 0 on sucess, negative value otherwise
 */
#define	VSENSOR_EVENT_NEW_REQUESTED_MRL 5

/** @brief Request to apply configuration
 *
 *  Expected event data: Configuration data (passed as VSensorConfig *)
 *
 *  When receiving this event, a VSensor is expected to:
 *   - Reconfigure algorithms (if any)
 *   - Forward configuration request to another VSensor (if needed)
 *   - Reconfigure HW ressources (if any)
 *
 *   VSensor should return 0 on sucess, negative value otherwise
 */
#define VSENSOR_EVENT_SET_CONFIG        6

/** @brief Request to return configuration
 *
 *  Expected event data: Configuration data buffer (passed as VSensorConfig *)
 *
 *  When receiving this event, a VSensor is expected to:
 *   - Return configuration value
 *   - Forward configuration request to another VSensor (if needed)
 *
 *   VSensor should return 0 on sucess (if output buffer was set), negative value otherwise
 */
#define VSENSOR_EVENT_GET_CONFIG        7

/** @brief Request to return last known data
 *
 *  Expected event data: VSensor data buffer (passed as corresponding VSensorData *)
 *
 *  When receiving this event, a VSensor is expected to:
 *   - Return last known data if saved
 *
 *   VSensor should return 0 on sucess (if output buffer was set), negative value otherwise
 */
#define VSENSOR_EVENT_GET_DATA          8

/** @brief Request to empty all internal data buffer
 *
 *  Expected event data: none
 *
 *  When receiving this event, a VSensor is expected to:
 *   - Flush its internal data buffer or HW FIFOs if any
 *   - Notify NEW_DATA event for each buffered data
 *   - Notify FLUSH_COMPLETE event to its listener once this is done
 *
 *   VSensor should return 0 on sucess, negative value otherwise
 */
#define VSENSOR_EVENT_FLUSH_DATA        9


/*
 * Standard events that can be send to a VSensorListener
 */

/** @brief Raised when a VSensor notifies of new data
 *
 *  Expected event data: VSensor data (passed as corresponding VSensorData *)
 */
#define VSENSOR_EVENT_NEW_DATA          1

/** @brief Raised after a VSensorListener has successfully subscribed to a Vsensor
 *
 *  @warning automatically handle by the framework after calling VSensorListener_enable()
 *
 *  Expected event data: none
 */
#define VSENSOR_EVENT_HAS_SUBSCRIBED    2

/** @brief Raised after a VSensorListener has successfully unregistered to a Vsensor
 *
 *  @warning automatically handle by the framework after calling VSensorListener_disable()
 *
 *  Expected event data: none
 */
#define VSENSOR_EVENT_HAS_UNSUBSCRIBED  3

/** @brief Raised when a VSensor's ERI field was updated
 *
 *  Expected event data: effective RI value (passed as uint32_t *)
 */
#define	VSENSOR_EVENT_NEW_EFFECTIVE_RI  4

/** @brief Raised when a VSensor's EMRL field was updated
 *
 *  Expected event data: effective MRL value (passed as uint32_t *)
 */
#define	VSENSOR_EVENT_NEW_EFFECTIVE_MRL 5

/** @brief Raised when a VSensor's internal config was updated
 *
 *  Expected event data: new config data (passed as VSensorConfig *)
 */
#define VSENSOR_EVENT_NEW_CONFIG        7

/** @brief Raised when a VSensor's has emptied all its internal data buffer
 *
 *  Expected event data: none
 */
#define VSENSOR_EVENT_FLUSH_COMPLETE    9

/*
 * Custom event definition boundaries
 */

#define VSENSOR_EVENT_CUSTOM_MIN       32 /**< First allowed value for custom event */
#define VSENSOR_EVENT_MAX              64 /**< Maximum value for any event */

/*
 * For backward compatibility - should not be used
 */

#define VSENSOR_EVENT_UPDATEDATA   VSENSOR_EVENT_NEW_DATA
#define VSENSOR_EVENT_UPDATERI     VSENSOR_EVENT_NEW_REQUESTED_RI
#define VSENSOR_EVENT_UPDATEMRL    VSENSOR_EVENT_NEW_REQUESTED_MRL

#ifdef __cplusplus
}
#endif

#endif /* _V_SENSOR_EVENT_H_ */

/** @} **/
