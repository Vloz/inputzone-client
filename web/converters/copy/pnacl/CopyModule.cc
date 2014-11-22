#include "ppapi/cpp/instance.h"
#include "ppapi/cpp/module.h"
#include "IzInstance.h"
#include "FileConverter.h"

class CopyConverter : public FileConverter {
public:
    CopyConverter( IzInstanceBase *instance,const pp::Var& var_message) : FileConverter(instance, var_message){
    };

    static int64_t estimateOutputSize(int64_t inputSize){
        return inputSize;
    }

    static std::string baseParameters(uint64_t taskId, std::string inputFullPath, uint64_t inputSize, std::string outputDirectoryPath, std::string baseName, std::string inputExtension){
        return "\\\""+inputFullPath+"\\\" \\\""+outputDirectoryPath+baseName+"2"+inputExtension+"\\\"";
    }

private:

    virtual int32_t main(int argc, char* argv[]) {
        FILE* input;
        FILE *output;
        char buf[1024*100];
        uint64_t output_offset = 0;

        if(argc<3){
            Error("Missing argument. Must be ./copy [input path] [output path]");
            return -1;
        }

        if((input = fopen(argv[1], "rb")) == NULL) {
            Error("Cannot create the input file.");
            return -1;
        }
        if((output=fopen(argv[2], "wb")) == NULL) {
            Error("Cannot create the output file.");
            return -1;
        }
        setvbuf ( output, NULL , _IOFBF , 32541536 );

        unsigned int n = 0;
        do {
            n = fread(buf, sizeof(char), sizeof(buf), input);
            fwrite(buf, sizeof(char), n, output);
            output_offset+=n;
            UpdateProgress((int8_t)((float)output_offset/(float)size_*100));
        }while(n!=0);
        fclose(output);

        return PP_OK;

    };

};


class CopyModule : public pp::Module {
public:
    CopyModule() : pp::Module() {}
    virtual ~CopyModule() {}

    virtual pp::Instance* CreateInstance(PP_Instance instance) {
        return new IzInstance<CopyConverter>(instance);
    }
};

namespace pp {
    Module* CreateModule() {
        return new CopyModule();
    }
}



