#include <uls/utap_extension.h>
#include <uls/common_data.h>
#include <stdexcept>
#include <charconv>
#include <iterator>
#include <cstring>

// Likely undefined behaviour here, release is failing but debug is succeeding

UTAP::declarations_t& navigate_template(UTAP::Document& doc, std::string_view path)
{
    int num_size = path.find_first_of(']');
    int template_index;
    auto result = std::from_chars(path.begin(), path.begin() + num_size, template_index);
    // These two exceptions reflect the behavior of std::stoi.
    if (result.ec == std::errc::invalid_argument) {
        throw std::invalid_argument{"invalid_argument"};
    } else if (result.ec == std::errc::result_out_of_range) {
        throw std::out_of_range{"out_of_range"};
    }

    auto template_it = doc.get_templates().begin();
    std::advance(template_it, template_index - 1);
    return *template_it;
}

UTAP::declarations_t& navigate_xpath(UTAP::Document& doc, std::string_view path)
{
    if (path.substr(0, 5) != "/nta/")
        throw std::invalid_argument{"Xpath did not start with '/nta/'"};

    path = path.substr(5);
    if (path.starts_with("declaration!"))
        return doc.get_globals();
    else if (path.starts_with("system!"))
        return doc.get_system_declarations();
    else if (path.starts_with("template["))
        return navigate_template(doc, path.substr(9));
    else if (path.starts_with("queries!"))  // Hard coded special case, due to weird handling of queries
        return doc.get_system_declarations();

    throw std::invalid_argument{"Path did not match anything"};
}

UTAP::declarations_t& navigate_xpath(UTAP::Document& doc, std::string_view path, uint32_t pos)
{
    if (path == "/nta/")
        return doc.get_globals();

    UTAP::declarations_t& decl = navigate_xpath(doc, path);

    for (auto& func : decl.functions) {
        TextRange range{doc, func.body_position};
        if (range.contains(pos))
            return *func.body;
    }

    return decl;
}

std::optional<std::reference_wrapper<UTAP::template_t>> find_process(UTAP::Document& doc, std::string_view name)
{
    for (auto& process : doc.get_processes()) {
        if (process.uid.get_name() == name) {
            if (process.templ)
                return *process.templ;
            return std::nullopt;
        }
    }
    return std::nullopt;
}