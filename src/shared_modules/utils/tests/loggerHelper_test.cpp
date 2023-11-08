/*
 * Wazuh shared modules utils
 * Copyright (C) 2015, Wazuh Inc.
 * Apr 24, 2023.
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General Public
 * License (version 2) as published by the FSF - Free Software
 * Foundation.
 */

#include "loggerHelper_test.h"
#include <chrono>
#include <regex>
#include <sstream>

constexpr auto INFO_REGEX = "info Tag .+\\.cpp \\d+ TestBody Testing Info log\\n";
constexpr auto ERROR_REGEX = "error Tag .+\\.cpp \\d+ TestBody Testing Error log\\n";
constexpr auto DEBUG_REGEX = "debug Tag .+\\.cpp \\d+ TestBody Testing Debug log\\n";
constexpr auto DEBUG_VERBOSE_REGEX = "debug_verbose Tag .+\\.cpp \\d+ TestBody Testing Debug Verbose log\\n";
constexpr auto WARNING_REGEX = "warning Tag .+\\.cpp \\d+ TestBody Testing Warning log\\n";

constexpr auto INFO_REGEX_THREAD = "info Tag .+\\.cpp \\d+ operator\\(\\) Testing Info log";
constexpr auto ERROR_REGEX_THREAD = "error Tag .+\\.cpp \\d+ operator\\(\\) Testing Error log";
constexpr auto DEBUG_REGEX_THREAD = "debug Tag .+\\.cpp \\d+ operator\\(\\) Testing Debug log";
constexpr auto DEBUG_VERBOSE_REGEX_THREAD = "debug_verbose Tag .+\\.cpp \\d+ operator\\(\\) Testing Debug Verbose log";
constexpr auto WARNING_REGEX_THREAD = "warning Tag .+\\.cpp \\d+ operator\\(\\) Testing Warning log";

constexpr auto TAG = "Tag";

void debugVerboseTestFunction(const char* tag, const char* file, int line, const char* func, const char* msg)
{
    ssOutput << "debug_verbose"
             << " " << tag << " " << file << " " << line << " " << func << " " << msg << std::endl;
}

void debugTestFunction(const char* tag, const char* file, int line, const char* func, const char* msg)
{
    ssOutput << "debug"
             << " " << tag << " " << file << " " << line << " " << func << " " << msg << std::endl;
}

void infoTestFunction(const char* tag, const char* file, int line, const char* func, const char* msg)
{
    ssOutput << "info"
             << " " << tag << " " << file << " " << line << " " << func << " " << msg << std::endl;
}

void warningTestFunction(const char* tag, const char* file, int line, const char* func, const char* msg)
{
    ssOutput << "warning"
             << " " << tag << " " << file << " " << line << " " << func << " " << msg << std::endl;
}

void errorTestFunction(const char* tag, const char* file, int line, const char* func, const char* msg)
{
    ssOutput << "error"
             << " " << tag << " " << file << " " << line << " " << func << " " << msg << std::endl;
}

void logFunctionWrapper(int level, const char* tag, const char* file, int line, const char* func, const char* msg)
{
    switch (level)
    {
        case (Log::LOGLEVEL_DEBUG): debugTestFunction(tag, file, line, func, msg); break;
        case (Log::LOGLEVEL_DEBUG_VERBOSE): debugVerboseTestFunction(tag, file, line, func, msg); break;
        case (Log::LOGLEVEL_INFO): infoTestFunction(tag, file, line, func, msg); break;
        case (Log::LOGLEVEL_WARNING): warningTestFunction(tag, file, line, func, msg); break;
        case (Log::LOGLEVEL_ERROR): errorTestFunction(tag, file, line, func, msg); break;
        default: break;
    }
}

TEST_F(LoggerHelperTest, simpleInfoTest)
{
    logInfo(TAG, "Testing Info log");
    EXPECT_TRUE(std::regex_match(ssOutput.str(), std::regex(INFO_REGEX)));
}

TEST_F(LoggerHelperTest, simpleErrorTest)
{
    logError(TAG, "Testing Error log");
    EXPECT_TRUE(std::regex_match(ssOutput.str(), std::regex(ERROR_REGEX)));
}

TEST_F(LoggerHelperTest, simpleDebugTest)
{
    logDebug1(TAG, "Testing Debug log");
    EXPECT_TRUE(std::regex_match(ssOutput.str(), std::regex(DEBUG_REGEX)));
}

TEST_F(LoggerHelperTest, simpleDebugVerboseTest)
{
    logDebug2(TAG, "Testing Debug Verbose log");
    EXPECT_TRUE(std::regex_match(ssOutput.str(), std::regex(DEBUG_VERBOSE_REGEX)));
}

TEST_F(LoggerHelperTest, simpleWarningTest)
{
    logWarn(TAG, "Testing Warning log");
    EXPECT_TRUE(std::regex_match(ssOutput.str(), std::regex(WARNING_REGEX)));
}
