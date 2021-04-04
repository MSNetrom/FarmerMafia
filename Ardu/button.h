#pragma once
#ifndef BUTTON_DETECT
#define BUTTON_DETECT

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
