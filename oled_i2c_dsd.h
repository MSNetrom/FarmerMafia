#pragma once
#ifndef OLED_I2C_DSD
#define OLED_I2C_DSD

//https://github.com/olikraus/u8g2/wiki/u8x8reference
//https ://github.com/olikraus/u8g2/wiki/fntlist8x8
#include <U8x8lib.h>

#define DISPLAY_SIZE 8

namespace myArdu {
  class TextRotator{
  private:
      char* text;
      char pos = 0;
      char l; //Lengde uten \0
      char showTill;

      void setEnd(char& e){
        text[showTill] = text[l]; 
        text[l] = text[e];
        text[e] = '\0';
        //Serial.println(text[e]);
        showTill = e;
      }
  public:
      // Kopier teksten
      TextRotator(char * textSrc) {
        for (l = 0; textSrc[l]; ++l);
        //++l;
        text = new char[l+1];
        text[0] = '\0';
        for (char i = 0; i < l+1; ++i){
          text[i] = textSrc[i];  
        }
        showTill = l;
      }
      ~TextRotator(){
        delete[] text;
      }

      // Roter teksten
      void rotate() {
          setEnd(l);
          char buf = text[0];
          char i = 1;
          while(text[i]) {
            text[i-1] = text[i];
            ++i;   
          }
          text[i-1] = buf;
          pos = abs((-pos-1) % (l));
      }

      void rotateAmount(char& amount){
        for (char i = 0; i < amount; ++i) rotate();  
      }

      void reset(){
        rotateAmount(pos); 
      }

      char* getStr(char len) {
        setEnd(min(len, l));
        return text; 
      }

      const char& getLength(){ return l; }
  };

  
	class OledDisp {
	private:
		//Selve skjermstyringen som vi bygger p�
		U8X8_SH1106_128X64_NONAME_HW_I2C u8x8;

	public:
		//Litt oppstart og instillinger
		void setup() {
			u8x8.begin();
			u8x8.setFont(u8x8_font_inr21_2x4_f);//2x4
		}

		//Print char
		void printState(char & data) {
      u8x8.setCursor(12, 0);
			u8x8.print((int)data);
		}

		//Print tekst
		void printText(char* tekst) {
			//u8x8.clearDisplay();
			u8x8.drawString(0, 0, tekst);
		}
	};
}

#endif
