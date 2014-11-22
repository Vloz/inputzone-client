#include <libc/bits/alltypes.h>
#include <libc/stdlib.h>
#include <libcxx/string>
#include <libc/limits.h>
#include <emscripten/emscripten.h>

#pragma once

#define WRITECHUNKSIZE 32541536
#define READCHUNKSIZE 8388608//32541536
#define READCHUNKCOUNT 3


//#define DEBUG

enum FILEEXISTS{
    CREATE_IF_NOT=0,
    ERROR_IF_NOT=1,
    ERROR_IF_YES=2
};

enum FSTYPE{
    MEMFS=0,
    HTML5TEMP=1,
    HTML5PERS=2
};

enum FILEMODE{
    FM_R=0,
    FM_W=1,
    FM_RW=2,
    FM_A=3,
    FM_AR=4
};


class IZ_FILE_CHUNK{
public:
    char* buffer;
    unsigned long startCursorIndex;
    unsigned long endCursorIndex;
    unsigned long useRank;
    unsigned long dataSize;
    unsigned long bufferSize;
    IZ_FILE_CHUNK(unsigned long start, unsigned long end){ // start is inclusive, end is exclusive
        startCursorIndex = start;
        endCursorIndex = end;
        bufferSize=end-start;
        buffer =(char*) malloc(bufferSize);
        dataSize=0;
    }
    ~IZ_FILE_CHUNK(){
        free(buffer);
    }
};

class IZ_FILE{
private:
    FILE* _mfile;
    int* _hfile;
    long _cursor;


    size_t _writeToHtml5File(IZ_FILE_CHUNK* chunktowrite){
        int64_t wroteBytes = (size_t)EM_ASM_INT({
            try {
                var array = Module.HEAP8.subarray($0, $0 + $1);
                var blob = new Blob([new Uint8Array(array)]);
                var writer = self.fs_writers[$2];
                writer.seek($3);
                writer.write(blob);
            }
            catch (e) {
                Module.print('ERROR DURING FFLUSH FILE WRITE:' + e);
                return 0;
            }
            var wrote = writer.position - $3;
            if (writer.position != $4)
                writer.seek($4);
            return wrote;

        }, chunktowrite->buffer, chunktowrite->dataSize, *_hfile,chunktowrite->startCursorIndex,_cursor);
        return wroteBytes;
    }
public:

    std::string basename;
    char* fullpath;
    std::string extension;
    bool isHTML5File;

    FILEMODE _html5filemode;
    //IZ_FILE_CHUNK* _chunkCache;
    IZ_FILE_CHUNK* _readchunk[READCHUNKCOUNT];
    IZ_FILE_CHUNK* _writechunk;

    IZ_FILE(const char* fullpath){
        _writechunk = NULL;
        for(int i=0;i<READCHUNKCOUNT;i++)
            _readchunk[i] = NULL;
        _mfile = NULL;
        _hfile = NULL;
        _cursor = 0;
        this->fullpath = (char*) malloc(256);
        strncpy(this->fullpath, fullpath, 255);
    }

    ~IZ_FILE(){
        if(isHTML5File){
            free(_hfile);
            for(int i=0;i<READCHUNKCOUNT;i++)
                if(_readchunk[i]!=NULL)
                    delete _readchunk[i];
        }
        free(fullpath);
    }

    size_t _writeToChunk(unsigned long start, unsigned long end, const void* buffer){
        size_t length = end-start;
        if(length>WRITECHUNKSIZE){
            IZ_FILE_CHUNK bigChunk(start,end);
            memcpy(bigChunk.buffer, buffer,length);
            bigChunk.dataSize = length;
            _writeToHtml5File(&bigChunk);
            return length;
        }else{
            if(_writechunk==NULL){
                _writechunk = new IZ_FILE_CHUNK(start,start+ WRITECHUNKSIZE);
            }else{
                if(!(end<=_writechunk->endCursorIndex && start>=_writechunk->startCursorIndex)){ //if not in the write_chunk range
                    _html5_flush();
                    _writechunk = new IZ_FILE_CHUNK(start,start+ WRITECHUNKSIZE);
                } else if(start>_writechunk->startCursorIndex+_writechunk->dataSize){ //there is a gap in the write chunk to fill
                    size_t readBytes = (size_t)EM_ASM_INT({
                        var b = self.fs_filereader.readAsArrayBuffer(self.fs_FILES[$2].file().slice($1,$3));
                        writeArrayToMemory(new Uint8Array(b), $0);
                        Module.print('FILL GAP:' + b.byteLength);
                        return b.byteLength;
                    },(char*)(_writechunk->buffer+_writechunk->dataSize),_writechunk->startCursorIndex+_writechunk->dataSize,*_hfile,start);
                }
            }

            size_t chunk_offset = start- _writechunk->startCursorIndex;
            size_t input_length = end-start;
            memcpy(_writechunk->buffer+chunk_offset, buffer,input_length);
            if(_writechunk->startCursorIndex+_writechunk->dataSize < end)
                _writechunk->dataSize = end - _writechunk->startCursorIndex;
            return length;
        }
    }

    int _html5_flush(){
        if(_writechunk!=NULL){
            _writeToHtml5File(_writechunk);
            delete  _writechunk;
            _writechunk = NULL;
        }
        return 0;
    }

    IZ_FILE_CHUNK* _getReadChunk(unsigned long start, unsigned long end){
        if(_writechunk != NULL && !(end<=_writechunk->startCursorIndex || start>=_writechunk->startCursorIndex+_writechunk->dataSize) ) {
            if (start >= _writechunk->startCursorIndex && end <= _writechunk->startCursorIndex + _writechunk->dataSize )
                return _writechunk;
            else {
                _html5_flush();
            }
        }

        for(int i=0; i<READCHUNKCOUNT;i++)
            if(_readchunk[i]!=NULL && _readchunk[i]->startCursorIndex <= start && _readchunk[i]->endCursorIndex>=end)
                return _readchunk[i];
        int new_i = _getChunkToErase();
        _readchunk[new_i] = new IZ_FILE_CHUNK(start,start+READCHUNKSIZE);
        size_t readBytes = (size_t)EM_ASM_INT({
                var b = self.fs_filereader.readAsArrayBuffer(self.fs_FILES[$2].file().slice($1,$3));
                writeArrayToMemory(new Uint8Array(b), $0);
                return b.byteLength;
        },_readchunk[new_i]->buffer,start,*_hfile,start+READCHUNKSIZE);
        _readchunk[new_i]->dataSize=readBytes;
        if(readBytes< READCHUNKSIZE) //This is the end of the file, we set its end cursor to max
            _readchunk[new_i]->endCursorIndex=ULONG_MAX;
        return _readchunk[new_i];
    }

    void _revokeReadChunk(unsigned long start, unsigned long end){
        for(int i=0; i<READCHUNKCOUNT;i++){
            if(!(_readchunk[i]->startCursorIndex>end || _readchunk[i]->endCursorIndex<start)){
                delete _readchunk[i];
                _readchunk[i]=NULL;
            }
        }
    }

    short _getChunkToErase(){
        short index=0;
        unsigned long lowestUseRank = ULONG_MAX;
        for(short i=0; i<READCHUNKCOUNT;i++){
            if(_readchunk[i]==NULL)
                return i;
            else if(_readchunk[i]->useRank<=lowestUseRank){
                lowestUseRank = _readchunk[i]->useRank;
                index = i;
            }
            _readchunk[i]->useRank = 0;
        }
        delete _readchunk[index];
        _readchunk[index] = NULL;
        return index;
    }

    void* _getFilePointer(){
        if(!isHTML5File)
            return _mfile;
        else
            return _hfile;
    }

    void _setHtml5FilePointer(int id){
        isHTML5File=true;
        _hfile = (int*)malloc(sizeof(int));
        *_hfile = id;
    }

    void _setMEMFSFilePointer(FILE* f){
        isHTML5File=false;
        _mfile = f;
    }

    long _getCursor(){
        if(!isHTML5File)
            return ftell(_mfile);
        else
            return _cursor;
    }

    void _setCursor(long value){
        if(!isHTML5File)
            fseek(_mfile, value, 0);
        else
            _cursor = value;
    }


};


IZ_FILE* iz_fopen(const char * filename, const char * mode );
size_t iz_fread( void * ptr, size_t size, size_t count, IZ_FILE* stream );
int iz_fgetc ( IZ_FILE * stream );
size_t iz_fwrite( const void * ptr, size_t size, size_t count, IZ_FILE* stream );
int iz_remove ( const char * filename );
int iz_remove ( const char * filename );
int iz_fclose( IZ_FILE* stream );
int iz_fseek ( IZ_FILE*  stream, long int offset, int origin );
long int iz_ftell ( IZ_FILE * stream );


#ifdef DEBUG
void iz_print_fs_count_log();
#endif