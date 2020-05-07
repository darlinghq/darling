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

#ifndef _CoreAnalytics_H_
#define _CoreAnalytics_H_

#include <string>
#include <functional>
#include <memory>
#include <string_view>
#include <AppleSauce/AppleSauce.h>
#include <dispatch/dispatch.h>
#include <xpc/xpc.h>

#import <Foundation/Foundation.h>

#import <CoreAnalytics/AnalyticsConfigurationObserver.h>
#import <CoreAnalytics/AnalyticsEventObserver.h>

/**
 * NOTE(@facekapow):
 * these method signatures are pretty much guesses.
 * good guesses, but guesses nonetheless
 *
 * unlike the guesses in AppleSauce, these guesses are
 * less reliable
 */

#ifdef __cplusplus
extern "C" {
#endif

bool AnalyticsIsEventUsed(void*);
void AnalyticsSendEvent(void*, void*);
bool AnalyticsSendEventLazy(void*);
void AnalyticsSetCallbackForQueriedEvent(void*, void(^)());
void AnalyticsSetCallbackForQueriedEventWithQueue(void*, int, void(^)());

#ifdef __cplusplus
};
#endif

#ifdef __cplusplus

std::string analytics_tool_dump(std::string);
void analytics_tool_save(std::string);
void analytics_tool_commit();
void analytics_tool_rollover();
bool analytics_tool_has_config();
std::string analytics_tool_config_dump();
std::string analytics_tool_config_info();
std::string analytics_tool_config_list();
void analytics_tool_send_barrier();
void analytics_tool_config_reload();
std::string analytics_tool_get_cache_list();
std::string analytics_tool_transform_info(std::string);
bool analytics_tool_transform_evict(std::string);
bool analytics_tool_transform_persist(std::string);
void analytics_tool_config_set_tasking(std::string);
std::string analytics_tool_get_queried_events();
bool analytics_tool_is_event_used_check_db(std::string);
void analytics_tool_config_set_tasking_ndjson(applesauce::xpc::object);

namespace CoreAnalytics {
	class EventObserver {
	public:
		~EventObserver();
	};
	class ConfigurationObserver {
	public:
		~ConfigurationObserver();
	};
	class Client {
	public:
		void barrierSync();
		void sendAwdMetric(unsigned int, void const*, unsigned long);
		void sendAwdTrigger(char const*, unsigned int, unsigned int, unsigned int, void const*, unsigned long);
		void sendEventAlways(char const*, applesauce::xpc::object const&);
		dispatch_queue_t getCallbackQueue();
		void sendEventWithTag(char const*, applesauce::xpc::object const&, applesauce::xpc::object);
		void updateDaemonState(int);
		void registerStateHandler(std::string const&, applesauce::dispatch::v1::queue, std::function<void(std::string const&, applesauce::xpc::object const&)>);
		void sendAwdMetricWithTag(char const*, unsigned int, applesauce::xpc::object, void const*, unsigned long);
		void addAwdQueriableMetric(unsigned int);
		void handleQueryState_sync(applesauce::xpc::dict const&);
		void registerEventObserver(std::weak_ptr<CoreAnalytics::EventObserver>);
		void sendAwdTriggerWithTag(char const*, unsigned int, unsigned int, applesauce::xpc::object, void const*, unsigned long);
		void sendManagementCommand(char const*);
		void sendManagementCommand(char const*, applesauce::xpc::dict);
		void registerAwdQueryHandler(std::function<void(unsigned int, char const*, void*)>);
		void sendCheckinMessage_sync();
		void handleServerMessage_sync(applesauce::xpc::dict);
		void handleQueryAwdMetric_sync(applesauce::xpc::dict const&);
		xpc_object_t sendManagementCommandSync(char const*);
		xpc_object_t sendManagementCommandSync(char const*, applesauce::xpc::dict);
		void handleSinkMessageEmit_sync(applesauce::xpc::dict const&);
		void sendStateHandlersList_sync();
		void registerConfigurationObserver(std::weak_ptr<ConfigurationObserver>);
		void handleConfigurationChange_sync(applesauce::xpc::dict const&);
		void sendAwdQueriableMetricsList_sync();
		static Client* get();
		void init();
		Client();
		~Client();
		void sendXpcMessage(applesauce::xpc::dict) const;
		bool isEventWhitelisted(std::string_view) const;
		bool isAwdMetricWhitelisted(unsigned int) const;
		bool hasConfig() const;
	};
};

#endif

#ifdef __cplusplus
extern "C" {
#endif

void analytics_cancel_queried_awd_metric(char const*, xpc_object_t);
bool analytics_is_awd_metric_enabled(unsigned int);
bool analytics_is_event_used(char const*);
void analytics_mark_awd_queriable(unsigned int);
void analytics_send_awd_metric(unsigned int, void const*, unsigned long);
void analytics_send_awd_trigger(char const*, unsigned int, unsigned int, unsigned int, void const*, unsigned long);
void analytics_send_event(char const*, xpc_object_t);
void analytics_send_event_lazy(char const*, xpc_object_t);
void analytics_send_queried_awd_metric(char const*, unsigned int, xpc_object_t, void const*, unsigned long);
void analytics_send_queried_awd_trigger(char const*, unsigned int, unsigned int, xpc_object_t, void const*, unsigned long);
void analytics_set_awd_query_callback(void(^)(unsigned int, char const*, void*));
void analytics_set_callback_for_queried_event(char const*, void(^)());
void analytics_set_callback_for_queried_event_with_queue(char const*, void*, void(^)());
void report_locale_prefs_to_analyticsd();

#ifdef __cplusplus
};
#endif

#endif
