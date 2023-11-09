/*
 * Wazuh content manager - Unit Tests
 * Copyright (C) 2015, Wazuh Inc.
 * Jun 07, 2023.
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General Public
 * License (version 2) as published by the FSF - Free Software
 * Foundation.
 */

#include "executionContext_test.hpp"
#include "executionContext.hpp"
#include "updaterContext.hpp"
#include "utils/rocksDBWrapper.hpp"
#include <filesystem>
#include <memory>
#include <string>

/**
 * @brief Removes the directory if it exists.
 *
 * @param outputFolder Folder to be removed.
 */
void removeOutputFolderIfExists(const std::string& outputFolder)
{
    if (std::filesystem::exists(outputFolder))
    {
        // Delete the output folder.
        std::filesystem::remove_all(outputFolder);
    }
}

/*
 * @brief Tests the instantiation of the ExecutionContext class
 */
TEST_F(ExecutionContextTest, instantiation)
{
    // Check that the ExecutionContext class can be instantiated
    EXPECT_NO_THROW(std::make_shared<ExecutionContext>());
}

/*
 * @brief Test default folder when there is no configuration for the output folder.
 */
TEST_F(ExecutionContextTest, TestDefaultFolderWhenThereIsNoConfigurationForTheOutputFolder)
{
    // Remove the output folder if exists
    removeOutputFolderIfExists(GENERIC_OUTPUT_FOLDER_PATH);

    m_spUpdaterBaseContext->configData.erase("outputFolder");

    m_spExecutionContext->handleRequest(m_spUpdaterBaseContext);

    EXPECT_EQ(m_spUpdaterBaseContext->outputFolder, GENERIC_OUTPUT_FOLDER_PATH);

    EXPECT_TRUE(std::filesystem::exists(m_spUpdaterBaseContext->outputFolder));
}

/*
 * @brief Test default folder when the output folder path is empty.
 */
TEST_F(ExecutionContextTest, TestDefaultFolderWhenTheOutputFolderPathIsEmpty)
{
    // Remove the output folder if exists
    removeOutputFolderIfExists(GENERIC_OUTPUT_FOLDER_PATH);

    m_spUpdaterBaseContext->configData["outputFolder"] = "";

    m_spExecutionContext->handleRequest(m_spUpdaterBaseContext);

    EXPECT_EQ(m_spUpdaterBaseContext->outputFolder, GENERIC_OUTPUT_FOLDER_PATH);

    EXPECT_TRUE(std::filesystem::exists(m_spUpdaterBaseContext->outputFolder));
}

/*
 * @brief Test valid case when the output folder path is not empty.
 */
TEST_F(ExecutionContextTest, TestValidCaseWhenTheOutputFolderPathIsNotEmpty)
{
    const auto expectedOutputFolder {m_spUpdaterBaseContext->configData.at("outputFolder").get<const std::string>()};

    // Remove the output folder if exists
    removeOutputFolderIfExists(expectedOutputFolder);

    m_spExecutionContext->handleRequest(m_spUpdaterBaseContext);

    EXPECT_EQ(m_spUpdaterBaseContext->outputFolder, expectedOutputFolder);

    EXPECT_TRUE(std::filesystem::exists(m_spUpdaterBaseContext->outputFolder));
}

/*
 * @brief Test valid case when the output folder path is not empty and already exists.
 */
TEST_F(ExecutionContextTest, TestValidCaseWhenTheOutputFolderPathIsNotEmptyAndExists)
{
    m_spUpdaterBaseContext->configData["outputFolder"] = "/tmp/output-folder";
    const auto expectedOutputFolder {m_spUpdaterBaseContext->configData.at("outputFolder").get<const std::string>()};

    // Remove the output folder if exists
    removeOutputFolderIfExists(expectedOutputFolder);

    // Create the output folder.
    std::filesystem::create_directory(expectedOutputFolder);

    m_spExecutionContext->handleRequest(m_spUpdaterBaseContext);

    EXPECT_EQ(m_spUpdaterBaseContext->outputFolder, expectedOutputFolder);

    EXPECT_TRUE(std::filesystem::exists(m_spUpdaterBaseContext->outputFolder));
}

/**
 * @brief Test the opening of the RocksDB database when it is not initialized.
 *
 */
TEST_F(ExecutionContextTest, NotCreatedDatabaseOpening)
{
    m_spUpdaterBaseContext->configData["databasePath"] = m_databasePath.string();

    EXPECT_THROW(m_spExecutionContext->handleRequest(m_spUpdaterBaseContext), std::runtime_error);
}

/**
 * @brief Test the opening of the RocksDB database when it is already initialized.
 *
 */
TEST_F(ExecutionContextTest, CreatedDatabaseOpening)
{
    // Create database.
    Utils::RocksDBWrapper(m_databasePath.string());

    m_spUpdaterBaseContext->configData["databasePath"] = m_databasePath.string();

    EXPECT_NO_THROW(m_spExecutionContext->handleRequest(m_spUpdaterBaseContext));
}
