// Fielder.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

/*
    #### INFORMASJON ####

    Denne fila inneholder main-funksjonen, og kan kompileres, for å lage konverteringsprogrammet,
    som konverterer GeoJson-filer fra QGIS, til vårt filformat, laget for Arduinonen.

    #### HVORDAN BRUKES PROGRAMMET? ####

    Programmet kjøres fra konsoll (cmd). Man bruker "cd" kommandoen for å gå til riktig mappe, så kjører
    man programmet ved å skrive "Fielder.exe geojson-filnavn navn-på-generert-fil".
    
    Man må følge 8.3-navnestandaren for filer. Det vil si at navn-på-generert-fil kan ha maks 8 bokstaver, og
    eventuelt en filending på 3 bokstaver. F.eks. har vi en åker vi kaller "Bekkenget". Denne har 9 bokstaver,
    så filnavnet for selvet åkeren, lar vi være "bekkeng", og filnavn for fotgangen til denne åkeren er "bekkeng.fot".
    (Vi bruker eget kart for rundene der man kjører fotganger, for å slippe at såmaskinen justeres hver gang
    man kommer til enden, når man kjører normalt.)

    #### Filformat som blir laget: ####

    - Format for saving file.
    - Each file starts with number of polygons, then number of points and the points for each polygon.
    uint8: Number of polygons
    int32: Reference x_value
    int32: Reference y_value
    ----Repeated polygyns----
    uint8: Number of points
    int8: Extra data, (sowing mass example)
         ----Reapeated points----
         uint16: x coordinate
         uint16: y coordinate

    (Reference x og y, er den laveste x og y verdien på hele åkeren. Koordinatene er lagret i forhold til disse,
    for å spare minne)

*/

#include <iostream>
#include <iomanip>
#include <fstream>
#include <cstdint>
#include <algorithm>
#include <climits>
#include "Poly.h"
#include "json.hpp"


//Namespace for json-reader
using json = nlohmann::json;

//Reads data from GeoJson, and return a PolyHolder (from Poly.h)
Poly::PolyHolder readGeoJson(std::string path) {
    //Make file and json ready
    //Read geojson file
    std::ifstream field_file(path);
    json field_json;
    if (field_file.is_open()) {
        field_json = json::parse(field_file);
    }
    else {
        throw std::runtime_error("Error opening file");
    }
    //Start reading data
    //Create a holder for polygons
    Poly::PolyHolder polyHolder{ (uint8_t)field_json["features"].size(), Poly::Point{0, 0}, {} };

    //Loop all polygons
    int p = 0;
    for (auto& polData : field_json["features"]) {
        //Loop multipolygon, except repeating last entry
        auto poller = polData["geometry"]["coordinates"][0];
        //Create a polygon struct
        polyHolder.polygons.push_back(Poly::PolygonStruct{ 
            (int8_t)polData["properties"]["value"].get<int>(),
            (uint8_t)(poller.size() - 1), {} });
        //Get amount
        for (int i = 0; i < (poller.size() - 1); i++) {
            polyHolder.polygons[p].poly_points.push_back(Poly::Point{ 
                (int32_t)(poller[i][0].get<double>() * 10000000),
                (int32_t)(poller[i][1].get<double>() * 10000000)
                });
        }
        p++;
    }
    return polyHolder;
}

//Find reference that makes all points positive (finds smallest x and y coordinate from field)
Poly::Point onlyPositiveRef(Poly::PolyHolder polys) {
    Poly::Point ref{ std::numeric_limits<int32_t>::max(), std::numeric_limits<int32_t>::max() };
    //Loop polygons
    for (Poly::PolygonStruct& pol : polys.polygons) {
        //Loop points
        for (Poly::Point& p : pol.poly_points) {
            ref.x = std::min(ref.x, p.x);
            ref.y = std::min(ref.y, p.y);
        }
    }
    return ref;
}

//Updates all points on a field (in a PolyHolder), when given a new reference
//Update reference. RealPoint = oldRef + oldPoint. Vil ha RealPoint = newRef + newPoint
//Får da newPoint = oldRef - newRef + oldPoint
void updateRef(Poly::PolyHolder & polys, Poly::Point ref) {
    //Loop polygons
    for (Poly::PolygonStruct& pol : polys.polygons) {
        //Loop points
        for (Poly::Point& p : pol.poly_points) {
            p.x = polys.ref.x - ref.x + p.x;
            p.y = polys.ref.y - ref.y + p.y;
        }
    }
    polys.ref = ref;
}


//Print info about polygons
/*
void print_poly_info(Poly::PolyHolder polys) {
    //Se på data, og skrive den
    std::cout << "Ref x: " << polys.ref.x << ", Ref y:" << polys.ref.y << std::endl << std::endl;
    for (Poly::PolygonStruct& pol : polys.polygons) {
        //Loop points
        for (Poly::Point& p : pol.poly_points) {
            std::cout << "x: " << p.x << ", y: " << p.y << std::endl;
        }
        std::cout << std::endl;
    }
}
*/

//Downgrades precicion by dividing by given divider
void downgradePrec(Poly::PolyHolder& polys, unsigned int divider) {
    //Loop polygons
    for (Poly::PolygonStruct& pol : polys.polygons) {
        //Loop points
        for (Poly::Point& p : pol.poly_points) {
            p.x /= divider;
            p.y /= divider;
        }
    }
    polys.ref.x /= divider;
    polys.ref.y /= divider;
}

//Denne funksjonen lager filen, som skal brukes på arduinoen
//Trenger at referansen er satt slik at vi har bare positive koordinater i punktene
//Krever også at punktene kan lagres på 16 bit
//Write for arduino
/*
    - Format for saving file.
    - Each file starts with number of polygons, then number of points and the points for each polygon.
    uint8: Number of polygons
    int32: Reference x_value
    int32: Reference y_value
    ----Repeated polygyns----
    uint8: Number of points
    int8: Extra data, (sowing mass example)
         ----Reapeated points----
         uint16: x coordinate
         uint16: y coordinate
*/
void arduWrite(Poly::PolyHolder polys, std::string path) {
    //Create file
    std::ofstream ardu_file(path, std::ios::binary);
    //Write number of polygons, and reference
    ardu_file.write(reinterpret_cast<char*>(&(polys.poly_num)), sizeof(polys.poly_num));
    ardu_file.write(reinterpret_cast<char*>(&(polys.ref.x)), sizeof(polys.ref.x));
    ardu_file.write(reinterpret_cast<char*>(&(polys.ref.y)), sizeof(polys.ref.y));
    //Loop polygons
    //Loop polygons
    for (Poly::PolygonStruct& pol : polys.polygons) {
        //Write number of points and mass data (value)
        ardu_file.write(reinterpret_cast<char*>(&(pol.num_points)), sizeof(pol.num_points));
        ardu_file.write(reinterpret_cast<char*>(&(pol.value)), sizeof(pol.value));
        //Loop points
        for (Poly::Point& p : pol.poly_points) {
            //Cast from 32 to 16 bit
            uint16_t x = (uint16_t)p.x;
            uint16_t y = (uint16_t)p.y;
            //Write points to file, x than y
            ardu_file.write(reinterpret_cast<char*>(&x), sizeof(x));
            ardu_file.write(reinterpret_cast<char*>(&y), sizeof(y));
        }
    }
    //Close file
    ardu_file.close();
}

//Bare for å avslutte konsollprogrammet
void GuiFinish() { std::cout << "Trykke en knapp for a avslutte..." << std::endl; std::cin.get();}

//Main-function
// Must have GeoJson-filename and generated-filename as arguments
int main(int argc, char* argv[])
{

    // Sjekke om vi fikk argumenter
    if (argc == 3) {

        try {
            //Argumenter til string
            std::string pathRead{ argv[1] };
            std::string pathWrite{ argv[2] };

            //Read data
            Poly::PolyHolder polys = readGeoJson(pathRead); //Read geo-data

            //Behandle data, lage positiv referansetype, og senke nøyaktighet til 6 desimaler
            updateRef(polys, onlyPositiveRef(polys));
            downgradePrec(polys, 10);

            //Write binary file
            arduWrite(polys, pathWrite);

            std::cout << "Lagde filen: " << pathWrite << ". Anbefaler a sjekke at filen ikke er 0 bytes, for da er noe galt." << std::endl;
        } catch(...) {
            std::cout << "Noe gikk alt! Sjekk at du skrev riktig filnavn, eller om det er andre problemer..." << std::endl;
        }

        //Pause
        GuiFinish();

        return 0;
    }
    else {
        std::cout << "Programmet krever at du gir filplassering og nytt filnavn som argumenter." << std::endl;
    }

    //Pause
    GuiFinish();

    return 1;
}
