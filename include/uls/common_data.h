#pragma once
#include "server_module.h"
#include <string>
#include <memory>
#include <nlohmann/json_fwd.hpp>
#include <utap/document.h>


struct Identifier {
    std::string xpath;
    uint32_t offset;
    std::string identifier;
};

template<>
struct Deserializer<Identifier> {
    static Identifier deserialize(const nlohmann::json& message);
};

struct TextRange{
    uint32_t begOffset;
    uint32_t endOffset;

    TextRange() : TextRange{0, std::numeric_limits<int32_t>::max()} {}
    TextRange(uint32_t start, uint32_t end) : begOffset{start}, endOffset{end} {}

    /** Create the range that contains the given symbol*/
    TextRange(const UTAP::Document& doc, const UTAP::position_t& symbol);

    /** Create a range that contains the symbol and everything after*/
    static TextRange from(const UTAP::Document& doc, const UTAP::position_t& symbol);

    /** Get the intersection between two ranges*/
    TextRange& intersect(const TextRange& other);

    bool contains(uint32_t offset);
};

template<>
struct Serializer<TextRange> {
    static nlohmann::json serialize(const TextRange& result);
};

struct TextLocation{
    std::shared_ptr<std::string> path;
    TextRange range;

    TextLocation(UTAP::Document& doc, const UTAP::position_t& pos);
};

template<>
struct Serializer<TextLocation> {
    static nlohmann::json serialize(const TextLocation& result);
};