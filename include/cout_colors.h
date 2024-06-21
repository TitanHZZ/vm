#pragma once
#include <iostream>

#ifdef _WIN32
    #include <windows.h>
#endif

#ifdef _WIN32
    #define RED     12
    #define GREEN   10
    #define BLUE    9
    #define YELLOW  14
#else
    #define RED     31
    #define GREEN   32
    #define BLUE    34
    #define YELLOW  33
#endif

void setConsoleColor(int color) {
    #ifdef _WIN32
        HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
        SetConsoleTextAttribute(hConsole, static_cast<WORD>(color));
    #else
        std::cout << "\033[" << color << "m";
    #endif
}

void resetConsoleColor() {
    #ifdef _WIN32
        HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
        SetConsoleTextAttribute(hConsole, 15); // Reset to default color
    #else
        std::cout << "\033[0m";
    #endif
}
