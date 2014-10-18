#include <Iz_EmLib.h>
#include <libc/unistd.h>
#include <libc/dirent.h>


TaskProps _taskProps;

void postMessage(iz_message message){
    _taskProps.postMessage((char*)message.c_str(), message.length());
}

std::string getOutputFullPath(){
    DIR           *d;
    struct dirent *dir;
    d = opendir((_taskProps.mountPoint+getMessageOutputDirectoryPath(_taskProps.startMessage)).c_str());
    if (d){
        while ((dir = readdir(d)) != NULL)
        {
            if(strncmp(dir->d_name, "..", 2) && strncmp(dir->d_name, ".", 1))
            return _taskProps.mountPoint+getMessageOutputDirectoryPath(_taskProps.startMessage)+"/"+dir->d_name;
        }

        closedir(d);
    }

    return "";
}

void onDownloadInputFinished(unsigned handle, void* args, const char* fullpath){
    EM_ASM_ARGS({
        Module.print('Download finished:'+Pointer_stringify($0));
    },fullpath);
    _taskProps.input = fopen(fullpath,"r");
    if(_taskProps.input==NULL)
    {
        iz_error("Couldn't create input file...");
        return;
    }
    postMessage(statusMsg(_taskProps.id,CONVERTING));
    (*_taskProps.init_onDone_func)(_taskProps.id,_taskProps.input,_taskProps.inputSize,_taskProps.mountPoint+ getMessageOutputDirectoryPath(_taskProps.startMessage)+"/", getBaseName(_taskProps.inputFilename),getExtension(_taskProps.inputFilename));
}

void onDownloadInputError(unsigned, void* err, int){

    EM_ASM_ARGS({
        Module.print("Error during download:"+Pointer_stringify($0));
    },(char*)err);
}

void onDownloadInputProgress(unsigned, void*, int progress ){

    iz_updatePreProgress((uint8_t)progress);
}

void iz_release(FILE* output){
    fclose(_taskProps.input);
    remove((_taskProps.mountPoint+getMessageInputFullPath(_taskProps.startMessage)).c_str());
    EM_ASM_ARGS({
        Module.print("Path:"+Pointer_stringify($0));
        var bytes = FS.readFile(Pointer_stringify($0), { encoding: 'binary' });
        Module.print("trying:");
        var oMyBlob = new Blob([bytes.buffer], { type: "image/png" });
        Module.print("trying:");
        var url = URL.createObjectURL(oMyBlob);
        Module.print("url:"+url);
        //var url = (window.webkitURL ? webkitURL : URL).createObjectURL(oMyBlob);
    },(char*) getOutputFullPath().c_str());
}

void iz_init(char *data, int size,iz_init_onDone_func converterCallback,iz_postMessage_func postMessageFunc, iz_postFinalMessage_func postFinalMessageFunc) {
    _taskProps.postMessage = postMessageFunc;
    _taskProps.postFinalMessage = postFinalMessageFunc;
    _taskProps.mountPoint = "/MEMFS";
    _taskProps.startMessage = data;
    _taskProps.id = getMessageFullId(data);
    _taskProps.inputSize = getMessageSize(data);
    _taskProps.init_onDone_func = converterCallback;
    _taskProps.inputFilename = getMessageFilename(data);
    postMessage(statusMsg(_taskProps.id, STARTING));
    EM_ASM_ARGS(
            {
                FS.mkdir(Pointer_stringify($2));
                FS.mount(MEMFS, {}, Pointer_stringify($2));
                FS.mkdir(Pointer_stringify($0));
                FS.mkdir(Pointer_stringify($1));
                FS.mkdir(Pointer_stringify($3));},
            (_taskProps.mountPoint+ getMessageDirectoryPath(data)).c_str(),(_taskProps.mountPoint+ getMessageInputDirectoryPath(data)).c_str(),_taskProps.mountPoint.c_str(),(_taskProps.mountPoint+ getMessageOutputDirectoryPath(data)).c_str()
    );
    emscripten_async_wget2(getMessageValue("url", data).c_str(), (_taskProps.mountPoint+ getMessageInputFullPath(data)).c_str(),"GET", "", (void*) getMessageShortId(data),
            onDownloadInputFinished, onDownloadInputError, onDownloadInputProgress);
}

void iz_updateProgress(uint8_t percentage){
    postMessage(progressMsg(_taskProps.id, percentage));
}

void iz_updatePreProgress(uint8_t percentage){
    postMessage(preprogressMsg(_taskProps.id, percentage));
}

void iz_error(std::string error){
    postMessage(errorMsg(_taskProps.id, error));
}

void iz_error(std::string error, int32_t numError){
    postMessage(errorMsg(_taskProps.id, error,numError));
}

void iz_print(std::string message){
    EM_ASM_ARGS({
        Module.print(Pointer_stringify($0));
    },message.c_str());
}


void iz_truncateInput(uint32_t spanLength, bool fromStart){

}