#include <iosfwd>
#include <ppapi/cpp/file_io.h>
#include <ppapi/cpp/instance.h>
#include <ppapi/c/ppb_file_io.h>
#include "InputDownloader.h"


InputDownloader::~InputDownloader() {
    delete urlLoader_;
    delete urlRequestInfo_;
    delete thread_;
    delete[] downloadBuffer_;
    delete input_;
    delete  inputRef_;
}

InputDownloader::InputDownloader(std::string url,std::string savePath,uint64_t size, pp::FileSystem* fileSystem,FileConverter &converter, pp::CompletionCallback& cc)
        : converter_(converter), url_(url),size_(size), cursorInput(0), fileSystem_(fileSystem),file_offset_(0), cc_(cc){
    downloadBuffer_ = new char[DOWNLOAD_BUFFER_SIZE];
    callbackFactory_ = new pp::CompletionCallbackFactory<InputDownloader>(this);
    thread_ = new pp::SimpleThread(converter_.getInstance_());
    inputRef_ = new pp::FileRef(*fileSystem_,std::string(savePath).c_str());
    input_ = new pp::FileIO(converter.getInstance_());
    timeoutPreviousTick_=clock();

}


void InputDownloader::Start(){
    int32_t r = input_->Open(*inputRef_,PP_FILEOPENFLAG_WRITE | PP_FILEOPENFLAG_CREATE | PP_FILEOPENFLAG_TRUNCATE,pp::BlockUntilComplete());
    if (r != PP_OK) {
        converter_.Error("Couldnt create the input file:",r);
        return;
    }

    urlRequestInfo_ = new pp::URLRequestInfo(converter_.getInstance_());

    urlRequestInfo_->SetURL(url_);
    urlRequestInfo_->SetMethod("GET");

    thread_->Start();
    thread_->message_loop().PostWork(callbackFactory_->NewCallback(&InputDownloader::DownloadTry));
    while (converter_.getStatus_() == STARTING){
        if(timeouTick_==timeoutPreviousTick_){
            converter_.getInstance_()->DebugMessage("TIMEOUT!!!");
            //delete thread_;<=====MEMORYLEAK thread and urLoader are forgotten
            //delete  urlLoader_;
            thread_ = new pp::SimpleThread(converter_.getInstance_());
            thread_->Start();
            thread_->message_loop().PostWork(callbackFactory_->NewCallback(&InputDownloader::DownloadTry));
        }
        else
            timeoutPreviousTick_=timeouTick_;

        struct timespec tim, tim2;
        tim.tv_sec  = 2;
        tim.tv_nsec = 500000000L;
        nanosleep(&tim , &tim2);
    }
}

void InputDownloader::DownloadTry(int32_t){
    converter_.getInstance_()->DebugMessage("FOOL!!!");
    urlLoader_ = new pp::URLLoader(converter_.getInstance_());
    urlLoader_->Open(*urlRequestInfo_, pp::BlockUntilComplete());
    uint64_t downloaded = 0;
    int32_t result = PP_OK;
    do {
        result = urlLoader_->ReadResponseBody(downloadBuffer_, DOWNLOAD_BUFFER_SIZE, pp::BlockUntilComplete());
        downloaded+=result;
        int64_t buffer_offset = 0;
        int32_t bytes_written = 0;
        if(result>0 && downloaded>file_offset_)
            do {
                bytes_written = input_->Write(file_offset_,
                        downloadBuffer_ + buffer_offset,
                        result,
                        pp::BlockUntilComplete());
                if (bytes_written > 0) {
                    buffer_offset += bytes_written;
                } else {
                    printf("File write failed");
                    return;
                }
            } while (bytes_written < static_cast<int64_t>(result));
        file_offset_+=bytes_written;
        timeouTick_ = clock();
        converter_.UpdatePreProgress((int8_t)((float)file_offset_/(float)size_*100));
        if(converter_.getIsCancelling_())
            return;

    } while (result > 0);
    cc_.Run(0);
    int32_t flush_result = input_->Flush(pp::BlockUntilComplete());
    if (flush_result != PP_OK) {
        converter_.Error("File fail to flush");
        cc_.Run(flush_result);
    }
    cc_.Run(PP_OK);


}