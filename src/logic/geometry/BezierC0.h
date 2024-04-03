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
    int id;
    std::string name;
    bool selected;

    std::vector<std::pair<int, std::reference_wrapper<Point>>> controlPoints;
    bool drawPolyline;
    int adaptationMultiplier=20;
    int instanceCount=10;

    BezierC0() : id(IdCounter::nextId()), name("Bezier C0 ("+std::to_string(id)+")"), selected(false), mesh({},{},GL_PATCHES),
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
        for(int i = 3; i < s; i += 3) {
            indices.push_back(i-3);
            indices.push_back(i-2);
            indices.push_back(i-1);
            indices.push_back(i);
        }
        int remainder = (int)(controlPoints.size()-1) % 3 + 1;
        for(int i=0; i< remainder; i++)
            indices.push_back(s - remainder + i);
        for(int i=remainder; i<4; i++)
            indices.push_back(s-1);

        mesh.update(std::move(vertices), std::move(indices));
    }

    void render(Shader &shader) {
        glLineWidth(2);
        shader.setUniform("fullCurves", (int)(controlPoints.size() - 1)/3);
        shader.setUniform("remainder", (int)((controlPoints.size()-1) % 3) + 1);
        shader.setUniform("division", instanceCount);
        shader.setUniform("adaptationMultiplier", adaptationMultiplier);
        glPatchParameteri(GL_PATCH_VERTICES, 4);
        mesh.render(instanceCount);
        glLineWidth(1);
    }
};

#endif //OPENGL_SANDBOX_BEZIERC0_H
