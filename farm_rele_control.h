#pragma once
#ifndef FARM_RELE_CONTROL
#define FARM_RELE_CONTROL

#include <stdint.h>

namespace myArdu {
    class Click {
    private:
        //Where do we click
        uint8_t click_pin = 0;
        //Time started to click
        uint32_t start_time_click;
        uint32_t start_time_wait;

        //States
        bool clicking = false;
        bool waiting = false;

        //Stop click, start waiting
        void stop_click() {
            digitalWrite(click_pin, LOW);
            start_time_wait = millis();
            clicking = false;
            waiting = true;
        }

    public:
        //Create and get pin
        Click() {}
        Click(const uint8_t click_pin) : click_pin(click_pin) {}

        //Loop to check if we are finishd, should only be runned if we function is not finished
        void loop(const uint16_t& click_time, const uint16_t& wait_time) {
            if ((millis() - start_time_click) >= click_time && clicking) {
                //Start waiting
                stop_click();
            }
            if ((millis() - start_time_wait) >= wait_time && waiting) {
                //Stop waiting
                waiting = false;
            }
        }
        //Check if we are finished
        bool finished() { return !(clicking || waiting); }

        //Start click
        void start_click() {
            clicking = true;
            start_time_click = millis();
            digitalWrite(click_pin, HIGH);
        }
    };

    // Kontrollere relle
    class LevelControl {
    private:
        //Save our level, expecting 0 as calibration midpoint
        int8_t level = 0;
        int8_t next_level = 0;

        //Timing variables
        const uint16_t click_time = 0; //How int32_t to click
        const uint16_t wait_time = 0; //Wait after last click

        //Create our clicker
        Click clicker;

        //Save pin for our stuff
        const uint8_t up_pin;
        const uint8_t mid_pin;
        const uint8_t down_pin;

        //Basic level control
        void zero_level() {
            level = 0;
            clicker = Click(mid_pin);
            clicker.start_click();
        }
        void level_up() {
            level++;
            clicker = Click(up_pin);
            clicker.start_click();
        }
        void level_down() {
            level--;
            clicker = Click(down_pin);
            clicker.start_click();
        }

    public:
        //Constructor
        LevelControl(const uint8_t up_pin, const uint8_t mid_pin, const uint8_t down_pin, 
            const uint16_t click_time, const uint16_t wait_time)
            : up_pin(up_pin), mid_pin(mid_pin), down_pin(down_pin), click_time(click_time), wait_time(wait_time) {}

        //Set new level
        void new_level(int8_t new_level) {
            //Serial.println(clicker.finished());
            next_level = new_level;
        }

        //Setup pins and stuff
        void setup() {
            pinMode(up_pin, OUTPUT);
            pinMode(mid_pin, OUTPUT);
            pinMode(down_pin, OUTPUT);
        }

        //Loop check for level
        void loop() {
            //First check if we are clicking
            if (clicker.finished()) {
                //Check if we are at our level
                if (level != next_level) {
                    //Check if we can do a zero_level first
                    if ((level < 0 && next_level >= 0) || (level > 0 && next_level <= 0)) {
                        zero_level();
                    }
                    else if (level < next_level) {
                        level_up();
                    }
                    else {
                        level_down();
                    }
                }
            }
            else {
                //Run click loop
                clicker.loop(click_time, wait_time);
            }
        }
    };
}

#endif // !FARM_RELE_CONTROL
