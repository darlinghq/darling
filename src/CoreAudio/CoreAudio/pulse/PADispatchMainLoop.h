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

#ifndef PAGCDMAINLOOP_H
#define PAGCDMAINLOOP_H
#include <dispatch/dispatch.h>
#include <pulse/pulseaudio.h>

class PADispatchMainLoop
{
public:
	PADispatchMainLoop();
	~PADispatchMainLoop();

	void suspend();
	void resume();
	
	pa_mainloop_api* getAPI();
private:
	static pa_io_event* io_new(pa_mainloop_api *a, int fd, pa_io_event_flags_t events, pa_io_event_cb_t cb, void *userdata);
	static void io_enable(pa_io_event *e, pa_io_event_flags_t events);
	static void	io_free(pa_io_event *e);
	static void	io_set_destroy(pa_io_event *e, pa_io_event_destroy_cb_t cb);

	static pa_time_event *time_new(pa_mainloop_api *a, const struct timeval *tv, pa_time_event_cb_t cb, void *userdata);
	static void	time_restart(pa_time_event *e, const struct timeval *tv);
	static void time_set_destroy(pa_time_event *e, pa_time_event_destroy_cb_t cb);
	static void	time_free(pa_time_event *e);

	static pa_defer_event *defer_new(pa_mainloop_api *a, pa_defer_event_cb_t cb, void *userdata);
	static void	defer_set_destroy(pa_defer_event *e, pa_defer_event_destroy_cb_t cb);
	static void defer_enable(pa_defer_event *e, int b);
	static void	defer_free(pa_defer_event *e);
	static void defer_event_fire(void* context);
private:
	dispatch_queue_t m_queue;
	pa_mainloop_api m_api = {0};
	bool m_suspended = true;
};

#endif
