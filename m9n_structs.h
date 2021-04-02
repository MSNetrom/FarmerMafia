#pragma once
#ifndef M9N_STRUCTS
#define M9N_STRUCTS

#include <stdint.h>

namespace myArdu {

    //Start of all ubx messages
    //const uint8_t UBX_HEADER[] = { 0xB5, 0x62 };
    //const uint8_t UBX_NAV_PVT_CLASS_ID[] = { 0x01, 0x07 };
    const uint8_t UBX_PVT_FULLARRAY[] = { 0x07, 0x01, 0x62, 0xB5 };
    uint32_t UBX_PVT_FULLID = 3043098887;

    //https://www.u-blox.com/en/docs/UBX-19035940
    //2 bytes før og etter, som ikke er del av structen
    //4 bytes med info før selve payload
    //92 bytes of payload
    //100 bytes total?
    struct NAV_PVT {
        uint8_t cls;
        uint8_t id;
        uint16_t len;

        //Payload
        uint32_t iTOW;          // GPS time of week of the navigation epoch (ms)
        uint16_t year;         // Year (UTC) 
        uint8_t month;         // Month, range 1..12 (UTC)
        uint8_t day;           // Day of month, range 1..31 (UTC)
        uint8_t hour;          // Hour of day, range 0..23 (UTC)
        uint8_t minute;        // Minute of hour, range 0..59 (UTC)
        uint8_t second;        // Seconds of minute, range 0..60 (UTC)

        int8_t valid;                  // Validity Flags (see graphic below)
        uint32_t tAcc;          // Time accuracy estimate (UTC) (ns)
        int32_t nano;                   // Fraction of second, range -1e9 .. 1e9 (UTC) (ns)
        uint8_t fixType;       // GNSSfix Type, range 0..5

        int8_t flags;                  // Fix Status Flags 
                                     //(bit 0: gnssFixOK - 1 = valid fix (i.e within DOP & accuracy masks)
                                     //(bit 1: diffSoln - 1 = differential corrections were applied)
                                     //(bit 5) har med heading å gjøre
        int8_t flags2;                 //Litt mer stuff
        uint8_t numSV;         // Number of satellites used in Nav Solution

        int32_t lon;                    // Longitude (deg)
        int32_t lat;                    // Latitude (deg)
        int32_t height;                 // Height above Ellipsoid (mm)
        int32_t hMSL;                   // Height above mean sea level (mm)
        uint32_t hAcc;          // Horizontal Accuracy Estimate (mm)
        uint32_t vAcc;          // Vertical Accuracy Estimate (mm)

        int32_t velN;                   // NED north velocity (mm/s)
        int32_t velE;                   // NED east velocity (mm/s)
        int32_t velD;                   // NED down velocity (mm/s)
        int32_t gSpeed;                 // Ground Speed (2-D) (mm/s)
        int32_t heading;                // HeadMot, Heading of motion 2-D (deg)
        uint32_t sAcc;          // Speed Accuracy Estimate
        uint32_t headingAcc;    // Heading Accuracy Estimate
        uint16_t pDOP;         // Position dilution of precision

        int8_t flags3;                 //(bit 0: Invalid lat and int32_t)
        uint8_t reserved[5];   //Some reserved data

        int32_t headVeh;                //Heading  of  vehicle  (2-D),valid  whenheadVehValid is set
                                     //otherwise the output is set to theheading of motion

        int16_t magDec;                  //Magnetic declination.
        uint16_t magAcc;         //Magnetic declination accuracy.
    };

    //Calculate checksum
    uint16_t calcChecksum(NAV_PVT& pvt) {
        uint8_t CK[2];
        memset(CK, 0, 2);
        for (int16_t i = 0; i < (int16_t)sizeof(NAV_PVT); i++) {
            CK[0] += ((uint8_t*)(&pvt))[i];
            CK[1] += CK[0];
        }
        return *((uint16_t*)CK);
    }
}
#endif