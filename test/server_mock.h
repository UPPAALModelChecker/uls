#pragma once
#include <uls/server.h>
#include <string>
#include <sstream>
#include <iostream>

#define CHECK_EOF(mock) CHECK(mock.read_raw() == ""); CHECK(mock.out_eof())

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

    bool expect_error(){
        nlohmann::json message;
        out_buf >> message;
        return message["type"] == "error";
    }
    
    bool out_eof(){
        return out_buf.eof();
    }

    std::string read_raw(){
        std::string x;
        out_buf >> x;
        return x;
    }

    bool handshake(){
        std::string x;
        out_buf >> x;
        return x == "json";
    }

};