#include <Iz_EmLib.h>
#include <libc/unistd.h>
#include <libc/dirent.h>
#include <libcxx/stack>
#include <libc/assert.h>

#define TRUNCBUFSIZE 5048576 //Buffer size used for the chunks of the truncation
#define DOWNLOADCHUNKSIZE 104857600
#define DOWNLOADCHUNKSIZEFIREFOX 471859200


TaskProps _taskProps;


BROWSER currentBrowser = UNKNOWNBROWSER;
FSTYPE currentFsType = MEMFS;

void postMessage(iz_message message){
    _taskProps.postMessage((char*)message.c_str(), message.length());
}

extern "C"{


void workerReady(char *data, int size){
    char c[]="Ready";
    emscripten_worker_respond(c,3);
}

void updateDownloadProgress(int percent){
    iz_updatePreProgress((uint8_t)percent);
}

void sendOutputUrl(char* url,char* filename){
    iz_print("insiiide!");

    iz_print(url);

    iz_print(filename);
    postMessage(sendOutputUrlMsg(_taskProps.id,"{\"url\":\""+std::string(url)+"\",\"filename\":\""+filename+"\"}"));
}

}

std::string getFileNameInDirectory(std::string directoryPath){
    DIR           *d;
    struct dirent *dir;
    d = opendir(directoryPath.c_str());
    if (d){
        while ((dir = readdir(d)) != NULL)
        {
            if(strncmp(dir->d_name, "..", 2) && strncmp(dir->d_name, ".", 1))
            return dir->d_name;
        }

        closedir(d);
    }

    return "";
}



void iz_release(IZ_FILE output){

    postMessage(statusMsg(_taskProps.id, COMPLETING));
    iz_fclose(_taskProps.input);
    _taskProps.convertEnd = clock();
    iz_print("Conversion done in " + std::to_string((double) (_taskProps.convertStart - _taskProps.convertEnd) / CLOCKS_PER_SEC)+" seconds");
    iz_remove(_taskProps.input.fullpath.c_str());
    EM_ASM_ARGS({
        if($2==1){//If HTML5FS
            var url ="filesystem:http://localhost:8080/temporary"+Pointer_stringify($0);
            Module.print("url:"+url);
            Module.ccall('sendOutputUrl', // name of C function
                    'void', // return type
            ['string',"string"], // argument types
            [url.toString(),Pointer_stringify($1)]);
        }else{//If MEMFS
            var bytes = FS.readFile(Pointer_stringify($0), { encoding: 'binary' });
            var oMyBlob = new Blob([bytes.buffer], { type: "application/rar" });
            var url = URL.createObjectURL(oMyBlob);

            Module.print("url:"+url);
            Module.ccall('sendOutputUrl', // name of C function
                    'void', // return type
            ['string',"string"], // argument types
            [url.toString(),Pointer_stringify($1)]); // arguments
            //var url = (window.webkitURL ? webkitURL : URL).createObjectURL(oMyBlob);
        }
    },(char*)output.fullpath.c_str(),
            (char*)(output.basename + output.extension).c_str(),(int)currentFsType);

    postMessage(statusMsg(_taskProps.id, COMPLETED));
   // fclose(output);
   // remove(getOutputFullPath().c_str());
}


void onDownloadInputFinished(unsigned handle, void* args, const char* fullpath){
/*    EM_ASM_ARGS({
        Module.print('Download finished:'+Pointer_stringify($0));
    },fullpath);
    _taskProps.input = fopen(fullpath,"r");
    if(_taskProps.input==NULL)
    {
        iz_error("Couldn't create input file...");
        return;
    }
    postMessage(statusMsg(_taskProps.id,CONVERTING));
    (*_taskProps.init_onDone_func)(_taskProps.id,_taskProps.input,_taskProps.inputSize,_taskProps.mountPoint+ getMessageOutputDirectoryPath(_taskProps.startMessage)+"/", getBaseName(_taskProps.inputFilename),getExtension(_taskProps.inputFilename));*/

    iz_print("begin");
/*    _taskProps.input = fopen((_taskProps.mountPoint+ getMessageInputDirectoryPath(_taskProps.startMessage)+std::to_string(_taskProps.inputIndex)).c_str(),"rb+");
    if(_taskProps.input==NULL)
    {
        iz_error("Couldn't create input file...");
        return;
    }*/
    postMessage(statusMsg(_taskProps.id,CONVERTING));
    _taskProps.convertStart = clock();
    iz_updatePreProgress((uint8_t)0);
    iz_print("end");
/*    (*_taskProps.init_onDone_func)(_taskProps.id,_taskProps.input,_taskProps.inputSize,_taskProps.mountPoint+ getMessageOutputDirectoryPath(_taskProps.startMessage)+"/",
            getBaseName(_taskProps.inputFilename),getExtension(_taskProps.inputFilename));*/
}

void onDownloadInputError(unsigned t, void* err, int size){

    EM_ASM_ARGS({
        Module.print("Error during downinload:"+Pointer_stringify($0));
    },(char*)err);
}

void onDownloadInputProgress(unsigned, void*, int progress ){

    iz_updatePreProgress((uint8_t)progress);
}

void iz_init(char *data, int size, std::string converterName, iz_init_onDone_func converterCallback,iz_postMessage_func postMessageFunc, iz_postFinalMessage_func postFinalMessageFunc) {

    _taskProps.inputIndex =0;
    _taskProps.postMessage = postMessageFunc;
    _taskProps.postFinalMessage = postFinalMessageFunc;
    _taskProps.mountPoint = "/MEMFS";
    _taskProps.startMessage = data;
    _taskProps.id = getMessageFullId(data);
    _taskProps.inputSize = getMessageSize(data);
    _taskProps.init_onDone_func = converterCallback;
    _taskProps.inputFilename = getMessageFilename(data);
    currentBrowser = (BROWSER)std::stoi( getMessageValue("browser", data),NULL,10);
    postMessage(statusMsg(_taskProps.id, STARTING));

    if(currentBrowser== CHROME){
        _taskProps.mountPoint="/InputZone/"+converterName;
        currentFsType = HTML5FS;
        EM_ASM_ARGS({
                self.fs_FILES = [];
                self.fs_cursors=[];
                self.fs_writers=[];
                self.fs_filereader = new FileReaderSync();
                self.fs = webkitRequestFileSystemSync(TEMPORARY, 1024*1024*1024*5 /*1MB*/);
                self.fs.root.getDirectory('/InputZone', {create: true});
                self.fs.root.getDirectory(Pointer_stringify($2), {create: true});
                self.fs.root.getDirectory(Pointer_stringify($0), {create: true});
                self.fs.root.getDirectory(Pointer_stringify($1), {create: true});
                self.fs.root.getDirectory(Pointer_stringify($3), {create: true});
                self.fs.root.getDirectory(Pointer_stringify($4), {create: true});
        },
        (_taskProps.mountPoint + getMessageDirectoryPath(data)).c_str(), (_taskProps.mountPoint + getMessageInputDirectoryPath(data)).c_str(), _taskProps.mountPoint.c_str(),
        (_taskProps.mountPoint + getMessageOutputDirectoryPath(data)).c_str(), (_taskProps.mountPoint + getMessageChunksDirectoryPath(data)).c_str());

    }
    else {
        EM_ASM_ARGS(
                {
                    FS.mkdir(Pointer_stringify($2));
                    FS.mount(MEMFS, {}, Pointer_stringify($2));
                    FS.mkdir(Pointer_stringify($0));
                    FS.mkdir(Pointer_stringify($1));
                    FS.mkdir(Pointer_stringify($3));
                    FS.mkdir(Pointer_stringify($4));
                },
                (_taskProps.mountPoint + getMessageDirectoryPath(data)).c_str(), (_taskProps.mountPoint + getMessageInputDirectoryPath(data)).c_str(), _taskProps.mountPoint.c_str(),
                (_taskProps.mountPoint + getMessageOutputDirectoryPath(data)).c_str(), (_taskProps.mountPoint + getMessageChunksDirectoryPath(data)).c_str()
        );
    }
    if(currentBrowser == CHROME ){
        unsigned  long chunksize = DOWNLOADCHUNKSIZE;
        EM_ASM_ARGS(
                {
                    var chunkSize = $3;
                    var lastpart = $2 % chunkSize;
                    var i;
                    var fileEntry = fs.root.getFile(Pointer_stringify($1), {create: true});
                    var writer = fileEntry.createWriter();
                    progress = Module.cwrap('updateDownloadProgress', 'void', ['number']);
                    for(i = 0; i<Math.floor($2 / chunkSize); i++){
                        var xhr = new XMLHttpRequest();
                        xhr.open('GET', Pointer_stringify($0), false);
                        xhr.setRequestHeader('Range', 'bytes='+i*chunkSize+'-'+((i+1)*chunkSize-1));
                        xhr.responseType = 'blob';
                        xhr.send(null);
                        writer.write(xhr.response);
                        //FS.write(stream, new Uint8Array(xhr.response), 0, xhr.response.byteLength );

                        progress(((i+1)*chunkSize-1)/$2*100);
                    }

                    var xhr = new XMLHttpRequest();
                    xhr.open('GET', Pointer_stringify($0), false);
                    xhr.setRequestHeader('Range', 'bytes='+i*chunkSize+'-'+(i*chunkSize+lastpart));
                    xhr.responseType = 'blob';
                    xhr.send(null);

                    writer.write(xhr.response);
                    //FS.write(stream, new Uint8Array(xhr.response), 0, xhr.response.byteLength );
                    //FS.close(stream);
                    progress(100);
                },getMessageValue("url", data).c_str(),(_taskProps.mountPoint+ getMessageInputFullPath(_taskProps.startMessage)).c_str(),(unsigned long)_taskProps.inputSize, chunksize);

        _taskProps.input = iz_fopen((_taskProps.mountPoint+ getMessageInputFullPath(_taskProps.startMessage)).c_str(),"rb+");
        if(!_taskProps.input.isValid)
        {
            iz_error("Couldn't create input file...");
            return;
        }
        postMessage(statusMsg(_taskProps.id,CONVERTING));
        _taskProps.convertStart = clock();
        iz_updatePreProgress((uint8_t)0);
        (*_taskProps.init_onDone_func)(_taskProps.id,_taskProps.input,_taskProps.inputSize,_taskProps.mountPoint+ getMessageOutputDirectoryPath(_taskProps.startMessage)+"/");
    }
    else if(currentBrowser == FIREFOX){

        unsigned  long chunksize =  DOWNLOADCHUNKSIZEFIREFOX;
        EM_ASM_ARGS(
                {
                    var chunkSize = $3;
                    var lastpart = $2 % chunkSize;
                    var i;
                    var stream = FS.open(Pointer_stringify($1), 'w');
                    progress = Module.cwrap('updateDownloadProgress', 'void', ['number']);
                    for(i = 0; i<Math.floor($2 / chunkSize); i++){
                        var xhr = new XMLHttpRequest();
                        xhr.open('GET', Pointer_stringify($0), false);
                        xhr.setRequestHeader('Range', 'bytes='+i*chunkSize+'-'+((i+1)*chunkSize-1));
                        xhr.responseType = 'arraybuffer';
                        xhr.send(null);
                        FS.write(stream, new Uint8Array(xhr.response), 0, xhr.response.byteLength );

                        progress(((i+1)*chunkSize-1)/$2*100);
                    }

                    var xhr = new XMLHttpRequest();
                    xhr.open('GET', Pointer_stringify($0), false);
                    xhr.setRequestHeader('Range', 'bytes='+i*chunkSize+'-'+(i*chunkSize+lastpart));
                    xhr.responseType = 'arraybuffer';
                    xhr.send(null);

                    FS.write(stream, new Uint8Array(xhr.response), 0, xhr.response.byteLength );
                    FS.close(stream);
                    progress(100);
                },getMessageValue("url", data).c_str(),(_taskProps.mountPoint+ getMessageInputDirectoryPath(_taskProps.startMessage)+std::to_string(_taskProps.inputIndex)).c_str(),(unsigned long)_taskProps.inputSize, chunksize);


/*        _taskProps.input = fopen((_taskProps.mountPoint+ getMessageInputDirectoryPath(_taskProps.startMessage)+std::to_string(_taskProps.inputIndex)).c_str(),"rb+");
        if(_taskProps.input==NULL)
        {
            iz_error("Couldn't create input file...");
            return;
        }*/
        postMessage(statusMsg(_taskProps.id,CONVERTING));
        _taskProps.convertStart = clock();
        iz_updatePreProgress((uint8_t)0);
        (*_taskProps.init_onDone_func)(_taskProps.id,_taskProps.input,_taskProps.inputSize,_taskProps.mountPoint+ getMessageOutputDirectoryPath(_taskProps.startMessage)+"/");
    }
    else{
        emscripten_async_wget2(getMessageValue("url", data).c_str(), (_taskProps.mountPoint+ getMessageInputDirectoryPath(_taskProps.startMessage)+std::to_string(_taskProps.inputIndex)).c_str(),"GET", "", (void*) getMessageShortId(data),
                onDownloadInputFinished, onDownloadInputError, onDownloadInputProgress);
    }



}

void iz_updateProgress(uint8_t percentage){
    if(_taskProps.currentProgress==percentage)
        return;
    _taskProps.currentProgress=percentage;
    postMessage(progressMsg(_taskProps.id, percentage));
}

void iz_updatePreProgress(uint8_t percentage){
    if(_taskProps.currentPreProgress==percentage)
        return;
    _taskProps.currentPreProgress=percentage;
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
/*

FILE* iz_truncateInput(uint64_t spanLength, bool fromStart){
    if(fromStart){
        std::string previousInputPath = (_taskProps.mountPoint+ getMessageInputDirectoryPath(_taskProps.startMessage)+std::to_string(_taskProps.inputIndex)).c_str();
        uint64_t nbBytesToCopy = _taskProps.inputSize - spanLength;
        long nbBytesCopied = 0;
        _taskProps.inputIndex++;
        char buffer[TRUNCBUFSIZE];

        FILE* previousInput = _taskProps.input;
        FILE* newInput = fopen((_taskProps.mountPoint+ getMessageInputDirectoryPath(_taskProps.startMessage)+std::to_string(_taskProps.inputIndex)).c_str(), "wb+");
        int n =0;
        int chunkIndex = 0;
        std::stack<std::string> chunksPath;
        off_t currentInputLength = (off_t) _taskProps.inputSize;
        while(nbBytesToCopy>nbBytesCopied) {
            //creating the file chunks from the previous input
            if(nbBytesToCopy-nbBytesCopied> TRUNCBUFSIZE)
            {
                fseek(previousInput,currentInputLength- TRUNCBUFSIZE, 0);
                n = fread(buffer, 1, TRUNCBUFSIZE, previousInput);
                nbBytesCopied+=n;
                std::string chunkPath = _taskProps.mountPoint+getMessageChunksDirectoryPath(_taskProps.startMessage)+"/"+std::to_string(chunkIndex);
                FILE* chunkFile = fopen(chunkPath.c_str(), "wb");
                //uint64_t wb =
                fwrite(buffer, 1, TRUNCBUFSIZE, chunkFile);
                chunksPath.push(chunkPath);
                chunkIndex++;
                fclose(chunkFile);
                //iz_print("read:"+std::to_string(n)+"  write chunk:"+std::to_string(wb)+"  INDEX:"+std::to_string(ftell(previousInput)));
                currentInputLength-= TRUNCBUFSIZE;
                truncate(previousInputPath.c_str(), currentInputLength);
            }

            //When reaching the last part
            else{
                //Directly write the last part (so it ll be the beginning of the file)
                long lastSize = (long)(nbBytesToCopy-nbBytesCopied);
                fseek(previousInput, currentInputLength-lastSize, 0);
                nbBytesCopied += fread(buffer, 1, (size_t)lastSize, previousInput);
                fwrite(buffer,1, (size_t)lastSize, newInput);

                //Close and delete the previous input and replace info by the new
                fclose(_taskProps.input);
                _taskProps.input = newInput;
                remove(previousInputPath.c_str());
                _taskProps.inputSize = (uint64_t)nbBytesCopied;

                //Assembling the chunks into the new input
                while (chunksPath.size()>0){
                    FILE* chnk = fopen (chunksPath.top().c_str(),"rb");
                    fread(buffer, 1, TRUNCBUFSIZE, chnk);
                    fwrite(buffer, 1, TRUNCBUFSIZE, newInput);
                    fclose(chnk);
                    remove(chunksPath.top().c_str());
                    chunksPath.pop();
                }
            }
        }

        rewind(_taskProps.input);

        return _taskProps.input;

    }
    return NULL;
}*/
