//
// Created by faliszewskii on 14.03.24.
//

#ifndef OPENGL_SANDBOX_BEZIERC2_H
#define OPENGL_SANDBOX_BEZIERC2_H

#include <vector>
#include <algorithm>
#include "Point.h"
#include "../vertices/PositionVertex.h"

class BezierC2 {
    Mesh<PositionVertex> mesh;
public:
    std::string name;
    bool selected;
    int id;

    bool drawPolyline;
    int adaptationMultiplier=20;
    std::vector<std::pair<int, std::reference_wrapper<Point>>> controlPoints;

    BezierC2() : name("Bezier C2"), selected(false), id(IdCounter::nextId()), mesh({},{},GL_PATCHES),
                 drawPolyline(false) {}

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
        int s = controlPoints.size();
        for(int i = 0; i < s; i ++) {
            indices.push_back(i-2>=0? i-2 : 0);
            indices.push_back(i-1>=0? i-1 : 0);
            indices.push_back(i);
            indices.push_back(i+1<s? i+1 : s-1);
            indices.push_back(i+2<s? i+2 : s-1);
        }
        mesh.update(std::move(vertices), std::move(indices));
    }

    void render(Shader &shader) {
        glLineWidth(2);
        shader.setUniform("adaptationMultiplier", adaptationMultiplier);
        glPatchParameteri(GL_PATCH_VERTICES, 5);
        mesh.render();
        glLineWidth(1);
    }
};


#endif //OPENGL_SANDBOX_BEZIERC2_H
