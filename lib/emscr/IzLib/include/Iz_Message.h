
#include <libcxx/sstream>
#pragma once

typedef std::string iz_message;

enum MESSAGETYPE{
    START=1, //--> Start a file convertion with the specified url
    CANCEL=2, //--> Cancelation query from ui
    PREPROGRESS=3, //<-- Update preprogress value of the task
    PROGRESS=4, //<--        '' progress ''
    CONSOLE=5, //<-- Console message
    STATUS=6, //<-- Status of a task see FileConverter.h for types of Status
    OUTPUTURL=7, //<-- Send the output file url to the ui for download
    DETAILS=8, //<-- Optional details wrote under the task UI
    PRERUN=9 //<-- Send answer to a Prerun query (estimate output size, and base params)

};

enum STATUSTYPE{
    CANCELED=0,
    STARTING=1,
    CONVERTING=2,
    COMPLETING=3,
    COMPLETED=4,
    ERRORED=5,
    CANCELING=6,
    OPTIMIZINGRAM=7,
    WAITINGQUOTA=8
};

enum BROWSER{
    UNKNOWNBROWSER=0,
    CHROME=1,
    FIREFOX=2,
    OPERA=3,
    SAFARI=4,
    INTERNETEXPLORER=5
};

std::string getMessageValue(std::string key, std::string message);
int getMessageParams(const std::string message,char* argv[]);
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
iz_message consoleMsg(std::string id,std::string body);
iz_message sendOutputUrlMsg(std::string id, std::string url);
iz_message detailsMsg(std::string id, std::string details);


std::string basename(std::string const& pathname);
std::string getExtension(std::string filename);
