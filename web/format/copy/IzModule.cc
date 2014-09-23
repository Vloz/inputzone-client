#include <sstream>
#include "ppapi/cpp/instance.h"
#include "ppapi/cpp/module.h"
#include "ppapi/cpp/var.h"
#include "IzModule.h"

enum MESSAGETYPE{
    START=1,
    CANCEL=2
};

IzInstance::IzInstance(PP_Instance instance) : pp::Instance(instance),fileTasks_()
{}

IzInstance::~IzInstance() {
    FreeClear(fileTasks_);
}


bool IzInstance::Init(uint32_t /*argc*/,const char * /*argn*/ [],const char * /*argv*/ []) {

    return true;
}

void IzInstance::HandleMessage(const pp::Var& var_message) {

    switch(getMessageType(var_message)){
        case START :

            ShowStatusMessage("3!");
            fileTasks_.insert(std::pair<uint64_t ,FileTask*>(getMessageId(var_message),new FileTask(this,var_message)));

            ShowStatusMessage("4!");
            //fileTasks_.insert(std::pair<uint64_t ,FileTask>(getMessageId(var_message)+1,FileTask(this,var_message)));
            break;
        default:
            ShowErrorMessage("Incorrect Iz message type!", getMessageType(var_message));
            return;
    }
    ShowStatusMessage("5!");


}

template <typename M> void IzInstance::FreeClear( M & amap ){
for ( typename M::iterator it = amap.begin(); it != amap.end(); ++it ) {
delete it->second;
}
amap.clear();
}





/// Encapsulates our simple javascript communication protocol
void IzInstance::ShowErrorMessage(const std::string& message, int32_t result) {
    std::stringstream ss;
    ss << "ERR|" << message << " -- Error #: " << result;
    PostMessage(ss.str());
}

/// Encapsulates our simple javascript communication protocol
void IzInstance::ShowStatusMessage(const std::string& message) {
    std::stringstream ss;
    ss << "STAT|" << message;
    PostMessage(ss.str());
}


uint8_t IzInstance::getMessageType(const pp::Var& var_message){
    return (uint8_t)strtoul(getMessageValue("type", var_message).c_str(),NULL,0);
}

uint64_t IzInstance::getMessageId(const pp::Var& var_message){
    return strtoul(getMessageValue("id", var_message).c_str(),NULL,0);
}

std::string IzInstance::getMessageValue(std::string key, const pp::Var& var_message) {
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


class IzModule : public pp::Module {
public:
    IzModule() : pp::Module() {}
    virtual ~IzModule() {}

    virtual pp::Instance* CreateInstance(PP_Instance instance) {
        return new IzInstance(instance);
    }
};



namespace pp {
    Module* CreateModule() {
        return new IzModule();
    }
}
