#pragma once
#include <nlohmann/json.hpp>

template <>
struct Deserializer<std::string>
{
    static std::string deserialize(const nlohmann::json& message);
};
template <>
struct Serializer<std::string>
{
    static nlohmann::json serialize(const std::string& str);
};

template <>
struct Deserializer<const nlohmann::json&>
{
    static const nlohmann::json& deserialize(const nlohmann::json& message) { return message; }
};
template <>
struct Serializer<nlohmann::json>
{
    static nlohmann::json serialize(nlohmann::json message) { return message; }
};

template <typename ItemType>
struct Deserializer<std::vector<ItemType>>
{
    static std::vector<ItemType> deserialize(const nlohmann::json& message)
    {
        std::vector<ItemType> data;
        for (const auto& item : message)
            data.push_back(Deserializer<ItemType>::deserialize(item));
        return data;
    }
};
template <typename ItemType>
struct Serializer<std::vector<ItemType>>
{
    static nlohmann::json serialize(const std::vector<ItemType>& data)
    {
        nlohmann::json json_array;
        for (const auto& item : data)
            json_array.push_back(Serializer<ItemType>::serialize(item));
        return json_array;
    }
};