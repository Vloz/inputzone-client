#pragma once

#include <iosfwd>
#include <ppapi/utility/threading/simple_thread.h>
#include <ppapi/cpp/file_system.h>
#include <ppapi/utility/threading/simple_thread.h>
#include <ppapi/utility/completion_callback_factory.h>
#include "IzModule.h"

class IzInstance;

class FileTask {
public:
     FileTask(IzInstance *instance,const pp::Var& var_message);
    ~FileTask();
private:
    void InitFileSystem(int32_t);


    pp::FileSystem *fileSystem_;

    pp::SimpleThread *thread_;
    pp::CompletionCallbackFactory<FileTask>* callbackFactory_;
    IzInstance* instance_;

    uint64_t id_;
    bool isFilesystemOpen;

};
