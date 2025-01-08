#include <iostream>
#include <string>
#include <utility>
#include <sstream>
#include <cassert>
#include <map>
#include "exceptions.h"
#include "message.h"
#include "message_serialization.h"
#include <algorithm>

std::string MessageTypeToStringFunc(MessageType type) {
    const std::map<MessageType, std::string> MessageTypeToString = {
    {MessageType::NONE, "NONE"},
    {MessageType::LOGIN, "LOGIN"},
    {MessageType::CREATE, "CREATE"},
    {MessageType::PUSH, "PUSH"},
    {MessageType::POP, "POP"},
    {MessageType::TOP, "TOP"},
    {MessageType::SET, "SET"},
    {MessageType::GET, "GET"},
    {MessageType::ADD, "ADD"},
    {MessageType::SUB, "SUB"},
    {MessageType::MUL, "MUL"},
    {MessageType::DIV, "DIV"},
    {MessageType::BEGIN, "BEGIN"},
    {MessageType::COMMIT, "COMMIT"},
    {MessageType::BYE, "BYE"},
    {MessageType::OK, "OK"},
    {MessageType::FAILED, "FAILED"},
    {MessageType::ERROR, "ERROR"},
    {MessageType::DATA, "DATA"}
};
    
    auto it = MessageTypeToString.find(type);
    if (it != MessageTypeToString.end()) {
        return it->second;
    }
    return "UNKNOWN";
}

MessageType StringToMessageTypeFunc(const std::string &str) {
    const std::map<std::string, MessageType> StringToMessageType = {
        {"NONE", MessageType::NONE},
        {"LOGIN", MessageType::LOGIN},
        {"CREATE", MessageType::CREATE},
        {"PUSH", MessageType::PUSH},
        {"POP", MessageType::POP},
        {"TOP", MessageType::TOP},
        {"SET", MessageType::SET},
        {"GET", MessageType::GET},
        {"ADD", MessageType::ADD},
        {"SUB", MessageType::SUB},
        {"MUL", MessageType::MUL},
        {"DIV", MessageType::DIV},
        {"BEGIN", MessageType::BEGIN},
        {"COMMIT", MessageType::COMMIT},
        {"BYE", MessageType::BYE},
        {"OK", MessageType::OK},
        {"FAILED", MessageType::FAILED},
        {"ERROR", MessageType::ERROR},
        {"DATA", MessageType::DATA}
    };

    auto it = StringToMessageType.find(str);
    if (it != StringToMessageType.end()) {
        return it->second;
    }
    return MessageType::NONE;
}

void MessageSerialization::encode(const Message &msg, std::string &encoded_msg) {
    std::ostringstream oss;
    oss << MessageTypeToStringFunc(msg.get_message_type()) << " ";

    switch (msg.get_message_type()) {
        case MessageType::LOGIN:
        case MessageType::CREATE:
            oss << msg.get_username();
            break;
        case MessageType::SET:
        case MessageType::GET:
            oss << msg.get_table() << " " << msg.get_key();
            break;
        case MessageType::PUSH:
        case MessageType::FAILED:
        case MessageType::ERROR:
        case MessageType::DATA:
            oss << msg.get_value();
            break;
        default:
            for (unsigned int i = 0; i < msg.get_num_args(); i++) {
                const auto &arg = msg.get_arg(i);  
                oss << arg << " ";
            }
            break;
    }

    encoded_msg = oss.str();
    if (!encoded_msg.empty() && encoded_msg.back() == ' ') {
        encoded_msg.pop_back(); // remove trailing space
    }
    encoded_msg += '\n';

    if (encoded_msg.length() > Message::MAX_ENCODED_LEN) {
        throw InvalidMessage("Encoded message exceeds maximum length");
    }
}

void MessageSerialization::decode(const std::string &encoded_msg, Message &msg) {
    msg.clear_args();
    std::istringstream iss(encoded_msg);

    std::vector<std::string> args;
    std::string arg;
    while (iss >> arg) {
        args.push_back(arg);
    }
    MessageType type = StringToMessageTypeFunc(args[0]);
    msg.set_message_type(type);
    switch (type) {
        case MessageType::LOGIN: {
            if (args.size() != 2) {
                throw InvalidMessage("Invalid message. ");
            }
            msg.push_arg(args[1]);
            break;
        }
        case MessageType::CREATE: {
            if (args.size() != 2) {
                throw InvalidMessage("Invalid message. ");
            }
            msg.push_arg(args[1]);
            break;
        }
        case MessageType::SET: {
            if (args.size() != 3) {
                throw InvalidMessage("Invalid message. ");
            }
            msg.push_arg(args[1]);
            msg.push_arg(args[2]);
            break;
        }
        case MessageType::GET: {
            if (args.size() != 3) {
                throw InvalidMessage("Invalid message. ");
            }
            msg.push_arg(args[1]);
            msg.push_arg(args[2]);
            break;
        }
        case MessageType::PUSH: {
            if (args.size() != 2) {
                throw InvalidMessage("Invalid message. ");
            }
            msg.push_arg(args[1]);
            break;
        }
        case MessageType::FAILED: {
            if(args.size() < 2){
                throw InvalidMessage("Invalid message. ");
            }
            std::stringstream ss;
            for(size_t i = 1; i < args.size() - 1; i++){
                ss << args[i];
                ss << " ";
            }
            ss<<args[args.size() - 1];
            msg.push_arg(ss.str());
            break;
        }   
        case MessageType::ERROR: {
            if(args.size() < 2){
                throw InvalidMessage("Invalid message. ");
            }
            std::stringstream ss;
            for(size_t i = 1; i < args.size() - 1; i++){
                ss << args[i];
                ss << " ";
            }
            ss<<args[args.size() - 1];
            msg.push_arg(ss.str());
            break;
        }
        case MessageType::DATA: {
            if(args.size() != 2){
                throw InvalidMessage("Invalid message. ");
            }
            msg.push_arg(args[1]);
            break;
        }
        default:
            while (iss >> arg) {
                msg.push_arg(arg);
            }
            break;
    }

    if (encoded_msg.back() != '\n') {
        throw InvalidMessage("Encoded message does not end with newline character");
    }
    if (!msg.is_valid()) {
        throw InvalidMessage("Decoded message is not valid");
    }
}
