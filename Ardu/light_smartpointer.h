#pragma once
#ifndef LIGHT_SMARTPOINTER
#define LIGHT_SMARTPOINTER

/*
   I denne fila er det definert to datatyper som brukes andre steder i prosjektet.

   shared_array-klassen lagrer en pointer til en array, av ønsket type.
   Minne som brukes av arrayen slettes automatisk når den siste
   kopien av sharred_array instansen slettes. Det betyr at man
   ikke trenger å tenke på minnehåndtering.

   light_sharred_vector-klassen er ment som en mini-vektor-klasse,
   som kan lagre en array pluss lengden på arrayen.
*/

#include <stdint.h>

namespace myArdu {
    //Simple shared pointer array, <array type>
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
