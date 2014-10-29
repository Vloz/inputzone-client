#include <Iz_EmLib.h>
#include <libcxx/iosfwd>
#include <libc/bits/alltypes.h>
#include <libcxx/string>


void Convert(std::string id,IZ_FILE input, uint64_t inputSize,std::string ouputDirectoryPath ){

    IZ_FILE output;
    char buf[1024*100];
    uint64_t output_offset = 0;

    std::string outputFullPath = ouputDirectoryPath+input.basename+"2"+input.extension;

    iz_print(outputFullPath);
    if(!(output=iz_fopen(outputFullPath.c_str(), "wb")).isValid) {
        iz_error("Cannot create the output file.");
        return;
    }


    uint64_t lastTruncate = 0;
    unsigned int n = 0;
    do {
        n = iz_fread(buf, sizeof(char), sizeof(buf), input);
       iz_fwrite(buf, sizeof(char), n, output);
        output_offset+=n;
        lastTruncate+=n;
        iz_updateProgress((uint8_t)((float)output_offset/(float)inputSize*100));
    }while(n!=0);

    iz_print("fileoutputSize="+std::to_string(output_offset));
    iz_release(output);

}


extern "C"{
void initWorker(char *data, int size){
    iz_init(data, size,"Copy", Convert,emscripten_worker_respond_provisionally,emscripten_worker_respond );
}

}
