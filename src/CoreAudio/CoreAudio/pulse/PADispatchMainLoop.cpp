/*
This file is part of Darling.

Copyright (C) 2020 Lubos Dolezel

Darling is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Darling is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Darling.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "PADispatchMainLoop.h"

PADispatchMainLoop::PADispatchMainLoop()
{
	// Create GCD queue
	m_queue = dispatch_queue_create("org.darlinghq.coreaudio.pulseaudio", nullptr);
}

PADispatchMainLoop* PADispatchMainLoop::instance()
{
	static PADispatchMainLoop inst;
	return &inst;
}

pa_mainloop_api* PADispatchMainLoop::getAPI()
{
	static pa_mainloop_api api;

	if (!api.userdata)
	{
		api.io_new = io_new;
		api.io_enable = io_enable;
		api.io_free = io_free;
		api.io_set_destroy = io_set_destroy;

		api.time_new = time_new;
		api.time_restart = time_restart;
		api.time_set_destroy = time_set_destroy;
		api.time_free = time_free;

		api.defer_new = defer_new;
		api.defer_set_destroy = defer_set_destroy;
		api.defer_enable = defer_enable;
		api.defer_free = defer_free;

		api.userdata = instance();
	}

	return &api;
}

struct dual_source
{
	dispatch_source_t sourceRead, sourceWrite;
	bool readResumed, writeResumed;
	pa_io_event_cb_t callback;
	pa_io_event_destroy_cb_t destroy = nullptr;
	pa_io_event_flags_t events;
	void* userdata;
};

pa_io_event* PADispatchMainLoop::io_new(pa_mainloop_api *a, int fd, pa_io_event_flags_t events, pa_io_event_cb_t cb, void *userdata)
{
	dispatch_queue_t q = static_cast<PADispatchMainLoop*>(a->userdata)->m_queue;

	dual_source* dual = new dual_source;

	dual->callback = cb;
	dual->events = events;
	dual->userdata = userdata;
	dual->sourceRead = dispatch_source_create(DISPATCH_SOURCE_TYPE_READ, fd, 0, q);
	dual->sourceWrite = dispatch_source_create(DISPATCH_SOURCE_TYPE_WRITE, fd, 0, q);

	dispatch_source_set_event_handler(dual->sourceRead, ^{
		dual->callback(getAPI(), reinterpret_cast<pa_io_event*>(dual), fd, PA_IO_EVENT_INPUT, dual->userdata);
	});

	dispatch_source_set_event_handler(dual->sourceWrite, ^{
		dual->callback(getAPI(), reinterpret_cast<pa_io_event*>(dual), fd, PA_IO_EVENT_OUTPUT, dual->userdata);
	});

	dispatch_source_set_cancel_handler(dual->sourceWrite, ^{
		if (dual->destroy)
		{
			dual->destroy(getAPI(), reinterpret_cast<pa_io_event*>(dual), dual->userdata);
			dual->destroy = nullptr;
		}
		delete dual;
	});

	if (events & (PA_IO_EVENT_INPUT | PA_IO_EVENT_ERROR | PA_IO_EVENT_HANGUP))
	{
		dispatch_resume(dual->sourceRead);
		dual->readResumed = true;
	}
	else
		dual->readResumed = false;

	if (events & (PA_IO_EVENT_OUTPUT))
	{
		dispatch_resume(dual->sourceWrite);
		dual->writeResumed = true;
	}
	else
		dual->writeResumed = false;
	
	return reinterpret_cast<pa_io_event*>(dual);
}

void PADispatchMainLoop::io_enable(pa_io_event *e, pa_io_event_flags_t events)
{
	dual_source* dual = reinterpret_cast<dual_source*>(e);

	if (events & (PA_IO_EVENT_INPUT | PA_IO_EVENT_ERROR | PA_IO_EVENT_HANGUP))
	{
		if (!dual->readResumed)
		{
			dispatch_resume(dual->sourceRead);
			dual->readResumed = true;
		}
	}
	else if (dual->readResumed)
	{
		dispatch_suspend(dual->sourceRead);
		dual->readResumed = false;
	}

	if (events & (PA_IO_EVENT_OUTPUT))
	{
		if (!dual->writeResumed)
		{
			dispatch_resume(dual->sourceWrite);
			dual->writeResumed = true;
		}
	}
	else if (dual->writeResumed)
	{
		dispatch_suspend(dual->sourceWrite);
		dual->writeResumed = false;
	}
}

void PADispatchMainLoop::io_free(pa_io_event *e)
{
	dual_source* dual = reinterpret_cast<dual_source*>(e);

	if (!dual->readResumed)
		dispatch_resume(dual->sourceRead);
	dispatch_source_cancel(dual->sourceRead);
	dispatch_release(dual->sourceRead);

	if (!dual->writeResumed)
		dispatch_resume(dual->sourceWrite);
	dispatch_source_cancel(dual->sourceWrite);
	dispatch_release(dual->sourceWrite);

	// dual is freed in the cancel handler
}

void PADispatchMainLoop::io_set_destroy(pa_io_event *e, pa_io_event_destroy_cb_t cb)
{
	dual_source* dual = reinterpret_cast<dual_source*>(e);
	dual->destroy = cb;
}

struct linux_timeval
{
	unsigned long tv_sec;
	long tv_usec;
};

struct pa_timer
{
	dispatch_source_t source;
	void* userdata;
	struct timespec when;
	struct linux_timeval tv;
	pa_time_event_destroy_cb_t destroy = nullptr;
};

pa_time_event *PADispatchMainLoop::time_new(pa_mainloop_api *a, const struct timeval *tv, pa_time_event_cb_t cb, void *userdata)
{
	const linux_timeval* real_tv = reinterpret_cast<const linux_timeval*>(tv);
	dispatch_queue_t q = static_cast<PADispatchMainLoop*>(a->userdata)->m_queue;

	pa_timer* timer = new pa_timer;

	timer->userdata = userdata;

	timer->source = dispatch_source_create(DISPATCH_SOURCE_TYPE_TIMER, 0, 0, q);

	timer->when.tv_sec = real_tv->tv_sec;
	timer->when.tv_nsec = real_tv->tv_usec * 1000; // convert to ns
	timer->tv = *real_tv;

	dispatch_source_set_timer(timer->source, dispatch_walltime(&timer->when, 0), 0, 0);

	dispatch_source_set_event_handler(timer->source, ^{
		cb(getAPI(), reinterpret_cast<pa_time_event*>(timer), reinterpret_cast<const struct timeval *>(&timer->tv), timer->userdata);
	});
	dispatch_source_set_cancel_handler(timer->source, ^{
		if (timer->destroy)
			timer->destroy(getAPI(), reinterpret_cast<pa_time_event*>(timer), timer->userdata);
		delete timer;
	});

	dispatch_resume(timer->source);

	return reinterpret_cast<pa_time_event*>(timer);
}

void PADispatchMainLoop::time_restart(pa_time_event *e, const struct timeval *tv)
{
	const linux_timeval* real_tv = reinterpret_cast<const linux_timeval*>(tv);
	pa_timer* timer = reinterpret_cast<pa_timer*>(e);

	timer->when.tv_sec = real_tv->tv_sec;
	timer->when.tv_nsec = real_tv->tv_usec * 1000; // convert to ns
	timer->tv = *real_tv;

	dispatch_source_set_timer(timer->source, dispatch_walltime(&timer->when, 0), 0, 0);
}

void PADispatchMainLoop::time_set_destroy(pa_time_event *e, pa_time_event_destroy_cb_t cb)
{
	pa_timer* timer = reinterpret_cast<pa_timer*>(e);

	timer->destroy = cb;
}

void PADispatchMainLoop::time_free(pa_time_event *e)
{
	pa_timer* timer = reinterpret_cast<pa_timer*>(e);

	dispatch_source_cancel(timer->source);
	dispatch_release(timer->source);
}

struct defer_event
{
	bool active = true;
	void* userdata;
	pa_defer_event_cb_t callback;
	pa_defer_event_destroy_cb_t destroy;
};

void PADispatchMainLoop::defer_event_fire(void* context)
{
	defer_event* ev = static_cast<defer_event*>(context);
	if (ev->active)
	{
		pa_mainloop_api* a = getAPI();
		dispatch_queue_t q = static_cast<PADispatchMainLoop*>(a->userdata)->m_queue;

		ev->callback(a, static_cast<pa_defer_event*>(context), ev->userdata);

		if (ev->active)
			dispatch_async_f(q, context, defer_event_fire);
	}
}

pa_defer_event *PADispatchMainLoop::defer_new(pa_mainloop_api *a, pa_defer_event_cb_t cb, void *userdata)
{
	dispatch_queue_t q = static_cast<PADispatchMainLoop*>(a->userdata)->m_queue;
	defer_event* e = new defer_event;

	e->userdata = userdata;
	e->callback = cb;

	return reinterpret_cast<pa_defer_event*>(e);
}

void PADispatchMainLoop::defer_set_destroy(pa_defer_event *e, pa_defer_event_destroy_cb_t cb)
{
	defer_event* ev = reinterpret_cast<defer_event*>(e);

	ev->destroy = cb;
}

void PADispatchMainLoop::defer_enable(pa_defer_event *e, int b)
{
	dispatch_queue_t q = instance()->m_queue;
	defer_event* ev = reinterpret_cast<defer_event*>(e);

	ev->active = !!b;
	if (ev->active)
		dispatch_async_f(q, e, defer_event_fire);
}

void PADispatchMainLoop::defer_free(pa_defer_event *e)
{
	dispatch_queue_t q = instance()->m_queue;
	defer_event* ev = reinterpret_cast<defer_event*>(e);

	dispatch_async(q, ^{
		if (ev->destroy)
			ev->destroy(getAPI(), e, ev->userdata);
		delete ev;
	});
}
