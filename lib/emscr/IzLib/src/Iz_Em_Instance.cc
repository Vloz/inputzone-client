
#include <Iz_Em_Instance.h>
#include <Iz_Message.h>




extern "C" {

int main(){
    return 0;
}

int initConverter(char* converterUrl){
    DebugMessage(converterUrl);
    converterWorker = emscripten_create_worker(converterUrl);
    return 0;
}


int ReceiveMessage(char* inputMessage) {
    std::string message = inputMessage;

    int id = getMessageShortId(inputMessage);

    switch(getMessageType(message)){
        case START :
            emscripten_call_worker(converterWorker, "initWorker", (char*)message.c_str(), message.length(), onWorkerMessage, (void*)(int)id);
            break;
        default:
            DebugErrorMessage("Incorrect Iz message type!", getMessageType(inputMessage));
            return -1;
    }

    return 0;
}

}


void onWorkerMessage(char *data, int size, void *arg) {
    PostMessage(std::string(data,(unsigned)size).c_str());
/*    DebugMessage("OUTSIDE!!"+std::to_string((int)arg));

    EM_ASM_ARGS({
            Module.print($0);
    },(int)arg);
    EM_ASM_ARGS({
        var bytes = new Uint8Array(Module.HEAPU8.buffer,$0,$1);
        var oMyBlob = new Blob([bytes.buffer]);
        var url = (window.webkitURL ? webkitURL : URL).createObjectURL(oMyBlob);
        Module.print('Blob url:'+url);
        Module.print('Blob url:'+url);
       // window.location.assign(url);
    },data, size);*/

}

void PostMessage(std::string message){
    EM_ASM_ARGS({
        SendMessage(Pointer_stringify($0));
    },message.c_str());
}


void DebugMessage(std::string message) {
    EM_ASM_ARGS({
        Module.print(Pointer_stringify($0));
    },message.c_str());
}

void DebugErrorMessage( std::string message, int32_t result) {
    DebugMessage(message+" : "+std::to_string(result));
}





/*int w1;


void sayLOL(){
    printf("lol");
}


void downloadFinished(char *data, int size, void *arg) {
    assert((int)arg == 93);
    int *x = (int*)data;
    sayLOL();
    printf("mdr");
    printf("c1: %d,%d\n", x[0], x[1]);
    sayLOL();
    //REPORT_RESULT();
}

int main() {
    w1 = emscripten_create_worker("/lib/inputdownloader.js");

    int x[2] = { 100, 6002 };
    emscripten_call_worker(w1, "one", (char*)x, sizeof(x), downloadFinished, (void*)93);


    return 0;
}*/
