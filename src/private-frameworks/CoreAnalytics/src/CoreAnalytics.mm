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

#include <CoreAnalytics/CoreAnalytics.h>
#include <iostream>

static bool verbose = false;

__attribute__((constructor))
static void initme() {
	verbose = getenv("STUB_VERBOSE") != NULL;
};

extern "C" {

bool AnalyticsIsEventUsed(void*) {
	if (verbose)
		std::cerr << "STUB: " << __PRETTY_FUNCTION__ << " called" << std::endl;
	return false;
};

void AnalyticsSendEvent(void*, void*) {
	if (verbose)
		std::cerr << "STUB: " << __PRETTY_FUNCTION__ << " called" << std::endl;
};

bool AnalyticsSendEventLazy(void*) {
	if (verbose)
		std::cerr << "STUB: " << __PRETTY_FUNCTION__ << " called" << std::endl;
	return false;
};

void AnalyticsSetCallbackForQueriedEvent(void*, void(^)()) {
	if (verbose)
		std::cerr << "STUB: " << __PRETTY_FUNCTION__ << " called" << std::endl;
};

void AnalyticsSetCallbackForQueriedEventWithQueue(void*, int, void(^)()) {
	if (verbose)
		std::cerr << "STUB: " << __PRETTY_FUNCTION__ << " called" << std::endl;
};

};

std::string analytics_tool_dump(std::string) {
	if (verbose)
		std::cerr << "STUB: " << __PRETTY_FUNCTION__ << " called" << std::endl;
	return std::string();
};

void analytics_tool_save(std::string) {
	if (verbose)
		std::cerr << "STUB: " << __PRETTY_FUNCTION__ << " called" << std::endl;
};

void analytics_tool_commit() {
	if (verbose)
		std::cerr << "STUB: " << __PRETTY_FUNCTION__ << " called" << std::endl;
};

void analytics_tool_rollover() {
	if (verbose)
		std::cerr << "STUB: " << __PRETTY_FUNCTION__ << " called" << std::endl;
};

bool analytics_tool_has_config() {
	if (verbose)
		std::cerr << "STUB: " << __PRETTY_FUNCTION__ << " called" << std::endl;
	return false;
};

std::string analytics_tool_config_dump() {
	if (verbose)
		std::cerr << "STUB: " << __PRETTY_FUNCTION__ << " called" << std::endl;
	return std::string();
};

std::string analytics_tool_config_info() {
	if (verbose)
		std::cerr << "STUB: " << __PRETTY_FUNCTION__ << " called" << std::endl;
	return std::string();
};

std::string analytics_tool_config_list() {
	if (verbose)
		std::cerr << "STUB: " << __PRETTY_FUNCTION__ << " called" << std::endl;
	return std::string();
};

void analytics_tool_send_barrier() {
	if (verbose)
		std::cerr << "STUB: " << __PRETTY_FUNCTION__ << " called" << std::endl;
};

void analytics_tool_config_reload() {
	if (verbose)
		std::cerr << "STUB: " << __PRETTY_FUNCTION__ << " called" << std::endl;
};

std::string analytics_tool_get_cache_list() {
	if (verbose)
		std::cerr << "STUB: " << __PRETTY_FUNCTION__ << " called" << std::endl;
	return std::string();
};

std::string analytics_tool_transform_info(std::string) {
	if (verbose)
		std::cerr << "STUB: " << __PRETTY_FUNCTION__ << " called" << std::endl;
	return std::string();
};

bool analytics_tool_transform_evict(std::string) {
	if (verbose)
		std::cerr << "STUB: " << __PRETTY_FUNCTION__ << " called" << std::endl;
	return false;
};

bool analytics_tool_transform_persist(std::string) {
	if (verbose)
		std::cerr << "STUB: " << __PRETTY_FUNCTION__ << " called" << std::endl;
	return false;
};

void analytics_tool_config_set_tasking(std::string) {
	if (verbose)
		std::cerr << "STUB: " << __PRETTY_FUNCTION__ << " called" << std::endl;
};

std::string analytics_tool_get_queried_events() {
	if (verbose)
		std::cerr << "STUB: " << __PRETTY_FUNCTION__ << " called" << std::endl;
	return std::string();
};

bool analytics_tool_is_event_used_check_db(std::string) {
	if (verbose)
		std::cerr << "STUB: " << __PRETTY_FUNCTION__ << " called" << std::endl;
	return false;
};

void analytics_tool_config_set_tasking_ndjson(applesauce::xpc::object) {
	if (verbose)
		std::cerr << "STUB: " << __PRETTY_FUNCTION__ << " called" << std::endl;
};

CoreAnalytics::EventObserver::~EventObserver() {
	if (verbose)
		std::cerr << "STUB: " << __PRETTY_FUNCTION__ << " called" << std::endl;
};

CoreAnalytics::ConfigurationObserver::~ConfigurationObserver() {
	if (verbose)
		std::cerr << "STUB: " << __PRETTY_FUNCTION__ << " called" << std::endl;
};

void CoreAnalytics::Client::barrierSync() {
	if (verbose)
		std::cerr << "STUB: " << __PRETTY_FUNCTION__ << " called" << std::endl;
};

void CoreAnalytics::Client::sendAwdMetric(unsigned int, void const*, unsigned long) {
	if (verbose)
		std::cerr << "STUB: " << __PRETTY_FUNCTION__ << " called" << std::endl;
};

void CoreAnalytics::Client::sendAwdTrigger(char const*, unsigned int, unsigned int, unsigned int, void const*, unsigned long) {
	if (verbose)
		std::cerr << "STUB: " << __PRETTY_FUNCTION__ << " called" << std::endl;
};

void CoreAnalytics::Client::sendEventAlways(char const*, applesauce::xpc::object const&) {
	if (verbose)
		std::cerr << "STUB: " << __PRETTY_FUNCTION__ << " called" << std::endl;
};

dispatch_queue_t CoreAnalytics::Client::getCallbackQueue() {
	if (verbose)
		std::cerr << "STUB: " << __PRETTY_FUNCTION__ << " called" << std::endl;
	return NULL;
};

void CoreAnalytics::Client::sendEventWithTag(char const*, applesauce::xpc::object const&, applesauce::xpc::object) {
	if (verbose)
		std::cerr << "STUB: " << __PRETTY_FUNCTION__ << " called" << std::endl;
};

void CoreAnalytics::Client::updateDaemonState(int) {
	if (verbose)
		std::cerr << "STUB: " << __PRETTY_FUNCTION__ << " called" << std::endl;
};

void CoreAnalytics::Client::registerStateHandler(std::string const&, applesauce::dispatch::v1::queue, std::function<void (std::string const&, applesauce::xpc::object const&)>) {
	if (verbose)
		std::cerr << "STUB: " << __PRETTY_FUNCTION__ << " called" << std::endl;
};

void CoreAnalytics::Client::sendAwdMetricWithTag(char const*, unsigned int, applesauce::xpc::object, void const*, unsigned long) {
	if (verbose)
		std::cerr << "STUB: " << __PRETTY_FUNCTION__ << " called" << std::endl;
};

void CoreAnalytics::Client::addAwdQueriableMetric(unsigned int) {
	if (verbose)
		std::cerr << "STUB: " << __PRETTY_FUNCTION__ << " called" << std::endl;
};

void CoreAnalytics::Client::handleQueryState_sync(applesauce::xpc::dict const&) {
	if (verbose)
		std::cerr << "STUB: " << __PRETTY_FUNCTION__ << " called" << std::endl;
};

void CoreAnalytics::Client::registerEventObserver(std::weak_ptr<CoreAnalytics::EventObserver>) {
	if (verbose)
		std::cerr << "STUB: " << __PRETTY_FUNCTION__ << " called" << std::endl;
};

void CoreAnalytics::Client::sendAwdTriggerWithTag(char const*, unsigned int, unsigned int, applesauce::xpc::object, void const*, unsigned long) {
	if (verbose)
		std::cerr << "STUB: " << __PRETTY_FUNCTION__ << " called" << std::endl;
};

void CoreAnalytics::Client::sendManagementCommand(char const*) {
	if (verbose)
		std::cerr << "STUB: " << __PRETTY_FUNCTION__ << " called" << std::endl;
};

void CoreAnalytics::Client::sendManagementCommand(char const*, applesauce::xpc::dict) {
	if (verbose)
		std::cerr << "STUB: " << __PRETTY_FUNCTION__ << " called" << std::endl;
};

void CoreAnalytics::Client::registerAwdQueryHandler(std::function<void (unsigned int, char const*, void*)>) {
	if (verbose)
		std::cerr << "STUB: " << __PRETTY_FUNCTION__ << " called" << std::endl;
};

void CoreAnalytics::Client::sendCheckinMessage_sync() {
	if (verbose)
		std::cerr << "STUB: " << __PRETTY_FUNCTION__ << " called" << std::endl;
};

void CoreAnalytics::Client::handleServerMessage_sync(applesauce::xpc::dict) {
	if (verbose)
		std::cerr << "STUB: " << __PRETTY_FUNCTION__ << " called" << std::endl;
};

void CoreAnalytics::Client::handleQueryAwdMetric_sync(applesauce::xpc::dict const&) {
	if (verbose)
		std::cerr << "STUB: " << __PRETTY_FUNCTION__ << " called" << std::endl;
};

xpc_object_t CoreAnalytics::Client::sendManagementCommandSync(char const*) {
	if (verbose)
		std::cerr << "STUB: " << __PRETTY_FUNCTION__ << " called" << std::endl;
	return NULL;
};

xpc_object_t CoreAnalytics::Client::sendManagementCommandSync(char const*, applesauce::xpc::dict) {
	if (verbose)
		std::cerr << "STUB: " << __PRETTY_FUNCTION__ << " called" << std::endl;
	return NULL;
};

void CoreAnalytics::Client::handleSinkMessageEmit_sync(applesauce::xpc::dict const&) {
	if (verbose)
		std::cerr << "STUB: " << __PRETTY_FUNCTION__ << " called" << std::endl;
};

void CoreAnalytics::Client::sendStateHandlersList_sync() {
	if (verbose)
		std::cerr << "STUB: " << __PRETTY_FUNCTION__ << " called" << std::endl;
};

void CoreAnalytics::Client::registerConfigurationObserver(std::weak_ptr<CoreAnalytics::ConfigurationObserver>) {
	if (verbose)
		std::cerr << "STUB: " << __PRETTY_FUNCTION__ << " called" << std::endl;
};

void CoreAnalytics::Client::handleConfigurationChange_sync(applesauce::xpc::dict const&) {
	if (verbose)
		std::cerr << "STUB: " << __PRETTY_FUNCTION__ << " called" << std::endl;
};

void CoreAnalytics::Client::sendAwdQueriableMetricsList_sync() {
	if (verbose)
		std::cerr << "STUB: " << __PRETTY_FUNCTION__ << " called" << std::endl;
};

CoreAnalytics::Client* CoreAnalytics::Client::get() {
	if (verbose)
		std::cerr << "STUB: " << __PRETTY_FUNCTION__ << " called" << std::endl;
	return NULL;
};

void CoreAnalytics::Client::init() {
	if (verbose)
		std::cerr << "STUB: " << __PRETTY_FUNCTION__ << " called" << std::endl;
};

CoreAnalytics::Client::Client() {
	if (verbose)
		std::cerr << "STUB: " << __PRETTY_FUNCTION__ << " called" << std::endl;
};

CoreAnalytics::Client::~Client() {
	if (verbose)
		std::cerr << "STUB: " << __PRETTY_FUNCTION__ << " called" << std::endl;
};

void CoreAnalytics::Client::sendXpcMessage(applesauce::xpc::dict) const {
	if (verbose)
		std::cerr << "STUB: " << __PRETTY_FUNCTION__ << " called" << std::endl;
};

bool CoreAnalytics::Client::isEventWhitelisted(std::string_view) const {
	if (verbose)
		std::cerr << "STUB: " << __PRETTY_FUNCTION__ << " called" << std::endl;
	return false;
};

bool CoreAnalytics::Client::isAwdMetricWhitelisted(unsigned int) const {
	if (verbose)
		std::cerr << "STUB: " << __PRETTY_FUNCTION__ << " called" << std::endl;
	return false;
};

bool CoreAnalytics::Client::hasConfig() const {
	if (verbose)
		std::cerr << "STUB: " << __PRETTY_FUNCTION__ << " called" << std::endl;
	return false;
};

extern "C" {

void analytics_cancel_queried_awd_metric(char const*, xpc_object_t) {
	if (verbose)
		std::cerr << "STUB: " << __PRETTY_FUNCTION__ << " called" << std::endl;
};

bool analytics_is_awd_metric_enabled(unsigned int) {
	if (verbose)
		std::cerr << "STUB: " << __PRETTY_FUNCTION__ << " called" << std::endl;
	return false;
};

bool analytics_is_event_used(char const*) {
	if (verbose)
		std::cerr << "STUB: " << __PRETTY_FUNCTION__ << " called" << std::endl;
	return false;
};

void analytics_mark_awd_queriable(unsigned int) {
	if (verbose)
		std::cerr << "STUB: " << __PRETTY_FUNCTION__ << " called" << std::endl;
};

void analytics_send_awd_metric(unsigned int, void const*, unsigned long) {
	if (verbose)
		std::cerr << "STUB: " << __PRETTY_FUNCTION__ << " called" << std::endl;
};

void analytics_send_awd_trigger(char const*, unsigned int, unsigned int, unsigned int, void const*, unsigned long) {
	if (verbose)
		std::cerr << "STUB: " << __PRETTY_FUNCTION__ << " called" << std::endl;
};

void analytics_send_event(char const*, xpc_object_t) {
	if (verbose)
		std::cerr << "STUB: " << __PRETTY_FUNCTION__ << " called" << std::endl;
};

void analytics_send_event_lazy(char const*, xpc_object_t) {
	if (verbose)
		std::cerr << "STUB: " << __PRETTY_FUNCTION__ << " called" << std::endl;
};

void analytics_send_queried_awd_metric(char const*, unsigned int, xpc_object_t, void const*, unsigned long) {
	if (verbose)
		std::cerr << "STUB: " << __PRETTY_FUNCTION__ << " called" << std::endl;
};

void analytics_send_queried_awd_trigger(char const*, unsigned int, unsigned int, xpc_object_t, void const*, unsigned long) {
	if (verbose)
		std::cerr << "STUB: " << __PRETTY_FUNCTION__ << " called" << std::endl;
};

void analytics_set_awd_query_callback(void(^)(unsigned int, char const*, void*)) {
	if (verbose)
		std::cerr << "STUB: " << __PRETTY_FUNCTION__ << " called" << std::endl;
};

void analytics_set_callback_for_queried_event(char const*, void(^)()) {
	if (verbose)
		std::cerr << "STUB: " << __PRETTY_FUNCTION__ << " called" << std::endl;
};

void analytics_set_callback_for_queried_event_with_queue(char const*, void*, void(^)()) {
	if (verbose)
		std::cerr << "STUB: " << __PRETTY_FUNCTION__ << " called" << std::endl;
};

void report_locale_prefs_to_analyticsd() {
	if (verbose)
		std::cerr << "STUB: " << __PRETTY_FUNCTION__ << " called" << std::endl;
};

};
