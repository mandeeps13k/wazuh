/*
 * Wazuh content manager
 * Copyright (C) 2015, Wazuh Inc.
 * March 25, 2023.
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General Public
 * License (version 2) as published by the FSF - Free Software
 * Foundation.
 */

#ifndef _CONTENT_MODULE_IMPLEMENTATION_HPP
#define _CONTENT_MODULE_IMPLEMENTATION_HPP

#include "contentProvider.hpp"
#include <filesystem>
#include <iostream>

constexpr auto CONTENT_MODULE_ENDPOINT_NAME {"content"};

class ContentModuleFacade final : public Singleton<ContentModuleFacade>
{
private:
    std::unordered_map<std::string, std::unique_ptr<ContentProvider>> m_providers;
    std::shared_mutex m_mutex;

public:
    void start();
    void stop();
    void addProvider(const std::string& name, const nlohmann::json& parameters);
    void startScheduling(const std::string& name, size_t interval);
    void startOndemand(const std::string& name);
    void changeSchedulerInterval(const std::string& name, size_t interval);
};

#endif //_CONTENT_MODULE_IMPLEMENTATION_HPP