#pragma once

#include <utap/document.h>
#include <utap/utap.h>
#include <uls/server.h>
#include <uls/common_data.h>
#include <string_view>
#include <variant>

/**
 * Uses a xpath to find the relevant declarations in the given document
 * 
 * Examples:
 * "/nta/declaration!" leads to the global declarations
 * "/nta/system!" leads to the system declarations
 * "/nta/template[1]/..." leads to the declarations of the first template
 */
UTAP::declarations_t& navigate_xpath(UTAP::Document& doc, std::string_view path);

/**
 * Similar to previous overload but will return function scope iff pos is inside said function
 */
UTAP::declarations_t& navigate_xpath(UTAP::Document& doc, std::string_view path, uint32_t pos);

UTAP::template_t& find_process(UTAP::Document& doc, std::string_view name);

/**
 * This class is used to iterate through symbols from a given declarations
 * Parent and nested scopes are also iterated through
 */
struct DeclarationsWalker{
    const UTAP::Document& doc;
    bool checkChildren;

    template<typename Func>
    void visit_symbols(UTAP::declarations_t& decls, Func f){
        auto range = TextRange{};

        if(checkChildren){
            for(auto& func : decls.functions){
                if(traverse_function(func, TextRange{doc, func.body_position}, f))
                    return;
            }
        }

        if(handle_symbols(decls.frame, range, f))
            return;

        traverse_parents(decls, f);
    }

private:

    template<typename Func>
    bool traverse_function(UTAP::function_t& function, const TextRange& func_range, Func f){
        
        for(auto& func : function.body->functions){
            auto range = TextRange::from(doc, func.uid.get_position());
            
            if(f(func.uid, range.intersect(func_range)))
                return true;

            if(traverse_function(func, TextRange{doc, func.body_position}, f))
                return true;
        }

        return handle_symbols(function.body->get_frame(), func_range, f);
    }

    template<typename Func>
    bool traverse_parents(UTAP::declarations_t& decls, Func f){
        UTAP::frame_t frame = decls.frame;
        while(frame.has_parent()){
            frame = frame.get_parent();
            for(auto& symbol : frame){
                if(f(symbol, TextRange{}))
                    return true;
            }
        }
        return false;
    }

    template<typename Func>
    bool handle_symbols(UTAP::frame_t frame, const TextRange& scope_range, Func f){
        for(auto& symbol : frame){
            if(!symbol.get_type().is_location()){
                auto range = TextRange::from(doc, symbol.get_position());
                if(f(symbol, range.intersect(scope_range)))
                    return true;
            }
        }

        return false;
    }
};

template<class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
template<class... Ts> overloaded(Ts...) -> overloaded<Ts...>;

using UtapEntity = std::variant<UTAP::symbol_t, UTAP::type_t>;