/*
 * Wazuh DBSYNC
 * Copyright (C) 2015-2020, Wazuh Inc.
 * June 11, 2020.
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General Public
 * License (version 2) as published by the FSF - Free Software
 * Foundation.
 */

#include <map>
#include <mutex>
#include "dbsync.h"
#include "dbsync_implementation.h"
#ifdef __cplusplus
extern "C" {
#endif
using namespace DbSync;
struct CJsonDeleter
{
    void operator()(char* json)
    {
        cJSON_free(json);
    }
};

static std::mutex gs_logFunctionsMutex;
static std::map<DBSYNC_HANDLE, log_fnc_t> gs_logFunctions;

static void add_log_function(const DBSYNC_HANDLE handle,
                             log_fnc_t log_function)
{
    if (handle && log_function)
    {
        std::lock_guard<std::mutex> lock{ gs_logFunctionsMutex };
        gs_logFunctions[handle] = log_function;
    }
}

static log_fnc_t get_log_function(const DBSYNC_HANDLE handle)
{
    std::lock_guard<std::mutex> lock{ gs_logFunctionsMutex };
    const auto it { gs_logFunctions.find(handle) };
    if (it != gs_logFunctions.end())
    {
        return it->second;
    }
    return nullptr;
}

static void log_message(const DBSYNC_HANDLE handle,
                        const std::string& msg)
{
    if (!msg.empty())
    {
        const auto log_function { get_log_function(handle) };
        if (log_function)
        {
            log_function(msg.c_str());
        }
    }
}

DBSYNC_HANDLE dbsync_initialize(const HostType host_type,
                                const DbEngineType db_type,
                                const char* path,
                                const char* sql_statement,
                                log_fnc_t log_function)
{
    DBSYNC_HANDLE ret_val{ nullptr };
    std::string error_message;
    if (nullptr == path ||
        nullptr == sql_statement)
    {
        error_message += "Invalid path or sql_statement.";
    }
    else
    {
        try
        {
            ret_val = DBSyncImplementation::instance().initialize(host_type, db_type, path, sql_statement);
            add_log_function(ret_val, log_function);
        }
        catch(const nlohmann::detail::exception& ex)
        {
            error_message += "json error, id: " + std::to_string(ex.id) + ". " + ex.what();
        }
        catch(const DbSync::dbsync_error& ex)
        {
            error_message += "DB error, id: " + std::to_string(ex.id()) + ". " + ex.what();
        }
        catch(...)
        {
            error_message += "Unrecognized error.";
        }
    }
    if (log_function && !error_message.empty())
    {
        log_function(error_message.c_str());
    }
    return ret_val;
}

int dbsync_insert_data(const DBSYNC_HANDLE handle,
                       const cJSON* json_raw)
{
    auto ret_val { -1 };
    std::string error_message;
    if (nullptr == handle ||
        nullptr == json_raw)
    {
        error_message += "Invalid handle or json.";
    }
    else
    {
        try
        {
            const std::unique_ptr<char, CJsonDeleter> spJsonBytes{cJSON_Print(json_raw)};
            DBSyncImplementation::instance().insertBulkData(handle, spJsonBytes.get());
            ret_val = 0;
        }
        catch(const nlohmann::detail::exception& ex)
        {
            error_message += "json error, id: " + std::to_string(ex.id) + ". " + ex.what();
            ret_val = ex.id;
        }
        catch(const DbSync::dbsync_error& ex)
        {
            error_message += "DB error, id: " + std::to_string(ex.id()) + ". " + ex.what();
            ret_val = ex.id();
        }
        catch(...)
        {
            error_message += "Unrecognized error.";
        }
    }
    log_message(handle, error_message);

    return ret_val;
}

int dbsync_update_with_snapshot(const DBSYNC_HANDLE handle,
                                const cJSON* json_snapshot,
                                cJSON** json_return_modifications)
{
    auto ret_val { -1 };
    std::string error_message;
    if (nullptr == handle ||
        nullptr == json_snapshot ||
        nullptr == json_return_modifications)
    {
        error_message += "Invalid input parameter.";
    }
    else
    {
        try
        {
            std::string result;
            const std::unique_ptr<char, CJsonDeleter> spJsonBytes{cJSON_PrintUnformatted(json_snapshot)};
            DBSyncImplementation::instance().updateSnapshotData(handle, spJsonBytes.get(), result);
            *json_return_modifications = cJSON_Parse(result.c_str());
            ret_val = 0;
        }
        catch(const nlohmann::detail::exception& ex)
        {
            error_message += "json error, id: " + std::to_string(ex.id) + ". " + ex.what();
            ret_val = ex.id;
        }
        catch(const DbSync::dbsync_error& ex)
        {
            error_message += "DB error, id: " + std::to_string(ex.id()) + ". " + ex.what();
            ret_val = ex.id();
        }
        catch(...)
        {
            error_message += "Unrecognized error.";
        }
    }
    log_message(handle, error_message);
    return ret_val;
}

int dbsync_update_with_snapshot_cb(const DBSYNC_HANDLE handle,
                                   const cJSON* json_snapshot,
                                   void* callback)
{
    auto ret_val { -1 };
    std::string error_message;
    if (nullptr == handle ||
        nullptr == json_snapshot ||
        nullptr == callback)
    {
        error_message += "Invalid input parameters.";
    }
    else
    {
        try
        {
            const std::unique_ptr<char, CJsonDeleter> spJsonBytes{cJSON_PrintUnformatted(json_snapshot)};
            DBSyncImplementation::instance().updateSnapshotData(handle, spJsonBytes.get(), callback);
            ret_val = 0;
        }
        catch(const nlohmann::detail::exception& ex)
        {
            error_message += "json error, id: " + std::to_string(ex.id) + ". " + ex.what();
            ret_val = ex.id;
        }
        catch(const DbSync::dbsync_error& ex)
        {
            error_message += "DB error, id: " + std::to_string(ex.id()) + ". " + ex.what();
            ret_val = ex.id();
        }
        catch(...)
        {
            error_message += "Unrecognized error.";
        }
    }
    log_message(handle, error_message);
    return ret_val;
}

void dbsync_teardown(void) {
  DBSyncImplementation::instance().release();
}

void dbsync_free_result(cJSON** json_result)
{
    if (nullptr != *json_result)
    {
        cJSON_Delete(*json_result);
    }
}

#ifdef __cplusplus
}
#endif