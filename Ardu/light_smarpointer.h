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

        void deallocate(){
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
    public:
        //Get pointer
        T* get() { return ptr; }

        //De-reference and indixing
        T & operator[] (uint16_t index) { return ptr[index]; }

        //Constructors
        shared_array<T>() { count = new uint16_t(0); }
        shared_array<T>(T* ptr) : ptr(ptr) { count = new uint16_t(1); }

        shared_array<T>& operator=(const shared_array<T>& other) {
            deallocate();
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
           deallocate();
        }
    };

    template<class T>
    class light_shared_vector : public shared_array<T>{
    private:
      uint16_t len;
    public:
      light_shared_vector<T>(uint16_t len) : len{len}, shared_array<T>{new T[len]} {}
      light_shared_vector<T>(const light_shared_vector<T>& other) 
      : shared_array<T>{other}, len{other.len} {}
      light_shared_vector<T>& operator=(const light_shared_vector& other){
        shared_array<T>::operator=(other);
        len = other.len;
        return *this;
      }
      const uint16_t& size() const { return len; }
    };
}

#endif // !LIGHT_SMARTPOINTER
