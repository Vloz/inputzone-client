
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
    CONSOLE=5, //<-- Console message
    STATUS=6, //<-- Status of a task see FileConverter.h for types of Status
    OUTPUTURL=7, //<-- Send the output file url to the ui for download
    DETAILS=8, //<-- Optional details wrote under the task UI
    PRERUN=9 //<-- First query for estimating size, return base parameters
};



class IzInstanceBase :  public pp::Instance {
public:
    explicit IzInstanceBase(PP_Instance instance);

    void DebugErrorMessage(const std::string& message, int32_t result);
    void DebugMessage(const std::string& message);

    std::string getMessageValue(std::string key, pp::Var const &var_message);

    int getMessageParams(const std::string message,char* argv[]);

    uint8_t getMessageType(pp::Var const &var_message);

    uint64_t getMessageId(pp::Var const &var_message);


    void UpdatePreProgress(uint64_t id, int8_t percent);

    void UpdateProgress(uint64_t id, int8_t percent);

    void UpdateTaskDetails(uint64_t id, std::string text);

    void Console(uint64_t id, std::string text);

    void UpdateTaskStatut(uint64_t id, int8_t statusType);

    void SendOutputURL(uint64_t id,std::string filename ,std::string url);

    void SendPrerunAnswer(uint64_t id,uint64_t outputSize, std::string baseParameters);

    std::string GetBaseName(std::string filename);
    std::string GetExtension(std::string filename);
    bool replace(std::string& str, const std::string& from, const std::string& to);
};


template <typename T>
class IzInstance : public IzInstanceBase {
public:

    static IzInstance<T>* current;



    explicit IzInstance(PP_Instance instance) : IzInstanceBase(instance), fileTasks_()
    {
        current=this;
    }


    virtual ~IzInstance(){
        FreeClear(fileTasks_);
    }

    virtual uint64_t estimateOutputSize(uint64_t inputSize){

    }

    virtual void HandleMessage(const pp::Var& var_message){

        switch(getMessageType(var_message)){
            case START :
                fileTasks_.insert(std::pair<uint64_t ,T*>(getMessageId(var_message),new T(this,var_message)));
                break;
            case CANCEL: {
                uint64_t id = getMessageId(var_message);
                fileTasks_[id]->Cancel();
                break;
            }

            case PRERUN: {
                uint64_t inputSize = strtoull(getMessageValue("inputSize", var_message).c_str(), NULL, 0);
                uint64_t outputSize = T::estimateOutputSize(inputSize);
                uint64_t id = getMessageId(var_message);
                std::string filename = getMessageValue("filename", var_message);
                uint64_t size = strtoull(getMessageValue("size", var_message).c_str(),NULL,0);
                std::string baseParameters = T::baseParameters(id, "/InputZone/"+std::to_string(id)+"/input/"+filename, size,"/InputZone/"+std::to_string(id)+"/output/", GetBaseName(filename), GetExtension(filename));
                SendPrerunAnswer(id,outputSize, baseParameters);
                break;
            }
            default:
                DebugErrorMessage("Incorrect Iz message type!", getMessageType(var_message));
                return;
        }


    }
    T* getTask(uint64_t id){
        return fileTasks_[id];
    }

    T* getTask(std::string id){
        uint64_t num = strtoull(id.c_str(),NULL,0);
        return getTask(num);
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

template<class T>
IzInstance<T>* IzInstance<T>::current = NULL;


