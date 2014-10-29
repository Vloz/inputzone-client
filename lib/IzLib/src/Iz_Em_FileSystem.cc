#include "Iz_EmLib.h"
#include "Iz_Em_FileSystem.h"

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
IZ_FILE iz_fopen(const char * filename, const char * mode ){
    IZ_FILE f;
    f._hfile=-1;
    f._mfile=NULL;
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
            f._filemode = FM_R;
            fe=ERROR_IF_NOT;
            break;
        case 0x7778: //wx
        case 0x776278: //wbx
            fe=ERROR_IF_YES;
        case 0x77: //w
        case 0x7762: //wb
            f._filemode = FM_W;
            break;
        case 0x61: //a
        case 0x6162: //ab
            f._filemode = FM_A;
            break;
        case 0x722b: //r+
        case 0x72622b: //rb+
        case 0x722b62: //r+b
            fe= ERROR_IF_NOT;
        case 0x772b: //w+
        case 0x77622b: //wb+
        case 0x772b62: //w+b
            f._filemode = FM_RW;
            break;
        case 0x772b78: //w+x
        case 0x77622b78: //wb+x
        case 0x772b6278: //w+bx
            fe=ERROR_IF_YES;
            f._filemode = FM_RW;
            break;
        case 0x612b: //a+
        case 0x61622b: //ab+
        case 0x612b62: //a+b
            f._filemode=FM_AR;
            break;
        default:
            iz_error(std::string("Wrong fopen mode:")+mode);
            f.isValid = false;
            return f;
    }
    iz_print("fm:"+std::to_string((int)f._filemode)+"  fe:"+std::to_string((int)fe));

    f.extension = getExtension(filename);
    f.basename = basename(filename);
    f.fullpath = filename;
    if(currentFsType == HTML5FS){
        f._hfile = EM_ASM_INT({
            var no = self.fs_FILES.length;
            try {
                if($1==0)
                    self.fs_FILES[no]= self.fs.root.getFile(Pointer_stringify($0), {create: true});
                else if($1==1)
                    self.fs_FILES[no]= self.fs.root.getFile(Pointer_stringify($0), {create: false});
                else
                    self.fs_FILES[no]= self.fs.root.getFile(Pointer_stringify($0), {create: true, exclusive: true});
                self.fs_cursors[no]=0;
                if($2==1 || $2==2 || $2==3 || $2==4)
                    self.fs_writers[no]= self.fs_FILES[no].createWriter();
            } catch (e) {
                Module.print(e);
                return -1;
            }
            return no;
        },filename, (int)fe,(int)f._filemode);

    }
    else{
        f._mfile = fopen(filename, mode);
    }
    if(f._hfile == -1 && f._mfile==NULL){
        iz_error("Error when trying to open the file:"+std::string(filename));
        f.isValid = false;
    }
    else
        f.isValid=true;


    return f;
}

size_t iz_fread( void * ptr, size_t size, size_t count, IZ_FILE stream ){

    if(currentFsType == HTML5FS){
    long readLength = (long)(size*count);
    int readBytes = EM_ASM_INT({
       var b = self.fs_filereader.readAsArrayBuffer(self.fs_FILES[$2].file().slice(self.fs_cursors[$2],self.fs_cursors[$2]+$1));
        self.fs_cursors[$2]+=b.byteLength;
        writeArrayToMemory(new Uint8Array(b), $0);
        return b.byteLength;
    },ptr,readLength,stream._hfile);
    //iz_print("Read bytes:"+std::string((char*)ptr));
    //iz_print("Read bytes:"+std::to_string(readBytes));

    return (size_t)readBytes;
    }
    else{
        return fread( ptr, size, count,  stream._mfile );
    }

}


size_t iz_fwrite( const void * ptr, size_t size, size_t count, IZ_FILE stream ){
    if(currentFsType == HTML5FS) {
        long writeLength = (long) (size * count);
        int wroteBytes = EM_ASM_INT({
            try {
                var array = Module.HEAP8.subarray($0, $0 + $1);
                var
                blob = new Blob([new Uint8Array(array)]);
                var writer = self.fs_writers[$2];
                if (writer.position != self.fs_cursors[$2])
                    writer.seek(self.fs_cursors[$2]);
                writer.write(blob);
            }
            catch (e) {
                Module.print('ERROR DURING WRITE:' + e);
                return 0;
            }
            var wrote = writer.position - self.fs_cursors[$2];
            self.fs_cursors[$2] = writer.position;
            return wrote;

        }, ptr, writeLength, stream._hfile);

        //iz_print("Wrote bytes:" + std::to_string(wroteBytes));
        return (size_t) wroteBytes;
    }
    else{
        return fwrite( ptr, size, count, stream._mfile );
    }
}

int iz_remove ( const char * filename ){
    if(currentFsType == HTML5FS) {
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

int iz_fclose( IZ_FILE stream ){
    if(currentFsType == HTML5FS) {
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
        }, stream._hfile);
    }
    else{
        return fclose(stream._mfile);
    }
}

