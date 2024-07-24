#ifndef PROGRESSUTIL_H
#define PROGRESSUTIL_H

#include <chrono>
#include <thread>
#include <iostream>

using std::cout; using std::endl;

class ProgressUtil{
public:
    enum Style{
        Spinner,
        Dots
    };

    ProgressUtil();
    void exec_simple(Style style = Spinner);
    void exec_progressFunction( std::string(*progressFunct)() );
    void stop();

    static void sleepMilliseconds(unsigned int milliseconds);
private:
    bool threadShouldStop = false;
    std::thread *thread = nullptr;

    static void exec_repeatingPattern(ProgressUtil *obj, Style style);
    static void exec_givenProgressFunction(ProgressUtil *obj);

    std::string(*progressFunct)() = nullptr;

    static std::string getNextProgressString(int i, Style style);
};

#endif // PROGRESSUTIL_H
