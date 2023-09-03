#include <iostream>
#include <stdint.h>
#include <math.h>

#include "nan_box.h"

#define TYPE_MASK (((1LL << 3LL) - 1LL) << 48LL)
#define VALUE_MASK ((1LL << 48LL) - 1LL)

/*void Nan_Box::print_bits_representation(uint8_t *const ptr, const size_t size) {
    for (const uint8_t* ptr_end = ptr + size - 1; ptr_end >= ptr; ptr_end--) {
        for (char i = 7; i >= 0; i--) {
            std::cout << ((*ptr_end >> i) & 0x1);
        }
    }

    std::cout << std::endl;
}*/

Nan_Type Nan_Box::get_type() const {
    if (!isnan(value))
        return Nan_Type::DOUBLE;

    const uint64_t *const value_ptr = (uint64_t*)(&value);
    return (Nan_Type) ((*value_ptr & TYPE_MASK) >> 48LL);
}

void Nan_Box::set_type(const Nan_Type type) {
    value = 0.0 / 0.0; // make a nan
    uint64_t *const value_ptr = (uint64_t*)(&this->value);
    *value_ptr = (*value_ptr & ~TYPE_MASK) | ((uint64_t)type << 48LL);
}

uint64_t Nan_Box::get_value() const {
    const uint64_t *const value_ptr = (uint64_t*)(&value);
    return *value_ptr & VALUE_MASK;
}

void Nan_Box::set_value(const uint64_t value) {
    uint64_t *const value_ptr = (uint64_t*)(&this->value);
    *value_ptr |= value & VALUE_MASK;
}

// operator overloading

Nan_Box& Nan_Box::operator+=(Nan_Box& rhs) {
    const Nan_Type type = this->get_type();
    const Nan_Type rhs_type = rhs.get_type();

    if (type == Nan_Type::DOUBLE) {
        if (rhs_type == Nan_Type::DOUBLE) {
            // sum of two doubles
            value += rhs.as_double();
        } else if (rhs_type == Nan_Type::INT) {
            // sum of a double and an int
            this->value += rhs.as_int();
        } else if (rhs_type == Nan_Type::PTR) {
            // return an exception (cannot add a pointer and a double)
            this->box_exception(Exception_Type::EXCEPTION_ADD_POINTER_AND_DOUBLE);
        }
    } else if (type == Nan_Type::INT) {
        if (rhs_type == Nan_Type::DOUBLE) {
            // sum of an int and a double
            this->box_double(this->as_int() + rhs.as_double()); 
        } else if (rhs_type == Nan_Type::INT) {
            // sum of two ints
            this->box_int(this->as_int() + rhs.as_int());
        } else if (rhs_type == Nan_Type::PTR) {
            // increment pointer as if it was a 'char *'
            this->box_ptr(this->as_int() + static_cast<char*>(rhs.as_ptr()));
        }
    } else if (type == Nan_Type::PTR) {
        if (type == Nan_Type::DOUBLE) {
            // return an exception (cannot add a pointer and a double)
            this->box_exception(Exception_Type::EXCEPTION_ADD_POINTER_AND_DOUBLE);
        } else if (rhs_type == Nan_Type::INT) {
            // increment pointer as if it was a 'char *'
            this->box_ptr(static_cast<char*>(this->as_ptr()) + rhs.as_int());
        } else if (rhs_type == Nan_Type::PTR) {
            // return an exception (cannot add two pointers)
            this->box_exception(Exception_Type::EXCEPTION_ADD_TWO_POINTERS);
        }
    }

    return *this;
}

Nan_Box& Nan_Box::operator-=(Nan_Box& rhs) {
    const Nan_Type type = this->get_type();
    const Nan_Type rhs_type = rhs.get_type();

    if (type == Nan_Type::DOUBLE) {
        if (rhs_type == Nan_Type::DOUBLE) {
            // subtract two doubles
            value -= rhs.as_double();
        } else if (rhs_type == Nan_Type::INT) {
            // subtract an int from a double
            this->value -= rhs.as_int();
        } else if (rhs_type == Nan_Type::PTR) {
            // return an exception (cannot subtract a pointer and a double)
            this->box_exception(Exception_Type::EXCEPTION_SUBTRACT_POINTER_AND_DOUBLE);
        }
    } else if (type == Nan_Type::INT) {
        if (rhs_type == Nan_Type::DOUBLE) {
            // subtract a double from an int
            this->box_double(this->as_int() - rhs.as_double());
        } else if (rhs_type == Nan_Type::INT) {
            // subtract two ints
            this->box_int(this->as_int() - rhs.as_int());
        } else if (rhs_type == Nan_Type::PTR) {
            // return an exception (cannot subtract a pointer and a int)
            this->box_exception(Exception_Type::EXCEPTION_SUBTRACT_POINTER_AND_INT);
        }
    } else if (type == Nan_Type::PTR) {
        if (type == Nan_Type::DOUBLE) {
            // return an exception (cannot subtract a pointer and a double)
            this->box_exception(Exception_Type::EXCEPTION_SUBTRACT_POINTER_AND_DOUBLE);
        } else if (rhs_type == Nan_Type::INT) {
            // decrement pointer as if it was a 'char *'
            this->box_ptr(static_cast<char*>(this->as_ptr()) - rhs.as_int());
        } else if (rhs_type == Nan_Type::PTR) {
            // subtract a pointer from a pointer
            this->box_ptr( (void*)(static_cast<char*>(this->as_ptr()) - static_cast<char*>(rhs.as_ptr())) );
        }
    }

    return *this;
}

Nan_Box& Nan_Box::operator*=(Nan_Box& rhs) {
    const Nan_Type type = this->get_type();
    const Nan_Type rhs_type = rhs.get_type();

    if (type == Nan_Type::DOUBLE) {
        if (rhs_type == Nan_Type::DOUBLE) {
            // multiply two doubles
            value *= rhs.as_double();
        } else if (rhs_type == Nan_Type::INT) {
            // multiply a double and an int
            this->value *= rhs.as_int();
        } else if (rhs_type == Nan_Type::PTR) {
            // return an exception (cannot multiply a pointer and a double)
            this->box_exception(Exception_Type::EXCEPTION_MUL_POINTER);
        }
    } else if (type == Nan_Type::INT) {
        if (rhs_type == Nan_Type::DOUBLE) {
            // multiply an int and a double
            this->box_double(this->as_int() * rhs.as_double()); 
        } else if (rhs_type == Nan_Type::INT) {
            // multiply two ints
            this->box_int(this->as_int() * rhs.as_int());
        } else if (rhs_type == Nan_Type::PTR) {
            // return an exception (cannot multiply a pointer and an int)
            this->box_exception(Exception_Type::EXCEPTION_MUL_POINTER);
        }
    } else if (type == Nan_Type::PTR) {
        // return exception (cannot multiply a pointer)
        this->box_exception(Exception_Type::EXCEPTION_MUL_POINTER);
    }

    return *this;
}

Nan_Box& Nan_Box::operator/=(Nan_Box& rhs) {
    const Nan_Type type = this->get_type();
    const Nan_Type rhs_type = rhs.get_type();

    if (type == Nan_Type::DOUBLE) {
        if (rhs_type == Nan_Type::DOUBLE) {
            if (rhs.as_double() == 0.0) {
                // return an exception (cannot divide by 0)
                this->box_exception(Exception_Type::EXCEPTION_DIV_BY_ZERO);
            } else {
                value /= rhs.as_double();
            }
        } else if (rhs_type == Nan_Type::INT) {
            if (rhs.as_int() == 0) {
                // return an exception (cannot divide by 0)
                this->box_exception(Exception_Type::EXCEPTION_DIV_BY_ZERO);
            } else {
                this->value /= rhs.as_int();
            }
        } else if (rhs_type == Nan_Type::PTR) {
            // return an exception (cannot divide a double by a pointer)
            this->box_exception(Exception_Type::EXCEPTION_DIV_POINTER);
        }
    } else if (type == Nan_Type::INT) {
        if (rhs_type == Nan_Type::DOUBLE) {
            if (rhs.as_double() == 0.0) {
                // return an exception (cannot divide by 0)
                this->box_exception(Exception_Type::EXCEPTION_DIV_BY_ZERO);
            } else {
                this->box_double(this->as_int() / rhs.as_double()); 
            }
        } else if (rhs_type == Nan_Type::INT) {
            if (rhs.as_int() == 0) {
                // return an exception (cannot divide by 0)
                this->box_exception(Exception_Type::EXCEPTION_DIV_BY_ZERO);
            } else {
                if (this->as_int() % rhs.as_int() == 0) {
                    this->box_int(this->as_int() / rhs.as_int());
                } else {
                    this->box_double((double)this->as_int() / (double)rhs.as_int());
                }
            }
        } else if (rhs_type == Nan_Type::PTR) {
            // return an exception (cannot divide an int by a pointer)
            this->box_exception(Exception_Type::EXCEPTION_DIV_POINTER);
        }
    } else if (type == Nan_Type::PTR) {
        // return an exception (cannot divide a pointer)
        this->box_exception(Exception_Type::EXCEPTION_DIV_POINTER);
    }

    return *this;
}

Nan_Box& Nan_Box::operator<<=(Nan_Box& rhs) {
    const Nan_Type type = this->get_type();
    const Nan_Type rhs_type = rhs.get_type();

    if (type != Nan_Type::INT || rhs_type != Nan_Type::INT) {
        this->box_exception(Exception_Type::EXCEPTION_BITWISE_NON_INT);
    } else {
        this->box_int(this->as_int() << rhs.as_int());
    }

    return *this;
}

Nan_Box& Nan_Box::operator>>=(Nan_Box& rhs) {
    const Nan_Type type = this->get_type();
    const Nan_Type rhs_type = rhs.get_type();

    if (type != Nan_Type::INT || rhs_type != Nan_Type::INT) {
        this->box_exception(Exception_Type::EXCEPTION_BITWISE_NON_INT);
    } else {
        this->box_int(this->as_int() >> rhs.as_int());
    }

    return *this;
}

Nan_Box& Nan_Box::operator&=(Nan_Box& rhs) {
    const Nan_Type type = this->get_type();
    const Nan_Type rhs_type = rhs.get_type();

    if (type != Nan_Type::INT || rhs_type != Nan_Type::INT) {
        this->box_exception(Exception_Type::EXCEPTION_BITWISE_NON_INT);
    } else {
        this->box_int(this->as_int() & rhs.as_int());
    }

    return *this;
}

Nan_Box& Nan_Box::operator|=(Nan_Box& rhs) {
    const Nan_Type type = this->get_type();
    const Nan_Type rhs_type = rhs.get_type();

    if (type != Nan_Type::INT || rhs_type != Nan_Type::INT) {
        this->box_exception(Exception_Type::EXCEPTION_BITWISE_NON_INT);
    } else {
        this->box_int(this->as_int() | rhs.as_int());
    }

    return *this;
}

Nan_Box& Nan_Box::operator^=(Nan_Box& rhs) {
    const Nan_Type type = this->get_type();
    const Nan_Type rhs_type = rhs.get_type();

    if (type != Nan_Type::INT || rhs_type != Nan_Type::INT) {
        this->box_exception(Exception_Type::EXCEPTION_BITWISE_NON_INT);
    } else {
        this->box_int(this->as_int() ^ rhs.as_int());
    }

    return *this;
}

Nan_Box& Nan_Box::operator~() {
    const Nan_Type type = this->get_type();

    if (type != Nan_Type::INT) {
        this->box_exception(Exception_Type::EXCEPTION_BITWISE_NON_INT);
    } else {
        this->box_int(~this->as_int());
    }

    return *this;
}

bool Nan_Box::operator==(Nan_Box& rhs) {
    const Nan_Type type = this->get_type();
    const Nan_Type rhs_type = rhs.get_type();

    if (type == rhs_type) {
        if (type == Nan_Type::DOUBLE) {
            return this->as_double() == rhs.as_double();
        } else if (type == Nan_Type::INT) {
            return this->as_int() == rhs.as_int();
        } else if (type == Nan_Type::PTR) {
            return this->as_ptr() == rhs.as_ptr();
        }
    }

    return false;
}
