#include <uls/system.h>
#include <utap/utap.h>

void SystemRepository::upload(const std::string& document){
    doc = std::make_unique<UTAP::Document>();
    parseXMLBuffer(document.c_str(), doc.get(), true, {});

    // Fire on document update event
    for(auto& handler : on_document_update)
        handler(*doc);
}


void SystemRepository::change_node(std::string xpath){
    current_node = std::move(xpath);

    // Fire on current document changed event
    for(auto& handler : on_current_node_changed)
        handler(current_node);
}

void SystemRepository::configure(Server& server) {
    server.add_command<std::string>("upload", [this](std::string doc_str){
        upload(doc_str);
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