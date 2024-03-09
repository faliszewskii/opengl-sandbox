//
// Created by faliszewskii on 09.03.24.
//

#ifndef OPENGL_SANDBOX_BEZIERC0_H
#define OPENGL_SANDBOX_BEZIERC0_H

#include <vector>
#include "Point.h"

class BezierC0 {
    std::vector<std::reference_wrapper<Point>> controlPoints;
public:
    std::string name;
    bool selected;
    int id;

    BezierC0() : name("Bezier C0"), selected(false), id(IdCounter::nextId()) {

    }
};

#endif //OPENGL_SANDBOX_BEZIERC0_H
