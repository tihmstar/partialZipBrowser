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

class pzb {
public:
    struct t_fileinfo{
        string name;
        uint32_t compressedSize;
        uint32_t size;
    };
private:
    void log(string msg);
    std::string *_url;
    partialzip_t* pzf;
    vector<t_fileinfo*> *files;
    uint32_t _biggestFileSize;
    
public:
    pzb(std::string url);
    vector<t_fileinfo*> getFiles();
    uint32_t biggestFileSize();
    bool downloadFile(string filename, string dst);
    ~pzb();
};

#endif /* pzb_hpp */
