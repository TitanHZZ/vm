#include <iostream>

#include "exceptions.h"

void exception_handler(Exception_Type exception) {
    std::cerr << "ERROR: Exception occured ";

    switch (exception) {
    case EXCEPTION_OK:
    case EXCEPTION_EXIT:
        return;

    case EXCEPTION_UNKNOWN_INSTRUCTION:
        std::cerr << "'EXCEPTION_UNKNOWN_INSTRUCTION'" << std::endl;
        exit(1);

    case EXCEPTION_STACK_OVERFLOW:
        std::cerr << "'EXCEPTION_STACK_OVERFLOW'" << std::endl;
        exit(1);

    case EXCEPTION_STACK_UNDERFLOW:
        std::cerr << "'EXCEPTION_STACK_UNDERFLOW'" << std::endl;
        exit(1);

    case EXCEPTION_INVALID_JMP_ADDR:
        std::cerr << "'EXCEPTION_INVALID_JMP_ADDR'" << std::endl;
        exit(1);

    case EXCEPTION_DIV_BY_ZERO:
        std::cerr << "'EXCEPTION_DIV_BY_ZERO'" << std::endl;
        exit(1);

    default:
        std::cerr << "when handling another exception: 'EXCEPTION_UNKNOWN'" << std::endl;
        exit(1);
    }
}
