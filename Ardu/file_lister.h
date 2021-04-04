#pragma once
#ifndef FIELD_LISTER
#define FIELD_LISTER

#include "SD.h"
#include "light_smarpointer.h"
#include "oled_i2c_dsd.h"
#include "globals.h"

namespace myArdu{
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
        //Serial.print("First: "); Serial.println(entry.name()[0]);
        ++i;
      }
    }
    /*for (unsigned char i = 0; i < options.size(); ++i){
      File entry = root.openNextFile();
      options[i] = myArdu::TextRotator(entry.name(), dispSize);
    }*/
    return options;
  }
}

#endif
