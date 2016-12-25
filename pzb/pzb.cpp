//
//  pzb.cpp
//  pzb
//
//  Created by tihmstar on 14.12.15.
//  Copyright © 2015 tihmstar. All rights reserved.
//

#include <iostream>
#include <vector>
#include <iomanip>
#include <exception>
#include <string.h>
#include <pzb.hpp>

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

static void fragmentzip_callback(uint progress){
    cout << "\x1b[A\033[J" ; //clear 2 lines
    printline((int)progress);
    cout <<endl;
}

void pzb::log(string msg){
    cout << msg << endl;
}

pzb::pzb(std::string url){
    _url = new string(url);
    log("init pzb: "+url);
    pzf = fragmentzip_open(url.c_str());
    
#warning TODO throw exception on failed init
    if (!pzf) throw -1;
    
    
    files = nullptr;
    _biggestFileSize = 0;
    log("init done");
}

vector<pzb::t_fileinfo*> pzb::getFiles(){
    if (!files) {
        files = new vector<t_fileinfo*>[pzf->cd_end->cd_entries];
        
        fragmentzip_cd* candidate = pzf->cd;
        for(int i = 0; i < pzf->cd_end->cd_entries; i++)
        {
            
            pzb::t_fileinfo *file = new pzb::t_fileinfo();
            char *name = (char*)malloc(candidate->len_filename+1);
            memcpy(name, candidate->filename, candidate->len_filename);
            name[candidate->len_filename] = '\0';
            file->name = name;
            free(name);
            
            file->compressedSize = candidate->size_compressed;
            file->size = candidate->size_uncompressed;
            files->push_back(file);
            candidate = fragmentzip_nextCD(candidate);
        }
    }
    return *files;
}

uint32_t pzb::biggestFileSize(){
    if (!_biggestFileSize) {
        if (!files) getFiles();
        for (t_fileinfo *f : *files){
            if (f->compressedSize > _biggestFileSize) _biggestFileSize = f->compressedSize;
        }
    }
    return _biggestFileSize;
}

bool pzb::downloadFile(string filename, string dst){
    fragmentzip_cd *file = fragmentzip_getCDForPath(pzf, filename.c_str());
    if (!file) return false;
    
    putFilename(file->filename, file->len_filename);
    cout << ":"<<endl;
    fragmentzip_callback(0); //print 0% state
    int dwn = fragmentzip_download_file(pzf, filename.c_str(), dst.c_str(), fragmentzip_callback);
    
    if (dwn == 0) {
        //            partialzip_callback(nullptr, file, 100); //print 100% state
        return true;
    }else{
        fragmentzip_callback(0); //print 0% state
        return false;
    }
}



pzb::~pzb(){
    delete _url;
    if (files) {
        for (t_fileinfo *f : *files){
            delete f;
        }
        delete [] files;
    }
    fragmentzip_close(pzf);
}

