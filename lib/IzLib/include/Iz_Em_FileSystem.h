#pragma once

#include <Iz_EmLib.h>

enum FILEEXISTS{
    CREATE_IF_NOT=0,
    ERROR_IF_NOT=1,
    ERROR_IF_YES=2
};

enum FSTYPE{
    MEMFS=0,
    HTML5FS=1
};

enum FILEMODE{
    FM_R=0,
    FM_W=1,
    FM_RW=2,
    FM_A=3,
    FM_AR=4
};


struct IZ_FILE{
    FILE* _mfile;
    int _hfile;
    std::string basename;
    std::string fullpath;
    std::string extension;
    bool isValid;
    FILEMODE _filemode;
};



IZ_FILE iz_fopen(const char * filename, const char * mode );
size_t iz_fread( void * ptr, size_t size, size_t count, IZ_FILE stream );
size_t iz_fwrite( const void * ptr, size_t size, size_t count, IZ_FILE stream );
int iz_remove ( const char * filename );
int iz_remove ( const char * filename );
int iz_fclose( IZ_FILE stream );