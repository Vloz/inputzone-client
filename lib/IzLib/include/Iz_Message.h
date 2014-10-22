#include <libcxx/iosfwd>
#include <libc/bits/alltypes.h>
#include <libcxx/string>
#include <libc/stdlib.h>
#include <libcxx/sstream>
#pragma once

typedef std::string iz_message;

enum MESSAGETYPE{
    START=1, //--> Start a file convertion with the specified url
    CANCEL=2, //--> Cancelation query from ui
    PREPROGRESS=3, //<-- Update preprogress value of the task
    PROGRESS=4, //<--        '' progress ''
    ERROR=5, //<-- General Module Error
    STATUS=6, //<-- Status of a task see FileConverter.h for types of Status
    OUTPUTURL=7, //<-- Send the output file url to the ui for download
    DETAILS=8 //<-- Optional details wrote under the task UI
};

enum STATUSTYPE{
    CANCELED=0,
    STARTING=1,
    CONVERTING=2,
    COMPLETING=3,
    COMPLETED=4,
    ERRORED=5,
    CANCELING=6,
    OPTIMIZINGRAM=7
};

std::string getMessageValue(std::string key, std::string message);
int32_t getMessageShortId(std::string message);
std::string getMessageFullId(std::string message);
uint8_t getMessageType(std::string message);
std::string getMessageInputFullPath(std::string message);
std::string getMessageInputDirectoryPath(std::string message);
std::string getMessageChunksDirectoryPath(std::string message);
std::string getMessageOutputDirectoryPath(std::string message);
std::string getMessageDirectoryPath(std::string message);
std::string getMessageFilename(std::string message);
uint64_t getMessageSize(std::string message);

iz_message statusMsg(std::string id, STATUSTYPE status);
iz_message progressMsg(std::string id, uint8_t value);
iz_message preprogressMsg(std::string id, uint8_t value);
iz_message errorMsg(std::string id,std::string body);
iz_message errorMsg(std::string id,std::string body, int32_t errornum);

std::string getBaseName(std::string filename);
std::string getExtension(std::string filename);