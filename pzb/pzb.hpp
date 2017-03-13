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
#include <string.h>
#include <libfragmentzip/libfragmentzip.h>

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
    fragmentzip_t* pzf;
    vector<t_fileinfo*> *files;
    uint32_t _biggestFileSize;
    
public:
    pzb(const std::string& url);
    pzb(const std::string& url, bool disable_ssl_validation);
    pzb(const std::string& url, bool disable_ssl_validation, const std::string& username);

    
    vector<t_fileinfo*> getFiles();
    uint32_t biggestFileSize();
    bool downloadFile(string filename, string dst);
    ~pzb();
};

#endif /* pzb_hpp */
