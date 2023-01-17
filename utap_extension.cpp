#include "utap_extension.h"
#include <stdexcept>
#include <charconv>
#include <iterator>
#include <cstring>

bool starts_with(std::string_view str, std::string_view other){
    if(str.length() < other.length())
        return false;

    return std::memcmp(str.begin(), other.begin(), other.length()) == 0;
}

UTAP::declarations_t& navigate_template(UTAP::Document& doc, std::string_view path){
    int num_size = path.find_first_of(']');
    int template_index;
    auto result = std::from_chars(path.begin(), path.begin() + num_size, template_index);
    // These two exceptions reflect the behavior of std::stoi.
    if (result.ec == std::errc::invalid_argument) {
        throw std::invalid_argument{"invalid_argument"};
    }
    else if (result.ec == std::errc::result_out_of_range) {
        throw std::out_of_range{"out_of_range"};
    }

    auto template_it = doc.getTemplates().begin();
    std::advance(template_it, template_index - 1);
    return *template_it;
}

UTAP::declarations_t& navigate_xpath(UTAP::Document& doc, std::string_view path){
    if(path.substr(0,5) != "/nta/")
        throw std::invalid_argument{"Xpath did not start with 'nta/'"};

    path = path.substr(5);
    if(starts_with(path, "declaration!"))
        return doc.getGlobals();
    else if(starts_with(path, "system!"))
        return doc.getGlobals(); // May be wrong
    else if(starts_with(path, "template["))
        return navigate_template(doc, path.substr(9));
    else
        throw std::invalid_argument{"Path did not match anything"};

    return doc.getGlobals();
}

TextRange::TextRange(const UTAP::Document& doc, const UTAP::position_t& symbol){
    auto doc_start = doc.findFirstPosition(symbol.start);

    begOffset = symbol.start - doc_start.position;
    endOffset = symbol.end - doc_start.position;
}

TextRange& TextRange::intersect(const TextRange& other){
    begOffset = std::max(begOffset, other.begOffset);
    endOffset = std::min(endOffset, other.endOffset);
    return *this;
}

TextRange TextRange::from(const UTAP::Document& doc, const UTAP::position_t& symbol){
    auto doc_start = doc.findFirstPosition(symbol.start);
    uint32_t start = symbol.start - doc_start.position;
    uint32_t end = std::numeric_limits<int32_t>::max();
    return {start, end};
}