#pragma once
#include "server_module.h"
#include <utap/utap.h>
#include <memory>
#include <functional>
#include <string>

struct WorkingDocument {
    std::string document;
public:
    void set_document(std::string document);
    void insert(std::string str, int offset);
    void remove(int offset, int length);
    std::unique_ptr<UTAP::Document> parse() const;
};

/** This module stores information about the current document and is mainly used to provide events for other modules */
class SystemRepository : public ServerModule {
    std::unique_ptr<UTAP::Document> doc;
    WorkingDocument working_doc;
    std::string current_node{"/nta/template[1]"};
    std::vector<std::function<void(UTAP::Document&)>> on_document_update;
    std::vector<std::function<void(const std::string&)>> on_current_node_changed;

    void upload(std::string document);
    void change_node(std::string xpath);

public:
    void configure(Server& server) override;

    void add_on_document_update(std::function<void(UTAP::Document&)> handler);
    void add_on_current_node_changed(std::function<void(const std::string&)> handler);

    // This should return a const reference but UTAP does not fully support const yet
    UTAP::Document& get_document() { return *doc; }
    const WorkingDocument& get_working_document() { return working_doc; }
    bool has_document() { return doc != nullptr; }

    const std::string& get_current_xpath() { return current_node; }
};