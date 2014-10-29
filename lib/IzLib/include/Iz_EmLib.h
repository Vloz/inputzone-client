#pragma once
#include <emscripten.h>
#include <libcxx/iosfwd>
#include <libcxx/string>
#include <Iz_Message.h>
#include <Iz_Em_FileSystem.h>

typedef void (*iz_init_onDone_func)(std::string,//taskId
        IZ_FILE,//file input
        uint64_t,//input size
        std::string//Ouput directory path
         );

typedef void (*iz_postMessage_func)(char*, int);
typedef void (*iz_postFinalMessage_func)(char*, int);



void iz_init(char *data, int size, std::string converterName, iz_init_onDone_func converterCallback,iz_postMessage_func postMessageFunc, iz_postFinalMessage_func postFinalMessageFunc);
void iz_release(IZ_FILE output);

void iz_updateProgress(uint8_t percentage);
void iz_updatePreProgress(uint8_t percentage);
void iz_error(std::string error);
void iz_error(std::string error, int32_t numError);
void iz_print(std::string message);

FILE* iz_truncateInput(uint64_t spanLength, bool fromStart);


extern BROWSER currentBrowser;



extern FSTYPE currentFsType;


struct TaskProps{
    std::string id;
    std::string startMessage;
    IZ_FILE input;
    uint64_t inputSize;
    uint32_t inputIndex;
    uint8_t currentProgress;
    uint8_t currentPreProgress;
    iz_init_onDone_func init_onDone_func;
    iz_postMessage_func postMessage;
    iz_postFinalMessage_func postFinalMessage;
    std::string mountPoint;
    std::string inputFilename;
    clock_t convertStart;
    clock_t convertEnd;
};





