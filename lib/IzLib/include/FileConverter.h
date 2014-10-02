#pragma once

#include <iosfwd>
#include <ppapi/utility/threading/simple_thread.h>
#include <ppapi/cpp/file_system.h>
#include <ppapi/cpp/file_io.h>
#include <ppapi/cpp/file_ref.h>
#include <ppapi/c/ppb_file_io.h>
#include <nacl_io/nacl_io.h>
#include <sys/mount.h>
#include <ppapi/cpp/directory_entry.h>
#include <ppapi/utility/threading/simple_thread.h>
#include <ppapi/utility/completion_callback_factory.h>
#include <ppapi/cpp/url_loader.h>
#include <ppapi/cpp/url_request_info.h>
#include "IzInstance.h"


#define BUFFER_SIZE 32000000

class IzInstanceBase;


enum STATUSTYPE{
    CANCELED=0,
    STARTING=1,
    CONVERTING=2,
    COMPLETING=3,
    COMPLETED=4,
    ERRORED=5
};

class FileConverter {
public:
     FileConverter(IzInstanceBase *instance,const pp::Var& var_message,std::string converterName = "NONAME");
    ~FileConverter();
    virtual int32_t Convert(double taskId, FILE* input,uint64_t inputSize, std::string directoryPath , std::string baseName, std::string inputExtension);


    std::string GetDirectoryPath();
    std::string GetBaseName(std::string filename);
    std::string GetExtension(std::string filename);

    void UpdateTaskStatus(STATUSTYPE statustype);
    void UpdateProgress(int8_t percent);
    void UpdatePreProgress(int8_t percent);
    void Error(std::string message);
    void Error(std::string message, int32_t result);

protected:
    IzInstanceBase* instance_;
private:

    pp::FileSystem *fileSystem_;
    pp::URLLoader *urlLoader_;
    pp::URLRequestInfo *urlRequestInfo_;

    pp::SimpleThread *thread_;
    pp::CompletionCallbackFactory<FileConverter>* callbackFactory_;




    uint64_t id_;
    std::string url_;
    std::string filename_;
    std::string converterName_;
    uint64_t size_;
    float currentProgress_;
    char* buffer_;
    bool isFilesystemOpen;


    void Start(int32_t);

    int32_t InitFileSystem();

    void SendOutputURL(int32_t result,const std::vector<pp::DirectoryEntry> entries,pp::FileRef /* unused_ref */);

    void Abort();


};
