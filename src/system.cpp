#include <uls/system.h>
#include <uls/server.h>
#include <utap/utap.h>
#include <iostream>

void SystemRepository::upload(std::string document){
    working_doc.set_document(std::move(document));
    doc = working_doc.parse();
    
    // Fire on document update event
    for(auto& handler : on_document_update)
        handler(*doc);
}


void SystemRepository::change_node(std::string xpath){
    current_node = std::move(xpath);

    if(doc == nullptr)
        return;

    // Fire on current document changed event
    for(auto& handler : on_current_node_changed)
        handler(current_node);
}

void SystemRepository::configure(Server& server) {
    server.add_command<std::string>("upload", [this](std::string doc_str){
        upload(std::move(doc_str));
        return OK_RESPONSE;
    });

    server.add_command<std::string>("change_node", [this](std::string xpath){
        change_node(std::move(xpath));
        return OK_RESPONSE;
    });
}

void SystemRepository::add_on_document_update(std::function<void(UTAP::Document&)> handler) {
    on_document_update.push_back(std::move(handler));
}


void SystemRepository::add_on_current_node_changed(std::function<void(const std::string&)> handler){
    on_current_node_changed.push_back(std::move(handler));
}

void WorkingDocument::set_document(std::string document){
    this->document = std::move(document);
}

std::unique_ptr<UTAP::Document> WorkingDocument::parse() const{
    auto doc = std::make_unique<UTAP::Document>();
    parse_XML_buffer(document.c_str(), doc.get(), true, {});
    return doc;
}

void WorkingDocument::insert(std::string str, int offset) {
    int decl_start = document.find("<declaration>") + 13;
    document = document.substr(0, decl_start + offset) + str + document.substr(decl_start + offset);
}

void WorkingDocument::remove(int offset, int length) {
    offset += document.find("<declaration>") + 12;
    while(length-->0)
        document[++offset] = ' '; // Replace with whitespace to avoid shifting contents
}