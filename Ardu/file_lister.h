#pragma once
#ifndef FIELD_LISTER
#define FIELD_LISTER

#include "SD.h"
#include "light_smartpointer.h"
#include "oled_i2c_dsd.h"

/*
 Kode for å få info om filer, lagret på SD-kortet.
*/

namespace myArdu{

  // Returner antall filer i root-mappen av SD-kortet
  unsigned char numFiles(){
   unsigned char count = 0;
   File root = SD.open("/");
   while(true){
     File entry = root.openNextFile();
     if (!entry){
      break;
     }
     if (!entry.isDirectory()){
      ++count;
     }
   }
   return count;
  }

  // Returnerer liste med filnavn, for bruk i menyen
  light_shared_vector<TextRotator> getOptions(char dispSize){
    light_shared_vector<TextRotator> options(numFiles());
    File root = SD.open("/");
    unsigned char i = 0;
    while (true){
      File entry = root.openNextFile();
      if (!entry){
        break;
      }
      if (!entry.isDirectory()){
        options[i] = TextRotator(entry.name(), dispSize);
        ++i;
      }
    }
    return options;
  }
}

#endif
