#ifndef UTILS_H_INCLUDED
#define UTILS_H_INCLUDED

#include <iostream>
#include<stdlib.h>
#include<string.h>

#include <fstream>
#include <string>
#include <ios>

#include <regex>

#include "ProgressUtil.h"

using namespace std;

std::ifstream::pos_type filesize(const char* filename){
    std::ifstream in(filename, std::ifstream::ate | std::ifstream::binary);
    return in.tellg();
}

string getTempFileSizeString(){
    char buff[50];
    sprintf(buff, "Filesize: %10ld Kib", (long int)filesize("temp.txt")/1024);
    return buff;
    //return "current Filesize: " + std::itoa(42)+ "";
}

void generateTxtFile(){
    ProgressUtil putil;
    cout << "using system function to generate list of files, this might take a while..." << endl;

    putil.exec_progressFunction(getTempFileSizeString);
    system("dir \"H:\\\" /A/OS/-C/A:-D/S > temp.txt");
    putil.stop();
}

int countLinesInFile(ifstream &stream){
    int res = 0;
    std::string str;
    while ( std::getline(stream, str) ) {
        res++;
    }
    return res;
}

int parseLine(string line, unsigned int *fileSize ){
    //cout << line << endl;

    if( line.find("Verzeichnis von") != string::npos ){
        //cout << "parent found" << endl;
        return 1;
    }
    //file eg <<28.08.2015  11:45>>

    //<16.12.2015  10:55         228670984 v2.7z>

    //cout << "recv <" << line << ">" << endl;

    if(line[2] == '.'
    && line[5] == '.'
    && line[14] == ':'){
        //cout << "likely file" << endl;

        int strt = 0;
        int stop = 0;
        int runner = 25; //start pos eg 25
        //jump to where there will most likely still be a space before the filesize

        //if already too far right move back
        while(line[runner] != ' '){
            runner--;
            //cout << "  jumping back to pos " << runner << endl;
        }

        while(line[runner] == ' '){
            runner++;
            //cout << "  searching start at pos " << runner << endl;
        }

        strt = runner;

        while(line[runner] != ' '){
            runner++;
        }
        stop = runner-1;

        //only string containing filesize
        line = line.substr(strt, stop-strt+1);

        *fileSize = atoi( line.c_str() );
        return 0;
    } else {
        return 2;
    }
}

int parseLineRegex(string line, unsigned int *fileSize ){
    //cout << line << endl;

    if( line.find("Verzeichnis von") != string::npos ){
        //cout << "parent found" << endl;
        return 1;
    }
    //file eg <<28.08.2015  11:45>>

    const std::string s = line;
    //std::regex rgx("^\\d\\d.\\d\\d.\\d\\d\\d\\d\\s+\\d\\d:\\d\\d\\s+(\\d+)\\s+(.+)");
    std::regex rgx("^\\d\\d.\\d\\d.\\d\\d\\d\\d\\s+\\d\\d:\\d\\d\\s+(\\d+)\\s+", std::regex::optimize);
    std::smatch match;


    if (std::regex_search(s.begin(), s.end(), match, rgx)){
        *fileSize = atoi( (match[1]).str().c_str() );
        //std::cout << " match: " << match[1] << endl;
        return 0;
    }
    return 2;
}









#endif // UTILS_H_INCLUDED
