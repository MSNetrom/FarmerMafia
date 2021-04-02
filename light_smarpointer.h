#pragma once
#ifndef LIGHT_SMARTPOINTER
#define LIGHT_SMARTPOINTER

#include <stdint.h>

namespace myArdu {
    //Simple shared pointer array, <array type, tracking of length size
    template<class T>
    class shared_array {
    private:
        T* ptr;
        uint16_t* count;

    public:
        //Get pointer
        T* get() { return ptr; }

        //De-reference and indixing
        T & operator[] (uint16_t index) { return ptr[index]; }

        //Constructors
        shared_array<T>() { count = new uint16_t(0); }
        shared_array<T>(T* ptr) : ptr(ptr) { count = new uint16_t(1); }

        shared_array<T> operator=(const shared_array<T>& other) {
            ptr = other.ptr;
            count = other.count;
            *count++;
            return *this;
        }
        shared_array<T>(const shared_array<T>& other) {
            ptr = other.ptr;
            count = other.count;
            *count++;
        }

        //Destruct stuff
        ~shared_array<T>() {
            if (count == 1) {
                delete count;
                delete[] ptr;
            }
            else if (count == 0) {
                delete count;
            } 
            else {
                count--;
            }
        }
    };
}

#endif // !LIGHT_SMARTPOINTER
