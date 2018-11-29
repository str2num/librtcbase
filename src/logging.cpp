/*
 *  Copyright (c) 2018 str2num. All Rights Reserved.
 *  Copyright (c) 2011, The WebRTC project authors. All rights reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree.
 */
  
/**
 * @file logging.cpp
 * @author str2num
 * @brief 
 *  
 **/

#include <algorithm>
#include <iomanip>
#include <ostream>
#include <vector>
#include <string.h>

#include <pthread.h>
#include <time.h>
#include <limits.h>

#include "critical_section.h"
#include "time_utils.h"
#include "string_encode.h"
#include "logging.h"

namespace rtcbase {

static const char k_libice[] = "ICE";

// By default, release builds don't log, debug builds at info level
#if !defined(NDEBUG)
LoggingSeverity LogMessage::_min_sev = LS_NOTICE;
LoggingSeverity LogMessage::_dbg_sev = LS_NOTICE;
#else
LoggingSeverity LogMessage::_min_sev = LS_NONE;
LoggingSeverity LogMessage::_dbg_sev = LS_NONE;
#endif
bool LogMessage::_log_to_stderr = true;

namespace {
    // Global lock for log subsystem, only needed to serialize access to _streams.
    CriticalSection g_log_crit;
}  // namespace

// The list of logging streams currently configured.
// Note: we explicitly do not clean this up, because of the uncertain ordering
// of destructors at program exit.  Let the person who sets the stream trigger
// cleanup by setting to NULL, or let it leak (safe at program exit).
LogMessage::StreamList LogMessage::_streams GUARDED_BY(g_log_crit);

// Boolean options default to false (0)
bool LogMessage::_thread = false;
bool LogMessage::_timestamp = false;

LogMessage::LogMessage(const char* file,
        int line,
        LoggingSeverity sev,
        LogErrorContext err_ctx,
        int err,
        const char* module)
    : _severity(sev), _tag(k_libice) 
{
    (void)module;

    if (_thread) {
        pthread_t id = pthread_self();
        _print_stream << "[" << std::dec << id << "] ";
    }

    if (_timestamp) {
        // Use SystemTimeMillis so that even if tests use fake clocks, the timestamp
        // in log messages represents the real system time.
        int64_t time = time_diff(system_time_millis(), log_start_time());
        // Also ensure WallClockStartTime is initialized, so that it matches
        // LogStartTime.
        wall_clock_start_time();
        _print_stream << "[" << std::setfill('0') << std::setw(3) << (time / 1000)
            << ":" << std::setw(3) << (time % 1000) << std::setfill(' ')
            << "] ";
    }

    if (file != NULL) {
        //_print_stream << "(" << filename_from_path(file)  << ":" << line << "): ";
        _print_stream << "(" << file  << ":" << line << "): ";
    }
    
    if (err_ctx != ERRCTX_NONE) {
        std::ostringstream tmp;
        tmp << "[0x" << std::setfill('0') << std::hex << std::setw(8) << err << "]";
        switch (err_ctx) {
            case ERRCTX_ERRNO:
                tmp << " " << strerror(err);
                break;
            default:
                break;
        }
        _extra = tmp.str();
    }
}

LogMessage::LogMessage(const char* file,
        int line,
        LoggingSeverity sev,
        const std::string& tag)
    : LogMessage(file, line, sev, ERRCTX_NONE, 0 /* err */, NULL /* module */) 
{
    _tag = tag;
    _print_stream << tag << ": ";
}

LogMessage::~LogMessage() {
    if (!_extra.empty()) {
        _print_stream << " : " << _extra;
    }
    _print_stream << std::endl;
    
    const std::string& str = _print_stream.str();
    if (_severity >= _dbg_sev) {
        output_to_debug(str, _severity, _tag);
    }
    
    CritScope cs(&g_log_crit);
    for (auto& kv : _streams) {
        if (_severity >= kv.second) {
            kv.first->on_log_message(str, _severity);
        }
    }
}

int64_t LogMessage::log_start_time() {
    static const int64_t g_start = system_time_millis();
    return g_start;
}

uint32_t LogMessage::wall_clock_start_time() {
    static const uint32_t g_start_wallclock = time(NULL);
    return g_start_wallclock;
}

void LogMessage::log_threads(bool on) {
    _thread = on;
}

void LogMessage::log_timestamps(bool on) {
    _timestamp = on;
}

void LogMessage::log_to_debug(LoggingSeverity min_sev) {
    _dbg_sev = min_sev;
    CritScope cs(&g_log_crit);
    update_min_log_severity();
}

void LogMessage::set_log_to_stderr(bool log_to_stderr) {
    _log_to_stderr = log_to_stderr;
}

int LogMessage::get_log_to_stream(LogSink* stream) {
    CritScope cs(&g_log_crit);
    LoggingSeverity sev = LS_NONE;
    for (auto& kv : _streams) {
        if (!stream || stream == kv.first) {
            sev = std::min(sev, kv.second);
        }
    }
    return sev;
}

void LogMessage::add_log_to_stream(LogSink* stream, LoggingSeverity min_sev) {
    CritScope cs(&g_log_crit);
    _streams.push_back(std::make_pair(stream, min_sev));
    update_min_log_severity();
}

void LogMessage::remove_log_to_stream(LogSink* stream) {
    CritScope cs(&g_log_crit);
    for (StreamList::iterator it = _streams.begin(); it != _streams.end(); ++it) {
        if (stream == it->first) {
            _streams.erase(it);
            break;
        }
    }
    update_min_log_severity();
}

void LogMessage::configure_logging(const char* params) {
    LoggingSeverity current_level = LS_TRACE;
    LoggingSeverity debug_level = get_log_to_debug();

    std::vector<std::string> tokens;
    tokenize(params, ' ', &tokens);

    for (const std::string& token : tokens) {
        if (token.empty()) {
            continue;
        }

        // Logging features
        if (token == "tstamp") {
            log_timestamps();
        } else if (token == "thread") {
            log_threads();

            // Logging levels
        } else if (token == "debug") {
            current_level = LS_DEBUG;
        } else if (token == "trace") {
            current_level = LS_TRACE;
        } else if (token == "notice") {
            current_level = LS_NOTICE;
        } else if (token == "warning") {
            current_level = LS_WARNING;
        } else if (token == "fatal") {
            current_level = LS_FATAL;
        } else if (token == "none") {
            current_level = LS_NONE;
        }
        
        // Logging targets
        if (token == "debug") {
            debug_level = current_level;
        }
    }

    log_to_debug(debug_level);
}

void LogMessage::update_min_log_severity() EXCLUSIVE_LOCKS_REQUIRED(g_log_crit) {
    LoggingSeverity min_sev = _dbg_sev;
    for (auto& kv : _streams) {
        min_sev = std::min(_dbg_sev, kv.second);
    }
    _min_sev = min_sev;
}

void LogMessage::output_to_debug(const std::string& str,
        LoggingSeverity severity,
        const std::string& tag) 
{
    (void)severity;
    (void)tag;

    bool log_to_stderr = _log_to_stderr;
    if (log_to_stderr) {
        fprintf(stderr, "%s", str.c_str());
        fflush(stderr);
    }
}

} // namespace rtcbase


