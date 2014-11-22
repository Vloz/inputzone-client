#include <irt_dev.h>
#include "IzInstance.h"


IzInstanceBase::IzInstanceBase(PP_Instance instance) : pp::Instance(instance)
{}


void IzInstanceBase::SendOutputURL(uint64_t id,std::string filename ,std::string url){
    PostMessage(pp::Var(std::to_string(id)+"|"+std::to_string(OUTPUTURL)+"|{\"filename\":\""+filename+"\",\"url\":\""+url+"\"}"));
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

void IzInstanceBase::Console(uint64_t id, std::string text) {
    PostMessage(pp::Var(std::to_string(id)+"|"+std::to_string(CONSOLE)+"|"+text));
}

void IzInstanceBase::SendPrerunAnswer(uint64_t id,uint64_t outputSize, std::string baseParameters) {
    PostMessage(pp::Var(std::to_string(id)+"|"+std::to_string(PRERUN)+"|{\"estitmateSize\":\""+std::to_string(outputSize)+"\",\"baseParameters\":\""+baseParameters+"\"}"));
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
        if(line==key){
            std::getline(is_file, line);
            return line;
        }else if(line=="params")
            return "";
    }
    return "";
}

int IzInstanceBase::getMessageParams(const std::string message,char* argv[]) {

    std::istringstream is_file(message);
    std::string line;
    while( std::getline(is_file, line) )
    {
        if(line=="params"){
            int r=0;
            while ( std::getline(is_file, line) ){
                argv[r] = (char*)malloc(line.length()+1);
                memcpy(argv[r], (line+'\0').c_str(), line.length()+1);
                r++;
            }
            return r;
        }
    }
    return 0;
}

std::string IzInstanceBase::GetBaseName(std::string filename) {
    if(filename.find_last_of(".") != std::string::npos)
        return filename.substr(0,filename.find_last_of("."));
    return filename;
}

std::string IzInstanceBase::GetExtension(std::string filename) {
    if(filename.find_last_of(".") != std::string::npos)
        return filename.substr(filename.find_last_of("."));
    return "";
}

bool IzInstanceBase::replace(std::string& str, const std::string& from, const std::string& to) {
    size_t start_pos = str.find(from);
    if(start_pos == std::string::npos)
        return false;
    str.replace(start_pos, from.length(), to);
    return true;
}
