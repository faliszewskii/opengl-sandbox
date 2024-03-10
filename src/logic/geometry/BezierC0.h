//
// Created by faliszewskii on 09.03.24.
//

#ifndef OPENGL_SANDBOX_BEZIERC0_H
#define OPENGL_SANDBOX_BEZIERC0_H

#include <vector>
#include <algorithm>
#include "Point.h"
#include "../vertices/PositionVertex.h"

class BezierC0 {
    Mesh<PositionVertex> mesh;
public:
    std::string name;
    bool selected;
    int id;

    bool drawPolyline;
    int segmentCount;
    std::vector<std::pair<int, std::reference_wrapper<Point>>> controlPoints;

    BezierC0() : name("Bezier C0"), selected(false), id(IdCounter::nextId()), mesh({},{},GL_LINES_ADJACENCY),
        segmentCount(255), drawPolyline(false) {}

    bool pointAlreadyAdded(Point &point) {
        return std::ranges::any_of(controlPoints, [&](auto &controlPoint){return controlPoint.first == point.id; });
    }

    void addPoint(Point &newPoint) { // TODO Add reaction to the event of deleting a point from pointSet.
        if(pointAlreadyAdded(newPoint)) return;
        controlPoints.emplace_back(newPoint.id, newPoint);
        updateMesh();
    }

    void removePoint(int i) {
        controlPoints.erase(controlPoints.begin() + i);
        updateMesh();
    }

    void updatePoint(Point &point, int i) {
        mesh.update({point.position}, i);
    }

    void updateMesh() {
        std::vector<PositionVertex> vertices;
        vertices.reserve(controlPoints.size());
        for(auto &point : controlPoints) {
            vertices.emplace_back(point.second.get().position);
        }
        std::vector<unsigned int> indices;
        for(int i = 3; i < controlPoints.size(); i += 3) {
            indices.push_back(i-3);
            indices.push_back(i-2);
            indices.push_back(i-1);
            indices.push_back(i);
        }
        mesh.update(std::move(vertices), std::move(indices));
    }

    void render(Shader &shader) {
        glLineWidth(2);
        shader.setUniform("selected", selected);
        // TODO Calculate the Bernstein polygon projection on the screen and calculate segmentCount := (2*h + 2*w).
        // TODO Display debug data on GUI. (w, h, segmentCount)
        shader.setUniform("segmentCount", segmentCount); // TODO Adaptive segment count.
        mesh.render();
        glLineWidth(1);
    }
};

#endif //OPENGL_SANDBOX_BEZIERC0_H