//
//  pzb.hpp
//  pzb
//
//  Created by tihmstar on 14.12.15.
//  Copyright © 2015 tihmstar. All rights reserved.
//

#ifndef pzb_hpp
#define pzb_hpp

#include <iostream>
#include <vector>
#include <iomanip>
#include <exception>
#include <libpartialzip-1.0/libpartialzip.h>
#include <string.h>

using namespace std;

static void putFilename(char *name, size_t len){
    size_t lpart = 0;
    while (len>lpart && name[(len-1) -(lpart++)] != '/');
    
    char *fname = (lpart != len) ? (name + 1 + len -lpart) : name;
    lpart = len - (fname - name);
    while (lpart--) putchar(*(fname++));
}

static void printline(int percent){
    cout << setw(3) << percent << "% [";
    for (int i=0; i<100; i++) putchar((percent >0) ? ((--percent > 0) ? '=' : '>') : ' ');
    cout << "]";
}

static void partialzip_callback(partialzip_t* info, partialzip_file_t* file, size_t progress){
    cout << "\x1b[A\033[J" ; //clear 2 lines
    printline((int)progress);
    cout <<endl;
}

class pzb {
    void log(string msg){
        cout << msg << endl;
    }
public:
    struct t_fileinfo{
        string name;
        uint32_t compressedSize;
        uint32_t size;
    };
private:
    std::string *_url;
    partialzip_t* pzf;
    vector<t_fileinfo*> *files;
    uint32_t _biggestFileSize;
    
public:
    
    pzb(std::string url){
        _url = new string(url);
        log("init pzb: "+url);
        pzf = partialzip_open(url.c_str());
        
#warning TODO throw exception on failed init
        if (!pzf) throw -1;
        
        
        files = nullptr;
        _biggestFileSize = 0;
        log("init done");
    }
    
    vector<t_fileinfo*> getFiles(){
        if (!files) {
            files = new vector<t_fileinfo*>[pzf->centralDirectoryDesc->CDDiskEntries];
            
            char* cur = pzf->centralDirectory;
            for(int i = 0; i < pzf->centralDirectoryDesc->CDEntries; i++)
            {
                partialzip_file_t* candidate = (partialzip_file_t*) cur;
                
                t_fileinfo *file = new t_fileinfo();
                char *name = (char*)malloc(candidate->lenFileName+1);
                memcpy(name, cur + sizeof(partialzip_file_t), candidate->lenFileName);
                name[candidate->lenFileName] = '\0';
                file->name = name;
                free(name);
                
                file->compressedSize = candidate->compressedSize;
                file->size = candidate->size;
                files->push_back(file);
                cur += sizeof(partialzip_file_t) + candidate->lenFileName + candidate->lenExtra + candidate->lenComment;
                
            }
        }
        return *files;
    }
    
    uint32_t biggestFileSize(){
        if (!_biggestFileSize) {
            if (!files) getFiles();
            for (t_fileinfo *f : *files){
                if (f->compressedSize > _biggestFileSize) _biggestFileSize = f->compressedSize;
            }
        }
        return _biggestFileSize;
    }
    
    bool downloadFile(string filename, string dst){
        partialzip_file_t *file = partialzip_find_file(pzf, filename.c_str());
        if (!file) return false;
        
        putFilename((char*)file + sizeof(partialzip_file_t), file->lenFileName);
        cout << ":"<<endl;
        partialzip_callback(nullptr, file, 0); //print 0% state
        int dwn = partialzip_download_file(pzf->url, filename.c_str(), dst.c_str(), partialzip_callback);
        
        if (dwn == 0) {
//            partialzip_callback(nullptr, file, 100); //print 100% state
            return true;
        }else{
            partialzip_callback(nullptr, file, 0); //print 0% state
            return false;
        }
    }
    

    
    ~pzb(){
        delete _url;
        if (files) {
            for (t_fileinfo *f : *files){
                delete f;
            }
            delete [] files;
        }
        partialzip_close(pzf);
    }
};

#endif /* pzb_hpp */
