#include "progressutil.h"

ProgressUtil::ProgressUtil(){

}

void ProgressUtil::exec_simple(Style style){
    threadShouldStop = false;
    thread = new std::thread(&ProgressUtil::exec_repeatingPattern, this, style);
}

void ProgressUtil::exec_progressFunction(std::string (*progressFunct)()){
    this->progressFunct = progressFunct;
    thread = new std::thread(&ProgressUtil::exec_givenProgressFunction, this);
}

void ProgressUtil::stop(){
    threadShouldStop = true;
    thread->join();
    delete thread;
    thread = nullptr;
}

void ProgressUtil::sleepMilliseconds(unsigned int milliseconds){
    std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
}

void ProgressUtil::exec_repeatingPattern(ProgressUtil *obj, Style style){
    int runner = 0;
    while(true){
        //check if is finished
        if(obj->threadShouldStop){
            break;
        }

        //show progress
        cout << "\rexecuting " << getNextProgressString(runner++, style) << std::flush;

        //sleep a while
        sleepMilliseconds(250);
    }

    //do cleanup
    cout << " finished" << endl;
}

void ProgressUtil::exec_givenProgressFunction(ProgressUtil *obj){
    while(true){
        //check if is finished
        if(obj->threadShouldStop){
            break;
        }

        //show progress
        cout << "\r" << obj->progressFunct() << std::flush;

        //sleep a while
        sleepMilliseconds(250);
    }

    //do cleanup
    cout << " finished" << endl;
}

std::string ProgressUtil::getNextProgressString(int i,  Style style){
    static std::string spinner_strings[4] = {"(/)", "(-)", "(\\)", "(|)"};
    static std::string dots_strings[4] = {".  ", ".. ", "...", "   "};

    switch (style) {
    case Spinner:
        return spinner_strings[i%4];
        break;
    case Dots:
        return dots_strings[i%4];
        break;
    default:
        return "invalid style";
    }
}















