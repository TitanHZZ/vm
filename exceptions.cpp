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

    case EXCEPTION_UNKNOWN_STACK_DATA_TYPE:
        std::cerr << "'EXCEPTION_UNKNOWN_STACK_DATA_TYPE'" << std::endl;
        exit(1);

    case EXCEPTION_SUBTRACT_POINTER_AND_DOUBLE:
        std::cerr << "'EXCEPTION_SUBTRACT_POINTER_AND_DOUBLE'" << std::endl;
        exit(1);

    case EXCEPTION_SUBTRACT_POINTER_AND_INT:
        std::cerr << "'EXCEPTION_SUBTRACT_POINTER_AND_INT'" << std::endl;
        exit(1);

    case EXCEPTION_ADD_POINTER_AND_DOUBLE:
        std::cerr << "'EXCEPTION_ADD_POINTER_AND_DOUBLE'" << std::endl;
        exit(1);

    case EXCEPTION_ADD_TWO_POINTERS:
        std::cerr << "'EXCEPTION_ADD_TWO_POINTERS'" << std::endl;
        exit(1);

    case EXCEPTION_MUL_POINTER:
        std::cerr << "'EXCEPTION_MUL_POINTER'" << std::endl;
        exit(1);

    case EXCEPTION_DIV_POINTER:
        std::cerr << "'EXCEPTION_DIV_POINTER'" << std::endl;
        exit(1);

    case EXCEPTION_INVALID_RET_ADDR:
        std::cerr << "'EXCEPTION_INVALID_RET_ADDR'" << std::endl;
        exit(1);

    case EXCEPTION_BITWISE_NON_INT:
        std::cerr << "'EXCEPTION_BITWISE_NON_INT'" << std::endl;
        exit(1);

    default:
        std::cerr << "when handling another exception: 'EXCEPTION_UNKNOWN'" << std::endl;
        exit(1);
    }
}
