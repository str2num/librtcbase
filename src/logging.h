/*
 *  Copyright (c) 2018 str2num. All Rights Reserved.
 *  Copyright (c) 2011, The WebRTC project authors. All rights reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree.
 */
 
 
/**
 * @file logging.h
 * @author str2num
 * @brief 
 *  
 **/


#ifndef  __RTCBASE_LOGGING_H_
#define  __RTCBASE_LOGGING_H_

#include <list>
#include <sstream>
#include <string>
#include <utility>

#include <errno.h>

#include "constructor_magic.h"

namespace rtcbase {

enum LoggingSeverity {
    LS_DEBUG,
    LS_TRACE,
    LS_NOTICE,
    LS_WARNING,
    LS_FATAL,
    LS_NONE,
    NOTICE = LS_NOTICE,
    WARNING = LS_WARNING,
    LFATAL = LS_FATAL
};

// LogErrorContext assists in interpreting the meaning of an error value.
enum LogErrorContext {
    ERRCTX_NONE,
    ERRCTX_ERRNO,     // System-local errno
    ERRCTX_HRESULT,   // Windows HRESULT
    ERRCTX_OSSTATUS,  // MacOS OSStatus

    // Abbreviations for LOG_E macro
    ERRCTX_EN = ERRCTX_ERRNO,     // LOG_E(sev, EN, x)
    ERRCTX_HR = ERRCTX_HRESULT,   // LOG_E(sev, HR, x)
    ERRCTX_OS = ERRCTX_OSSTATUS,  // LOG_E(sev, OS, x)
};

// Virtual sink interface that can receive log messages.
class LogSink {
public:
    LogSink() {}
    virtual ~LogSink() {}
    virtual void on_log_message(const std::string& message, LoggingSeverity severity) = 0;
};

class LogMessage {
public:
    LogMessage(const char* file, int line, LoggingSeverity sev,
            LogErrorContext err_ctx = ERRCTX_NONE, int err = 0,
            const char* module = NULL);

    LogMessage(const char* file,
            int line,
            LoggingSeverity sev,
            const std::string& tag);

    ~LogMessage();

    static inline bool loggable(LoggingSeverity sev) { return (sev >= _min_sev); }
    std::ostream& stream() { return _print_stream; }
    
    // Returns the time at which this function was called for the first time.
    // The time will be used as the logging start time.
    // If this is not called externally, the LogMessage ctor also calls it, in
    // which case the logging start time will be the time of the first LogMessage
    // instance is created.
    static int64_t log_start_time();

    // Returns the wall clock equivalent of |LogStartTime|, in seconds from the
    // epoch.
    static uint32_t wall_clock_start_time();

    //  LogThreads: Display the thread identifier of the current thread
    static void log_threads(bool on = true);

    //  LogTimestamps: Display the elapsed time of the program
    static void log_timestamps(bool on = true);

    // These are the available logging channels
    //  Debug: Debug console on Windows, otherwise stderr
    static void log_to_debug(LoggingSeverity min_sev);
    static LoggingSeverity get_log_to_debug() { return _dbg_sev; }
    
    // Sets whether logs will be directed to stderr in debug mode.
    static void set_log_to_stderr(bool log_to_stderr); 

    //  Stream: Any non-blocking stream interface.  LogMessage takes ownership of
    //   the stream. Multiple streams may be specified by using AddLogToStream.
    //   LogToStream is retained for backwards compatibility; when invoked, it
    //   will discard any previously set streams and install the specified stream.
    //   GetLogToStream gets the severity for the specified stream, of if none
    //   is specified, the minimum stream severity.
    //   RemoveLogToStream removes the specified stream, without destroying it.
    static int get_log_to_stream(LogSink* stream = NULL);
    static void add_log_to_stream(LogSink* stream, LoggingSeverity min_sev);
    static void remove_log_to_stream(LogSink* stream);     
    
    // Parses the provided parameter stream to configure the options above.
    // Useful for configuring logging from the command line.
    static void configure_logging(const char* params);

private:
    // Updates min_sev_ appropriately when debug sinks change.
    static void update_min_log_severity();
    
    // These write out the actual log messages.
    static void output_to_debug(const std::string& msg,
            LoggingSeverity severity,
            const std::string& tag);

private:
    typedef std::pair<LogSink*, LoggingSeverity> StreamAndSeverity;
    typedef std::list<StreamAndSeverity> StreamList;

private: 
    // The ostream that buffers the formatted message before output
    std::ostringstream _print_stream;
    
    // The severity level of this message
    LoggingSeverity _severity;

    // The Android debug output tag.
    std::string _tag;

    // String data generated in the constructor, that should be appended to
    // the message before output.
    std::string _extra;

    // dbg_sev_ is the thresholds for those output targets
    // min_sev_ is the minimum (most verbose) of those levels, and is used
    //  as a short-circuit in the logging macros to identify messages that won't
    //  be logged.
    // ctx_sev_ is the minimum level at which file context is displayed
    static LoggingSeverity _min_sev, _dbg_sev, _ctx_sev;

    // The output streams and their associated severities
    static StreamList _streams;    

    // Flags for formatting options
    static bool _thread, _timestamp;

    // Determines if logs will be directed to stderr in debug mode.
    static bool _log_to_stderr;

    RTC_DISALLOW_COPY_AND_ASSIGN(LogMessage);  
};

class LogMessageVoidify {
public:
    LogMessageVoidify() { }
    // This has to be an operator with a precedence lower than << but
    // higher than ?:
    void operator&(std::ostream&) { }
};

#define LOG_SEVERITY_PRECONDITION(sev) \
  !(rtcbase::LogMessage::loggable(sev)) \
    ? (void) 0 \
    : rtcbase::LogMessageVoidify() &

#define LOG(sev) \
    LOG_SEVERITY_PRECONDITION(rtcbase::sev) \
    rtcbase::LogMessage(__FILE__, __LINE__, rtcbase::sev).stream()

// The _V version is for when a variable is passed in.  It doesn't do the
// namespace concatination.
#define LOG_V(sev) \
    LOG_SEVERITY_PRECONDITION(sev) \
    rtcbase::LogMessage(__FILE__, __LINE__, sev).stream()

#define LOG_E(sev, ctx, err, ...) \
    LOG_SEVERITY_PRECONDITION(rtcbase::sev) \
        rtcbase::LogMessage(__FILE__, __LINE__, rtcbase::sev, \
            rtcbase::ERRCTX_ ## ctx, err , ##__VA_ARGS__).stream()

#define LOG_ERRNO_EX(sev, err) LOG_E(sev, ERRNO, err)
#define LOG_ERRNO(sev) LOG_ERRNO_EX(sev, errno)

#define LOG_ERR_EX(sev, err) LOG_ERRNO_EX(sev, err)
#define LOG_ERR(sev) LOG_ERRNO(sev)
#define LAST_SYSTEM_ERROR (errno)

} // namespace rtcbase

#endif  //__RTCBASE_LOGGING_H_


