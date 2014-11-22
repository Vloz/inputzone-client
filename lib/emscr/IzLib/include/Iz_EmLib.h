#pragma once
#include <emscripten.h>
#include <Iz_Message.h>
#include <Iz_Em_FileSystem.h>

typedef int (*iz_main_func)(int argc, char* argv[]);

typedef void (*iz_postMessage_func)(char*, int);
typedef void (*iz_postFinalMessage_func)(char*, int);


std::string createPrerunRequest(char *data, int size);
void iz_init(char *data, int size, iz_main_func mainCallback,iz_postMessage_func postMessageFunc, iz_postFinalMessage_func postFinalMessageFunc);
void iz_release(IZ_FILE* output);

void iz_updateProgress(uint8_t percentage);
void iz_updatePreProgress(uint8_t percentage);
void iz_error(std::string error);
void iz_console(std::string message);
void iz_print(std::string message);
void iz_details(std::string details);

//FILE* iz_truncateInput(uint64_t spanLength, bool fromStart);


extern BROWSER currentBrowser;



extern FSTYPE currentFsType;


struct TaskProps{
    std::string id;
    std::string startMessage;
    uint64_t inputSize;
    uint8_t currentProgress;
    uint8_t currentPreProgress;
    iz_main_func main_func;
    iz_postMessage_func postMessage;
    iz_postFinalMessage_func postFinalMessage;
    std::string mountPoint;
    std::string inputFilename;
    std::string inputFullpath;
    clock_t convertStart;
    clock_t convertEnd;
};





