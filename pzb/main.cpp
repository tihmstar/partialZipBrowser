//
//  main.cpp
//  pzb
//
//  Created by tihmstar on 14.12.15.
//  Copyright © 2015 tihmstar. All rights reserved.
//

#include <iostream>
#include <iomanip>
#include "pzb.hpp"
#include <sys/stat.h>
#include <getopt.h>


vector<pzb::t_fileinfo*> find_quiet(vector<pzb::t_fileinfo*> files, string currentDir){
    vector<pzb::t_fileinfo*> ret;
    for (pzb::t_fileinfo *f: files){
        //filter root dirs if in subdir
        if (currentDir.length() && f->name.substr(0,currentDir.length()) != currentDir.c_str()) continue;
        ret.push_back(f);
    }
    return ret;
}

vector<pzb::t_fileinfo*> ls_quiet(vector<pzb::t_fileinfo*> files, string currentDir){
    vector<pzb::t_fileinfo*> ret;
    for (pzb::t_fileinfo *f: find_quiet(files, currentDir)){
        
        //filter files in subdirs, but keep subdirs
        if (count(f->name.begin(), f->name.end(), '/')-1 > count(currentDir.begin(), currentDir.end(), '/')){
            continue;
        }
        if (count(f->name.begin(), f->name.end(), '/') > count(currentDir.begin(), currentDir.end(), '/') && f->name.substr(f->name.length()-1) != "/"){
            continue;
        }
        
        ret.push_back(f);
    }
    return ret;
}

bool isDir(string path){
    return path.substr(path.length()-1) == "/";
}

bool existFile(vector<pzb::t_fileinfo*> files, string path){
    if (path.substr(path.length()-1) == "/") {
        return false; //is a dir
    }
    for (pzb::t_fileinfo *f : files){
        if (f->name == path) return true;
    }
    return false;
}

int ls(vector<pzb::t_fileinfo*> files, int slen, string currentDir){
    vector<pzb::t_fileinfo*> nf = ls_quiet(files, currentDir);
    
    if (nf.size()==0) {
        cout << "Error: directory " << currentDir << " not found"<<endl;
        return -2;
    }for (pzb::t_fileinfo *f: nf){
        
        cout << setw(slen) << f->compressedSize << ((isDir(f->name)) ? " d " : " f ") << f->name.substr(currentDir.length()) << endl;
    }
    return 0;
}

int find(vector<pzb::t_fileinfo*> files, int slen, string fdir){
    vector<pzb::t_fileinfo*> nf = find_quiet(files, fdir);
    
    if (nf.size()==0) {
        cout << "Error: directory " << fdir << " not found"<<endl;
        return -2;
    }
    
    for (pzb::t_fileinfo *f: nf){
        
        cout << setw(slen) << f->compressedSize << ((isDir(f->name)) ? " d " : " f ") << f->name.substr(fdir.length()) << endl;
    }
    return 0;
}

void cd(vector<pzb::t_fileinfo*> files, string &cdir, string ndir){
    if (ndir == "/") {
        cdir = "";
    }else if (ndir.substr(0,2) == ".."){
        if (cdir != "") {
            if (count(cdir.begin(), cdir.end(), '/') == 1) {
                cdir = "";
            }else{
                cdir = cdir.substr(0,cdir.substr(0,cdir.length()-1).find_last_of('/')+1);
            }
        }
        
        if (ndir.length()>2) {
            cd(files, cdir, ndir.substr(2));
        }
    }else{
        string ncdir = cdir;
        
        if (ndir.substr(ndir.length()-1) != "/") ndir.push_back('/');
        ncdir += ndir;
        
        vector<pzb::t_fileinfo*> dirs = find_quiet(files, ncdir);
        if (dirs.size() != 0){
            if (dirs[0]->name.substr(dirs[0]->name.length()-1) != "/") {
                cout << "Error: " << ncdir << " is not a directory" <<endl;
            }else{
                cdir = ncdir;
            }
        }else{
            cout << "Error: directory " << ncdir << " not found"<<endl;
        }
    }
}

void makeDirs(string dirs){
    //parse
    size_t pos = 0;
    size_t poss = 0;
    std::string token;
    while ((pos = dirs.substr(poss).find("/")) != std::string::npos) {
        token = dirs.substr(0, poss + pos);
        mkdir(token.c_str(),S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH);
        cout <<token<<endl;
        poss += pos+1;
    }
}

int cmd_get(pzb &obj, string file, string dst, bool mkdirs){
    
    if (!existFile(obj.getFiles(), file)){
        cout << "Error: file " << file << " does not exist, or is a directory"<<endl;
        return -2;
    }
    cout << "getting: "<<file<<endl;
    
    
    if (mkdirs) makeDirs(dst);
    else {
        size_t pos = 0;
        if ((pos = dst.find_last_of("/")) != std::string::npos) {
            dst = dst.substr(pos+1);
        }
    }
    
    bool dwn = obj.downloadFile(file,dst);
    if (!dwn){
        cout << "download failed"<<endl;
        return -3;
    }else{
        cout << "download succeeded"<<endl;
        return 0;
    }
}

int cmd_get_dir(pzb &obj, string dir, string dstdir, bool mkdirs){
    
    vector<pzb::t_fileinfo*> files = find_quiet(obj.getFiles(), dir);
    int ret = 0;
    for (pzb::t_fileinfo *f : files){
        if (f->name.substr(f->name.length()-1) == "/") continue;
        string fdst = f->name;
        size_t pos = 0;
        if ((pos = fdst.find_last_of("/")) != std::string::npos) {
            fdst = fdst.substr(pos+1);
        }
        fdst = dstdir + fdst;
    
        if (cmd_get(obj, f->name, fdst, mkdirs) != 0) ret++; //count failed downloads
    }
    
    return ret;
}

void cmd_help(const char *progname){
    cout << "Usage: "<<progname<< " [parameter] <url to zip>\n";
    cout << "Browse and download files and directories from remote zip\n";
    cout << "Specifying no parameter starts an interactive console\n\n";
    
    cout << "Usage: parameter <required argument> [optional argument]\n";
    cout << "Following parameter are avaliable: \n";
    
    cout << "  -l                         \tshows contents and subdirectories of zip\n";
    cout << "      --list=[path]          \tshows contents and subdirectories of [path] in zip\n";
    cout << "      --nosubdirs            \tdon't show subdirectories. Does nothing without -l or --list\n";
    cout << "  -c, --create-directories   \tdownload files with it' directories and subdirectories\n";
    cout << "  -h, --help                 \tshows this help\n";
    cout << "  -g, --get       <path>     \tdownloads remote file\n";
    cout << "  -d, --directory            \tdownload remote directory recursively instead of sindle file\n";
    cout << "                             \tuse this with -g (--get)\n";
    cout << "  -o, --output    <path>     \tspecify dst filename when downloading\n";
    cout << "\n";
}

static struct option longopts[] = {
    { "list",               optional_argument,  NULL, 'l' },
    { "nosubdirs",          no_argument,        NULL, 'l' },
    { "help",               no_argument,        NULL, 'h' },
    { "directory",          no_argument,        NULL, 'd' },
    { "get",                required_argument,  NULL, 'g' },
    { "output",             required_argument,  NULL, 'o' },
    { "create-directories", no_argument,        NULL, 'c' },
    { NULL, 0, NULL, 0 }
};

#define FLAG_LIST       1 << 0
#define FLAG_NOSUBDIRS  1 << 1
#define FLAG_DOWNLOAD   1 << 2
#define FLAG_DIRECTORY  1 << 3

int main(int argc, const char * argv[]) {
    const char *progname = argv[0];
    
    size_t spos=strlen(progname)-1;
    while (spos && *(progname + (spos--) - 2) != '/');
    progname = argv[0] + spos;
    
    pzb *myobj;
    const char *url;
    bool mkdirs = false;
    
    int opt = 0;
    int optindex = 0;
    
    if (argc < 2) {
        cout << "Error url parameter required!\n";
        cmd_help(argv[0]);
        return -1;
    }
    
    long paramFlags = 0;
    char *listdir = 0;
    char *downloadPath = 0;
    char *outputPath = 0;
    
    while ((opt = getopt_long(argc, (char * const *)argv, "lhd:o:", longopts, &optindex)) > 0) {
        switch (opt) {
            case 'h':
                cmd_help(progname);
                return 0;
                
            case 'l':
                if ((strncmp(argv[optind-1],"--nosubdirs",strlen("--nosubdirs")) == 0)) {
                    paramFlags |= FLAG_NOSUBDIRS;
                    break;
                }else paramFlags |= FLAG_LIST;
                
                if (optarg) listdir = optarg;
                break;
                
            case 'g':
                paramFlags |= FLAG_DOWNLOAD;
                if (strncmp((downloadPath = optarg), "-", 1) == 0){
                    cout << "Error: --download requires a parameter!\n";
                    return -1;
                };
                break;
            case 'o':
                if (strncmp((outputPath = optarg), "-", 1) == 0){
                    cout << "Error: --output requires a parameter!\n";
                    return -1;
                };
                break;
            case 'c':
                mkdirs = true;
                break;
            case 'd':
                paramFlags |= FLAG_DIRECTORY;
                break;
                
            default:
                break;
        }
        if (optopt) return -1;
    }
    
    if ((argc-optind) == 1){
        url = argv[optind];
    }else{
        cout << "Error: url required!\n";
        return -1;
    }
    
    
    try {
        myobj = new pzb(url);
        
#warning TODO make this correct
    } catch (...) {
        cout << "Error init failed\n";
        return -1;
    }
    
    vector<pzb::t_fileinfo*> files =  myobj->getFiles();
    int slen =0;
    for (int i=myobj->biggestFileSize(); i>0; i /=10) {
        slen++;
    }

    
    if (paramFlags) {
        if (paramFlags & FLAG_LIST) {
            string dir = (listdir) ? listdir : "";
            if (dir.length() && dir.substr(dir.length()-1) != "/") dir += "/";
            
            return (paramFlags & FLAG_NOSUBDIRS) ? ls(myobj->getFiles(), slen, dir) : find(myobj->getFiles(), slen, dir);
            
        }else if (paramFlags & FLAG_DOWNLOAD){
            string file = downloadPath;
            size_t pos = 0;
            string dst = (outputPath) ? outputPath : file.substr(((pos = file.find_last_of("/")) == string::npos) ? 0 : pos +1);
            
#warning TODO implement dst dir when downloading directories
            return (paramFlags & FLAG_DIRECTORY) ? cmd_get_dir(*myobj, file, "", mkdirs)
                                                    : cmd_get(*myobj, file, dst, mkdirs);
            
        }
        return 0;
    }
    

    
    string uinput;
    string currentDir = "";
    
    do{
        cout <<currentDir << " $ ";
        getline(std::cin,uinput);
        
        //parse
        vector<string> argv;
        size_t pos = 0;
        std::string token;
        while ((pos = uinput.find(" ")) != std::string::npos) {
            token = uinput.substr(0, pos);
            argv.push_back(token);
            uinput.erase(0, pos + 1);
        }
        argv.push_back(uinput);
        
        
        string dir = currentDir;
        string file;
        if (argv.size()>1) {
            dir = (argv[1][0] == '/') ? argv[1] : currentDir + argv[1];
            file = dir;
            if (dir[0] == '/') dir = dir.substr(1);
        }
        if (dir.length() && dir.substr(dir.length()-1) != "/") dir += "/";
        
        if (argv[0] == "ls") {
            ls(files, slen, dir);
            
        }else if (argv[0] == "find") {
            find(files, slen, dir);
            
        }else if (argv[0] == "cd") {
            if (argv.size() == 1) {
                cout << "Error: cd needs a parameter"<<endl;
                continue;
            }
            cd(files, currentDir, argv[1]);
            
        }else if (argv[0] == "get") {
            if (argv.size() == 1) {
                cout << "Error: get needs a parameter"<<endl;
                continue;
            }
            
            string dst = file.substr(currentDir.length());
            
            cmd_get(*myobj, file, dst, mkdirs);
            
        }else if (argv[0] == "getd") {
            if (argv.size() == 1) {
                cout << "Error: getd needs a parameter"<<endl;
                continue;
            }
            
            cmd_get_dir(*myobj, dir, "", mkdirs);

        }else if (argv[0] == "mkdirs"){
            if (argv.size() >= 1 && argv[1] == "toggle") {
                mkdirs = (!mkdirs);
            }
            cout << "[" << ((mkdirs) ? "true" : "false") << "] directories are" << ((mkdirs) ? " " : " not ") << "being downloaded"<<endl;
            
        }else if (argv[0] == "help") {
            cout << "Interactive console\n";
            cout << "Usage: command <required parameter> [optional parameter]\n";
            cout << "Following commands avaliable: \n";
            cout << "cd     <dir>       \tchanges current directory to \"dir\"\n";
            cout << "ls     [dir]       \tshows files in \"dir\" (if speficied)\n";
            cout << "find   [dir]       \tshows files in \"dir\" (if speficied) and subdirectories\n";
            cout << "help               \tshows this help screen\n";
            cout << "get    <file>      \tdownloads file\n";
            cout << "getd   <dir>       \tdownloads directory recursively\n";
            cout << "mkdirs <\"toggle\">\tshows if directorys are downloaded too or only files\n";
            cout << "                   \ttoggles this setting if \"toggle\" is specified\n";
            cout << "exit               \texits interactive mode\n";
            cout << endl;
        }else if (argv[0] == "exit") {
            break;
        }else{
            cout << argv[0]<< ": command not found"<< endl;
        }
        //TODO cmds: getd
    } while (true);
    
    
    return 0;
}
