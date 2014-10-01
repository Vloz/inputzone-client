#include "FileConverter.h"

FileConverter::FileConverter( IzInstanceBase *instance,const pp::Var& var_message, std::string converterName) :  instance_(instance), converterName_(converterName) , isFilesystemOpen(false),currentProgress_(0.0)
{
    id_ =  instance_->getMessageId(var_message);
    url_ = instance_->getMessageValue("url", var_message);
    filename_ = instance_->getMessageValue("filename", var_message);
    size_ = strtoull(instance_->getMessageValue("size", var_message).c_str(),NULL,0);
    urlLoader_ = new pp::URLLoader(instance_);
    urlRequestInfo_ = new pp::URLRequestInfo(instance_);

    urlRequestInfo_->SetURL(url_);
    urlRequestInfo_->SetMethod("GET");
    urlRequestInfo_->SetRecordDownloadProgress(true);

    thread_ = new pp::SimpleThread(instance);
    callbackFactory_ = new pp::CompletionCallbackFactory<FileConverter>(this);
    fileSystem_ = new pp::FileSystem(instance,PP_FILESYSTEMTYPE_LOCALTEMPORARY);
    thread_->Start();
    thread_->message_loop().PostWork(callbackFactory_->NewCallback(&FileConverter::Start));
    buffer_ = new char[BUFFER_SIZE];

/*    thread_->Join();

    if(isFilesystemOpen)
        instance_->ShowStatusMessage("FS READY!!!");
    else
        instance_->ShowStatusMessage("FS NOT READY!!!");*/

}

FileConverter::~FileConverter() {
    delete thread_;
    delete  callbackFactory_;
    if(isFilesystemOpen)
        fileSystem_->detach();
    delete fileSystem_;
    delete urlLoader_;
    delete urlRequestInfo_;
    delete[] buffer_;
}

void FileConverter::Start(int32_t){
    int fs_r= InitFileSystem();
    if(fs_r!=PP_OK){
        Error("Cannot create the internal file system:",fs_r);
        return;
    }
    urlLoader_->Open(*urlRequestInfo_, pp::BlockUntilComplete());

    pp::FileRef(*fileSystem_, GetDirectoryPath().c_str()).MakeDirectory(PP_MAKEDIRECTORYFLAG_WITH_ANCESTORS, pp::BlockUntilComplete());
    pp::FileRef ref(*fileSystem_,std::string(GetDirectoryPath()+filename_).c_str());
    pp::FileIO file(instance_);
    int32_t open_result = file.Open(ref,PP_FILEOPENFLAG_WRITE | PP_FILEOPENFLAG_CREATE | PP_FILEOPENFLAG_TRUNCATE, pp::BlockUntilComplete());
    if (open_result != PP_OK) {
        Error("Couldnt create the input file!", open_result);
        return;
    }

    uint64_t downloaded = 0, file_offset = 0;
    int32_t result = PP_OK;
    do {
        result = urlLoader_->ReadResponseBody(buffer_, BUFFER_SIZE, pp::BlockUntilComplete());
        downloaded+=result;
        int64_t buffer_offset = 0;
        int32_t bytes_written = 0;
        if(result>0)
        do {
            bytes_written = file.Write(file_offset,
                    buffer_ + buffer_offset,
                    result,
                    pp::BlockUntilComplete());
            if (bytes_written > 0) {
                buffer_offset += bytes_written;
            } else {
                Error("File write failed");
                return;
            }
        } while (bytes_written < static_cast<int64_t>(result));
        file_offset+=bytes_written;
        UpdatePreProgress((int8_t)((float)file_offset/(float)size_*100));
    } while (result > 0);
    int32_t flush_result = file.Flush(pp::BlockUntilComplete());
    if (flush_result != PP_OK) {
        Error("File fail to flush");
        return;
    }
    printf("Finished:%llu/%llu",size_, downloaded);
    instance_->DebugMessage("Save success");


    nacl_io_init_ppapi(instance_->pp_instance(), pp::Module::Get()->get_browser_interface());
    FILE* input = NULL;
    int ret = umount( "/" );
    mount("",
            "/temporary",
            "html5fs",
            0,
            "type=TEMPORARY,expected_size=53687091200");

    std::string fullInputPath = "/temporary"+GetDirectoryPath()+filename_;
    input = fopen(fullInputPath.c_str(), "r");
    clock_t begin, end;
    begin = clock();
    if(Convert(id_,input,size_, "/temporary"+GetDirectoryPath(), GetBaseName(filename_), GetExtension(filename_)) == PP_OK) {
        end = clock();
        instance_->DebugMessage("Conversion done in " + std::to_string((double) (end - begin) / CLOCKS_PER_SEC)+" seconds");
        UpdateTaskStatut("Complete");
    }
    fclose(input);
}



int32_t FileConverter::InitFileSystem(){
    int32_t rv = fileSystem_->Open(1024 * 1024 * 1024 * 50, pp::BlockUntilComplete());
    return rv;
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

std::string FileConverter::GetDirectoryPath(){
    return "/InputZone/"+converterName_+"/"+std::to_string(id_)+"/";
}

void FileConverter::Abort(){
    UpdateTaskStatut("Cancelled");
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

void FileConverter::UpdateTaskStatut(std::string statut) {
    instance_->UpdateTaskStatut(id_, statut);
}

void FileConverter::Error(std::string message) {
    instance_->TaskError(id_, message);
}

void FileConverter::Error(std::string message, int32_t result) {
    instance_->TaskError(id_, message+"::"+std::to_string(result));
}