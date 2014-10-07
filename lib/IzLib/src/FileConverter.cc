#include <search.h>
#include "FileConverter.h"

FileConverter::FileConverter( IzInstanceBase *instance,const pp::Var& var_message, std::string converterName)
        :  instance_(instance), converterName_(converterName) , isCancelling_(false), currentProgress_(0.0)
{
    id_ =  instance_->getMessageId(var_message);
    url_ = instance_->getMessageValue("url", var_message);
    filename_ = instance_->getMessageValue("filename", var_message);
    size_ = strtoull(instance_->getMessageValue("size", var_message).c_str(),NULL,0);
    thread_ = new pp::SimpleThread(instance);
    callbackFactory_ = new pp::CompletionCallbackFactory<FileConverter>(this);
    fileSystem_ = new pp::FileSystem(instance,PP_FILESYSTEMTYPE_LOCALTEMPORARY);
    thread_->Start();
    thread_->message_loop().PostWork(callbackFactory_->NewCallback(&FileConverter::Start));
    thread_->message_loop().PostWork(callbackFactory_->NewCallback(&FileConverter::EndOfThread));
}

FileConverter::~FileConverter() {
    delete thread_;
    delete  callbackFactory_;
    delete fileSystem_;
    delete inputDownloader_;
}

void FileConverter::Start(int32_t){
    UpdateTaskStatus(STARTING);
    int fs_r= fileSystem_->Open(1024 * 1024 * 1024 * 50, pp::BlockUntilComplete());
    mountPoint_="/temporary";
    if(fs_r!=PP_OK){
        Error("Cannot create the internal file system:",fs_r);
        return;
    }

    pp::FileRef(*fileSystem_, GetInputDirectoryPath().c_str()).MakeDirectory(PP_MAKEDIRECTORYFLAG_WITH_ANCESTORS, pp::BlockUntilComplete());
    pp::FileRef(*fileSystem_, GetOutputDirectoryPath().c_str()).MakeDirectory(PP_MAKEDIRECTORYFLAG_WITH_ANCESTORS, pp::BlockUntilComplete());
    inputDownloader_ = new InputDownloader(url_,GetInputDirectoryPath()+filename_,size_,fileSystem_,*this);
    inputDownloader_->Start();
    InputDownloadDone();
}

void FileConverter::InputDownloadDone() {
    if(status_!=CONVERTING)
        return;
    UpdatePreProgress(0);

    nacl_io_init_ppapi(instance_->pp_instance(), pp::Module::Get()->get_browser_interface());
    FILE* input = NULL;
    umount( "/" );
    mount("",
            "/temporary",
            "html5fs",
            0,
            "type=TEMPORARY,expected_size=53687091200");
    std::string fullInputPath = mountPoint_+ GetInputDirectoryPath()+filename_;
    input = fopen(fullInputPath.c_str(), "r");
    clock_t begin, end; //to measure conversion time
    begin = clock();
    int32_t conversionResult = Convert(id_,input,size_, mountPoint_+ GetOutputDirectoryPath(), GetBaseName(filename_), GetExtension(filename_));
    fclose(input);
    delete input;
    if(status_==CANCELED)
        return;

    if( conversionResult == PP_OK) {
        end = clock();
        instance_->DebugMessage("Conversion done in " + std::to_string((double) (end - begin) / CLOCKS_PER_SEC)+" seconds");
        UpdateTaskStatus(COMPLETED);
        DeleteInputDirectory();

        pp::FileRef refdir(*fileSystem_, GetOutputDirectoryPath().c_str());
        refdir.ReadDirectoryEntries(callbackFactory_->NewCallbackWithOutput(&FileConverter::SendOutputURL, refdir));
    }
    else{
        DeleteTaskDirectory();
    }
}



void FileConverter::EndOfThread(int32_t){
    instance_->DebugMessage("Task id:"+std::to_string(id_)+"===> TERMINATED!");
}

void FileConverter::SendOutputURL(int32_t result, const std::vector<pp::DirectoryEntry> entries, pp::FileRef /* unused_ref */) {
    if (result != PP_OK) {
        Error("List failed", result);
        return;
    }
    instance_->SendOutputURL(id_, mountPoint_+GetOutputDirectoryPath()+entries.front().file_ref().GetName().AsString());
}


void FileConverter::UpdateProgress(int8_t percent){
    if(percent!=currentProgress_){
        currentProgress_=percent;
        instance_->UpdateProgress(id_, percent);
    }
}

void FileConverter::UpdatePreProgress(int8_t percent){
    if(percent!=currentProgress_){
        currentProgress_=percent;
        instance_->UpdatePreProgress(id_, percent);
    }
}

std::string FileConverter::GetMainDirectoryPath(){
    return "/InputZone/"+converterName_+"/"+std::to_string(id_);
}

std::string FileConverter::GetInputDirectoryPath(){
    return GetMainDirectoryPath()+"/input/";
}

std::string FileConverter::GetOutputDirectoryPath(){
    return GetMainDirectoryPath()+"/output/";
}


int32_t FileConverter::Convert(double taskId, FILE* input, uint64_t inputSize, std::string directoryPath , std::string baseName, std::string inputExtension) {
    return PP_OK;
}

std::string FileConverter::GetBaseName(std::string filename) {
    if(filename.find_last_of(".") != std::string::npos)
        return filename.substr(0,filename.find_last_of("."));
    return filename;
}

std::string FileConverter::GetExtension(std::string filename) {
    if(filename.find_last_of(".") != std::string::npos)
        return filename.substr(filename.find_last_of("."));
    return "";
}

void FileConverter::UpdateTaskStatus(STATUSTYPE statustype) {
    status_=statustype;
    instance_->UpdateTaskStatut(id_, statustype);
}

void FileConverter::Error(std::string message) {
    UpdateTaskStatus(ERRORED);
    UpdateTaskDetails(message);
}

void FileConverter::Error(std::string text, int32_t result) {
    switch (result){
        case PP_ERROR_NOQUOTA:
            Error(text+"Not enough space available!");
            break;
        default:
            Error(text+std::to_string(result));
    }
    DeleteTaskDirectory();
}

void FileConverter::UpdateTaskDetails(std::string text) {
    instance_->UpdateTaskDetails(id_, text);
}

void FileConverter::DeleteTaskDirectory() {
    pp::FileRef outputdir(*fileSystem_, GetOutputDirectoryPath().c_str());
    pp::FileRef inputdir(*fileSystem_, GetInputDirectoryPath().c_str());
    outputdir.ReadDirectoryEntries(callbackFactory_->NewCallbackWithOutput(
            &FileConverter::DeleteDirectory, outputdir));
    inputdir.ReadDirectoryEntries(callbackFactory_->NewCallbackWithOutput(
            &FileConverter::DeleteDirectory, inputdir));
    pp::FileRef(*fileSystem_, GetMainDirectoryPath().c_str()).Delete(pp::BlockUntilComplete()); //Doesnt works return -2 ///TODO:Find out why!
}

void FileConverter::DeleteInputDirectory() {
    pp::FileRef inputdir(*fileSystem_, GetInputDirectoryPath().c_str());
    inputdir.ReadDirectoryEntries(callbackFactory_->NewCallbackWithOutput(
            &FileConverter::DeleteDirectory, inputdir));
    pp::FileRef(*fileSystem_, GetMainDirectoryPath().c_str()).Delete(pp::BlockUntilComplete()); //<<== useless line, return -2, but seems to wake up the filesystem...
}


void FileConverter::DeleteDirectory(int32_t result, const std::vector<pp::DirectoryEntry> entries, pp::FileRef directory) {
    if (result == PP_OK) {
        int r = 0;
        for (auto &entry : entries){
            r=entry.file_ref().Delete(pp::BlockUntilComplete());
            if(r!=PP_OK)
                instance_->DebugErrorMessage("Could not delete the file "+entry.file_ref().GetName().AsString()+" : ", r);
        }

        r= directory.Delete(pp::BlockUntilComplete());
        if(r!=PP_OK)
            instance_->DebugErrorMessage("Could not delete the directory:", r);
    }
    else
        instance_->DebugErrorMessage("Could not read into the directory to delete:", result);
}

void FileConverter::Cancel() {
    UpdateTaskStatus(CANCELING);
    isCancelling_=true;
}
