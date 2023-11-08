/*
 * Wazuh content manager
 * Copyright (C) 2015, Wazuh Inc.
 * May 02, 2023.
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General Public
 * License (version 2) as published by the FSF - Free Software
 * Foundation.
 */

#ifndef _FILE_DOWNLOADER_HPP
#define _FILE_DOWNLOADER_HPP

#include "HTTPRequest.hpp"
#include "chainOfResponsability.hpp"
#include "hashHelper.h"
#include "json.hpp"
#include "stringHelper.h"
#include "updaterContext.hpp"
#include <array>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include <utility>

/**
 * @class FileDownloader
 *
 * @brief Class in charge of downloading a file from a given URL as a step of a chain of responsibility.
 *
 */
class FileDownloader final : public AbstractHandler<std::shared_ptr<UpdaterContext>>
{
private:
    /**
     * @brief Pushes the state of the current stage into the data field of the context.
     *
     * @param contextData Reference to the context data.
     * @param status Status to be pushed.
     */
    void pushStageStatus(nlohmann::json& contextData, std::string status) const
    {
        auto statusObject = nlohmann::json::object();
        statusObject["stage"] = "FileDownloader";
        statusObject["status"] = std::move(status);

        contextData.at("stageStatus").push_back(std::move(statusObject));
    }

    /**
     * @brief Function to calculate the hash of a file.
     *
     * @param filepath Path to the file.
     * @return std::string Digest vector.
     */
    std::string hashFile(const std::filesystem::path& filepath) const
    {
        if (std::ifstream inputFile(filepath, std::fstream::in); inputFile)
        {
            constexpr int BUFFER_SIZE {4096};
            std::array<char, BUFFER_SIZE> buffer {};

            Utils::HashData hash;
            while (inputFile.read(buffer.data(), buffer.size()))
            {
                hash.update(buffer.data(), inputFile.gcount());
            }
            hash.update(buffer.data(), inputFile.gcount());

            return Utils::asciiToHex(hash.hash());
        }

        throw std::runtime_error {"Unable to open '" + filepath.string() + "' for hashing."};
    };

    /**
     * @brief Download the file given by the config URL.
     *
     * @param context Updater context.
     */
    void download(UpdaterContext& context) const
    {
        const auto& url {context.spUpdaterBaseContext->configData.at("url").get_ref<const std::string&>()};

        // Check if file is compressed.
        const auto& compressed {
            "raw" != context.spUpdaterBaseContext->configData.at("compressionType").get_ref<const std::string&>()};

        // Generate output file path. If the input file is compressed, the output file will be in the downloads
        // folder and if it's not compressed, in the contents folder.
        const auto& contentFileName {
            context.spUpdaterBaseContext->configData.at("contentFileName").get_ref<const std::string&>()};
        const auto outputFilePath {(compressed ? context.spUpdaterBaseContext->downloadsFolder
                                               : context.spUpdaterBaseContext->contentsFolder) /
                                   contentFileName};

        // Lambda used on error case.
        const auto onError {[](const std::string& errorMessage, const long& errorCode)
                            {
                                throw std::runtime_error {"(" + std::to_string(errorCode) + ") " + errorMessage};
                            }};

        // Download and store file.
        HTTPRequest::instance().download(HttpURL(url), outputFilePath, onError);

        // Just process the new file if the hash is different from the last one.
        auto inputFileHash {hashFile(outputFilePath)};
        if (context.spUpdaterBaseContext->downloadedFileHash == inputFileHash)
        {
            std::cout << "Content file didn't change from last download" << std::endl;
            return;
        }

        // Store new hash.
        context.spUpdaterBaseContext->downloadedFileHash = std::move(inputFileHash);

        // Download finished: Update context paths.
        context.data.at("paths").push_back(outputFilePath);
    }

public:
    /**
     * @brief Download a file from the URL set in the input config.
     *
     * @param context Updater context.
     * @return std::shared_ptr<UpdaterContext>
     */
    std::shared_ptr<UpdaterContext> handleRequest(std::shared_ptr<UpdaterContext> context) override
    {
        try
        {
            download(*context);
        }
        catch (const std::exception& e)
        {
            // Push error state.
            pushStageStatus(context->data, "fail");

            throw std::runtime_error("Download failed: " + std::string(e.what()));
        }

        // Push success state.
        pushStageStatus(context->data, "ok");

        std::cout << "FileDownloader - Download done successfully" << std::endl;

        return AbstractHandler<std::shared_ptr<UpdaterContext>>::handleRequest(context);
    }
};

#endif // _FILE_DOWNLOADER_HPP
