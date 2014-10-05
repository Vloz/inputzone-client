
#pragma once

#include <sstream>
#include "ppapi/cpp/instance.h"
#include "ppapi/cpp/module.h"
#include "ppapi/cpp/var.h"
#include "FileConverter.h"
#include <map>

class FileConverter;

enum MESSAGETYPE{
    START=1, //--> Start a file convertion with the specified url
    CANCEL=2, //--> Cancelation query from ui
    PREPROGRESS=3, //<-- Update preprogress value of the task
    PROGRESS=4, //<--        '' progress ''
    ERROR=5, //<-- General Module Error
    STATUS=6, //<-- Status of a task see FileConverter.h for types of Status
    OUTPUTURL=7, //<-- Send the output file url to the ui for download
    DETAILS=8 //<-- Optional details wrote under the task UI
};



class IzInstanceBase :  public pp::Instance {
public:
    explicit IzInstanceBase(PP_Instance instance);

    void DebugErrorMessage(const std::string& message, int32_t result);
    void DebugMessage(const std::string& message);

    std::string getMessageValue(std::string key, pp::Var const &var_message);

    uint8_t getMessageType(pp::Var const &var_message);

    uint64_t getMessageId(pp::Var const &var_message);


    void UpdatePreProgress(uint64_t id, int8_t percent);

    void UpdateProgress(uint64_t id, int8_t percent);

    void UpdateTaskDetails(uint64_t id, std::string text);

    void UpdateTaskStatut(uint64_t id, int8_t statusType);

    void SendOutputURL(uint64_t id, std::string url);
};


template <typename T>
class IzInstance : public IzInstanceBase {
public:

    explicit IzInstance(PP_Instance instance) : IzInstanceBase(instance),fileTasks_()
    {}


    virtual ~IzInstance(){
        FreeClear(fileTasks_);
    }

    virtual void HandleMessage(const pp::Var& var_message){

        switch(getMessageType(var_message)){
            case START :
                fileTasks_.insert(std::pair<uint64_t ,T*>(getMessageId(var_message),new T(this,var_message)));
                break;
            case CANCEL:
            {

                uint64_t id = getMessageId(var_message);
                fileTasks_[id]->Cancel();
                break;
            }
            default:
                DebugErrorMessage("Incorrect Iz message type!", getMessageType(var_message));
                return;
        }


    }

private:
    std::map<uint64_t,T*> fileTasks_;
    template <typename M> void FreeClear( M & amap ){
        for ( typename M::iterator it = amap.begin(); it != amap.end(); ++it ) {
            delete it->second;
        }
        amap.clear();
    }


};

