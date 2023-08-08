#include <uls/common_data.h>
#include <nlohmann/json.hpp>
#include <utap/document.h>

Identifier Deserializer<Identifier>::deserialize(const nlohmann::json& message){
    return {
        message["xpath"].get<std::string>(),
        message["offset"].get<uint32_t>(),
        message["identifier"].get<std::string>()
    };
}

nlohmann::json Serializer<TextLocation>::serialize(const TextLocation& result){
    return {
        {"xpath", *result.path},
        {"start", result.range.begOffset},
        {"end", result.range.endOffset}
    };
}

nlohmann::json Serializer<TextRange>::serialize(const TextRange& range){
    return {
        {"start", range.begOffset},
        {"end", range.endOffset}
    };
}



TextRange::TextRange(const UTAP::Document& doc, const UTAP::position_t& symbol){
    auto doc_start = doc.find_first_position(symbol.start);

    begOffset = symbol.start - doc_start.position;
    endOffset = symbol.end - doc_start.position;
}

TextRange& TextRange::intersect(const TextRange& other){
    begOffset = std::max(begOffset, other.begOffset);
    endOffset = std::min(endOffset, other.endOffset);
    return *this;
}

TextRange TextRange::from(const UTAP::Document& doc, const UTAP::position_t& symbol){
    auto doc_start = doc.find_first_position(symbol.start);
    uint32_t start = symbol.start - doc_start.position;
    uint32_t end = std::numeric_limits<int32_t>::max();
    return {start, end};
}

bool TextRange::contains(uint32_t offset){
    return begOffset <= offset && offset <= endOffset;
}

TextLocation::TextLocation(UTAP::Document& doc, const UTAP::position_t& pos){
    auto doc_start = doc.find_first_position(pos.start);

    path = doc_start.path;
    range.begOffset = pos.start - doc_start.position;
    range.endOffset = pos.end - doc_start.position;
}