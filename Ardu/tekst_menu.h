#pragma once
#ifndef TEKST_MENU
#define TEKST_MENU

#include "light_smartpointer.h"
#include "oled_i2c_dsd.h"
#include "button.h"

/*
  Denne koden inneholder selve logikken i menyen. Den sørger
  for gi ut teksten som skal skrives til menyen, og å rotere
  på teksten. Den gir også ut filnavn for valgt fil, og har
  noen flere funksjoner.
*/

namespace myArdu{
  class Clock{
    private:
      long lastTime = 0;
      bool lastStatus = false;
      int dela;
    public:
      Clock(int dela) : dela{dela} {}
      bool checkpoint() {
        if ((millis()-dela) > lastTime){
          lastTime = millis();
          return true;  
        }
        return false;
      }
  };
  
  class Menu{
  private:
    long t = 0;
    char curChoise = 0;
    char lastChoise;
    char choise;
    Button buttons[3];
    light_shared_vector<TextRotator> texts;
  public:
    Menu(char upPin, char confirmPin, char downPin)
    : buttons{Button{upPin}, Button{confirmPin}, Button{downPin}}, texts{0} {}

    void setup(light_shared_vector<TextRotator> setupTexts){
      texts = setupTexts;
      choise = texts.size();
      lastChoise = texts.size();
      for (char i = 0; i < 3; ++i){
        buttons[i].setup();  
      }
    }

    // Sjekker om i har bladd i menyen
    bool newChoise() {
      if ((millis()-300) > t){
        char i = 0;
        for(; i < 3 && !buttons[i].newClick(); ++i);
        if (i < 3){
          if (i == 0) {
            curChoise = (curChoise+1)%texts.size();
          }
          else if (i == 1) {
            choise = curChoise;
          }
          else if (i == 2) {
            curChoise = (curChoise-1+texts.size())%texts.size();
          }
          t = millis();
          return true;
        }
      }
      return false;
    }

    // Sjekker om vi har valgt ny fil
    bool newFile(){
      if (lastChoise == choise){
        return false;
      }
      lastChoise = choise;
      return true;
    }

    // Får teksten til relevant meny-valg
    char* getText(){
      return texts[curChoise].getStr();
    }

    // Returner filnavn på valgt åker/fil
    char* getFileName(){
      return texts[curChoise].getFull();
    }

    // Reuturnerer symbol, for å vise om åkeren er valgt eller ikke
    char getSelected() const {
      if (curChoise == choise) {
        return 'V';  
      }
      return ' ';
    }

    // Rotererer den aktuelle teksten
    void rotate() {
      texts[curChoise].rotate();
    }

    light_shared_vector<TextRotator>& getOptions() { return texts; }
  };
}

#endif
