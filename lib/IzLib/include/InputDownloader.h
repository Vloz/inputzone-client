
#pragma once

#include <ppapi/cpp/url_request_info.h>
#include <ppapi/cpp/url_loader.h>
#include <ppapi/utility/threading/simple_thread.h>
#include <ppapi/utility/completion_callback_factory.h>
#include "FileConverter.h"

#define DOWNLOAD_BUFFER_SIZE 32000000

class FileConverter;

class InputDownloader{

public:

    InputDownloader(std::string url,std::string savePath,uint64_t size,  pp::FileSystem* fileSystem,FileConverter &converter,pp::CompletionCallback& cc);

    virtual ~InputDownloader();


    void Start();
private:


    FileConverter& converter_;
    std::string url_;
    uint64_t size_;
    pp::FileIO *input_;
    pp::FileRef *inputRef_;
    char* downloadBuffer_;
    pp::URLLoader *urlLoader_;
    pp::URLRequestInfo *urlRequestInfo_;
    uint64_t cursorInput;
    pp::SimpleThread *thread_;
    pp::CompletionCallbackFactory<InputDownloader>* callbackFactory_;
    pp::FileSystem* fileSystem_;
    clock_t timeouTick_;
    clock_t timeoutPreviousTick_;
    uint64_t file_offset_;
    pp::CompletionCallback& cc_;

    void DownloadTry(int);

};