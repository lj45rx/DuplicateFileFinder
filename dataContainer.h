#ifndef DATACONTAINER_H_INCLUDED
#define DATACONTAINER_H_INCLUDED

#include <iostream>
#include <vector>
#include <algorithm>    // std::find
#include <regex>

#include "ProgressUtil.h"

using namespace std;


struct Entry{
    unsigned int fileSize;
    unsigned int parentIndex;
    unsigned int filePosition;

    //TODO for large input divisor falls to 0 -> loop never ends
    // for more than GiB unsigned int not large enough
    string getFormatedFileSize_problematic(){
        static string stringNames[5] = {"B", "KiB", "MiB", "GiB", "TiB"};
        unsigned long long divisor = 1024;

        int step = 0;
        while( divisor < fileSize ){
            divisor *= 1024;
            step++;
        }
        divisor /= 1024;

        char buff[20];
        sprintf(buff, "%3.3f %s", this->fileSize/(float)divisor, stringNames[step].c_str());
        return buff;
    }

    string getFormatedFileSize(){
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
};

struct PotentialDupEntry : public Entry{
    string filename = "";
    string path = "";
    PotentialDupEntry *next = nullptr;

    PotentialDupEntry(Entry ent){
        fileSize = ent.fileSize;
        parentIndex = ent.parentIndex;
        filePosition = ent.filePosition;
    }

    PotentialDupEntry(unsigned int _fileSize, unsigned int _parentIndex, unsigned int _filePosition){
        fileSize = _fileSize;
        parentIndex = _parentIndex;
        filePosition = _filePosition;
    }
};


class Container{
    unsigned int maxCapacity = 0;
    unsigned int nextPos = 0;
    unsigned int lastPercentage = 0; //progress

    vector<unsigned int> hits;
    vector<unsigned int> potentialHitIndices;

    public:
    Entry *data = nullptr;
    Entry *data_tempSortBuffer = nullptr;
    vector<PotentialDupEntry> pData;

    Container(int noElements){
        maxCapacity = noElements;
        data = new Entry[noElements];
        cout << "DataContainer created with space for " << noElements
            << " elements (" << sizeof(Entry)*noElements << "B)" << endl;
    }

    ~Container(){ //TODO if nut null etc
        delete data;
    }

    void insertElement(unsigned int position, unsigned int filesize, unsigned int parentIndex, unsigned int skipSmallerThan = 0){
        if(filesize < skipSmallerThan){
            return;
        }

        data[nextPos].filePosition = position;
        data[nextPos].fileSize = filesize;
        data[nextPos].parentIndex = parentIndex;
        //printElement(nextPos);
        nextPos++;
        if( nextPos*100/maxCapacity >= lastPercentage ){
            cout << "\r" << lastPercentage << "% filled (" << nextPos << ")" << std::flush;
            lastPercentage++;
        }
    }

    void sortElements_BubbleSort(){
        lastPercentage = 0;
        Entry temp;
        for(int stop = nextPos-1; stop >= 1; stop--){
                if( 100-stop*100/nextPos > lastPercentage ){
                    cout << "sorting " << ++lastPercentage << "%" << endl;
                }
            //cout << 100-stop*100/nextPos << "% sorted" << endl;
            for(int i = 0; i < stop-1; i++){
                //cout << "cmp(" << i << ", " << i+1 << endl;
                if(data[i+1].fileSize < data[i].fileSize){
                    temp = data[i+1];
                    data[i+1] = data[i];
                    data[i] = temp;
                }
            }
        }
    }

    void mergeSortStep(int lStrt, int rStop, int windowSize){
        bool print = false;
        if(print) cout << "  window size: " << windowSize << endl;
        if(print) cout << "    work " << lStrt << " -> " << rStop << " : " << rStop-lStrt+1 << " elems" <<endl;

        int nextOutPos = lStrt;

        //copy only if already ok - due to array size not power of 2
        if( rStop-lStrt+1 <= windowSize/2 ){
            while( lStrt <= rStop ){
                data_tempSortBuffer[nextOutPos++] = data[lStrt++];
            }
            if(print) cout << "      subarray ok" << endl;
            return;
        }

        int lStop = lStrt + windowSize/2 -1;
        int rStrt = lStop+1;
        if(print) cout << "      with " << lStrt << ", " << lStop << " and " << rStrt << ", " << rStop << endl;

        //while one side not empty
        while( lStrt <= lStop && rStrt <= rStop ){
            if( data[lStrt].fileSize > data[rStrt].fileSize ){
                //if right is smaller use right
                data_tempSortBuffer[nextOutPos++] = data[rStrt++];
            } else {
                //else use left
                data_tempSortBuffer[nextOutPos++] = data[lStrt++];
            }
        }

        //add side which still contains elements
            //will skip if empty already
        while( lStrt <= lStop ){
            data_tempSortBuffer[nextOutPos++] = data[lStrt++];
        }

        while( rStrt <= rStop ){
            data_tempSortBuffer[nextOutPos++] = data[rStrt++];
        }
    }

    void printFirstNElements(unsigned int cnt){
        cout << "[ ";
        for(unsigned int i = 0; i < cnt && i < nextPos; i++){
            cout << data[i].fileSize << " ";
        }
        cout << "]" << endl;
    }

    void sortElements_Mergesort(){
        bool print = false;
        /*
            Idea:
            merge sort - buttom up
            walk through list work on len=2 subarrays, then len=4, len=8, ...
        */

        Entry *tempForSwap = nullptr;
        data_tempSortBuffer = new Entry[nextPos];

        if(print) cout << "sorting" << endl;
        unsigned int windowSize = 1;
        while( windowSize < nextPos ){
            windowSize *=2;
            if(print) cout << "  window size: " << windowSize << endl;
            unsigned int stop;
            //printFirstNElements(12);
            for(unsigned int i = 0; i < nextPos; i+=windowSize){
                stop = i+windowSize-1;
                if(stop >= nextPos) stop = nextPos-1;
                mergeSortStep(i, stop, windowSize);
            }
            //swap arrays
            tempForSwap = data;
            data = data_tempSortBuffer;
            data_tempSortBuffer = tempForSwap;
        }

        //clear temp array used for sort
        delete data_tempSortBuffer;

        //printFirstNElements(12);
        //exit(0);
    }

    void checkSorted(){
        int errors = 0;
        // nextpos ist erste leere position --> -1
        // i schaut auf i�1 --> -1
        for(unsigned int i = 0; i < nextPos-2; i++){
            if(data[i].fileSize > data[i+1].fileSize){
                cout << "error " << i << " > " << i+1 << endl;
                errors++;
            }
        }
        if(errors){
            cout << "ERROR in " << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << endl;
            cout << "is not sorted " << errors << " errors" << endl;
            exit(1);
        }
        //cout << "is sorted" << endl;
    }

    void printPositionsToLookUp(){
        for(unsigned int i = 0; i < hits.size(); i++){
            cout << i << ": " << hits[i] << endl;
        }
    }

    void prepPotentialDups(bool print = false){
        lastPercentage = 0;
        unsigned int lastSize = 0;
        for(unsigned int i = 0; i < hits.size(); i++){
            if( 100*i/hits.size() > lastPercentage){
                cout << "\rprep dupMemory " << ++lastPercentage << "%" << std::flush;
            }
            //cout << __FUNCTION__ << " " << data[potentialHitIndices[i]].fileSize
            //                    << " " << data[potentialHitIndices[i]].filePosition << endl;;
            pData.push_back( PotentialDupEntry(data[potentialHitIndices[i]]) );

            if(i != 0 && lastSize == data[potentialHitIndices[i]].fileSize){
                pData[i-1].next = &pData[i];
            }
            lastSize = data[potentialHitIndices[i]].fileSize;
        }
        cout << "\rprep dupMemory finished" << endl;

        //---------------------------------------------------------------------
        if(!print) return;

        bool isFirst = true;
        for(unsigned int i = 0; i < pData.size(); i++){
            if(isFirst){
                cout << "Files with size: " << pData[i].fileSize << endl;
                isFirst = false;
            }
            cout << "    " << pData[i].filePosition << endl;
            if(pData[i].next == nullptr){
                isFirst = true;
            }
        }
    }

    void printLinesFromPositions(ifstream &inFile){
        cout << __FUNCTION__ << endl;
        string line;
        for(unsigned int i = 0; i < hits.size(); i++){
            cout << "searching " << i << ": " << hits[i] << endl;
            inFile.clear();
            inFile.seekg( hits[i] );
            std::getline(inFile, line);
            cout << "<<<<" << line << ">>>>" << endl;
        }
    }

    string parseFolderStringRegex(string line){
        const std::string s = line;
        std::regex rgx("^ Verzeichnis von (.+)");
        std::smatch match;

        std::regex_search(s.begin(), s.end(), match, rgx);
        return match[1].str();
    }

    //run from the front - overhead but safer
    string parseFolderString(string line){
        // starts with " Verzeichnis von "
        //terefore our data starts at pos 17

        return line.substr(17, line.length()-18);
    }

    string parseFolderStringNewReverse_Buggy(string line){
        int runner = line.length();
        int strt = runner;
        int len = 0;

        while(line[runner] != ' '){
            //cout << line[runner] << endl;
            runner--;
            len++;
        }
        strt = runner+1;

        return line.substr(strt, len);
    }

    string parseFileStringRegex(string line){
        //std::string s = inpLine;
        //cout << "recieved " << line << endl;
        const std::string s = line;
        std::regex rgx("^\\d\\d.\\d\\d.\\d\\d\\d\\d\\s+\\d\\d:\\d\\d\\s+\\d+\\s+(.+)");
        std::smatch match;

        std::regex_search(s.begin(), s.end(), match, rgx);
        return match[1].str();
    }

    string parseFileString(string line){
        // eg "07.01.2011  22:32           2141774 CIMG0001.JPG"
        //terefore our data starts at pos 36

        return line.substr(36, line.length()-37);
    }

    string parseFileStringNewReverse_Buggy(string line){
        int runner = line.length();
        int strt = runner;
        int len = 0;

        while(line[runner] != ' '){
            //cout << line[runner] << endl;
            runner--;
            len++;
        }
        strt = runner+1;

        return line.substr(strt, len);
    }

    void getLinesForPotentialHits(ifstream &inFile){
        lastPercentage = 0;
        vector<string> parentPool;
        vector<unsigned int> parentIdPool;
        vector<unsigned int>::iterator it;
        string temp;

        for(unsigned int i = 0; i < pData.size(); i++){
            //get folder line
            //cout << "searching parent " << pData[i].parentIndex << endl;
                //check if parent already found
            it = find(parentIdPool.begin(), parentIdPool.end(), pData[i].parentIndex);
            if (it != parentIdPool.end()){ //found
                //cout << "  parent already found, is at pos " << std::distance(parentIdPool.begin(), it) << endl;
                pData[i].path = parentPool[ std::distance(parentIdPool.begin(), it) ];
            } else {
                inFile.clear();
                inFile.seekg( pData[i].parentIndex );
                std::getline(inFile, temp);
                temp = parseFolderString(temp);
                pData[i].path = temp;
                parentPool.push_back( temp );
                parentIdPool.push_back(pData[i].parentIndex);
            }
            //get file line
            //cout << "searching file " << pData[i].filePosition << endl;
            if( i*100/pData.size() > lastPercentage ){
                lastPercentage++;
                if( lastPercentage%1 == 0 )
                cout << "\rfetching info " << lastPercentage << "%" << std::flush;
            }
            inFile.clear();
            inFile.seekg( pData[i].filePosition );
            std::getline(inFile, temp);
            temp = parseFileString(temp);
            pData[i].filename = temp;
        }
        cout << "\rfetching info finished" << endl;
    }

    void printPotentialHits(bool writeToFile = true, bool print = false){
        ProgressUtil *progUtil;
        ofstream outfile;
        if(writeToFile){
            cout << "writing to file " << endl;
            outfile.open ("res.txt");
            if(false && !print){
                progUtil = new ProgressUtil;
                progUtil->exec_simple();
            }
        }

        if(print) cout << endl << endl << endl << endl;
        bool isFirst = true;
        for(auto entry : pData){
            if(isFirst){
                if(print) cout << "Files with size: " << entry.fileSize << endl;
                if(writeToFile) outfile << "Files with size: " << entry.getFormatedFileSize() << endl;
                isFirst = false;
            }

            if(print) cout << "    " << entry.path
                << "        " << entry.filename
                << endl;

            //write path / filename
            if(writeToFile) outfile << "    " << entry.path
                << "/" << entry.filename
                << endl;

            if(entry.next == nullptr){
                isFirst = true;
            }
        }

        unsigned int duplicateSize = 0;
        for(unsigned int i = 0; i < pData.size(); i++){
            if(pData[i].next != nullptr){
                duplicateSize += pData[i].fileSize;
            }
        }
        if(print) cout << "max potential duplicate size:" << endl << "    " << duplicateSize << endl;
        if(writeToFile) outfile << "max potential duplicate size:" << endl << "    " << duplicateSize << endl;


        if(writeToFile){
            cout << "done writing file" << endl;
            outfile.close();
            if(false && !print){
                progUtil->stop();
                delete progUtil;
            }
        }
    }

    // find for which filesize -> "hits" files exists with same size -> write to "potentialHitIndices"
    void findSameSizeFiles(bool print = false){
        int strt = 0;
        for(unsigned int i = 0; i < nextPos-1; i++){
            if(data[i].fileSize == data[i+1].fileSize){
                if(print) cout << data[i].fileSize << ": ";
                strt = i;
                while( i < nextPos && //assure bounds
                    data[i].fileSize == data[strt].fileSize ){
                    if(print) cout << data[i].filePosition << " ";
                    hits.push_back(data[i].filePosition);
                    potentialHitIndices.push_back(i);
                    i++;
                }
                if(print) cout << endl;
                i--; //counteract overcounting in while
            }
        }
    }

    void printElement(int index){
        cout << "[" << index << "]: "
            << data[index].parentIndex << ", "
            << data[index].fileSize << ", "
            << data[index].filePosition << endl;
    }


};

#endif // DATACONTAINER_H_INCLUDED
