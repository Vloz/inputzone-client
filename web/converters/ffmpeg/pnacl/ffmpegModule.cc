/*
 * Copyright (c) 2000-2003 Fabrice Bellard
 *
 * This file is part of FFmpeg.
 *
 * FFmpeg is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * FFmpeg is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with FFmpeg; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

/**
 * @file
 * multimedia converter based on the FFmpeg libraries
 */
#include <ffmpegMain.h>
#include "ppapi/cpp/instance.h"
#include "ppapi/cpp/module.h"
#include "IzInstance.h"
#include "FileConverter.h"

#define MSGBUF 8192

const char program_name[] = "ffmpeg";
const int program_birth_year = 2000;


class ffmpegConverter : public FileConverter {
public:
    ffmpegConverter( IzInstanceBase *instance,const pp::Var& var_message) : FileConverter(instance, var_message){
    };

    static int64_t estimateOutputSize(int64_t inputSize){
        return inputSize;
    }

    static std::string baseParameters(uint64_t taskId, std::string inputFullPath, uint64_t inputSize, std::string outputDirectoryPath, std::string baseName, std::string inputExtension){
        return "-i \\\""+inputFullPath+"\\\" %PARAMS% \\\""+outputDirectoryPath+baseName+"%OUTPUT_EXT%\\\"";
    }

private:

    pp::SimpleThread *msgThread_;
    pp::CompletionCallbackFactory<ffmpegConverter>* innerCallbackFactory_;
    uint64_t inputDuration;

    void msgLoop(int32_t){
        inputDuration = 0;
        char buf[MSGBUF];
        FILE* fstderr = NULL;
        FILE* fstdin = NULL;

        fstdin = fopen((mountPoint_+this->GetMainDirectoryPath()+"/stdin.txt").c_str(), "a");
        while(getStatus_() == CONVERTING || getStatus_() == CANCELING){
            if(isCancelling_)
            {
                fputc('q', fstdin);
                fflush(fstdin);
                fflush(stdin);
                UpdateTaskStatus(CANCELED);
                return;
            }
            if( inputDuration==0){
                if(ftell(stderr)> MSGBUF)
                    return;
                fflush(stderr);
                fstderr =  fopen((mountPoint_+this->GetMainDirectoryPath()+"/stderr.txt").c_str(), "r");
                int l = fread(buf, 1, MSGBUF, fstderr);
                if(l>0){
                    buf[l] = '\0';
                    const char* p = strstr(buf, "Duration: ");
                    if(p != NULL){
                        unsigned hours = ((p+10)[0] - '0') * 10 + ((p+10)[1] - '0');
                        unsigned mins = ((p+13)[0] - '0') * 10 + ((p+13)[1] - '0');
                        unsigned secs = ((p+16)[0] - '0') * 10 + ((p+16)[1] - '0');
                        if(hours+mins+secs<210)
                            inputDuration = hours*60*60 + mins*60 + secs;
                    }
                }

                fclose(fstderr);
            }else{
                fstderr =  fopen((mountPoint_+this->GetMainDirectoryPath()+"/stderr.txt").c_str(), "r");
                fseek(fstderr, -80, SEEK_END);
                int l = fread(buf, 1, MSGBUF, fstderr);
                if(l>0) {
                    buf[l] = '\0';
                    const char* p = strstr(buf, "time=");
                    if(p != NULL){
                        unsigned hours = ((p+5)[0] - '0') * 10 + ((p+5)[1] - '0');
                        unsigned mins = ((p+8)[0] - '0') * 10 + ((p+8)[1] - '0');
                        unsigned secs = ((p+11)[0] - '0') * 10 + ((p+11)[1] - '0');
                        if(hours+mins+secs<210){
                            uint64_t currentTime = hours*60*60 + mins*60 + secs;
                            UpdateProgress((int8_t)( 100*currentTime/inputDuration));
                        }
                    }
                }
                fclose(fstderr);
            }

            struct timespec tim, tim2;
            tim.tv_sec  = 0;
            tim.tv_nsec = 500000000L;
            nanosleep(&tim , &tim2);
        }
    }

    int main(int argc, char *argv[])
    {
        innerCallbackFactory_ = new pp::CompletionCallbackFactory<ffmpegConverter>(this);
        //freopen((mountPoint_+this->GetMainDirectoryPath()+"/stdout.txt").c_str(), "w", stdout);
        freopen((mountPoint_+this->GetMainDirectoryPath()+"/stderr.txt").c_str(), "w", stderr);
        FILE* in = fopen((mountPoint_+this->GetMainDirectoryPath()+"/stdin.txt").c_str(), "w");
        fclose(in);
        freopen((mountPoint_+this->GetMainDirectoryPath()+"/stdin.txt").c_str(), "r", stdin);
        msgThread_ = new pp::SimpleThread(instance_);
        msgThread_->Start();
        msgThread_->message_loop().PostWork(innerCallbackFactory_->NewCallback(&ffmpegConverter::msgLoop));
       return ffmpeg(argc,argv);
    }

};


class ffmpegModule : public pp::Module {
public:
    ffmpegModule() : pp::Module() {}
    virtual ~ffmpegModule() {}

    virtual pp::Instance* CreateInstance(PP_Instance instance) {
        return new IzInstance<ffmpegConverter>(instance);
    }
};

namespace pp {
    Module* CreateModule() {
        return new ffmpegModule();
    }
}


