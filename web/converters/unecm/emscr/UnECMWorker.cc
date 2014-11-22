#include <Iz_EmLib.h>
#include <libc/bits/alltypes.h>
#include <libcxx/string>


/*
"UNECM - Decoder for Error Code Modeler format v1.0\n"
    "Copyright (C) 2002 Neill Corlett\n\n"
 */

/* Data types */
typedef uint8_t ecc_uint8;
typedef uint16_t ecc_uint16;
typedef uint32_t ecc_uint32;

/* LUTs used for computing ECC/EDC */
static ecc_uint8 ecc_f_lut[256];
static ecc_uint8 ecc_b_lut[256];
static ecc_uint32 edc_lut[256];

/* Init routine */
static void eccedc_init(void) {
    ecc_uint32 i, j, edc;
    for(i = 0; i < 256; i++) {
        j = (i << 1) ^ (i & 0x80 ? 0x11D : 0);
        ecc_f_lut[i] = j;
        ecc_b_lut[i ^ j] = i;
        edc = i;
        for(j = 0; j < 8; j++) edc = (edc >> 1) ^ (edc & 1 ? 0xD8018001 : 0);
        edc_lut[i] = edc;
    }
}

ecc_uint32 edc_partial_computeblock(
        ecc_uint32  edc,
        const ecc_uint8  *src,
        ecc_uint16  size
) {
    while(size--) edc = (edc >> 8) ^ edc_lut[(edc ^ (*src++)) & 0xFF];

    return edc;
}

void edc_computeblock(
        const ecc_uint8  *src,
        ecc_uint16  size,
        ecc_uint8  *dest
) {
    ecc_uint32 edc = edc_partial_computeblock(0, src, size);
    dest[0] = (edc >>  0) & 0xFF;
    dest[1] = (edc >>  8) & 0xFF;
    dest[2] = (edc >> 16) & 0xFF;
    dest[3] = (edc >> 24) & 0xFF;
}

/***************************************************************************/
/*
** Compute ECC for a block (can do either P or Q)
*/
static void ecc_computeblock(
        ecc_uint8 *src,
        ecc_uint32 major_count,
        ecc_uint32 minor_count,
        ecc_uint32 major_mult,
        ecc_uint32 minor_inc,
        ecc_uint8 *dest
) {
    ecc_uint32 size = major_count * minor_count;
    ecc_uint32 major, minor;
    for(major = 0; major < major_count; major++) {
        ecc_uint32 index = (major >> 1) * major_mult + (major & 1);
        ecc_uint8 ecc_a = 0;
        ecc_uint8 ecc_b = 0;
        for(minor = 0; minor < minor_count; minor++) {
            ecc_uint8 temp = src[index];
            index += minor_inc;
            if(index >= size) index -= size;
            ecc_a ^= temp;
            ecc_b ^= temp;
            ecc_a = ecc_f_lut[ecc_a];
        }
        ecc_a = ecc_b_lut[ecc_f_lut[ecc_a] ^ ecc_b];
        dest[major              ] = ecc_a;
        dest[major + major_count] = ecc_a ^ ecc_b;
    }
}

/*
** Generate ECC P and Q codes for a block
*/
static void ecc_generate(
        ecc_uint8 *sector,
        int        zeroaddress
) {
    ecc_uint8 address[4], i;
    /* Save the address and zero it out */
    if(zeroaddress) for(i = 0; i < 4; i++) {
            address[i] = sector[12 + i];
            sector[12 + i] = 0;
        }
    /* Compute ECC P code */
    ecc_computeblock(sector + 0xC, 86, 24,  2, 86, sector + 0x81C);
    /* Compute ECC Q code */
    ecc_computeblock(sector + 0xC, 52, 43, 86, 88, sector + 0x8C8);
    /* Restore the address */
    if(zeroaddress) for(i = 0; i < 4; i++) sector[12 + i] = address[i];
}

/***************************************************************************/
/*
** Generate ECC/EDC information for a sector (must be 2352 = 0x930 bytes)
** Returns 0 on success
*/
void eccedc_generate(ecc_uint8 *sector, int type) {
    ecc_uint32 i;
    switch(type) {
        case 1: /* Mode 1 */
            /* Compute EDC */
            edc_computeblock(sector + 0x00, 0x810, sector + 0x810);
            /* Write out zero bytes */
            for(i = 0; i < 8; i++) sector[0x814 + i] = 0;
            /* Generate ECC P/Q codes */
            ecc_generate(sector, 0);
            break;
        case 2: /* Mode 2 form 1 */
            /* Compute EDC */
            edc_computeblock(sector + 0x10, 0x808, sector + 0x818);
            /* Generate ECC P/Q codes */
            ecc_generate(sector, 1);
            break;
        case 3: /* Mode 2 form 2 */
            /* Compute EDC */
            edc_computeblock(sector + 0x10, 0x91C, sector + 0x92C);
            break;
    }
}

/***************************************************************************/

unsigned long mycounter;
unsigned long mycounter_total;

void resetcounter(unsigned long total) {
    mycounter = 0;
    mycounter_total = total;
}


void setcounter(unsigned long n) {
    if((n >> 20) != (mycounter >> 20)) {
        unsigned long a = (n+64)/128;
        unsigned long d = (mycounter_total+64)/128;
        if(!d) d = 1;
        iz_updateProgress((uint8_t)((100*a) / d));
    }
    mycounter = n;
}

int unecmify(
        IZ_FILE *in,
        IZ_FILE *out
) {
    uint32_t checkedc = 0;
    unsigned char sector[2352];
    unsigned type;
    unsigned num;
    iz_fseek(in, 0, SEEK_END);
    resetcounter(iz_ftell(in));
    iz_fseek(in, 0, SEEK_SET);
    if(
            (iz_fgetc(in) != 'E') ||
                    (iz_fgetc(in) != 'C') ||
                    (iz_fgetc(in) != 'M') ||
                    (iz_fgetc(in) != 0x00)
            ) {
        fprintf(stderr, "Header not found!\n");
        goto corrupt;
    }
    for(;;) {
        int c = iz_fgetc(in);
        int bits = 5;
        if(c == EOF) goto uneof;
        type = c & 3;
        num = (c >> 2) & 0x1F;
        while(c & 0x80) {
            c = iz_fgetc(in);
            if(c == EOF) goto uneof;
            num |= ((unsigned)(c & 0x7F)) << bits;
            bits += 7;
        }
        if(num == 0xFFFFFFFF) break;
        num++;
        if(num >= 0x80000000) goto corrupt;
        if(!type) {
            while(num) {
                int b = num;
                if(b > 2352) b = 2352;
                if(iz_fread(sector, 1, b, in) != b) goto uneof;
                checkedc = edc_partial_computeblock(checkedc, sector, b);
                iz_fwrite(sector, 1, b, out);
                num -= b;
                setcounter(iz_ftell(in));
            }
        } else {
            while(num--) {
                memset(sector, 0, sizeof(sector));
                memset(sector + 1, 0xFF, 10);
                switch(type) {
                    case 1:
                        sector[0x0F] = 0x01;
                        if(iz_fread(sector + 0x00C, 1, 0x003, in) != 0x003) goto uneof;
                        if(iz_fread(sector + 0x010, 1, 0x800, in) != 0x800) goto uneof;
                        eccedc_generate(sector, 1);
                        checkedc = edc_partial_computeblock(checkedc, sector, 2352);
                        iz_fwrite(sector, 2352, 1, out);
                        setcounter(iz_ftell(in));
                        break;
                    case 2:
                        sector[0x0F] = 0x02;
                        if(iz_fread(sector + 0x014, 1, 0x804, in) != 0x804) goto uneof;
                        sector[0x10] = sector[0x14];
                        sector[0x11] = sector[0x15];
                        sector[0x12] = sector[0x16];
                        sector[0x13] = sector[0x17];
                        eccedc_generate(sector, 2);
                        checkedc = edc_partial_computeblock(checkedc, sector + 0x10, 2336);
                        iz_fwrite(sector + 0x10, 2336, 1, out);
                        setcounter(iz_ftell(in));
                        break;
                    case 3:
                        sector[0x0F] = 0x02;
                        if(iz_fread(sector + 0x014, 1, 0x918, in) != 0x918) goto uneof;
                        sector[0x10] = sector[0x14];
                        sector[0x11] = sector[0x15];
                        sector[0x12] = sector[0x16];
                        sector[0x13] = sector[0x17];
                        eccedc_generate(sector, 3);
                        checkedc = edc_partial_computeblock(checkedc, sector + 0x10, 2336);
                        iz_fwrite(sector + 0x10, 2336, 1, out);
                        setcounter(iz_ftell(in));
                        break;
                }
            }
        }
    }
    if(iz_fread(sector, 1, 4, in) != 4) goto uneof;
    printf( "Decoded %ld bytes -> %ld bytes", iz_ftell(in), iz_ftell(out));
    if(
            (sector[0] != ((checkedc >>  0) & 0xFF)) ||
                    (sector[1] != ((checkedc >>  8) & 0xFF)) ||
                    (sector[2] != ((checkedc >> 16) & 0xFF)) ||
                    (sector[3] != ((checkedc >> 24) & 0xFF))
            ) {
        fprintf(stderr, "EDC error (%08X, should be %02X%02X%02X%02X)\n",
                checkedc,
                sector[3],
                sector[2],
                sector[1],
                sector[0]
        );
        goto corrupt;
    }
    return 0;
    uneof:
    iz_error("Unexpected end of file!");
    corrupt:
    iz_error("Corrupt ECM file!");
    return -1;
}




/*void Convert(std::string id,IZ_FILE* input, uint64_t inputSize,std::string ouputDirectoryPath ){
    IZ_FILE* output;
    std::string outputFullPath = ouputDirectoryPath+input->basename;
    if((output=iz_fopen(outputFullPath.c_str(), "wb")) == NULL) {
        iz_error("Cannot create the output file.");
        return;
    }
    fopen("lol","wb+");
    eccedc_init();
    if(unecmify(input, output)==0)
        iz_release(output);
    iz_print_fs_count_log();

}*/

int main(int argc, char **argv) {
    IZ_FILE *fin, *fout;
    char *infilename;
    char *outfilename;
    /*
    ** Initialize the ECC/EDC tables
    */
    eccedc_init();
    /*
    ** Check command line
    */
    if((argc != 2) && (argc != 3)) {
        iz_error("usage:"+std::string(argv[0])+" ecmfile [outputfile]");
        return 1;
    }
    /*
    ** Verify that the input filename is valid
    */
    infilename = argv[1];
    if(strlen(infilename) < 5) {
        iz_error("filename '"+std::string(infilename)+"' is too short");
        return 1;
    }
    if(strcasecmp(infilename + strlen(infilename) - 4, ".ecm")) {
        iz_error("filename must end in .ecm");
        return 1;
    }
    /*
    ** Figure out what the output filename should be
    */
    if(argc == 3) {
        outfilename = argv[2];
    } else {
        outfilename = (char*)malloc(strlen(infilename) - 3);
        if(!outfilename) abort();
        memcpy(outfilename, infilename, strlen(infilename) - 4);
        outfilename[strlen(infilename) - 4] = 0;
    }
    iz_console("Decoding "+std::string(infilename)+" to "+std::string(outfilename)+" .");
    /*
    ** Open both files
    */
    fin = iz_fopen(infilename, "rb");
    if(!fin) {
        iz_error("Couldn't open input file:"+std::string(infilename));
        return 1;
    }
    fout = iz_fopen(outfilename, "wb");
    if(!fout) {
        iz_error("Couldn't open output file:"+std::string(outfilename));
        iz_fclose(fin);
        return 1;
    }
    /*
    ** Decode
    */
    //setvbuf ( fout, NULL , _IOFBF , 32541536 );
    unecmify(fin, fout);
    /*
    ** Close everything
    */
    iz_release(fout);
    iz_fclose(fout);
    iz_fclose(fin);
    return 0;
}



uint64_t estimateOutputSize(uint64_t inputSize){
    return 750*1024*1024;
}

std::string baseParameters(std::string taskId, std::string inputFullPath, uint64_t inputSize, std::string outputDirectoryPath, std::string baseName, std::string inputExtension){
    return "\\\""+inputFullPath+"\\\" \\\""+outputDirectoryPath+baseName+"\\\"";
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
