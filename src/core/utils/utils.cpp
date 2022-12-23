#include <iostream>
#include <cstring> 
#include <cpuid.h>  // GCC only
#include <unistd.h>
#include <climits>

#include "utils.hpp"


int change_cwd_to_selfpath(){
    char buff[PATH_MAX];
    int i;
    ssize_t len = ::readlink("/proc/self/exe", buff, sizeof(buff)-1);

    if (len != -1) {
        i = len;
        while(buff[i-1] != '/' && i >= 0)
            i--;
        buff[i] = '\0';
    }
    else{
        std::cerr << "change_cwd_to_selfpath: Could not retrieve path to executable (errno "
                  << errno << ")" << std::endl;
        return EXIT_FAILURE;
    }
    if(chdir(buff) != 0){
        std::cerr << "change_cwd_to_selfpath: Could not change the cwd to executable path (errno "
                  << errno << "). Path: " << buff << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}


int file_to_str(const std::string& filename, std::string& content){
    std::ifstream file;
    std::stringstream buffer;
    
    file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    try{
        file.open (filename.c_str());
        buffer << file.rdbuf();
        file.close();
        content = buffer.str();
    }
    catch(std::ifstream::failure &e){
        std::cerr << "file_to_str: Exception opening/reading/closing file" << filename 
                  << "(" << e.what() << ")" << "\n";
        log("file_to_str: Exception opening/reading/closing file ", filename, " (", e.what(), ")");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}


int file_to_str(const char* filename, std::string& content){
    std::ifstream file;
    std::stringstream buffer;
    
    file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    try{
        file.open (filename);
        buffer << file.rdbuf();
        file.close();
        content = buffer.str();
    }
    catch(std::ifstream::failure &e){
        std::cerr << "file_to_str: Exception opening/reading/closing file " << filename << " (" 
                  << e.what() << ")" << "\n";
        log("file_to_str: Exception opening/reading/closing file ", filename, " (", e.what(), ")");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}


#include <stdio.h> //change

// doesn't seem to work, use stb?
int greyscale_to_bmp(const char* filename, int w, int h, const unsigned char* buffer){
    FILE *f;
    //unsigned char *img = nullptr, c;
    int filesize = 54 + w*h;  //w is your image width, h is image height, both int

    unsigned char bmpfileheader[14] = {'B','M', 0,0,0,0, 0,0, 0,0, 54,0,0,0};
    unsigned char bmpinfoheader[40] = {40,0,0,0, 0,0,0,0, 0,0,0,0, 1,0, 8,0};
    unsigned char bmppad[3] = {0,0,0};

    bmpfileheader[2] = (unsigned char)(filesize);
    bmpfileheader[3] = (unsigned char)(filesize>> 8);
    bmpfileheader[4] = (unsigned char)(filesize>>16);
    bmpfileheader[5] = (unsigned char)(filesize>>24);

    //bmpinfoheader[14] = (unsigned char)(8);

    bmpinfoheader[4] = (unsigned char)(w);
    bmpinfoheader[5] = (unsigned char)(w>>8);
    bmpinfoheader[6] = (unsigned char)(w>>16);
    bmpinfoheader[7] = (unsigned char)(w>>24);
    bmpinfoheader[8] = (unsigned char)(h);
    bmpinfoheader[9] = (unsigned char)(h>>8);
    bmpinfoheader[10] = (unsigned char)(h>>16);
    bmpinfoheader[11] = (unsigned char)(h>>24);

    f = fopen(filename,"wb");
    fwrite(bmpfileheader,1,14,f);
    fwrite(bmpinfoheader,1,40,f);
    for(int i=0; i<h; i++){
        //fwrite(buffer+(w*(h-i-1)),1,w,f);
        fwrite(buffer+(w*(h-i)),1,w,f);
        fwrite(bmppad,1,(4-(w)%4)%4,f);
    }

    fclose(f);

    return 1;
}


void wstrcpy(wchar_t* dest, const wchar_t* source, uint max){
    uint i = 0;
    while(source[i] != '\0' && i < max)
        i++;
    std::memcpy(dest, source, (i + 1) * sizeof(wchar_t));
}


size_t ucs2wcs(wchar_t* dest, const unsigned char* source, uint max){
    uint i = 0;
    while(source[i] != '\0' && i < max)
        i++;
    for(uint j=0; j<=i; j++)
        dest[j] = source[j];
    return (i-1) * sizeof(wchar_t);
}


int get_cpu_model(char* stringbuffer){
    uint32_t modelname[64];

    if(!__get_cpuid_max(0x80000004, NULL)){
        std::cerr << "get_cpu_model: Feature not implemented." << std::endl;
        log("get_cpu_model: Feature not implemented.");
        return EXIT_FAILURE;
    }

    __get_cpuid(0x80000002, modelname+0x0, modelname+0x1, modelname+0x2, modelname+0x3);
    __get_cpuid(0x80000003, modelname+0x4, modelname+0x5, modelname+0x6, modelname+0x7);
    __get_cpuid(0x80000004, modelname+0x8, modelname+0x9, modelname+0xa, modelname+0xb);

    for(int i=0; i <= 0xb; i++)
        memcpy(&stringbuffer[i*4], modelname+i, 4);
        
    return EXIT_SUCCESS;
}


unsigned long long get_sys_memory(){
    long pages = sysconf(_SC_PHYS_PAGES);
    long page_size = sysconf(_SC_PAGE_SIZE);
    return pages * page_size;
}


