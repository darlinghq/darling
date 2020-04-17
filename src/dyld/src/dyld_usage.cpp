/*
 * Copyright (c) 2017 Apple Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 *
 * "Portions Copyright (c) 1999 Apple Computer, Inc.  All Rights
 * Reserved.  This file contains Original Code and/or Modifications of
 * Original Code as defined in and that are subject to the Apple Public
 * Source License Version 1.0 (the 'License').  You may not use this file
 * except in compliance with the License.  Please obtain a copy of the
 * License at http://www.apple.com/publicsource and read it before using
 * this file.
 *
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE OR NON-INFRINGEMENT.  Please see the
 * License for the specific language governing rights and limitations
 * under the License."
 *
 * @APPLE_LICENSE_HEADER_END@
 */

#include <map>
#include <set>
#include <memory>
#include <string>
#include <cstring>
#include <array>
#include <vector>
#include <iomanip>
#include <sstream>
#include <iostream>
#include <cinttypes>
#include <dlfcn.h>
#include <unistd.h>
#include <stdint.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <libutil.h>
#include <ktrace/session.h>
#include <dispatch/dispatch.h>
#include <System/sys/kdebug.h>

#include "Tracing.h"

#define DBG_FUNC_ALL    (DBG_FUNC_START | DBG_FUNC_END)


/*
 * MAXCOLS controls when extra data kicks in.
 * MAX_WIDE_MODE_COLS controls -w mode to get even wider data in path.
 */
#define MAXCOLS (132)
#define MAXWIDTH (MAXCOLS + 64)

unsigned int columns = 0;
ktrace_session_t s;
bool wideflag = false;
bool RAW_flag = false;
bool JSON_flag = false;
bool JSON_Tracing_flag = false;
dispatch_source_t sigwinch_source;
static void
exit_usage(void)
{
    fprintf(stderr, "Usage: dyld_usage [-e] [-f mode] [-t seconds] [-R rawfile [-S start_time] [-E end_time]] [pid | cmd [pid | cmd] ...]\n");
    fprintf(stderr, "  -e    exclude the specified list of pids from the sample\n");
    fprintf(stderr, "        and exclude dyld_usage by default\n");
    fprintf(stderr, "  -t    specifies timeout in seconds (for use in automated tools)\n");
    fprintf(stderr, "  -R    specifies a raw trace file to process\n");
    fprintf(stderr, "  pid   selects process(s) to sample\n");
    fprintf(stderr, "  cmd   selects process(s) matching command string to sample\n");
    fprintf(stderr, "By default (no options) the following processes are excluded from the output:\n");
    fprintf(stderr, "dyld_usage, Terminal, telnetd, sshd, rlogind, tcsh, csh, sh\n\n");

    exit(1);
}

static void
get_screenwidth(void)
{
    struct winsize size;

    columns = MAXCOLS;

    if (isatty(STDOUT_FILENO)) {
        if (ioctl(1, TIOCGWINSZ, &size) != -1) {
            columns = size.ws_col;

            if (columns > MAXWIDTH)
                columns = MAXWIDTH;
        }
    }
}

std::map<uint64_t, uint64_t> gActiveStringIDs;
std::map<uint64_t, std::string> gActiveStrings;

const std::string& stringForID(uint64_t id) {
    static std::string emptyString = "";
    auto i = gActiveStrings.find(id);
    if (i == gActiveStrings.end())
        return emptyString;
    return i->second;
}

static uint64_t
mach_to_nano(uint64_t mach)
{
    uint64_t nanoseconds = 0;
    assert(ktrace_convert_timestamp_to_nanoseconds(s, mach, &nanoseconds) == 0);
    return nanoseconds;
}

static
std::string safeStringFromCString(const char *str) {
    if (str) {
        return str;
    }
    return "";
}

struct output_renderer {
    output_renderer(ktrace_session_t S, ktrace_event_t E) :
    _commandName(safeStringFromCString(ktrace_get_execname_for_thread(s, E->threadid))),
    _threadid((unsigned long)(E->threadid)), _pid(ktrace_get_pid_for_thread(s, E->threadid)) {}
    void recordEvent(ktrace_event_t event) {
        uint32_t code = event->debugid & KDBG_EVENTID_MASK;
        if (event->debugid & DBG_FUNC_START) {
            switch(code) {
                case DBG_DYLD_TIMING_DLOPEN: enqueueEvent<dlopen>(event, true); break;
                case DBG_DYLD_TIMING_DLOPEN_PREFLIGHT: enqueueEvent<dlopen_preflight>(event, true); break;
                case DBG_DYLD_TIMING_LAUNCH_EXECUTABLE: enqueueEvent<app_launch>(event, true); break;
                case DBG_DYLD_TIMING_DLSYM: enqueueEvent<dlsym>(event, true); break;
                case DBG_DYLD_TIMING_STATIC_INITIALIZER: enqueueEvent<static_init>(event, false); break;
                case DBG_DYLD_TIMING_MAP_IMAGE: enqueueEvent<map_image>(event, false); break;
                case DBG_DYLD_TIMING_APPLY_FIXUPS: enqueueEvent<apply_fixups>(event, false); break;
                case DBG_DYLD_TIMING_ATTACH_CODESIGNATURE: enqueueEvent<attach_signature>(event, false); break;
                case DBG_DYLD_TIMING_BUILD_CLOSURE: enqueueEvent<build_closure>(event, false); break;
                case DBG_DYLD_TIMING_DLADDR: enqueueEvent<dladdr>(event, true); break;
                case DBG_DYLD_TIMING_DLCLOSE: enqueueEvent<dlclose>(event, true); break;
                case DBG_DYLD_TIMING_FUNC_FOR_ADD_IMAGE: enqueueEvent<add_image_callback>(event, false); break;
                case DBG_DYLD_TIMING_FUNC_FOR_REMOVE_IMAGE: enqueueEvent<remove_image_callback>(event, false); break;
                case DBG_DYLD_TIMING_OBJC_INIT: enqueueEvent<objc_image_init>(event, false); break;
                case DBG_DYLD_TIMING_OBJC_MAP: enqueueEvent<objc_images_map>(event, false); break;
            }
        } else {
            switch(code) {
                case DBG_DYLD_TIMING_DLOPEN: dequeueEvent<dlopen>(event, [&](dlopen* endEvent){
                    endEvent->result = event->arg2;
                }); break;
                case DBG_DYLD_TIMING_DLOPEN_PREFLIGHT: dequeueEvent<dlopen_preflight>(event, [&](dlopen_preflight* endEvent){
                    endEvent->result = event->arg2;
                }); break;
                case DBG_DYLD_TIMING_LAUNCH_EXECUTABLE: dequeueEvent<app_launch>(event, [&](app_launch* endEvent){
                    endEvent->launchMode = event->arg4;
                }); break;
                case DBG_DYLD_TIMING_DLSYM: dequeueEvent<dlsym>(event, [&](dlsym* endEvent){
                    endEvent->result = event->arg2;
                }); break;
                case DBG_DYLD_TIMING_STATIC_INITIALIZER: dequeueEvent<static_init>(event, [&](static_init* endEvent){}); break;
                case DBG_DYLD_TIMING_MAP_IMAGE: dequeueEvent<map_image>(event, [&](map_image* endEvent){
                    endEvent->result = event->arg2;
                }); break;
                case DBG_DYLD_TIMING_APPLY_FIXUPS: dequeueEvent<apply_fixups>(event, [&](apply_fixups* endEvent){}); break;
                case DBG_DYLD_TIMING_ATTACH_CODESIGNATURE: dequeueEvent<attach_signature>(event, [&](attach_signature* endEvent){}); break;
                case DBG_DYLD_TIMING_BUILD_CLOSURE: dequeueEvent<build_closure>(event, [&](build_closure* endEvent){
                    endEvent->closureBuildState = event->arg2;
                }); break;
                case DBG_DYLD_TIMING_DLCLOSE: dequeueEvent<dlclose>(event, [&](dlclose* endEvent){
                    endEvent->result = (int)event->arg2;
                }); break;
                case DBG_DYLD_TIMING_DLADDR: dequeueEvent<dladdr>(event, [&](dladdr* endEvent){
                    endEvent->result = (int)event->arg2;
                    endEvent->imageAddress = (int)event->arg3;
                    endEvent->symbolAddress = (int)event->arg4;
                }); break;
                case DBG_DYLD_TIMING_FUNC_FOR_ADD_IMAGE: dequeueEvent<add_image_callback>(event, [&](add_image_callback* endEvent){}); break;
                case DBG_DYLD_TIMING_FUNC_FOR_REMOVE_IMAGE: dequeueEvent<remove_image_callback>(event, [&](remove_image_callback* endEvent){}); break;
                case DBG_DYLD_TIMING_OBJC_INIT: dequeueEvent<objc_image_init>(event, [&](objc_image_init* endEvent){}); break;
                case DBG_DYLD_TIMING_OBJC_MAP: dequeueEvent<objc_images_map>(event, [&](objc_images_map* endEvent){}); break;
            }
        }
    }

    bool empty() { return !_currentRootEvent && _eventStack.empty() && _rootEvents.empty(); }
private:
    template<typename T>
    void enqueueEvent(ktrace_event_t event, bool rootEvent) {
        auto sharedEvent = std::make_shared<T>(event);
        if (!_currentRootEvent) {
            if (!rootEvent) return;
            assert(_eventStack.empty());
            _currentRootEvent = sharedEvent;
        } else {
            sharedEvent->setDepth(_eventStack.size());
            _eventStack.back()->addChild(sharedEvent);
        }
        _eventStack.push_back(sharedEvent);
    }

    template<typename T>
    void dequeueEvent(ktrace_event_t event, std::function<void(T*)> lambda) {
        if (_eventStack.empty()) return;
        auto currentEvent = dynamic_cast<T*>(_eventStack.back().get());
        currentEvent->setEndEvent(event);
        lambda(currentEvent);
        _eventStack.pop_back();
        if (_currentRootEvent && _eventStack.empty()) {
            output(_currentRootEvent);
            _currentRootEvent = nullptr;
        }
    }

    struct event_pair {
        event_pair(ktrace_event_t E) : _startTime(E->timestamp), _walltime(E->walltime), _threadid((unsigned long)(E->threadid)), _depth(0),
                                       _eventCode(KDBG_EXTRACT_CODE(E->debugid)) {};
        virtual ~event_pair(){}
        std::vector<std::shared_ptr<event_pair>>& children() { return _children; }
        void setDepth(uint64_t D) { _depth = D; }
        uint64_t depth() { return _depth; }
        struct timeval walltime() { return _walltime; };
        uint64_t startTimestamp() { return _startTime; };
        uint64_t endTimestamp() { return _endTime; }
        unsigned long threadid() { return _threadid; }
        void addChild(std::shared_ptr<event_pair> child) {_children.push_back(child);}
        void setEndEvent(ktrace_event_t E) { _endTime = E->timestamp; }
        uint16_t eventCode() const { return _eventCode; }
    private:
        std::vector<std::shared_ptr<event_pair>> _children;
        uint64_t _startTime;
        uint64_t _endTime;
        uint64_t _depth;
        unsigned long _threadid;
        uint16_t _eventCode;
        struct timeval _walltime;
    };

    time_t _lastTimeWallSeconds = -1;
    std::string _timestampStr;
    std::string _commandName;
    pid_t _pid;
    unsigned long _threadid;

    std::string timestamp(std::shared_ptr<event_pair> event, bool extended) {
        std::ostringstream result;
        struct timeval now_walltime = event->walltime();
        assert(now_walltime.tv_sec || now_walltime.tv_usec);

        /* try and reuse the timestamp string */
        if (_lastTimeWallSeconds != now_walltime.tv_sec) {
            char timestamp[32];
            (void)strftime(timestamp, sizeof (timestamp), "%H:%M:%S", localtime(&now_walltime.tv_sec));
            _lastTimeWallSeconds = now_walltime.tv_sec;
            _timestampStr = timestamp;
        }
        result << _timestampStr;

        if (extended) {
            result << "." << std::setw(6) << std::setfill('0') << std::to_string(now_walltime.tv_usec);
        }
        return result.str();
    }

    std::string process(std::shared_ptr<event_pair> event, bool extended) {
        if (extended) {
            std::ostringstream result;
            result << _commandName << "." << std::to_string(event->threadid());
            return result.str();
        } else {
            std::string result = _commandName;
            result.resize(12, ' ');
            return result;
        }
    }

    std::string duration(std::shared_ptr<event_pair> event) {
        std::ostringstream result;
        uint64_t usecs = (mach_to_nano(event->endTimestamp() - event->startTimestamp()) + (NSEC_PER_USEC - 1)) / NSEC_PER_USEC;
        uint64_t secs = usecs / USEC_PER_SEC;
        usecs -= secs * USEC_PER_SEC;
        result << secs << "." << std::setw(6) << std::setfill('0') << usecs;
        return result.str();
    }

public:
    void outputConsole(std::shared_ptr<event_pair> node, uint64_t width, std::ostringstream& sstr, uint64_t depth) {
        std::ostringstream line;
        bool extended = false;
        if (auto dlopenNode = dynamic_cast<dlopen *>(node.get())) {
            line << "dlopen(\"" << dlopenNode->path << "\", " << dlopenNode->flagString() << ") -> 0x" << dlopenNode->result;
        } else if (auto dlopenPreflightNode = dynamic_cast<dlopen_preflight *>(node.get())) {
            line << "dlopen_preflight(\"" << dlopenPreflightNode->path << ") -> 0x" << dlopenPreflightNode->result;
        } else if (auto dlsymNode = dynamic_cast<dlsym *>(node.get())) {
            line << "dlsym(0x" << std::hex << dlsymNode->handle << ", \"" << dlsymNode->symbol << "\") -> " << dlsymNode->result;
        } else if (auto mapImageNode = dynamic_cast<map_image *>(node.get())) {
            line << "map file \"" << mapImageNode->path << "\" -> 0x" << std::hex << mapImageNode->result;
        } else if (auto sigNode = dynamic_cast<attach_signature *>(node.get())) {
            line << "attach codesignature";
        } else if (auto buildClosureNode = dynamic_cast<build_closure *>(node.get())) {
            line << "build closure -> " << buildClosureNode->buildStateString();
        } else if (auto launchNode = dynamic_cast<app_launch *>(node.get())) {
            line << "app launch (dyld" << std::dec << launchNode->launchMode << ") -> 0x" << std::hex << launchNode->address;
        } else if (auto initNode  = dynamic_cast<static_init *>(node.get())) {
            line << "run static initializer 0x" << std::hex << initNode->funcAddress;
        } else if (auto initNode  = dynamic_cast<apply_fixups *>(node.get())) {
            line << "apply fixups";
        } else if (auto dlcloseNode  = dynamic_cast<dlclose *>(node.get())) {
            line << "dlclose(0x" << std::hex << dlcloseNode->handle << ") -> " << dlcloseNode->result;
        } else if (auto dladdrNode  = dynamic_cast<dladdr *>(node.get())) {
            line << "dladdr(0x" << dladdrNode->address << ") -> image: 0x" << std::hex << dladdrNode->imageAddress;
            line << ", symbol: 0x" << dladdrNode->symbolAddress;
        } else if (auto addImageNode  = dynamic_cast<add_image_callback *>(node.get())) {
            line << std::hex << "add image callback(0x" << addImageNode->funcAddress << ") for image 0x" << addImageNode->libraryAddress;
        } else if (auto removeImageNode  = dynamic_cast<remove_image_callback *>(node.get())) {
            line << std::hex << "remove image callback(0x" << removeImageNode->funcAddress << ") for image 0x" << removeImageNode->libraryAddress;
        } else if (auto objcInitNode  = dynamic_cast<objc_image_init *>(node.get())) {
            line << std::hex << "objC init image(0x" << objcInitNode->libraryAddress << ")";
        } else if (auto objcMapNode  = dynamic_cast<objc_images_map *>(node.get())) {
            line << std::hex << "objC map images callback";
        }

        if (width > MAXCOLS) {
            extended = true;
        }

        std::string timestampStr = timestamp(node, extended);
        std::string lineStr = line.str();
        std::string commandStr = process(node, extended);
        std::string durationStr = duration(node);
        size_t lineMax = (size_t)width - (timestampStr.length() + commandStr.length() + durationStr.length() + 2*(size_t)depth + 3);
        lineStr.resize(lineMax, ' ');

        sstr << timestampStr << " ";
        std::fill_n(std::ostream_iterator<char>(sstr), 2*depth, ' ');
        sstr << lineStr << " " << durationStr << " " << commandStr << std::endl;

        for (const auto& child : node->children()) {
            outputConsole(child, width, sstr, depth+1);
        }
    }

    void outputJSON(std::shared_ptr<event_pair> node, std::ostringstream& sstr) {
        if (auto dlopenNode = dynamic_cast<dlopen *>(node.get())) {
            sstr << std::hex;
            sstr << "{\"type\":\"dlopen\",\"path\":\"" << dlopenNode->path << "\",\"flags\":\"0x" << dlopenNode->flags << "\"";
            sstr << ",\"result\":\"" << dlopenNode->result << "\"";
        } else if (auto dlopenPreflightNode = dynamic_cast<dlopen_preflight *>(node.get())) {
            sstr << std::hex;
            sstr << "{\"type\":\"dlopen_preflight\",\"path\":\"" << dlopenPreflightNode->path << "\"";
            sstr << ",\"result\":\"" << dlopenPreflightNode->result << "\"";
        } else if (auto dlsymNode = dynamic_cast<dlsym *>(node.get())) {
            sstr << std::hex  << "{\"type\":\"dlsym\",\"symbol\":\"" << dlsymNode->symbol << "\",\"handle\":\"0x";
            sstr << dlsymNode->handle << "\",\"result\":\"0x" << dlsymNode->result << "\"";
        } else if (auto mapImageNode = dynamic_cast<map_image *>(node.get())) {
            sstr << std::hex;
            sstr << "{\"type\":\"map_image\",\"path\":\"" << mapImageNode->path << "\",\"result\":\"0x" << mapImageNode->result << "\"";
        } else if (auto sigNode = dynamic_cast<attach_signature *>(node.get())) {
            sstr << "{\"type\":\"attach_codesignature\"";
        } else if (auto buildClosureNode = dynamic_cast<build_closure *>(node.get())) {
            sstr << "{\"type\":\"build_closure\", \"state\":\"" << buildClosureNode->buildStateString() << "\"";
        } else if (auto launchNode = dynamic_cast<app_launch *>(node.get())) {
            sstr << std::hex;
            sstr << "{\"type\":\"app_launch\",\"address\":\"0x";
            sstr << launchNode->address << "\",\"mode\":" << launchNode->launchMode << "";
        } else if (auto initNode  = dynamic_cast<static_init *>(node.get())) {
            sstr << std::hex;
            sstr << "{\"type\":\"static_init\",\"image_address\":\"0x"  << initNode->libraryAddress;
            sstr << "\",\"function_address\":\"0x" << initNode->funcAddress << "\"";
        } else if (auto initNode  = dynamic_cast<apply_fixups *>(node.get())) {
            sstr << "{\"type\":\"apply_fixups\"";
        } else if (auto dlcloseNode  = dynamic_cast<dlclose *>(node.get())) {
            sstr << std::hex << "{\"type\":\"dlclose\",\"handle\":\"0x";
            sstr << dlcloseNode->handle << "\",\"result\":\"0x" << dlcloseNode->result << "\"";
        } else if (auto dladdrNode  = dynamic_cast<dladdr *>(node.get())) {
            sstr << std::hex << "{\"type\":\"dladdr\",\"address\":\"0x" << dladdrNode->address << "\",\"result\":\"0x";
            sstr << dladdrNode->result << "\",\"symbol_address\":\"0x" << dladdrNode->symbolAddress;
            sstr << "\",\"image_address\":\"0x" << dladdrNode->imageAddress << "\"";
        } else {
            sstr << "{\"type\":\"unknown\"";
        }

        if (!node->children().empty()) {
            bool firstChild = true;
            sstr << ",\"children\":[";
            for (const auto& child : node->children()) {
                if (!firstChild) {
                    sstr << ",";
                }
                firstChild = false;
                outputJSON(child, sstr);
            }
            sstr << "]";
        }
        sstr << std::dec << ",\"start_nano\":\"" << mach_to_nano(node->startTimestamp());
        sstr << "\",\"end_nano\":\"" << mach_to_nano(node->endTimestamp()) << "\"}";
    }

    void outputTracingJSON(std::shared_ptr<event_pair> node, std::ostringstream& sstr) {
        auto emitEventInfo = [&](bool isStart) {
            if (auto dlopenNode = dynamic_cast<dlopen *>(node.get())) {
                sstr << "{\"name\": \"dlopen(" << dlopenNode->path << ")\", \"cat\": \"" << "dlopen" << "\"";
            } else if (auto dlopenPreflightNode = dynamic_cast<dlopen_preflight *>(node.get())) {
                sstr << "{\"name\": \"dlopen_preflight(" << dlopenPreflightNode->path << ")\", \"cat\": \"" << "dlopen_preflight" << "\"";
            } else if (auto dlsymNode = dynamic_cast<dlsym *>(node.get())) {
                sstr << "{\"name\": \"dlsym(" << dlsymNode->symbol << ")\", \"cat\": \"" << "dlsym" << "\"";
            } else if (auto mapImageNode = dynamic_cast<map_image *>(node.get())) {
                sstr << "{\"name\": \"map_image(" << mapImageNode->path << ")\", \"cat\": \"" << "map_image" << "\"";
            } else if (auto sigNode = dynamic_cast<attach_signature *>(node.get())) {
                sstr << "{\"name\": \"" << "attach_codesignature" << "\", \"cat\": \"" << "attach_codesignature" << "\"";
            } else if (auto buildClosureNode = dynamic_cast<build_closure *>(node.get())) {
                sstr << "{\"name\": \"" << "build_closure" << "\", \"cat\": \"" << "build_closure" << "\"";
            } else if (auto launchNode = dynamic_cast<app_launch *>(node.get())) {
                sstr << "{\"name\": \"" << "app_launch" << "\", \"cat\": \"" << "app_launch" << "\"";
            } else if (auto initNode  = dynamic_cast<static_init *>(node.get())) {
                sstr << "{\"name\": \"" << "static_init" << "\", \"cat\": \"" << "static_init" << "\"";
            } else if (auto initNode  = dynamic_cast<apply_fixups *>(node.get())) {
                sstr << "{\"name\": \"" << "apply_fixups" << "\", \"cat\": \"" << "apply_fixups" << "\"";
            } else if (auto dlcloseNode  = dynamic_cast<dlclose *>(node.get())) {
                sstr << "{\"name\": \"" << "dlclose" << "\", \"cat\": \"" << "dlclose" << "\"";
            } else if (auto dladdrNode  = dynamic_cast<dladdr *>(node.get())) {
                sstr << "{\"name\": \"" << "dladdr" << "\", \"cat\": \"" << "dladdr" << "\"";
            } else if (auto addImageNode  = dynamic_cast<add_image_callback *>(node.get())) {
                sstr << "{\"name\": \"" << "add_image" << "\", \"cat\": \"" << "add_image" << "\"";
            } else if (auto removeImageNode  = dynamic_cast<remove_image_callback *>(node.get())) {
                sstr << "{\"name\": \"" << "remove_image" << "\", \"cat\": \"" << "remove_image" << "\"";
            } else if (auto objcInitNode  = dynamic_cast<objc_image_init *>(node.get())) {
                sstr << "{\"name\": \"" << "objc_init" << "\", \"cat\": \"" << "objc_init" << "\"";
            } else if (auto objcMapNode  = dynamic_cast<objc_images_map *>(node.get())) {
                sstr << "{\"name\": \"" << "objc_map" << "\", \"cat\": \"" << "objc_map" << "\"";
            } else {
                sstr << "{\"name\": \"" << "unknown" << "\", \"cat\": \"" << node->eventCode() << "\"";
            }
            if (isStart) {
                sstr << ", \"ph\": \"B\", \"pid\": " << _pid << ", \"tid\": " << _threadid << ", \"ts\": " << mach_to_nano(node->startTimestamp()) << "},";
            } else {
                sstr << ", \"ph\": \"E\", \"pid\": " << _pid << ", \"tid\": " << _threadid << ", \"ts\": " << mach_to_nano(node->endTimestamp()) << "}";
            }
        };

        emitEventInfo(true);
        emitEventInfo(false);

        if (!node->children().empty()) {
            for (const auto& child : node->children()) {
                sstr << ", ";
                outputTracingJSON(child, sstr);
            }
        }
    }

    const std::vector<std::shared_ptr<event_pair>>& rootEvents() const { return _rootEvents; }

private:

    void output(std::shared_ptr<event_pair> root) {
        std::ostringstream ostream;
        if (JSON_flag) {
            ostream << "{\"command\":\"" << _commandName << "\",\"pid\":\"" << _pid << "\",\"thread\":\"";
            ostream << _threadid << "\", \"event\":";
            outputJSON(root, ostream);
            ostream << "}" << std::endl;
        } else if (JSON_Tracing_flag) {
            _rootEvents.push_back(root);
        } else {
            outputConsole(root, columns, ostream, 0);
        }
        std::cout << ostream.str();
        if (!RAW_flag)
            fflush(stdout);
    }

    struct dlopen : event_pair {
    dlopen(ktrace_event_t E) : event_pair(E), path(stringForID(E->arg2)), flags((int)E->arg3) {}
        std::string flagString() {
            std::vector<std::string> flagStrs;
            uint64_t flagCheck = 0;
            std::string flagString;

            if (flags & RTLD_LAZY) {
                flagStrs.push_back("RTLD_LAZY");
                flagCheck |= RTLD_LAZY;
            }
            if (flags & RTLD_NOW) {
                flagStrs.push_back("RTLD_NOW");
                flagCheck |= RTLD_NOW;
            }
            if (flags & RTLD_LOCAL) {
                flagStrs.push_back("RTLD_LOCAL");
                flagCheck |= RTLD_LOCAL;
            }
            if (flags & RTLD_GLOBAL) {
                flagStrs.push_back("RTLD_GLOBAL");
                flagCheck |= RTLD_GLOBAL;
            }
            if (flags & RTLD_NOLOAD) {
                flagStrs.push_back("RTLD_NOLOAD");
                flagCheck |= RTLD_NOLOAD;
            }
            if (flags & RTLD_NODELETE) {
                flagStrs.push_back("RTLD_NODELETE");
                flagCheck |= RTLD_NODELETE;
            }
            if (flags & RTLD_FIRST) {
                flagStrs.push_back("RTLD_FIRST");
                flagCheck |= RTLD_FIRST;
            }

            if (flagCheck == flags) {
                for (auto& flagStr : flagStrs) {
                    if (!flagString.empty()) {
                        flagString += "|";
                    }
                    flagString += flagStr;
                }
            }

            return flagString;
        }
        std::string path;
        int flags;
        uint64_t result;
    };

    struct dlopen_preflight : event_pair {
    dlopen_preflight(ktrace_event_t E) : event_pair(E), path(stringForID(E->arg2)) {}
        std::string path;
        uint64_t result;
    };

    struct dlsym : event_pair {
        dlsym(ktrace_event_t E) : event_pair(E), handle(E->arg2), symbol(stringForID(E->arg3)) {}
        std::string symbol;
        uint64_t handle;
        uint64_t result;
    };

    struct dladdr : event_pair {
        dladdr(ktrace_event_t E) : event_pair(E), address(E->arg2), imageAddress(0), symbolAddress(0)  {}
        uint64_t address;
        uint64_t imageAddress;
        uint64_t symbolAddress;
        int result;
    };

    struct dlclose : event_pair {
        dlclose(ktrace_event_t E) : event_pair(E), handle(E->arg2) {}
        uint64_t handle;
        int result;
    };

    struct app_launch : event_pair {
        app_launch(ktrace_event_t E) : event_pair(E), address(E->arg2) {}
        uint64_t address;
        uint64_t launchMode;
        std::vector<event_pair *> _children;
    };

    struct static_init : event_pair {
        static_init(ktrace_event_t E) : event_pair(E), libraryAddress(E->arg2), funcAddress(E->arg3) {}
        uint64_t libraryAddress;
        uint64_t funcAddress;
    };

    struct map_image : event_pair {
        map_image(ktrace_event_t E) : event_pair(E), path(stringForID(E->arg2)) {}
        std::string path;
        uint64_t result;
    };

    struct apply_fixups : event_pair {
        apply_fixups(ktrace_event_t E) : event_pair(E) {}
    };

    struct attach_signature : event_pair {
        attach_signature(ktrace_event_t E) : event_pair(E) {}
    };

    struct build_closure : event_pair {
        build_closure(ktrace_event_t E) : event_pair(E), closureBuildState(0) {}
        uint64_t closureBuildState;

        std::string buildStateString() const {
            switch ((dyld3::DyldTimingBuildClosure)closureBuildState) {
                case dyld3::DyldTimingBuildClosure::ClosureBuildFailure:
                    return "failed to build closure";
                case dyld3::DyldTimingBuildClosure::LaunchClosure_Built:
                    return "built launch closure";
                case dyld3::DyldTimingBuildClosure::DlopenClosure_UsedSharedCacheDylib:
                    return "used shared cache dylib closure";
                case dyld3::DyldTimingBuildClosure::DlopenClosure_UsedSharedCacheOther:
                    return "used shared cache dlopen closure";
                case dyld3::DyldTimingBuildClosure::DlopenClosure_NoLoad:
                    return "dlopen was no load";
                case dyld3::DyldTimingBuildClosure::DlopenClosure_Built:
                    return "built dlopen closure";
            }
        };
    };

    struct add_image_callback : event_pair {
        add_image_callback(ktrace_event_t E) : event_pair(E), libraryAddress(E->arg2), funcAddress(E->arg3) {}
        uint64_t libraryAddress;
        uint64_t funcAddress;
    };

    struct remove_image_callback : event_pair {
        remove_image_callback(ktrace_event_t E) : event_pair(E), libraryAddress(E->arg2), funcAddress(E->arg3) {}
        uint64_t libraryAddress;
        uint64_t funcAddress;

    };

    struct objc_image_init : event_pair {
        objc_image_init(ktrace_event_t E) : event_pair(E), libraryAddress(E->arg2) {}
        uint64_t libraryAddress;
    };

    struct objc_images_map : event_pair {
        objc_images_map(ktrace_event_t E) : event_pair(E) {}
    };

    std::shared_ptr<event_pair> _currentRootEvent;
    std::vector<std::shared_ptr<event_pair>> _eventStack;
    std::vector<std::shared_ptr<event_pair>> _rootEvents;
};

struct OutputManager {
    std::map<unsigned long, std::unique_ptr<output_renderer>> sOutputRenders;

    void flush() {
        if (JSON_Tracing_flag) {
            std::ostringstream ostream;
            ostream << "{\"displayTimeUnit\":\"ns\"";
            ostream << ", \"traceEvents\": [";
            bool firstEvent = true;
            for (const auto& renderer : sOutputRenders) {
                for (const auto& root : renderer.second->rootEvents()) {
                    if (firstEvent)
                        firstEvent = false;
                    else
                        ostream << ", ";
                    renderer.second->outputTracingJSON(root, ostream);
                }
            }
            ostream << "]";
            ostream << "}" << std::endl;
            std::cout << ostream.str();
            if (!RAW_flag)
                fflush(stdout);
        }
    }
};

static OutputManager sOutputManager;

void
setup_ktrace_callbacks(void)
{
    ktrace_events_single(s, TRACEDBG_CODE(DBG_TRACE_STRING, TRACE_STRING_GLOBAL), ^(ktrace_event_t event){
        char argChars[33] = {0};
        memset(&argChars[0], 0, 33);
        if ((event->debugid & DBG_FUNC_START) == DBG_FUNC_START) {
            uint64_t str_id = event->arg2;
            if (((event->debugid & DBG_FUNC_END) == DBG_FUNC_END)
                && str_id != 0) {
                auto i = gActiveStrings.find(str_id);
                if (i != gActiveStrings.end()) {
                    gActiveStrings.erase(i);
                }
            }
            *((uint64_t*)&argChars[0]) = event->arg3;
            *((uint64_t*)&argChars[8]) = event->arg4;
            gActiveStringIDs.insert(std::make_pair(event->threadid, str_id));
            gActiveStrings.insert(std::make_pair(str_id, argChars));
        } else {
            // Not a start, so lets grab our data
            *((uint64_t*)&argChars[0]) = event->arg1;
            *((uint64_t*)&argChars[8]) = event->arg2;
            *((uint64_t*)&argChars[16]) = event->arg3;
            *((uint64_t*)&argChars[24]) = event->arg4;

            auto i = gActiveStringIDs.find(event->threadid);
            if (i != gActiveStringIDs.end()) {
                auto j = gActiveStrings.find(i->second);
                if (j != gActiveStrings.end()) {
                    j->second += argChars;
                }
            }
        }

        if ((event->debugid & DBG_FUNC_END) == DBG_FUNC_END) {
            auto i = gActiveStringIDs.find(event->threadid);
            if (i != gActiveStringIDs.end()) {
                gActiveStringIDs.erase(i);
            }
        };
    });

    // Event though our events are paired, we process them individually so we can
    // render nested events
    ktrace_events_range(s, KDBG_EVENTID(DBG_DYLD, DBG_DYLD_INTERNAL_SUBCLASS, 0), KDBG_EVENTID(DBG_DYLD, DBG_DYLD_API_SUBCLASS+1, 0), ^(ktrace_event_t event){
        if ((event->debugid & KDBG_FUNC_MASK) == 0)
            return;
        auto i = sOutputManager.sOutputRenders.find((size_t)(event->threadid));
        if (i == sOutputManager.sOutputRenders.end()) {
            sOutputManager.sOutputRenders.emplace(std::make_pair(event->threadid, std::make_unique<output_renderer>(s, event)));
            i = sOutputManager.sOutputRenders.find((size_t)(event->threadid));
        }
        i->second->recordEvent(event);
        if (i->second->empty()) {
            sOutputManager.sOutputRenders.erase(i);
        }
    });
}

int
main(int argc, char *argv[])
{
    char ch;
    int rv = 0;
    bool exclude_pids = false;
    uint64_t time_limit_ns = 0;

    get_screenwidth();

    s = ktrace_session_create();
    assert(s);

    while ((ch = getopt(argc, argv, "jJeR:t:")) != -1) {
        switch (ch) {
            case 'j':
                JSON_flag = true;
                break;
            case 'J':
                JSON_Tracing_flag = true;
                break;
            case 'e':
                exclude_pids = true;
                break;
            case 't':
                time_limit_ns = (uint64_t)(NSEC_PER_SEC * atof(optarg));
                if (time_limit_ns == 0) {
                    fprintf(stderr, "ERROR: could not set time limit to %s\n",
                            optarg);
                    exit(1);
                }
                break;
            case 'R':
                RAW_flag = true;
                rv = ktrace_set_file(s, optarg);
                if (rv) {
                    fprintf(stderr, "ERROR: reading trace from '%s' failed (%s)\n", optarg, strerror(errno));
                    exit(1);
                }
                break;
            default:
                exit_usage();
        }
    }

    argc -= optind;
    argv += optind;

    if (time_limit_ns > 0) {
        if (RAW_flag) {
            fprintf(stderr, "NOTE: time limit ignored when a raw file is specified\n");
        } else {
            dispatch_after(dispatch_time(DISPATCH_TIME_NOW, time_limit_ns),
                           dispatch_get_global_queue(QOS_CLASS_USER_INITIATED, 0),
                           ^{
                               ktrace_end(s, 0);
                           });
        }
    }

    if (!RAW_flag) {
        if (geteuid() != 0) {
            fprintf(stderr, "'dyld_usage' must be run as root...\n");
            exit(1);
        }

        /*
         * ktrace can't both *in*clude and *ex*clude pids, so: if we are
         * already excluding pids, or if we are not explicitly including
         * or excluding any pids, then exclude the defaults.
         *
         * if on the other hand we are explicitly including pids, we'll
         * filter the defaults out naturally.
         */
        if (exclude_pids || argc == 0) {
            ktrace_exclude_process(s, "dyld_usage");
            ktrace_exclude_process(s, "Terminal");
            ktrace_exclude_process(s, "telnetd");
            ktrace_exclude_process(s, "telnet");
            ktrace_exclude_process(s, "sshd");
            ktrace_exclude_process(s, "rlogind");
            ktrace_exclude_process(s, "tcsh");
            ktrace_exclude_process(s, "csh");
            ktrace_exclude_process(s, "sh");
            ktrace_exclude_process(s, "zsh");
#if TARGET_OS_EMBEDDED
            ktrace_exclude_process(s, "dropbear");
#endif /* TARGET_OS_EMBEDDED */
        }
    }

    /*
     * Process the list of specified pids, and in/exclude them as
     * appropriate.
     */
    while (argc > 0) {
        pid_t pid;
        char *name;
        char *endptr;

        name = argv[0];
        pid = (pid_t)strtoul(name, &endptr, 10);

        if (*name != '\0' && *endptr == '\0') {
            if (exclude_pids) {
                rv = ktrace_exclude_pid(s, pid);
            } else {
                if (pid != 0)
                    rv = ktrace_filter_pid(s, pid);
            }
        } else {
            if (exclude_pids) {
                rv = ktrace_exclude_process(s, name);
            } else {
                if (strcmp(name, "kernel_task"))
                    rv = ktrace_filter_process(s, name);
            }
        }

        if (rv == EINVAL) {
            fprintf(stderr, "ERROR: cannot both include and exclude simultaneously\n");
            exit(1);
        } else {
            assert(!rv);
        }

        argc--;
        argv++;
    }
    /* provides SIGINT, SIGHUP, SIGPIPE, SIGTERM handlers */
    ktrace_set_signal_handler(s);
    ktrace_set_completion_handler(s, ^{
        sOutputManager.flush();
        exit(0);
    });

    signal(SIGWINCH, SIG_IGN);
    sigwinch_source = dispatch_source_create(DISPATCH_SOURCE_TYPE_SIGNAL, SIGWINCH, 0, dispatch_get_main_queue());
    dispatch_source_set_event_handler(sigwinch_source, ^{
        get_screenwidth();
    });
    dispatch_activate(sigwinch_source);

    setup_ktrace_callbacks();

    ktrace_set_dropped_events_handler(s, ^{
        fprintf(stderr, "dyld_usage: buffer overrun, events generated too quickly\n");

        /* clear any state that is now potentially invalid */
    });

    ktrace_set_execnames_enabled(s, KTRACE_FEATURE_LAZY);
    ktrace_set_vnode_paths_enabled(s, false);
    /* no need to symbolicate addresses */
    ktrace_set_uuid_map_enabled(s, KTRACE_FEATURE_DISABLED);

    rv = ktrace_start(s, dispatch_get_main_queue());

    if (rv) {
        perror("ktrace_start");
        exit(1);
    }

    dispatch_main();

    return 0;
}
