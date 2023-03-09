#pragma once

#include <utap/document.h>
#include <utap/utap.h>
#include <uls/server.h>
#include <uls/common_data.h>
#include <string_view>

/**
 * Uses a xpath to find the relevant declarations in the given document
 * 
 * Examples:
 * "/nta/declaration!" leads to the global declarations
 * "/nta/system!" leads to the system declarations
 * "/nta/template[1]/*" leads to the declarations of the first template
 */
UTAP::declarations_t& navigate_xpath(UTAP::Document& doc, std::string_view path);

/**
 * Similar to previous overload but will return function scope iff pos is inside said function
 */
UTAP::declarations_t& navigate_xpath(UTAP::Document& doc, std::string_view path, uint32_t pos);

/**
 * This class is used to iterate through symbols from a given declarations
 * Parent and nested scopes are also iterated through
 */
struct DeclarationsWalker{
    const UTAP::Document& doc;
    bool checkChildren;

    template<typename Func>
    void visit_symbols(const UTAP::declarations_t& decls, Func f){
        auto range = TextRange{};

        if(checkChildren){
            for(const auto& func : decls.functions)
                traverse_function(func, TextRange{doc, func.body_position}, f);
        }

        handle_symbols(decls.frame, range, f);
        traverse_parents(decls, f);
    }

private:

    template<typename Func>
    void traverse_function(const UTAP::function_t& function, const TextRange& func_range, Func f){
        
        for(const auto& func : function.body->functions){
            auto range = TextRange::from(doc, func.uid.get_position());
            f(func.uid, range.intersect(func_range));
            traverse_function(func, TextRange{doc, func.body_position}, f);
        }

        handle_symbols(function.body->get_frame(), func_range, f);
    }

    template<typename Func>
    void traverse_parents(const UTAP::declarations_t& decls, Func f){
        UTAP::frame_t frame = decls.frame;
        while(frame.has_parent()){
            frame = frame.get_parent();
            for(const auto& symbol : frame){
                f(symbol, TextRange{});
            }
        }
    }

    template<typename Func>
    void handle_symbols(const UTAP::frame_t& frame, const TextRange& scope_range, Func f){
        for(const auto& symbol : frame){
            auto range = TextRange::from(doc, symbol.get_position());
            f(symbol, range.intersect(scope_range));
        }
    }
};