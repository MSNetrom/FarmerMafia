#pragma once
#ifndef M9N_UART2
#define M9N_UART2

#include <stdint.h>
#include "m9n_structs.h"

#define M9N_MY_BAUD2 115200

namespace myArdu {

    class MY_GPS_UART2 {
    private:
        //Read to this struct
        NAV_PVT _pvt_buff;

        //Have a buffer in addition
        uint32_t buffer = 0;
        void _read_to_buff() {
            ((byte*)(&buffer))[3] = ((byte*)(&buffer))[2];
            ((byte*)(&buffer))[2] = ((byte*)(&buffer))[1];
            ((byte*)(&buffer))[1] = ((byte*)(&buffer))[0];
            ((byte*)(&buffer))[0] = _get_next_byte();
        }

        //Save the serial we are using
        HardwareSerial& serial;

        //Save last time we strarted reading data, and period
        bool period_started = false;
        unsigned long lastRead = 0;
        const unsigned int period;
        
        void _time_update() {
            if (!period_started) {
                lastRead = millis();
                period_started = true;
            }
        }
        
        //Empties uart
        void empty_buffer() { while (serial.available()) serial.read(); }

        //Wait for, and read next byte
        byte _get_next_byte() {
            while (!serial.available());
            return serial.read();
        }

        //Function for trying to read
        bool _try_read() {
            //Some constants we will use
            uint8_t offset = 2; //Offset when reading data to _pvt
            const uint8_t payloadSize = sizeof(NAV_PVT);

            //Look for header, class and id, and check if we got it
            _read_to_buff();
            _time_update();
            if (buffer != UBX_PVT_FULLID) return false;
            _pvt_buff.cls = ((byte*)(&buffer))[1];
            _pvt_buff.id = ((byte*)(&buffer))[0];
   
            //Loop for 94 bytes of payload, before checksum
            while (offset < sizeof(NAV_PVT)) {
                ((uint8_t*)(&_pvt_buff))[offset] = _get_next_byte();
                offset++;
            }

            //Get checksum
            uint16_t check = calcChecksum(_pvt_buff);

            //Read checksum
            _read_to_buff();
            if (((uint8_t*)(&buffer))[0] != ((uint8_t*)(&check))[0]) return false;
            _read_to_buff();
            //Return is checksum is correct
            return ((uint8_t*)(&buffer))[0] == ((uint8_t*)(&check))[1];
        }
    public:
        //Constructor to get serial
        MY_GPS_UART2(HardwareSerial& serial, unsigned int update_period, unsigned int look_time = 1) 
            : serial(serial), period(update_period-look_time) {}

        //Setup stuff (baud rate)
        void setup() { serial.begin(M9N_MY_BAUD2); }

        //Check the gpsFix
        bool gnssFixOk() { return _pvt_buff.flags & 1; } // Check bit 0

        //Wait until we got pvt, and empty buffer, return read only pvt
        const NAV_PVT & get_next_pvt() {
            //Serial.println(*((uint32_t*)(UBX_PVT_FULLARRAY)));
            //Timing is updated inside _try_read()
            period_started = false;
            //Wait for valid pack
            while (!_try_read());
            //while (true);
            //Empty buffer
            empty_buffer();
            //Return reference to data object
            return _pvt_buff;
        }

        //Check we should look for new package
        bool should_look() {
            if ((millis() - lastRead) >= period) {
                return true;
            }
            return false;
        }
    };
}

#endif