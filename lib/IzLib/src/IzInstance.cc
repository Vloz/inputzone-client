#include <irt_dev.h>
#include "IzInstance.h"


IzInstanceBase::IzInstanceBase(PP_Instance instance) : pp::Instance(instance)
{}


void IzInstanceBase::SendOutputURL(uint64_t id, std::string url){
    PostMessage(pp::Var(std::to_string(id)+"|"+std::to_string(OUTPUTURL)+"|"+url));
}

void IzInstanceBase::UpdateProgress(uint64_t id, int8_t percent){
    PostMessage(pp::Var(std::to_string(id)+"|"+std::to_string(PROGRESS)+"|"+std::to_string((int)percent)));
}

void IzInstanceBase::UpdatePreProgress(uint64_t id, int8_t percent){
    PostMessage(pp::Var(std::to_string(id)+"|"+std::to_string(PREPROGRESS)+"|"+std::to_string((int)percent)));
}


void IzInstanceBase::UpdateTaskStatut(uint64_t id, int8_t statusType){
    PostMessage(pp::Var(std::to_string(id)+"|"+std::to_string(STATUS)+"|"+std::to_string(statusType)));
}


void IzInstanceBase::UpdateTaskDetails(uint64_t id, std::string text) {
    PostMessage(pp::Var(std::to_string(id)+"|"+std::to_string(DETAILS)+"|"+text));
}


void IzInstanceBase::DebugErrorMessage(const std::string& message, int32_t result) {
    printf("DEBUG ERROR |%s|No:%i",message.c_str(),result);
    fflush(stdout);
}

void IzInstanceBase::DebugMessage(const std::string& message) {
    printf("%s",message.c_str());
    fflush(stdout);
}


uint8_t IzInstanceBase::getMessageType(const pp::Var& var_message){
    return (uint8_t)strtoul(getMessageValue("type", var_message).c_str(),NULL,0);
}

uint64_t IzInstanceBase::getMessageId(const pp::Var& var_message){
    return strtoull(getMessageValue("id", var_message).c_str(),NULL,0);
}

std::string IzInstanceBase::getMessageValue(std::string key, const pp::Var& var_message) {
    if (!var_message.is_string())
        return "";
    std::string message = var_message.AsString();
    std::istringstream is_file(message);
    std::string line;
    while( std::getline(is_file, line) )
    {
        std::istringstream is_line(line);
        if(line==key){
            std::getline(is_file, line);
            return line;
        }
    }
    return "";
}

