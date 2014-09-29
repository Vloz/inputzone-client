#include "ppapi/cpp/instance.h"
#include "ppapi/cpp/module.h"
#include "IzModule.h"
#include "FileTask.h"

class CopyTask : FileTask{
public:
    CopyTask( IzInstanceBase *instance,const pp::Var& var_message) : FileTask(instance, var_message, "Copy"){
    };
private:

    virtual int32_t Convert(double taskId, FILE* input, uint64_t inputSize, std::string directoryPath, std::string baseName, std::string inputExtension) {
        FILE *output;
        char buf[1024*100];
        uint64_t output_offset = 0;

        std::string outputFullPath = directoryPath+baseName+"2"+inputExtension;

        if((output=fopen(outputFullPath.c_str(), "wb")) == NULL) {
            Error("Cannot create the output file.");
            return -1;
        }

        unsigned int n = 0;
        do {
            n = fread(buf, sizeof(char), sizeof(buf), input);
            fwrite(buf, sizeof(char), n, output);
            output_offset+=n;
            UpdateProgress((int8_t)((float)output_offset/(float)inputSize*100));
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
        return new IzInstance<CopyTask>(instance);
    }
};

namespace pp {
    Module* CreateModule() {
        return new CopyModule();
    }
}



