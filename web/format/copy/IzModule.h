
#pragma once


#include "FileTask.h"
#include <map>
class FileTask;

class IzInstance : public pp::Instance {
public:
    explicit IzInstance(PP_Instance instance);

    virtual ~IzInstance();

    virtual void HandleMessage(const pp::Var& var_message);

    virtual bool Init(uint32_t /*argc*/,
            const char * /*argn*/ [],
            const char * /*argv*/ []);



    void ShowErrorMessage(const std::string& message, int32_t result);
    void ShowStatusMessage(const std::string& message);

    std::string getMessageValue(std::string key, pp::Var const &var_message);
    uint8_t getMessageType(pp::Var const &var_message);
    uint64_t getMessageId(pp::Var const &var_message);


private:
    std::map<uint64_t,FileTask*> fileTasks_;
    template <typename M> void FreeClear( M & amap );


};

