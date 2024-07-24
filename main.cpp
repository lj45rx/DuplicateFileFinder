#include "utils.h"
#include "dataContainer.h"

#include <iostream>
#include <stdlib.h>
#include <string.h>

#include <fstream>
#include <string>
#include <ios>

#include <regex>

using namespace std;

//TODO hier ueberraschend gefaehrlich fuer große zahlen
// unsigned int, unsigned long, fehler bereits ab GiB
// divisor laeuft ueber, faellt auf =0, dann endlosschleife
string test_getFormatedFileSize(unsigned int fileSize){
    static string stringNames[5] = {"B", "KiB", "MiB", "GiB", "TiB"};

    int step = 0;
    float sizeFactor = (float)fileSize;
    while( 1023 < sizeFactor ){
        sizeFactor /= 1024;
        step++;
    }

    char buff[20];
    sprintf(buff, "%3.3f %s", sizeFactor, stringNames[step].c_str());
    return buff;
}

int main(){
    /*
    unsigned int fileSize = 1355035520;
    cout << fileSize << endl;
    cout << test_getFormatedFileSize(fileSize) << endl;
    */

    ifstream inFile;
    inFile.open("temp.txt",ios::binary);
    if(inFile.fail()){
        // call system function to get list of directories/files/filesizes on specified dive
        // TODO give parameter to set which drive
        generateTxtFile();
    }

    // count lines in created file
    int noLinesInFile = countLinesInFile(inFile);
    inFile.clear();
    inFile.seekg( 0, std::ios_base::beg );
    cout << noLinesInFile << " Lines in File" << endl;

    // not all lines are actually files, but more than noLines is not possible
    // fill container with entries for each file
    Container cnt(noLinesInFile);
    unsigned int skipFilesSmallerThan = 100000;
    std::string line;
    unsigned int currentParentIndex = 0;
    unsigned int pos = inFile.tellg();
    unsigned int tempSize = 0;
    int parseResultCode = -1;
    while ( std::getline(inFile, line) ) {
        parseResultCode = parseLine(line, &tempSize);

        switch(parseResultCode){
        case 0: //is file
            cnt.insertElement(pos, tempSize, currentParentIndex, skipFilesSmallerThan);
            break;
        case 1: //is folder
            //index der linie die zu parent fuehr merken
            currentParentIndex = pos;
            break;
        case 2: //is not interesting
            break;
        }

        pos = inFile.tellg();
    }
    cout << "\rinserting elements finished" << endl;

    // sort/group elements by filesize
    cout << "sorting" << std::flush;
    cnt.sortElements_Mergesort();
        //cnt.sortElements_BubbleSort();
    cout << "\rsorting finished" << endl;
    cnt.checkSorted();

    // find sizes and indices
    cnt.findSameSizeFiles();
    // create objects from indices for easier handling
    cnt.prepPotentialDups();

    cnt.getLinesForPotentialHits(inFile);
    cnt.printPotentialHits(true);

    inFile.close();

    //cout << "Press Enter to exit..." << endl;
    //cin.get();
    return 0;
}
