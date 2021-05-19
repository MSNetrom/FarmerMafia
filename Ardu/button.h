#pragma once
#ifndef BUTTON_DETECT
#define BUTTON_DETECT

/*
  Button-klassen brukes for å sjekke om det trykkes på en av meny-knappene.
*/

namespace myArdu {
    class Button {
    private:
        char pin;
        bool state = HIGH;
    public:
        Button(char pin) : pin{ pin } {}
        void setup() {
            pinMode(pin, INPUT_PULLUP);
        }
        // Denne funksjonen returnerer True ved nytt klikk
        bool newClick() {
            bool newState = digitalRead(pin);
            if (!newState) {
                state = LOW;
                return true;
            }
            else if (!state && newState) {
                state = HIGH;
            }
            return false;
        }
    };
}

#endif
