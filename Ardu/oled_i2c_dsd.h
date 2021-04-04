#pragma once
#ifndef OLED_I2C_DSD
#define OLED_I2C_DSD

//https://github.com/olikraus/u8g2/wiki/u8x8reference
//https://github.com/olikraus/u8g2/wiki/fntlist8x8
#include <U8x8lib.h>

#define DISPLAY_SIZE 8

namespace myArdu {
  class TextRotator{
  private:
      char* text;
      char pos = 0;
      char len; //Lengde uten \0
      char showTill = 0;
      char realLen = 0;
      char dispSize;

      void setEnd(char e){
        text[showTill] = text[len]; 
        text[len] = text[e];
        text[e] = '\0';
        showTill = e;
      }
  public:
      // Lag en default
      TextRotator() : len{0}, text{new char[1]}, showTill{0} {
        text[0] = '\0';
      }
      // Kopier teksten
      TextRotator(char * textSrc, char dispSize) : dispSize{dispSize} {
        char l;
        for (l = 0; textSrc[l]; ++l);
        realLen = l;
        len = max(l, dispSize)+1; //Lengde med " "
        text = new char[len+1];
        for (char i = 0; i < len; ++i){
          text[i] = ' ';
        }
        for (char i = 0; i < l; ++i){
          text[i] = textSrc[i];
        }
        text[len] = '\0';
        showTill = len; 
      }

      TextRotator(const TextRotator& other) 
      : len{other.len}, pos{other.pos}, showTill{other.showTill}, dispSize{dispSize}, realLen{realLen} {
        text = new char[len+1];
        for (char i = 0; i < len+1; ++i) { text[i] = other.text[i]; }
      }

      TextRotator& operator=(const TextRotator& other) {
        delete[] text;
        len = other.len;
        pos = other.pos;
        showTill = other.showTill;
        dispSize = other.dispSize;
        realLen = other.realLen;
        text = new char[len+1];
        for (char i = 0; i < len+1; ++i) { text[i] = other.text[i]; }
        return *this;
      }
      
      ~TextRotator(){
        delete[] text;
      }

      // Roter teksten
      void rotate() {
          setEnd(len);
          char buf = text[0];
          char i = 1;
          while(text[i]) {
            text[i-1] = text[i];
            ++i;   
          }
          text[i-1] = buf;
          --pos;
          if (pos < 0) {
            pos = len-1;
          }
          //Serial.print("Pos: "); Serial.println((int)pos);
      }

      void rotateAmount(char amount){
        for (char i = 0; i < amount; ++i) rotate();  
      }

      void reset(){
        rotateAmount(pos); 
      }

      char* getStr() {
        setEnd(dispSize);
        return text; 
      }

      char* getFull(){
        reset();
        setEnd(realLen);
        return text;
      }

      const char& getLength(){ return len; }
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
      //printText(0, 4, "GPS:");
		}

		//Print char
		void printState(char data) {
      u8x8.setCursor(10, 4);
			u8x8.print((int)data);
		}

    void resetState(){
      u8x8.setCursor(10, 4);
      u8x8.print(' ');
    }

    void printCheck(char v){
      u8x8.setCursor(14, 0);
      u8x8.print(v);
    }

    void printInsideStatus(char v){
      u8x8.setCursor(14, 4);
      u8x8.print(v);
    }

    void printGpsStatus(char* tekst){
      u8x8.drawString(0, 4, tekst);
    }

		//Print tekst
		void printText(char x, char y, char* tekst) {
			//u8x8.clearDisplay();
			u8x8.drawString(x, y, tekst);
		}

    
	};
}

#endif
