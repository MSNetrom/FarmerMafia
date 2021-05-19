#pragma once
#ifndef POLY_H
#define POLY_H

/*
    #### INFORMASJON ####

    Denne fila inneholder strukturer som holder på polygondataen for en åker
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
        int8_t value; // Såmengde/gjødselsmengde
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