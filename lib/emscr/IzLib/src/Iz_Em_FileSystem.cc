
#include "Iz_Em_FileSystem.h"

#include <Iz_EmLib.h>

#ifdef DEBUG
clock_t fs_start;
uint64_t ftell_count=0; clock_t ftell_avg_time=0;
uint64_t fseek_count=0; clock_t fseek_avg_time=0;
uint64_t fgetc_count=0; clock_t fgetc_avg_time=0;
uint64_t fread_count=0; clock_t fread_avg_time=0;
uint64_t fwrite_count=0; clock_t fwrite_avg_time=0;
#endif


struct MatchPathSeparator
{
    bool operator()( char ch ) const
    {
        return ch == '/';
    }
};

std::string basename( std::string const& pathname )
{
    std::string filename = std::string(
            std::find_if( pathname.rbegin(), pathname.rend(),
                    MatchPathSeparator() ).base(),
            pathname.end() );

    if(filename.find_last_of(".") != std::string::npos)
        return filename.substr(0,filename.find_last_of("."));
    return filename;
}

std::string getExtension(std::string filename) {
    if(filename.find_last_of(".") != std::string::npos)
        return filename.substr(filename.find_last_of("."));
    return "";
}

///TODO: Support for binary/txt file distinction
IZ_FILE* iz_fopen(const char * filename, const char * mode ){
    IZ_FILE* f = new IZ_FILE(filename);
    FILEEXISTS fe = CREATE_IF_NOT;

    int Mode_ASCII = 0;
    std::string m(mode);
    for(int i = m.length();i>0;i--){
        int i2 = m.length()-i;
        int v = *(mode+i2);
        for(int i3 = i-1; i3>0;i3--)
            v*=256;
        Mode_ASCII+=v;
    }
    switch (Mode_ASCII){
        case 0x72: //r
        case 0x7262: //rb
            f->_html5filemode = FM_R;
            fe=ERROR_IF_NOT;
            break;
        case 0x7778: //wx
        case 0x776278: //wbx
            fe=ERROR_IF_YES;
        case 0x77: //w
        case 0x7762: //wb
            f->_html5filemode = FM_W;
            break;
        case 0x61: //a
        case 0x6162: //ab
            f->_html5filemode = FM_A;
            break;
        case 0x722b: //r+
        case 0x72622b: //rb+
        case 0x722b62: //r+b
            fe= ERROR_IF_NOT;
        case 0x772b: //w+
        case 0x77622b: //wb+
        case 0x772b62: //w+b
            f->_html5filemode = FM_RW;
            break;
        case 0x772b78: //w+x
        case 0x77622b78: //wb+x
        case 0x772b6278: //w+bx
            fe=ERROR_IF_YES;
            f->_html5filemode = FM_RW;
            break;
        case 0x612b: //a+
        case 0x61622b: //ab+
        case 0x612b62: //a+b
            f->_html5filemode=FM_AR;
            break;
        default:
            iz_error(std::string("Wrong fopen mode:")+mode);
            return NULL;
    }
    std::string fp(filename); ///TODO: Dummy line... crash if removed... try to remove it in the future...
    f->extension = getExtension(filename);
    f->basename = basename(filename);
    if(currentFsType != MEMFS){
        f->_setHtml5FilePointer( EM_ASM_INT({
            var no = self.fs_FILES.length;
            try {
                if($1==0)
                    self.fs_FILES[no]= self.fs.root.getFile(Pointer_stringify($0), {create: true});
                else if($1==1)
                    self.fs_FILES[no]= self.fs.root.getFile(Pointer_stringify($0), {create: false});
                else
                    self.fs_FILES[no]= self.fs.root.getFile(Pointer_stringify($0), {create: true, exclusive: true});
                //self.fs_cursors[no]=0;
                //if($2==1 || $2==2 || $2==3 || $2==4)<== need filewriter for fseek on readonly files
                    self.fs_writers[no]= self.fs_FILES[no].createWriter();
            } catch (e) {
                Module.print(e);
                return -1;
            }
            return no;
        },filename, (int)fe,(int)f->_html5filemode));
    }
    else{
        FILE* mfile = fopen(filename, mode);
        if(mfile==NULL)
            return NULL;
        f->_setMEMFSFilePointer(mfile);
    }
    return f;
}

size_t iz_fread( void * ptr, size_t size, size_t count, IZ_FILE* stream ){
#ifdef DEBUG
    fread_count++;
    fs_start = clock();
#endif
    size_t readBytes=0;
    if(currentFsType != MEMFS){
        long readLength = (long)(size*count);
        long start = stream->_getCursor();
        long end = start + readLength;
         IZ_FILE_CHUNK* chunk = stream->_getReadChunk(start,end);
        long chunkRealEnd = chunk->startCursorIndex + chunk->dataSize;
        if(end>chunkRealEnd) {
            readBytes = chunkRealEnd - start;
        }
        else
            readBytes=end-start;
        memcpy (ptr, chunk->buffer+start - chunk->startCursorIndex,readBytes);

        stream->_setCursor((long)readBytes+stream->_getCursor());

    }
    else
        readBytes = fread( ptr, size, count,  (FILE*)stream->_getFilePointer());
#ifdef DEBUG
    fread_avg_time = (clock_t)((fread_avg_time * (fread_count-1) + clock()-fs_start)/fread_count);
#endif
    return readBytes;
}


size_t iz_fwrite( const void * ptr, size_t size, size_t count, IZ_FILE* stream ){
#ifdef DEBUG
    fwrite_count++;
    fs_start = clock();
#endif
    size_t wroteBytes = 0;
    if(currentFsType != MEMFS) {
        long writeLength = (long) (size * count);
        long start = stream->_getCursor();
        wroteBytes = stream->_writeToChunk(start, start+writeLength, ptr);
        stream->_setCursor(stream->_getCursor()+(long)wroteBytes);
        stream->_revokeReadChunk(start, start+wroteBytes-1);
    }
    else
        wroteBytes = fwrite( ptr, size, count, (FILE*)stream->_getFilePointer() );
#ifdef DEBUG
    fwrite_avg_time = (clock_t)((fwrite_avg_time * (fwrite_count-1) + clock()-fs_start)/fwrite_count);
#endif
    return wroteBytes;
}

int iz_remove ( const char * filename ){
    if(currentFsType != MEMFS) {
        return EM_ASM_INT({
           try {
                var f = self.fs.root.getFile(Pointer_stringify($0), {create: false});
                f.remove();
            }
            catch (e) {
                Module.print('ERROR DURING DELETE FILE:' + e);
                return -1;
            }
            return 0;

        }, filename);
    }
    else{
        return remove(filename);
    }
}

int iz_fclose( IZ_FILE* stream ){
    if(currentFsType != MEMFS) {
        stream->_html5_flush();
        int hfile=*((int*)stream->_getFilePointer());
        delete stream;
        return EM_ASM_INT({
            try {
                self.fs_FILES[$0]=null;
                self.fs_writers[$0]=null;
            }
            catch (e) {
                Module.print('ERROR DURING CLOSE FILE:' + e);
                return -1;
            }
            return 0;
        }, hfile);
    }
    else{
        FILE* mfile = (FILE*)stream->_getFilePointer();
        free(stream);
        return fclose(mfile);
    }
}


int iz_fseek( IZ_FILE*  stream, long int offset, int origin ){
#ifdef DEBUG
    fseek_count++;
    fs_start = clock();
#endif
    int result = 0;
    if(currentFsType != MEMFS) {
        switch(origin){
            case SEEK_SET:
                origin=0;
                break;
            case SEEK_CUR:
                origin=stream->_getCursor();
                break;
            case SEEK_END:
                #ifdef DEBUG
                EM_ASM({
                    Module.print('SEEK END FLUSH');
                });
                #endif
                stream->_html5_flush();
                origin=EM_ASM_INT({
                    return self.fs_writers[$0].length;
                }, *((int*)stream->_getFilePointer()));
                break;
        }
        stream->_setCursor(origin+offset);
        result = 0;
    } else
        result = fseek((FILE*)stream->_getFilePointer(), offset, origin);
#ifdef DEBUG
    fseek_avg_time = (clock_t)((fseek_avg_time * (fseek_count-1) + clock()-fs_start)/fseek_count);
#endif
    return result;
}

int iz_fgetc ( IZ_FILE * stream ){
#ifdef DEBUG
    fgetc_count++;
    fs_start = clock();
#endif
    int result = EOF;
    if(currentFsType != MEMFS) {
        char buf[1];
        if(iz_fread(buf, 1, 1, stream)){
            result = (unsigned int)buf[0];
            if(result == EOF)
                result = 255;
        }
        else
            result = EOF;
    }
    else
        result = fgetc((FILE*)stream->_getFilePointer());
#ifdef DEBUG
    fgetc_avg_time = (clock_t)((fgetc_avg_time * (fgetc_count-1) + clock()-fs_start)/fgetc_count);
#endif
    return result;
}

long int iz_ftell ( IZ_FILE * stream ){
#ifdef DEBUG
    ftell_count++;
    fs_start = clock();
#endif
    long int result = -1;
    if(currentFsType != MEMFS)
        result = stream->_getCursor();
    else
        result = ftell((FILE*)stream->_getFilePointer());
#ifdef DEBUG
    ftell_avg_time = (clock_t)((ftell_avg_time * (ftell_count-1) + clock()-fs_start)/ftell_count);
#endif
    return result;
}

#ifdef DEBUG
void iz_print_fs_count_log(){
    iz_print("fread {calls counts:"+std::to_string(fread_count)+",average time:"+std::to_string((double)fread_avg_time/CLOCKS_PER_SEC)+"s}");
    iz_print("fwrite {calls counts:"+std::to_string(fwrite_count)+",average time:"+std::to_string((double)fwrite_avg_time/CLOCKS_PER_SEC)+"s}");
    iz_print("ftell {calls counts:"+std::to_string(ftell_count)+",average time:"+std::to_string((double)ftell_avg_time/CLOCKS_PER_SEC)+"s}");
    iz_print("fseek {calls counts:"+std::to_string(fseek_count)+",average time:"+std::to_string((double)fseek_avg_time/CLOCKS_PER_SEC)+"s}");
    iz_print("fgetc {calls counts:"+std::to_string(fgetc_count)+",average time:"+std::to_string((double)fgetc_avg_time/CLOCKS_PER_SEC)+"s}");
}
#endif
