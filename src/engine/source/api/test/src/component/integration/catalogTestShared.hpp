#ifndef _CATALOG_TEST_SHARED_HPP
#define _CATALOG_TEST_SHARED_HPP

#include <memory>

#include <fmt/format.h>

#include <api/catalog/catalog.hpp>
#include <api/integration/integration.hpp>
#include <store/mockStore.hpp>

using namespace store::mocks;

const base::Name successName({"decoder", "name", "ok"});
const base::Name failName {{"decoder", "name", "fail"}};
const store::Doc successJson(fmt::format("{{\"name\": \"{}\"}}", successName.fullName()).c_str());
const store::Col successCollection {successName};
const std::string successCollectionJson(fmt::format("[\"{}\"]", successName.fullName()));
const std::string successYml(fmt::format("name: {}", successName.fullName()).c_str());
const std::string successCollectionYml(fmt::format("- {}", successName.fullName()));
const store::Doc validJson(R"({})");
const store::Doc invalidJson(R"([])");
const std::string schema {R"({"type": "object"})"};
const base::Name successSchemaName({"schema", "name", "ok"});
const base::Name failSchemaName({"schema", "name", "fail"});

const api::catalog::Resource successResourceAssetJson {
    base::Name({api::catalog::Resource::typeToStr(api::catalog::Resource::Type::decoder),
                successName.parts()[1],
                successName.parts()[2]}),
    api::catalog::Resource::Format::json};

const api::catalog::Resource successResourceAssetYml {
    base::Name({api::catalog::Resource::typeToStr(api::catalog::Resource::Type::decoder),
                successName.parts()[1],
                successName.parts()[2]}),
    api::catalog::Resource::Format::yaml};

const api::catalog::Resource failResourceAsset {
    base::Name({api::catalog::Resource::typeToStr(api::catalog::Resource::Type::decoder),
                failName.parts()[1],
                failName.parts()[2]}),
    api::catalog::Resource::Format::json};

const api::catalog::Resource successCollectionAssetJson {
    base::Name({api::catalog::Resource::typeToStr(api::catalog::Resource::Type::decoder)}),
    api::catalog::Resource::Format::json};

const api::catalog::Resource successCollectionAssetYml {
    base::Name({api::catalog::Resource::typeToStr(api::catalog::Resource::Type::decoder)}),
    api::catalog::Resource::Format::yaml};

const api::catalog::Resource policyNoIntegrations {
    base::Name({api::catalog::Resource::typeToStr(api::catalog::Resource::Type::policy),
                "no_integrations",
                successName.parts()[2]}),
    api::catalog::Resource::Format::json};

const json::Json policyNoIntegrationsJson(R"({
        "name": "policy/no_integrations/ok"
})");

const api::catalog::Resource policyResource {
    base::Name({api::catalog::Resource::typeToStr(api::catalog::Resource::Type::policy),
                successName.parts()[1],
                successName.parts()[2]}),
    api::catalog::Resource::Format::json};

const json::Json policyJson(R"({
        "name": "policy/name/ok",
        "integrations": []
    })");

const api::catalog::Resource policyDuplicated {
    base::Name({api::catalog::Resource::typeToStr(api::catalog::Resource::Type::policy),
                "duplicated",
                successName.parts()[2]}),
    api::catalog::Resource::Format::json};

const api::catalog::Resource integrationResource {
    base::Name({api::catalog::Resource::typeToStr(api::catalog::Resource::Type::integration),
                successName.parts()[1],
                successName.parts()[2]}),
    api::catalog::Resource::Format::json};

const json::Json policyDuplicatedJson(R"({
        "name": "policy/duplicated/ok",
        "integrations": [
            "integration/name/ok"
        ]
})");

class FakeValidator : public builder::IValidator
{
public:
    ~FakeValidator() = default;

    std::optional<base::Error> validatePolicy(const json::Json& json) const override
    {
        if (json.isObject())
        {
            return std::nullopt;
        }

        return base::Error {"error"};
    }

    std::optional<base::Error> validateIntegration(const json::Json& json) const override
    {
        if (json.isObject())
        {
            return std::nullopt;
        }

        return base::Error {"error"};
    }

    std::optional<base::Error> validateAsset(const json::Json& json) const override
    {
        if (json.isObject())
        {
            return std::nullopt;
        }

        return base::Error {"error"};
    }
};

inline api::catalog::Config getConfig(bool schemaOk = true)
{
    api::catalog::Config config;
    auto mockStore = std::make_shared<MockStore>();
    config.store = mockStore;
    config.validator = std::make_shared<FakeValidator>();

    EXPECT_CALL(*mockStore, readDoc(::testing::_))
        .WillRepeatedly(::testing::Invoke(
            [&](const base::Name& name) -> base::RespOrError<store::Doc>
            {
                if (name == policyResource.m_name)
                {
                    return policyJson;
                }
                if (name == policyDuplicated.m_name)
                {
                    return policyDuplicatedJson;
                }
                if (name == policyNoIntegrations.m_name)
                {
                    return policyNoIntegrationsJson;
                }
                if (name == integrationResource.m_name)
                {
                    return json::Json(R"({
                        "name": "integration/name/ok"
                    })");
                }
                if (name.parts()[2] == successName.parts()[2])
                {
                    return successJson;
                }

                return base::Error {"error"};
            }));

    EXPECT_CALL(*mockStore, readCol(testing::_, testing::_))
        .WillRepeatedly(testing::Invoke(
            [&](const base::Name& name, const store::NamespaceId& namespaceId) -> base::RespOrError<store::Col>
            {
                if (name == successCollectionAssetJson.m_name)
                {
                    return storeReadColResp(successCollection);
                }

                return base::Error {"error"};
            }));

    EXPECT_CALL(*mockStore, createDoc(testing::_, testing::_, testing::_))
        .WillRepeatedly(testing::Invoke(
            [&](const base::Name& name, const store::NamespaceId& namespaceId, const json::Json& content)
                -> base::OptError
            {
                if (name.parts()[2] == successName.parts()[2])
                {
                    return base::noError();
                }

                return base::Error {"error"};
            }));

    EXPECT_CALL(*mockStore, updateDoc(testing::_, testing::_))
        .WillRepeatedly(testing::Invoke(
            [&](const base::Name& name, const json::Json& content) -> base::OptError
            {
                if (name.parts()[2] == successName.parts()[2])
                {
                    return base::noError();
                }

                return base::Error {"error"};
            }));

    EXPECT_CALL(*mockStore, upsertDoc(testing::_, testing::_, testing::_))
        .WillRepeatedly(testing::Invoke(
            [&](const base::Name& name, const store::NamespaceId& namespaceId, const json::Json& content)
                -> base::OptError
            {
                if (name.parts()[2] == successName.parts()[2])
                {
                    return base::noError();
                }

                return base::Error {"error"};
            }));

    EXPECT_CALL(*mockStore, deleteDoc(testing::_))
        .WillRepeatedly(testing::Invoke(
            [&](const base::Name& name) -> base::OptError
            {
                if (name.parts()[2] == successName.parts()[2])
                {
                    return base::noError();
                }

                return base::Error {"error"};
            }));

    EXPECT_CALL(*mockStore, deleteCol(testing::_, testing::_))
        .WillRepeatedly(testing::Invoke(
            [&](const base::Name& name, const store::NamespaceId& namespaceId) -> base::OptError
            {
                if (name == successCollectionAssetJson.m_name)
                {
                    return base::noError();
                }

                return base::Error {"error"};
            }));

    if (schemaOk)
    {
        config.assetSchema = successSchemaName.fullName();
        config.environmentSchema = successSchemaName.fullName();
    }
    else
    {
        config.assetSchema = failSchemaName.fullName();
        config.environmentSchema = failSchemaName.fullName();
    }

    return config;
}

inline api::integration::Integration getIntegration()
{
    auto config = getConfig();
    auto catalog = std::make_shared<api::catalog::Catalog>(config);
    return api::integration::Integration(catalog);
}

#endif // _CATALOG_TEST_SHARED_HPP