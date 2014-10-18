#pragma once
#include <emscripten.h>
#include <libcxx/iosfwd>
#include <libcxx/string>
#include <Iz_Message.h>

typedef void (*iz_init_onDone_func)(std::string,//taskId
        FILE*,//file input
        uint64_t,//input size
        std::string,//directory path
        std::string ,//file basename
        std::string  //input extension
         );

typedef void (*iz_postMessage_func)(char*, int);
typedef void (*iz_postFinalMessage_func)(char*, int);



void iz_init(char *data, int size,iz_init_onDone_func converterCallback,iz_postMessage_func postMessageFunc, iz_postFinalMessage_func postFinalMessageFunc);
void iz_release(FILE* output);

void iz_updateProgress(uint8_t percentage);
void iz_updatePreProgress(uint8_t percentage);
void iz_error(std::string error);
void iz_error(std::string error, int32_t numError);
void iz_print(std::string message);


struct TaskProps{
    std::string id;
    std::string startMessage;
    FILE* input;
    uint64_t inputSize;
    iz_init_onDone_func init_onDone_func;
    iz_postMessage_func postMessage;
    iz_postFinalMessage_func postFinalMessage;
    std::string mountPoint;
    std::string inputFilename;
};