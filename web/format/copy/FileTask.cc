#include "FileTask.h"

FileTask::FileTask( IzInstance *instance,const pp::Var& var_message) :  instance_(instance), isFilesystemOpen(false)
{
    instance_->ShowStatusMessage("test test!");
    printf("\ntest test test\n");
    fflush(stdout);
    id_ =  instance_->getMessageId(var_message);
    thread_ = new pp::SimpleThread(instance);
    callbackFactory_ = new pp::CompletionCallbackFactory<FileTask>(this);
    fileSystem_ = new pp::FileSystem(instance,PP_FILESYSTEMTYPE_LOCALTEMPORARY);
    thread_->Start();
    thread_->message_loop().PostWork(callbackFactory_->NewCallback(&FileTask::InitFileSystem));
/*    thread_->Join();

    if(isFilesystemOpen)
        instance_->ShowStatusMessage("FS READY!!!");
    else
        instance_->ShowStatusMessage("FS NOT READY!!!");*/

}

FileTask::~FileTask() {
    delete thread_;
    delete  callbackFactory_;
    if(isFilesystemOpen)
        fileSystem_->detach();
    delete fileSystem_;
}

void FileTask::InitFileSystem(int32_t){
    instance_->ShowStatusMessage("1!");

    int32_t rv = fileSystem_->Open(1024 * 50, pp::BlockUntilComplete());

    instance_->ShowStatusMessage("2!");
    if (rv == PP_OK) {
        // Notify the user interface that we're ready
        printf("\nFILE SYSTEM READY| %llu\n",id_);
        isFilesystemOpen=true;
    } else {
        printf("\nFailed to open file system:Error%i\nFor id:%llu\n", rv, id_);
    }
    fflush(stdout);
}




