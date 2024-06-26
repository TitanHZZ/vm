#pragma once
#include <iostream>

#ifdef _WIN32
    #define WIN32_LEAN_AND_MEAN // include less WinAPI stuff
    #include <windows.h>
    #undef EXCEPTION_STACK_OVERFLOW // 'EXCEPTION_STACK_OVERFLOW' is a WinAPI macro and also a variant in the 'Exception_Type' enum
#endif

#ifdef _WIN32
    #define RED     12
    #define GREEN   10
    #define BLUE    9
    #define YELLOW  14
    #define BG_BLUE 16
#else
    #define RED     31
    #define GREEN   32
    #define BLUE    34
    #define YELLOW  33
    #define BG_BLUE 44
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
