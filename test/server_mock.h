#pragma once
#include <uls/server.h>
#include <string>
#include <sstream>
#include <iostream>

struct MockIO : IOStream{
    std::stringstream in_buf{};
    std::stringstream out_buf{};

    MockIO() : IOStream{in_buf, out_buf} {}

    void send(const std::string& cmd, const nlohmann::json& args){
        in_buf << nlohmann::json{{"cmd", cmd}, {"args", args}} << std::endl;
    }

    void send_cmd(const std::string& cmd){
        in_buf << nlohmann::json{{"cmd", cmd}, {"args", ""}} << std::endl;
    }

    nlohmann::json receive(){
        nlohmann::json message;
        out_buf >> message;
        return message["msg"];
    }

    bool output_empty(){
        std::string x;
        out_buf >> x;
        return x == "" && out_buf.eof();
    }

    bool handshake(){
        std::string x;
        out_buf >> x;
        return x == "json";
    }

};