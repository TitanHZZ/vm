#pragma once
#include <stdint.h>
#include <ostream>

#include "exceptions.h"

typedef enum {
    DOUBLE = 0,
    INT,
    PTR,
    EXCEPTION
} Nan_Type;

class Nan_Box {
public:
    Nan_Box() {}
    Nan_Box(const double db_value)   : m_value(db_value) {}
    Nan_Box(const int64_t int_value) : m_value(box_int(int_value)) {}
    Nan_Box(const void *ptr_value)   : m_value(box_ptr(ptr_value)) {}

    // 'type' related functions
    Nan_Type get_type() const;

    // 'boxing' functions
    inline double box_double(const double value)     { this->m_value = value; return this->m_value; }
    double box_int(const int64_t value)              { this->set_type(Nan_Type::INT);       this->set_value((uint64_t)value); return this->m_value; }
    double box_ptr(const void *const ptr)            { this->set_type(Nan_Type::PTR);       this->set_value((uint64_t)ptr);   return this->m_value; }
    double box_exception(const Exception_Type value) { this->set_type(Nan_Type::EXCEPTION); this->set_value(value);           return this->m_value; }

    // casting functions
    inline double  as_double () const { return m_value; }
    inline int64_t as_int    () const { return (int64_t) get_value(); }
    inline void   *as_ptr    () const { return (void *)  get_value(); }
    inline Exception_Type as_exception() { return (Exception_Type) get_value(); }

    // debug function
    // void print_bits_representation(uint8_t *const ptr, const size_t size);

    friend std::ostream& operator<<(std::ostream& os, const Nan_Box& obj) {
        switch (obj.get_type()) {
        case Nan_Type::DOUBLE:
            os << obj.as_double();
            break;

        case Nan_Type::INT:
            os << obj.as_int();
            break;
        
        case Nan_Type::PTR:
            os << obj.as_ptr();
            break;

        case Nan_Type::EXCEPTION:
        default:
            // TODO
            break;
        }

        return os;
    }

    // TODO: add exception handling to these operators
    Nan_Box& operator+=(Nan_Box& rhs);
    Nan_Box& operator-=(Nan_Box& rhs);
    Nan_Box& operator*=(Nan_Box& rhs);
    Nan_Box& operator/=(Nan_Box& rhs);
    Nan_Box& operator%=(Nan_Box& rhs);
    Nan_Box& operator<<=(Nan_Box& rhs);
    Nan_Box& operator>>=(Nan_Box& rhs);
    Nan_Box& operator&=(Nan_Box& rhs);
    Nan_Box& operator|=(Nan_Box& rhs);
    Nan_Box& operator^=(Nan_Box& rhs);
    Nan_Box& operator~();
    bool operator==(Nan_Box& rhs);

private:
    double m_value;

    // 'type' related functions
    void set_type(const Nan_Type type);

    // 'value' related functions
    uint64_t get_value() const;
    void     set_value(const uint64_t value);
};
