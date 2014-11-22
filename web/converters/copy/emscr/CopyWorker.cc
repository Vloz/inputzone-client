#include <Iz_EmLib.h>
#include <libcxx/iosfwd>
#include <libc/bits/alltypes.h>
#include <libcxx/string>


int main(int argc,char* argv[]){

    IZ_FILE *input, *output;
    char buf[1024*100];
    uint64_t output_offset = 0;

    std::string inputFullPath = argv[1];
    if((input=iz_fopen(inputFullPath.c_str(), "rb"))==NULL) {
        iz_error("Cannot create the input file.");
        return -1;
    }

    std::string outputFullPath = argv[2];
    if((output=iz_fopen(outputFullPath.c_str(), "wb"))==NULL) {
        iz_error("Cannot create the output file.");
        return -1;
    }


    uint64_t lastTruncate = 0;
    unsigned int n = 0;
    do {
        n = iz_fread(buf, sizeof(char), sizeof(buf), input);
        iz_print("Read bytes in converter:"+std::to_string(n));
       iz_fwrite(buf, sizeof(char), n, output);
        output_offset+=n;
        lastTruncate+=n;
        //iz_updateProgress((uint8_t)((float)output_offset/(float)inputSize*100));
    }while(n!=0);

/*    int c=0;
    while ((c= iz_fgetc(input))!=EOF){
        iz_print(std::to_string(c));
    }*/

    //iz_print_fs_count_log();
    //iz_print("fileoutputSize="+std::to_string(output_offset));
    iz_release(output);
    return 0;

}

uint64_t estimateOutputSize(uint64_t inputSize){
    return inputSize;
}

std::string baseParameters(std::string taskId, std::string inputFullPath, uint64_t inputSize, std::string outputDirectoryPath, std::string baseName, std::string inputExtension){
    return "\\\""+inputFullPath+"\\\" \\\""+outputDirectoryPath+baseName+"2"+inputExtension+"\\\"";
}

extern "C"{
void initWorker(char *data, int size){
    iz_init(data, size, main,emscripten_worker_respond_provisionally,emscripten_worker_respond );
}

void prerunRequest(char *data, int size){
    std::string answer = createPrerunRequest(data, size);
    emscripten_worker_respond_provisionally((char*)answer.c_str(),answer.length());
}

}
