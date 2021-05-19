
/*
  #### INFORMASJON ####

  Dette er hovedfila, for koden som regulerer såkorn- og gjødselsmengde på såmaskinen.
  Alle åkerfiler lagret i "root"-området på SD-kortet, vil bli lest inn i menyen, og kan
  velges fra menyen.

  Definerer også en del konstanter her, som kan forandres etter ønske.
  
*/

#include <SPI.h>
#include <SD.h>
#include "field_reader.h"
#include "mygps_m9n_uart2.h"
#include "farm_rele_control.h"
#include "oled_i2c_dsd.h"
#include "tekst_menu.h"
#include "file_lister.h"

//Creat a polygon holder, to hold data from a field
myArdu::PolyHolder fieldHolder;

//Create gps
#define GPS_PERIOD 1000 // Leser hvert sekund fra GPS (må stilles inn på GPSen)
#define GPS_LOOK_TIME 5 // Ser etter GPS-signal i 5 millisekunder
myArdu::MY_GPS_UART2 myGps(Serial3, GPS_PERIOD, GPS_LOOK_TIME);

//Constants for relays
#define CLICK_TIME 290
#define WAIT_TIME 190

//Create rele control for seeds
#define UP_PIN_SEED 24
#define MID_PIN_SEED 23
#define DOWN_PIN_SEED 22
myArdu::LevelControl levelControlSeed(UP_PIN_SEED, MID_PIN_SEED, DOWN_PIN_SEED, CLICK_TIME, WAIT_TIME);

//Create rele control for gjødning
#define UP_PIN_GJOD 27
#define MID_PIN_GJOD 26
#define DOWN_PIN_GJOD 25
myArdu::LevelControl levelControlGjod(UP_PIN_GJOD, MID_PIN_GJOD, DOWN_PIN_GJOD, CLICK_TIME, WAIT_TIME);

//Create display
#define DISPLAY_SIZE 5
myArdu::OledDisp display;

// Test tekst
myArdu::Menu menu(2, 3, 4);
myArdu::Clock rotationClock(500);

// The setup function runs once when you press reset or power the board
void setup() {
	//Prepare pin for file reading
	pinMode(53, OUTPUT);

	if (!SD.begin(53)) {
		while (true);
	}

  //Setup display
  display.setup();

  //Setup optoins for menu, and wait for choise
  menu.setup(myArdu::getOptions(DISPLAY_SIZE));

  //Vent på at åker skal velges
  while(!menu.newFile()){
    if (rotationClock.checkpoint()){
      long tiger = millis();
      menu.rotate();
      display.printText(0, 0, menu.getText());
    }
    if (menu.newChoise()) {
      display.printText(0, 0, menu.getText()); 
      display.printCheck(menu.getSelected());
    }
  }

	//Open file
	File myFile = SD.open(menu.getFileName());
  display.printText(0, 0, menu.getText()); 

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
}

// the loop function runs over and over again until power down or reset
void loop() {
	//Check if we should look for GPS data
  //Vi sjekker GPS om det har gått mer tid en (GPS_PERIOD - GPS_LOOK_TIME) som definert lenger opp
	if (myGps.should_look()) {
		//Get next gps data (wait for it)
		const myArdu::NAV_PVT& pvt = myGps.get_next_pvt();

		//Contruct a point with our pos, and get 6 decimal presision
		myArdu::Point32 pos = myArdu::Point32();
		pos.x = pvt.lon / 10;
		pos.y = pvt.lat / 10;

		//Få verdi for området vi er i
		char state = fieldHolder.value(pos);
		//127 betyr at vi ikke er innenfor definert område
		if (state == 127) {
			//Printe at vi er utenfor omr�det
			display.printInsideStatus('?');
		}
		else {
			//Skifte til ny verdi
			levelControlSeed.new_level(state);
      levelControlGjod.new_level(state);
			//Let click start before display-printing
			levelControlSeed.loop();
      levelControlGjod.loop();
			//Print til skjerm
			display.printState(state);
      display.printInsideStatus(' ');
		}
    // Printer GPS-status til skjerm
		if (myGps.gnssFixOk()) {
			display.printGpsStatus("OK");
		}
    else{
      display.printGpsStatus("!!");
    }
	}
 
	//Kjør levelcontrol loop
	levelControlSeed.loop();
  levelControlGjod.loop();

  //Denne har i hovedsak med skjermen og menyknapper å gjøre
  if (rotationClock.checkpoint()){
    menu.rotate();
    display.printText(0, 0, menu.getText());
  }
  if (menu.newChoise()) { 
    if (menu.newFile()){
      File myFile = SD.open(menu.getFileName());
      fieldHolder = myArdu::PolyHolder{};
      fieldHolder = myArdu::read_field(myFile);
      myFile.close();
    }
    display.printText(0, 0, menu.getText()); 
    display.printCheck(menu.getSelected());
  }
}
