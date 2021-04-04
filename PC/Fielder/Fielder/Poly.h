#pragma once
#ifndef POLY_H
#define POLY_H

#include <cstdint>
#include <vector>

namespace Poly {
    //Saving a point
    struct Point {
        int32_t x;
        int32_t y;
    };

    //Polygon struct
    struct PolygonStruct {
        int8_t value;
        uint8_t num_points;
        std::vector<Point> poly_points;
    };

    //Holding all polygons
    struct PolyHolder {
        uint8_t poly_num;
        Point ref;
        std::vector<PolygonStruct> polygons;
    };

    //Check if horizontal line to the right of point ref, crosses line between to other points, where p1.x is smaller than p2.x
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
            int32_t diff_r = (p2.x - p1.x) * (p1.y - ref.y);
            int32_t x_left;

            //Already knows that ref.x is smaller than p2.x, so right x value is ref.x
            //Find our left x value
            if (ref.x > p1.x) x_left = ref.x;
            else x_left = p1.x;

            int32_t res_left = diff_r + (p2.y - p1.y) * (x_left - p1.x);
            int32_t res_right = diff_r + (p2.y - p1.y) * (p2.x - p1.x);
            //Do the actual math
            if ((res_left <= 0 && res_right >= 0) || (res_left >= 0 && res_right <= 0)) {
                crossed = true;
            }
        }
        return crossed;
    }

    //Save a polygon
    class Polygon {
    public:
        Point* poly_points;
        int poly_size;
        int value; //For knowing how much sowing to do at this spot
        //Initialize with known polygon size
        Polygon(char num_points, char mass) {
            poly_size = num_points;
            poly_points = new Point[poly_size];
            value = mass;
        }
        //Delete data used
        ~Polygon() {
            delete[] poly_points;
        }
        //Check if point is inside polygon
        bool point_inside(Point& ref) {
            int crossings = 0;
            //Loop over point array, and check if we are crossing
            crossings += CrossLine(ref, poly_points[0], poly_points[poly_size - 1]);
            for (int i = 1; i < poly_size; i++) {
                //Add crossing to count
                crossings += CrossLine(ref, poly_points[i - 1], poly_points[i]); //poly_points[i];
            }
            return (crossings % 2) == 1;
        }
    };
}

#endif