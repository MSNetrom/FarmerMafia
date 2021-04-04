// Fielder.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <iomanip>
#include <fstream>
#include <cstdint>
#include <limits>
#include <algorithm>
#include <climits>
#include "Poly.h"
#include "json.hpp"


//Namespace for json
using json = nlohmann::json;

//[[[[ 11.17582596288425200726, 63.68808994925201716342 ], [11.17807487097943131005, 63.68812322121714686318], [11.17705832779379448993, 63.68637085716464696361], [11.17457989263179385375, 63.686756977865265128], [11.17582596288425200726, 63.68808994925201716342]] ] ]

//Read data to polystruct
Poly::PolyHolder SMSreadGeoJson(std::string path) {
    //Make file and json ready
    //Read geojson file
    std::ifstream field_file(path);
    json field_json;
    if (field_file.is_open()) {
        field_json = json::parse(field_file);
    }
    else {
        std::cout << "Kunne ikke apne fil. Brukte du riktig filnavn/filsti?" << std::endl;
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
            (int8_t)polData["properties"]["Tgt_Rate_k"].get<int>(),
            (uint8_t)(poller.size() - 1), {} });
        //Get amount
        //std::cout << polData["properties"]["Tgt_Rate_k"] << std::endl;
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

//Find reference that makes all points positive
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


//Print some poly data
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

//Downgrade precicion by dividing by given
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

void GuiFinish() { std::cout << "Trykke en knapp for a avslutte..." << std::endl; std::cin.get();}

int main()
{
    //Check if system is 8 bit
    std::cout << "System is running on " << CHAR_BIT << " bits per char." << std::endl;

    if (CHAR_BIT != 8) { std::cout << "Systemet må kjøre på 8 bit per char. Avslutter..." << std::endl; GuiFinish(); }

    //Read data
    std::string pathRead = "plassen.geojson";
    Poly::PolyHolder polys = SMSreadGeoJson(pathRead); //Read geo-data generated by SMS Advanced agleader

    //Behandle data, lage positiv referansetype, og senke nøyaktighet til 6 desimaler
    updateRef(polys, onlyPositiveRef(polys));
    downgradePrec(polys, 10);

    //Write binary file
    std::string pathWrite = "plassen.sve";
    arduWrite(polys, pathWrite);

    std::cout << "Lagde filen: " << pathWrite << ", med suksse. Da er det bare a sa." << std::endl;
    //Pause
    GuiFinish();
    
    return 0;
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
