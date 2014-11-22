#include <Iz_Message.h>


std::string getMessageValue(std::string key, std::string message) {
    std::istringstream is_file(message);
    std::string line;
    while( std::getline(is_file, line) )
    {
        if(line==key){
            std::getline(is_file, line);
            return line;
        }else if(line=="params")
            return "";
    }
    return "";
}

int getMessageParams(const std::string message,char* argv[]) {
    std::istringstream is_file(message);
    std::string line;
    while( std::getline(is_file, line) )
    {
        if(line=="params"){
            int r=0;
            while ( std::getline(is_file, line) ){
                argv[r] = (char*)malloc(line.length()+1);
                memcpy(argv[r], (line+'\0').c_str(), line.length()+1);
                r++;
            }
            return r;
        }
    }
    return 0;
}


uint8_t getMessageType(std::string message){
    return (uint8_t)strtoul(getMessageValue("type", message).c_str(),NULL,0);
}

int32_t getMessageShortId(std::string message){
    return (int)strtoull(getMessageValue("id", message).c_str(),NULL,0);
}

std::string getMessageFullId(std::string message){
    return getMessageValue("id", message);
}

std::string getMessageInputFullPath(std::string message){
    return getMessageInputDirectoryPath(message)+"/"+ getMessageFilename(message);
}

std::string getMessageInputDirectoryPath(std::string message){
    return getMessageDirectoryPath(message)+"/input";
}

std::string getMessageChunksDirectoryPath(std::string message){
    return getMessageDirectoryPath(message)+"/temp";
}

std::string getMessageOutputDirectoryPath(std::string message){
    return getMessageDirectoryPath(message)+"/output";
}

std::string getMessageDirectoryPath(std::string message){
    return "/InputZone/"+getMessageFullId(message);
}


std::string getMessageFilename(std::string message){
    return getMessageValue("filename", message);
}

uint64_t getMessageSize(std::string message){
    return strtoull(getMessageValue("size", message).c_str(),NULL,0);
}


iz_message consoleMsg(std::string id,std::string body){
    return id+"|"+std::to_string(CONSOLE)+"|"+body;
}


iz_message preprogressMsg(std::string id, uint8_t value){
    return id+"|"+std::to_string(PREPROGRESS)+"|"+std::to_string(value);
}

iz_message progressMsg(std::string id, uint8_t value){
    return id+"|"+std::to_string(PROGRESS)+"|"+std::to_string(value);
}

iz_message statusMsg(std::string id, STATUSTYPE status){
    return id+"|"+std::to_string(STATUS)+"|"+std::to_string(status);
}

iz_message sendOutputUrlMsg(std::string id, std::string url){
    return id+"|"+std::to_string(OUTPUTURL)+"|"+url;
}

iz_message detailsMsg(std::string id, std::string details){
    return id+"|"+std::to_string(DETAILS)+"|"+details;
}

/*
std::string getBaseName(std::string filename) {
    if(filename.find_last_of(".") != std::string::npos)
        return filename.substr(0,filename.find_last_of("."));
    return filename;
}

std::string getExtension(std::string filename) {
    if(filename.find_last_of(".") != std::string::npos)
        return filename.substr(filename.find_last_of("."));
    return "";
}
*/
