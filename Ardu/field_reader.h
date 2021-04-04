#pragma once
#ifndef FIELD_READER
#define FIELD_READER

#include <stdint.h>
#include <SD.h>
#include "light_smarpointer.h"

namespace myArdu {

    //Saving a point
    struct Point {
        uint16_t x;
        uint16_t y;
    };

    //Saving long point
    struct Point32 {
        int32_t x;
        int32_t y;
    };

    //Check if horizontal line to the right of point ref, crosses line between to other points
    bool CrossLine(Point& ref, Point p1, Point p2) {
        //Change order if neccecary
        Point temp = p1;
        if (p1.x > p2.x) {
            temp = p2;
            p2 = p1;
            p1 = temp;
        }
        //Start checking
        bool crossed = false;
        //First check that ref is to the left of p2, and between the y_values
        if (ref.x <= p2.x && ((p1.y < ref.y && p2.y >= ref.y) || (p1.y >= ref.y && p2.y < ref.y)) && p1.y != p2.y) {
            int32_t diff_r = ((int32_t)p2.x - (int32_t)p1.x) * ((int32_t)p1.y - (int32_t)ref.y);
            int32_t x_left;

            //Already knows that ref.x is smaller than p2.x, so right x value is ref.x
            //Find our left x value
            if (ref.x > p1.x) x_left = ref.x;
            else x_left = p1.x;

            int32_t res_left = diff_r + ((int32_t)p2.y - (int32_t)p1.y) * (x_left - (int32_t)p1.x);
            int32_t res_right = diff_r + ((int32_t)p2.y - (int32_t)p1.y) * ((int32_t)p2.x - (int32_t)p1.x);
            //Do the actual math
            if ((res_left <= 0 && res_right >= 0) || (res_left >= 0 && res_right <= 0)) {
                crossed = true;
            }
        }
        return crossed;
    }

    //Polygon struct
    class PolygonStruct {
    private:
        //Save if there are someone who copied this, because then this will not destruct
        uint8_t add_count = 0;
        uint8_t num_points;
        shared_array<Point> poly_points;
    public:
        //Data value for this field
        int8_t value;

        //Constructor and destructor
        PolygonStruct() {}
        PolygonStruct(const uint8_t& num_points) : num_points(num_points) {
            poly_points = shared_array<Point>(new Point[num_points]);
        }

        // Add point
        void add_point(Point& point) {
            if (add_count < num_points) {
                poly_points[add_count] = point;
                ++add_count;
            }
        }

        //Check if point is inside polygon
        bool point_inside(Point& ref) {
            uint8_t crossings = 0;
            //Loop over point array, and check if we are crossing
            crossings += CrossLine(ref, poly_points[0], poly_points[num_points - 1]);
            for (uint8_t i = 1; i < num_points; i++) {
                //Add crossing to count
                crossings += CrossLine(ref, poly_points[i - 1], poly_points[i]); //poly_points[i];
            }
            return (crossings % 2) == 1;
        }
    };

    //Holding all polygons
    class PolyHolder {
    private:
        uint8_t add_count = 0;
        uint8_t poly_num = 0;
        int32_t ref_x;
        int32_t ref_y;
        shared_array<PolygonStruct> polygons;
        
    public:
        //Constructor and destructor
        PolyHolder() {};
        PolyHolder(uint8_t& poly_num, int32_t& ref_x, int32_t& ref_y) 
            : poly_num(poly_num), ref_x(ref_x), ref_y(ref_y) {
            polygons = shared_array<PolygonStruct>(new PolygonStruct[poly_num]);
        }

        //Add polygon
        void add_polygon(PolygonStruct& poly) {
            if (add_count < poly_num) {
                polygons[add_count] = poly;
                ++add_count;
            }
        }

        //Get value for point
        //Return 127 if point is not inside any polygon(not inside field)
        int8_t value(Point32& ref) {
            for (uint8_t i = 0; i < poly_num; i++) {
                Point shiftPoint = Point{ ref.x - ref_x, ref.y - ref_y };
                if (polygons[i].point_inside(shiftPoint)) {
                    return polygons[i].value;
                }
            }
            return 127;
        }
    };

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
        uint8: Extra data, (sowing mass example)
             ----Reapeated points----
             uint16: x coordinate
             uint16: y coordinate
    */

    //Needs file to open file first
    PolyHolder read_field(File& myFile) {
        //Get these values first
        uint8_t num_polys;
        int32_t ref_x;
        int32_t ref_y;
        // Read number of polygons
        num_polys = static_cast<uint8_t>(myFile.read());
        // Read reference x and reference y
        myFile.read(reinterpret_cast<char*>(&ref_x), sizeof(ref_x));
        myFile.read(reinterpret_cast<char*>(&ref_y), sizeof(ref_y));

        // Create polygon-holder
        PolyHolder polyHolder(num_polys, ref_x, ref_y);

        // Loop to read polygons
        for (uint8_t i = 0; i < num_polys; i++) {
            // Read number of points
            uint8_t num_points;
            num_points = static_cast<uint8_t>(myFile.read());

            //Creat polygon
            PolygonStruct poly(num_points);

            //Read value
            poly.value = static_cast<int8_t>(myFile.read());

            //Loop over points
            for (uint8_t h = 0; h < num_points; h++) {
                //Create point
                Point point;
                //Read to point
                myFile.read(reinterpret_cast<char*>(&point.x), sizeof(point.x));
                myFile.read(reinterpret_cast<char*>(&point.y), sizeof(point.y));

                //Add point to polygon
                poly.add_point(point);
            }

            //Add polygon to holder
            polyHolder.add_polygon(poly);
        }

        //Do a little check if we succeded
        if (myFile.position() != myFile.size()) {
            Serial.println("Avlesningfeil");
        }
        return polyHolder;
    }
}

#endif // !FIELD_READER
