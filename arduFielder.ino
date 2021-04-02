/*
 Name:		ardu_fielder.ino
 Created:	1/7/2021 5:11:04 PM
 Author:	Morten
*/

#include <SPI.h>
#include <SD.h>
#include "field_reader.h"
#include "mygps_m9n_uart2.h"
#include "farm_rele_control.h"
#include "oled_i2c_dsd.h"

//Create file object
File myFile;

//Creat a polygon holder, to hold a fiel
myArdu::PolyHolder fieldHolder;

//Create gps
#define GPS_PERIOD 1000
#define GPS_LOOK_TIME 5
myArdu::MY_GPS_UART2 myGps(Serial3, GPS_PERIOD, GPS_LOOK_TIME);

//Constants for reles
#define CLICK_TIME 290
#define WAIT_TIME 190

//Create rele control for seeds
#define UP_PIN_SEED 22
#define MID_PIN_SEED 23
#define DOWN_PIN_SEED 24
myArdu::LevelControl levelControlSeed(UP_PIN_SEED, MID_PIN_SEED, DOWN_PIN_SEED, CLICK_TIME, WAIT_TIME);

//Create rele control for gjødning
#define UP_PIN_GJOD 25
#define MID_PIN_GJOD 26
#define DOWN_PIN_GJOD 27
myArdu::LevelControl levelControlGjod(UP_PIN_GJOD, MID_PIN_GJOD, DOWN_PIN_GJOD, CLICK_TIME, WAIT_TIME);

//Create display
myArdu::OledDisp display;

// Test tekst
myArdu::TextRotator texter("Huset");

// the setup function runs once when you press reset or power the board
void setup() {
	//Setup pc communication
	Serial.begin(115200);
	while (!Serial);

	//Prepare pin for file reading
	pinMode(53, OUTPUT);

	if (!SD.begin(53)) {
		while (true);
	}

	//Open file
	myFile = SD.open("huset.sve"); //plassen.sve

	//Check if file opened successfully
	if (!myFile) {
		while (true);
	}

	//Here we should read file
	fieldHolder = myArdu::read_field(myFile);

	//Close file
	myFile.close();

	//Setup gps
	myGps.setup();

	//Setup rele control
	levelControlSeed.setup();
  levelControlGjod.setup();

	//Setup display
	display.setup();
}

// the loop function runs over and over again until power down or reset
void loop() {
	//Check if we should look for GPS data
	if (myGps.should_look()) {
    display.printText(texter.getStr(4));
    texter.rotate();
		//Get next gps data (wait for it)
		const myArdu::NAV_PVT& pvt = myGps.get_next_pvt();

		//Contruct a point with our pos, and get 6 decimal presision
		myArdu::Point32 pos = myArdu::Point32();
		pos.x = pvt.lon / 10;
		pos.y = pvt.lat / 10;

    //Serial.print(pos.x); Serial.print(" "); Serial.println(pos.y);

		//F� verdi for omr�det vi er i
		char state = fieldHolder.value(pos);
		//127 betyr at vi ikke er innenfor definert omr�de
		if (state == 127) {
			//Printe at vi er utenfor omr�det
			//display.printText("? ");
		}
		else {
			//Skifte til ny verdi
			levelControlSeed.new_level(state);
      levelControlGjod.new_level(state);
			//Let loop start before check
			levelControlSeed.loop();
      levelControlGjod.loop();
			//Print til skjerm
			display.printState(state);
		}
		if (!myGps.gnssFixOk()) {
			//display.printText("??");
		}
	}
	//Kj�r levelcontrol loop for updates
	levelControlSeed.loop();
  levelControlGjod.loop();
}
