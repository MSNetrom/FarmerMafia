#pragma once
#ifndef POLY_H
#define POLY_H

/*
    #### INFORMASJON ####

    Denne fila inneholder strukturer som holder p� polygondataen for en �ker
*/

#include <cstdint>
#include <vector>

namespace Poly {
    //Holding a point
    struct Point {
        int32_t x;
        int32_t y;
    };

    //Holding all points in a polygon
    struct PolygonStruct {
        int8_t value; // S�mengde/gj�dselsmengde
        uint8_t num_points;
        std::vector<Point> poly_points;
    };

    //Holding all polygons from field
    struct PolyHolder {
        uint8_t poly_num;
        Point ref;
        std::vector<PolygonStruct> polygons;
    };
}

#endif