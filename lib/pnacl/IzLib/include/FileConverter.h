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
#include "InputDownloader.h"


class IzInstanceBase;

class InputDownloader;

enum STATUSTYPE{
    CANCELED=0,
    STARTING=1,
    CONVERTING=2,
    COMPLETING=3,
    COMPLETED=4,
    ERRORED=5,
    CANCELING=6
};

enum FSTYPE{
    MEMFS=0,
    HTML5TEMP=1,
    HTML5PERS=2,
};

class FileConverter {
public:

    void Cancel();
     FileConverter(IzInstanceBase *instance,const pp::Var& var_message);
    ~FileConverter();
    //virtual int32_t Convert(double taskId, std::string inputFullPath, uint64_t inputSize, std::string directoryPath , std::string baseName, std::string inputExtension);
    virtual int main(int argc, char *argv[]);

    std::string GetMainDirectoryPath();


    void UpdateTaskStatus(STATUSTYPE statustype);
    void UpdateTaskDetails(std::string text);
    void Console(std::string text);
    void UpdateProgress(int8_t percent);
    void UpdatePreProgress(int8_t percent);
    void Error(std::string message);
    void Error(std::string message, int32_t result);

protected:
    IzInstanceBase* instance_;
    bool isCancelling_;
    uint64_t id_;
    uint64_t size_;
    std::string mountPoint_;
    pp::CompletionCallbackFactory<FileConverter>* callbackFactory_;
private:

    pp::FileSystem *fileSystem_;
    pp::SimpleThread *thread_;

    const std::string startMessage_;
    std::string url_;
    std::string filename_;
    float currentProgress_;
    FSTYPE fstype_;
private:
    STATUSTYPE status_;
    InputDownloader* inputDownloader_;

    void Start(int32_t);

public:
    IzInstanceBase *getInstance_() const {
        return instance_;
    }

    bool getIsCancelling_() const {
        return isCancelling_;
    }

    STATUSTYPE const &getStatus_() const {
        return status_;
    }


private:
    void SendOutputURL(int32_t result,const std::vector<pp::DirectoryEntry> entries,pp::FileRef directory);

    void DeleteTaskDirectory();

    std::string GetOutputDirectoryPath();

    std::string GetInputDirectoryPath();

    void DeleteDirectory(int32_t result, std::vector<pp::DirectoryEntry> const entries, pp::FileRef directory);

    void DeleteInputDirectory();

    void InputDownloadDone();

    void EndOfThread(int32_t);

};
