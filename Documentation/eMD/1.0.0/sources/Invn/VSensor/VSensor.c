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

#include "VSensor.h"
#include "VSensorListener.h"

#include <string.h>

CASSERT(VSENSOR_EVENT_NEW_REQUESTED_RI == VSENSOR_EVENT_NEW_EFFECTIVE_RI);
CASSERT(VSENSOR_EVENT_NEW_REQUESTED_MRL == VSENSOR_EVENT_NEW_EFFECTIVE_MRL);

#define EVENT_UPDATERI  VSENSOR_EVENT_NEW_REQUESTED_RI
#define EVENT_UPDATEMRL VSENSOR_EVENT_NEW_REQUESTED_MRL

#define STATUS_FLAG_SETUP_MASK 0x03

static int setupVSensor(VSensor * vsensor)
{
	ASSERT(vsensor && vsensor->update);

	// check weither we are already initialized
	if((vsensor->status & STATUS_FLAG_SETUP_MASK) == 0) {

		/* Unknown type, assuming VSensor is not inialized */
		if(vsensor->type == VSENSOR_TYPE_UNKNOWN) {
			return -1;
		}

		// Update status flag before calling SETUP method so that 
		// any circular dependency does not end with infinite loop
		vsensor->status |= VSENSOR_STATUS_FLAG_SETUP_OK;
		if(vsensor->update(vsensor, VSENSOR_EVENT_SETUP, 0) != 0) {
			vsensor->status &= ~VSENSOR_STATUS_FLAG_SETUP_OK;
			vsensor->status |= VSENSOR_STATUS_FLAG_SETUP_ERROR;

			return -1;
		}
		else {
			vsensor->status &= ~VSENSOR_STATUS_FLAG_SETUP_ERROR;
			vsensor->status |= VSENSOR_STATUS_FLAG_SETUP_OK;
		}
	}
	else if(vsensor->status & VSENSOR_STATUS_FLAG_SETUP_ERROR) {
		return -1;
	}

	return 0;
}

static struct InvList * getList(VSensorListener * listener)
{
	return &listener->vsensor->list;
}

static void getMinRimrl(struct InvList * list,
		uint32_t *minri, uint32_t *minmrl)
{
	struct InvList * cur = InvList_head(list);

	if(cur == 0) {
		*minri = 0;
		*minmrl = 0;
	}
	else {
		const VSensorListener *listener = INVLIST_GET(cur, VSensorListener, next);

		*minri = listener->ri;
		*minmrl = listener->mrl;

		while ((cur = InvList_next(cur)) != 0) {
			const VSensorListener *listener = INVLIST_GET(cur, VSensorListener, next);

			if(listener->ri < *minri) {
				*minri = listener->ri;
			}

			if(listener->mrl < *minmrl) {
				*minmrl = listener->mrl;
			}
		}
	}
}

static void apply(VSensorListener * listener, int flag, int force)
{
	uint32_t eri;
	uint32_t emrl;
	struct VSensor * vsensor = listener->vsensor;

	/* Get min RI/MRL from */
	getMinRimrl(getList(listener), &eri, &emrl);

	/* Saturate RI according to vsensor attribute */
	if(eri < vsensor->attr.min_ri) {
		eri = vsensor->attr.min_ri;
	}

	/* Check for change */
	if(vsensor->eri != eri) {
		flag |= (1 << EVENT_UPDATERI);
		vsensor->eri = eri;
	}

	if(vsensor->emrl != emrl) {
		flag |= (1 << EVENT_UPDATEMRL);
		vsensor->emrl = emrl;
	}

	/* Force notify of RI/MRL on first subscribe */
	if(flag & (1 << VSENSOR_EVENT_SUBSCRIBE)) {
		flag |= (1 << EVENT_UPDATERI);
		flag |= (1 << EVENT_UPDATEMRL);
	}

	/* Do not notify of RI/MRL when unsubscribing */
	if(flag & (1 << VSENSOR_EVENT_UNSUBSCRIBE)) {
		flag &= ~(1 << EVENT_UPDATERI);
		flag &= ~(1 << EVENT_UPDATEMRL);
	}

	/* Apply to VSensor */

	/*
	 * Unsubscribing event is applied in VSensorListener_disable()
	 */

	if(flag & (1 << EVENT_UPDATERI)) {
		VSensor_update(vsensor, EVENT_UPDATERI, &vsensor->eri);
	}

	if(flag & (1 << EVENT_UPDATEMRL)) {
		VSensor_update(vsensor, EVENT_UPDATEMRL, &vsensor->emrl);
	}

	if(flag & (1 << VSENSOR_EVENT_SUBSCRIBE)) {
		VSensor_update(vsensor, VSENSOR_EVENT_SUBSCRIBE, 0);
	}

	/* ERI, EMRL value was changed by the sensor during update */
	if(vsensor->eri != eri) {
		flag |= (1 << EVENT_UPDATERI);
	}

	if(vsensor->emrl != emrl) {
		flag |= (1 << EVENT_UPDATEMRL);
	}

	/* Notify to all subscribers that a new config was applied only
	   if something new is to be signaled */

	if(flag & (1 << EVENT_UPDATERI)) {
		VSensor_notifyEvent(vsensor, EVENT_UPDATERI, &vsensor->eri);
	}

	if(flag & (1 << EVENT_UPDATEMRL)) {
		VSensor_notifyEvent(vsensor, EVENT_UPDATEMRL, &vsensor->emrl);
	}

	/* No change but listeners asked to be notified anyway */

	if((force & (1 << EVENT_UPDATERI)) != 0
			&& (flag & (1 << EVENT_UPDATERI)) == 0) {
		listener->handler(listener, EVENT_UPDATERI, &vsensor->eri);
	}

	if((force & (1 << EVENT_UPDATEMRL)) != 0
			&& (flag & (1 << EVENT_UPDATEMRL)) == 0) {
		listener->handler(listener, EVENT_UPDATEMRL, &vsensor->emrl);
	}
}

static void dummyHandler(VSensorListener * listener, int event, const void * data)
{
	(void)listener, (void)event, (void)data;
}

int VSensorListener_attach(VSensorListener *listener, struct VSensor *vsensor,
		VSensorListenerHandler handler, void * arg)
{
	if(listener && vsensor) {
		InvList_init(&listener->next);
		listener->handler = (handler != 0) ? handler : dummyHandler;
		listener->vsensor = vsensor;
		listener->ri      = 0;
		listener->mrl     = 0;
		listener->arg     = arg;

		return setupVSensor(vsensor);
	}

	return -1;
}

static inline int isListenerValid(VSensorListener *listener)
{
	if(!listener || !listener->vsensor || (listener->vsensor->status & VSENSOR_STATUS_FLAG_SETUP_ERROR)) {
		/* invalid */
		return 0;
	}

	return 1;
}

void VSensorListener_setRi(VSensorListener *listener, uint32_t ri)
{
	if(!isListenerValid(listener))
		return;

	listener->ri = ri;

	if(InvList_isIn(&listener->vsensor->list, &listener->next)) {
		apply(listener, 0, (1 << EVENT_UPDATERI));
	}
}

void VSensorListener_setMrl(VSensorListener *listener, uint32_t mrl)
{
	if(!isListenerValid(listener))
		return;

	listener->mrl = mrl;

	if(InvList_isIn(&listener->vsensor->list, &listener->next)) {
		apply(listener, 0, (1 << EVENT_UPDATEMRL));
	}
}

void VSensorListener_enable(VSensorListener *listener)
{
	int  flag = 0;

	if(!isListenerValid(listener))
		return;

	if(InvList_isEmpty(&listener->vsensor->list))
		flag |= (1 << VSENSOR_EVENT_SUBSCRIBE);

	if(InvList_add(&listener->vsensor->list, &listener->next) == 0) {
		/* notify this listener it has successfuly subscribed to the vsensor */
		listener->handler(listener, VSENSOR_EVENT_HAS_SUBSCRIBED, 0);

		apply(listener, flag, (1 << EVENT_UPDATERI) | (1 << EVENT_UPDATEMRL));
	}
}

void VSensorListener_disable(VSensorListener *listener)
{
	if(!isListenerValid(listener))
		return;

	if(InvList_remove(&listener->vsensor->list, &listener->next) == 0) {
		if(InvList_isEmpty(&listener->vsensor->list)) {
			apply(listener, (1 << VSENSOR_EVENT_UNSUBSCRIBE), 0);

			/*
			 * Add node back to list and apply update:
			 * this allows to call VSensorListener_notify() when handling UNSUBSCRIBE event
			 */
			InvList_add(&listener->vsensor->list, &listener->next);
			VSensor_update(listener->vsensor, VSENSOR_EVENT_UNSUBSCRIBE, 0);
			InvList_remove(&listener->vsensor->list, &listener->next);
		}
		else {
			apply(listener, 0, 0);
		}

		/* notify this listener it has successfuly unsubscribed from the vsensor */
		listener->handler(listener, VSENSOR_EVENT_HAS_UNSUBSCRIBED, 0);
	}
}

void VSensor_init(VSensor * vsensor, VSensorUpdateCb update,
		int type, unsigned sdata, const VSensorAttr * attr, void * arg)
{
	memset(vsensor, 0, sizeof(* vsensor));
	vsensor->type   = type;
	vsensor->sdata  = sdata;
	vsensor->update = update;
	vsensor->arg    = arg;

	if(attr) {
		vsensor->attr = *attr;
	} else {
		vsensor->attr.min_ri = VSENSOR_ATTR_MIN_RI_DEFAULT;
//		vsensor->attr.max_ri = VSENSOR_ATTR_MAX_RI_DEFAULT;
	}
}

void VSensor_notifyEvent(VSensor *vsensor, int event, const void * data)
{
	struct InvList *cur = InvList_head(&vsensor->list);

	while (cur != 0) {
		VSensorListener *listener = INVLIST_GET(cur, VSensorListener, next);
		listener->handler(listener, event, data);
		cur = InvList_next(cur);
	}
}


static void vsensorSmartListenerHandler(struct VSensorListener * listener, int event, const void * data)
{
	struct VSensorSmartListener * smartlistener = (struct VSensorSmartListener *)listener;
	const void * new_data;
	int result, icount = 0;

	do {
		result = smartlistener->decimator(smartlistener, event, data, &new_data);
		if(result) {
			smartlistener->handler(listener, event, new_data);
		}
	} while(result > 1 && ++icount < 10);
}

int VSensorSmartListener_attach(VSensorSmartListener * listener, struct VSensor * vsensor,
		VSensorListenerHandler handler, void * arg, VSensorDecimatorCb decimator)
{
	listener->decimator = decimator;
	listener->handler = handler;

	if(!decimator) {
		/* no decimator provided, acts as a standard VSensorListener */
		return VSensorListener_attach(&listener->base, vsensor, handler, arg);
	}
	else {
		return VSensorListener_attach(&listener->base, vsensor, vsensorSmartListenerHandler, arg);
	}
}

int VSensorSmartListenerRIR_decimator(struct VSensorSmartListener * smartlistener, int event,
		const void * data, const void ** new_data)
{
	struct VSensorSmartListenerRIR * rir = (struct VSensorSmartListenerRIR *)smartlistener;

	switch(event) {
	case VSENSOR_EVENT_NEW_EFFECTIVE_RI:
	{
		const uint32_t eri = *(const uint32_t *)data;
		if(eri < smartlistener->base.ri && eri != 0) {
			rir->rate = smartlistener->base.ri / eri;
			rir->cnt = rir->rate;
		}
		else {
			rir->cnt = 0;
			rir->rate = 0;
		}
		break;
	}

	case VSENSOR_EVENT_NEW_DATA:
		if(++rir->cnt < rir->rate) {
			/* drop data */
			return 0;
		}
		/* keep data */
		rir->cnt = 0;
		break;

	default:
		break;
	}

	*new_data = data;
	return 1;
}

int VSensorSmartListenerRIR_attach(VSensorSmartListenerRIR * listener, struct VSensor * vsensor,
		VSensorListenerHandler handler, void * arg)
{
	listener->cnt = 0;
	listener->rate = 0;

	return VSensorSmartListener_attach(&listener->base, vsensor, handler, arg, VSensorSmartListenerRIR_decimator);
}
