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
#include <termios.h>
#include <unistd.h>
#include <signal.h>
#include <algorithm>
#include <functional>
#include "all_pzb.h"

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
        if (!mkdirs && (pos = fdst.find_last_of("/")) != std::string::npos) {
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
    cout << "  -k, --insecure             \tdisable ssl validation\n";
    cout << "  -u, --user[:password]      \tauthenticate to webserver\n";
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
    { "insecure",           no_argument,        NULL, 'k' },
    { "user",               required_argument,  NULL, 'u' },
    { NULL, 0, NULL, 0 }
};

#define FLAG_LIST       1 << 0
#define FLAG_NOSUBDIRS  1 << 1
#define FLAG_DOWNLOAD   1 << 2
#define FLAG_DIRECTORY  1 << 3
#define FLAG_IGNORE_SSL 1 << 4

enum t_specialKey{
    kSpecialKeyUndefined = 0,
    kSpecialKeyArrowkeyUp = 1,
    kSpecialKeyArrowkeyDown = 2,
    kSpecialKeyArrowkeyRight = 3,
    kSpecialKeyArrowkeyLeft = 4,
    kSpecialKeyBackspace,
    kSpecialKeyDelete,
    kSpecialKeyTab
};

#warning TODO implement ctrl-c to clear command
//static int lastcltrc = 0;
//void ctrlchandler(int s){
//    printf("Caught signal %d\n",s);
//
//    if (lastcltrc >=2) exit(1);
//    lastcltrc++;
//}

string getCommand(const string &currentDir, vector<string> &history, std::function<string(string curcmd, size_t tabcount)> tabfunc){
#define ret history[w]
    printf("%s $ ",currentDir.c_str());
    
    //instructing terminal to directly pass us all characters instead of filling a buffer
    //and not echoing characters back to the user
    static struct termios oldt, newt;
    tcgetattr( STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON);
    newt.c_lflag &= ~(ECHO);
    tcsetattr( STDIN_FILENO, TCSANOW, &newt);
    
    unsigned char c;
    int i=0;
    
    
    if (history.size() <1 || history[history.size()-1].length() != 0)
        history.push_back("");

    size_t k=0; //charcounter
    size_t w=history.size()-1; //wordcounter
    
    size_t jumplast = 0;
    int isLastchar = 1;
    size_t tabcounter = 0;
    while ((c = getchar()) != '\n') {
//        lastcltrc = 0; //TODO
        t_specialKey ak = kSpecialKeyUndefined;
        size_t savtab = tabcounter;
        tabcounter = 0;
        
        if (i==0 && c == 27) i++;
        else if (i == 1 && c == 91) i++;
        else if (i == 2){
            if (c == 51){
                getchar(); //read additional special byte
                ak = kSpecialKeyDelete;
                goto normalflow;
            }else if (!(65 <= c && c <= 68)){
//#warning DEBUG
//                printf("UNRECOGNISED KEY PRESSED=%d\n",c);
                i=0;
                continue;
            }
            ak = (t_specialKey)(c-64);
            goto normalflow;
        }
        else{
        normalflow:
            if (c == 127) ak = kSpecialKeyBackspace;
            else if (c == '\t') ak = kSpecialKeyTab;
            
            i=0;
            if (ak != kSpecialKeyUndefined){
                switch (ak) {
                    case kSpecialKeyBackspace:
                        if (k>0){
                            jumplast = --k;
                            ret.erase(k,1);
                        }
                        break;
                    case kSpecialKeyDelete:
                        if (k < ret.length()){
                            ret.erase(k,1);
                            goto printStuff;
                        }
                        break;
                    case kSpecialKeyArrowkeyLeft:
                        if (k>0){
                            jumplast = --k;
                            putchar('\b');
                        }
                        break;
                    case kSpecialKeyArrowkeyRight:
                        if (k < ret.length()){
                            putchar(ret.c_str()[k++]);
                            jumplast = k;
                        }
                        break;
                    case kSpecialKeyArrowkeyUp:
                        if (w > 0){
                            w--;
                            if (jumplast>k) k = jumplast;
                            if (isLastchar == 1 || k >= ret.length()) k = ret.length(),isLastchar=2;
                        }
                        break;
                    case kSpecialKeyArrowkeyDown:
                        if (w<history.size()-1){
                            w++;
                            if (jumplast > k) k = jumplast;
                            if (isLastchar == 1 || k >= ret.length()) k = ret.length(),isLastchar=2;
                        }
                        break;
                    case kSpecialKeyTab:
                        {
                            tabcounter = savtab+1;
                            string tmp = tabfunc(ret.substr(0,k),tabcounter);
                            if (tmp.size() && tmp != ret){
                                ret = tmp + ret.substr(k);
                                k=tmp.length();
                                tabcounter = 0;
                            }
                        }
                        break;
                        
                    default:
//#warning DEBUG
//                        printf("pressed key=%d\n",ak);
                        break;
                }
                goto printStuff;
            }
            
            ret.insert(k, 1, (char)c);
            jumplast = ++k;
            isLastchar = (k == ret.length());
        printStuff:
            printf("\x1b[2K\r%s $ %s",currentDir.c_str(),ret.c_str());
            for (int i=0; i<ret.length()-k; i++) putchar('\b');
        }
    }
    putchar('\n');
    
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt); //restore old terminal behavior
    
    return ret;
}


static vector<string>shellcmds{"ls","cd","find","help","get","getd","mkdirs","exit"};

int main(int argc, const char * argv[]) {
    const char *progname = argv[0];
    
    printf("Version: " PZB_VERSION_COMMIT_SHA" - " PZB_VERSION_COMMIT_COUNT"\n");
    printf("%s\n",fragmentzip_version());
    
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
    string login;
    
    while ((opt = getopt_long(argc, (char * const *)argv, "lhdku:cg:o:", longopts, &optindex)) > 0) {
        switch (opt) {
            case 'h':
                cmd_help(progname);
                return 0;
            
            case 'k':
                paramFlags |= FLAG_IGNORE_SSL;
                break;
            
            case 'u':
                login = optarg;
                break;
                
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
        myobj = new pzb(url,paramFlags & FLAG_IGNORE_SSL,login);
        
#warning TODO make this correct
    } catch (...) {
        cout << "Error init failed\n";
        return -1;
    }
    
    vector<pzb::t_fileinfo*> files =  myobj->getFiles();
    unsigned int slen =0;
    for (uint64_t i=myobj->biggestFileSize(); i>0; i /=10) {
        slen++;
    }

    
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
    
//TODO ctrl-c handler
//    struct sigaction sigIntHandler;
//
//    sigIntHandler.sa_handler = ctrlchandler;
//    sigemptyset(&sigIntHandler.sa_mask);
//    sigIntHandler.sa_flags = 0;
//    
//    sigaction(SIGINT, &sigIntHandler, NULL);
    
    string uinput;
    string currentDir = "";
    vector<string> history;
    do{
        uinput = getCommand(currentDir, history,[&files,&currentDir](string curcmd, size_t tabcount)->string{
            
            const char *space = NULL;
            string preCmd;
            
            vector<pair<string, bool>> cdfiles;
            vector<pair<string, bool>> mayMatchCMD;
            if ((space = strstr(curcmd.c_str(), " "))){
                preCmd = curcmd.substr(0,space+1-curcmd.c_str());
                curcmd = curcmd.substr(preCmd.length());
                for (auto f : find_quiet(files, currentDir)){
                    bool d = isDir(f->name);
                    string c = f->name.substr(currentDir.length());
                    
                    if (count(curcmd.begin(),curcmd.end(),'/') < count(c.begin(),c.end(),'/') && c[c.length()-1] != '/') continue;
                    
                    if (c.length() <= curcmd.length() && c[c.length()-1] == '/') continue;
                    
                    if (strncmp(c.c_str(), curcmd.c_str(), min(c.length(),curcmd.length())) == 0 && curcmd.length() <= c.length()){
                        mayMatchCMD.push_back({c,d});
                    }else if (!mayMatchCMD.size()){
                        cdfiles.push_back({c,d});
                    }
                }
            }else{
                for (auto c : shellcmds){
                    if (strncmp(c.c_str(), curcmd.c_str(), min(c.length(),curcmd.length())) == 0 && curcmd.length() <= c.length()){
                        mayMatchCMD.push_back({c,0});
                    }
                }
            }
            
            if (!mayMatchCMD.size() && !cdfiles.size()) return "";
            else if (mayMatchCMD.size() == 1) return preCmd + mayMatchCMD[0].first+ (mayMatchCMD[0].second ? "" : " ");
            
            string bestMatch;
            if (mayMatchCMD.size()) {
                while (true) {
                    size_t pos = bestMatch.size();
                    char c = mayMatchCMD[0].first[pos];
                    for (auto s : mayMatchCMD){
                        if (pos >= s.first.size() || s.first[pos] != c)
                            goto foundBestMatch;
                    }
                    bestMatch.push_back(c);
                }
            }
        foundBestMatch:
            
            if (tabcount >1){
                printf("\x1b[2K\r");
                if (!preCmd.size() || !curcmd.size() || curcmd == bestMatch){
                    for (auto m : (mayMatchCMD.size() ? mayMatchCMD : cdfiles)){
                        string s;
                        if (preCmd.size()) {
                            s = m.first.substr(curcmd.length());
                            
                            if (const char * t= strstr(m.first.c_str(),"/")){
                                while (const char *tt = strstr(t, "/")) {
                                    if (tt-m.first.c_str() >= curcmd.length()) {
                                        break;
                                    }
                                    s = t = tt+1;
                                }
                            }
                        }else{
                            s = m.first;
                        }
                        cout << s << (space ? "\n" : " ");
                    }
                }
                cout <<endl;
            }
            
            
            
            return (bestMatch.size() ? preCmd + bestMatch : "");
        });
        if (!uinput.size()) continue;
        
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
                cd(files, currentDir, "/");
            }else{
                cd(files, currentDir, argv[1]);
            }
            
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
    } while (true);
    
    
    return 0;
}
