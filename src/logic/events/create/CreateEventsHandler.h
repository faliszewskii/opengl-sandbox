//
// Created by faliszewskii on 21.03.24.
//

#ifndef OPENGL_SANDBOX_CREATEEVENTSHANDLER_H
#define OPENGL_SANDBOX_CREATEEVENTSHANDLER_H

#include "../../state/AppState.h"
#include "CreateGregoryPatchEvent.h"

namespace CreateEventsHandler {

    template<typename T>
    inline void addPointToCurve(Point &point, EntityType &curveET) {
        if(holds_alternative<std::reference_wrapper<T>>(curveET)) {
            T &curve = std::get<std::reference_wrapper<T>>(curveET);
            curve.addPoint(point);
        }
    }

    template<typename T>
    inline void createCurve(AppState &appState, std::map<int, std::unique_ptr<T>> &map) {
        auto &selectedEntities = appState.selectedEntities;
        auto curve = std::make_unique<T>();
        appState.lastIdCreated = curve->id;
        auto result = map.emplace(curve->id, std::move(curve));

        // Add all currently selected points;
        for(auto &el : selectedEntities) { // TODO Consider separating selected entities into object types.
            if(std::holds_alternative<std::reference_wrapper<Point>>(el.second)) {
                Point &point = std::get<std::reference_wrapper<Point>>(el.second);
                result.first->second->addPoint(point);
            }
        }
    }

    void setup(AppState &appState) {
        auto &eventPublisher = appState.eventPublisher;
        auto &selectedEntities = appState.selectedEntities;

        eventPublisher.subscribe([&](const CreateTorusEvent &event){
            auto torus = std::make_unique<Torus>(event.position);
            appState.lastIdCreated = torus->id;
            appState.torusSet.emplace(torus->id, std::move(torus));
        });

        eventPublisher.subscribe([&](const CreatePointEvent &event) {
            auto pointTemp = std::make_unique<Point>(event.position);
            auto result = appState.pointSet.emplace(pointTemp->id, std::move(pointTemp));
            auto &point = *result.first->second;
            if(!selectedEntities.empty()) { // TODO Move this to entirely different handler for bezier that is called on PointCreatedEvent.
                auto &last = (selectedEntities.rbegin())->second;
                addPointToCurve<BezierC0>(point, last);
                addPointToCurve<BezierC2>(point, last);
                addPointToCurve<InterpolatedC2>(point, last);
            }
            appState.lastIdCreated = point.id;
        });

        eventPublisher.subscribe([&](const CreateBezierC0Event &event) {
            createCurve(appState, appState.bezierC0Set);
        });

        eventPublisher.subscribe([&](const CreateBezierC2Event &event) {
            createCurve(appState, appState.bezierC2Set);
        });

        eventPublisher.subscribe([&](const CreateInterpolatedC2Event &event) {
            createCurve(appState, appState.interpolatedC2Set);
        });

        eventPublisher.subscribe([&](const CreateBezierPatch &event) {

            std::vector<PositionVertex> vertices;
            for(auto &pointId : event.controlPoints) {
                auto &point = *appState.pointSet[pointId];
                vertices.push_back({point.position});
            }

            std::variant<std::monostate, std::reference_wrapper<std::unique_ptr<PatchC0>>, std::reference_wrapper<std::unique_ptr<PatchC2>>> ref;
            if(!event.C2) {
                auto patch = std::make_unique<PatchC0>(vertices, event.patchIndices, event.gridIndices);
                auto result = appState.patchC0Set.emplace(patch->id, std::move(patch));
                result.first->second->patchCountX = event.patchCountX;
                result.first->second->patchCountY = event.patchCountY;
                result.first->second->wrapped = event.wrapped;
                ref = result.first->second;
                appState.lastIdCreated = result.first->second->id;
            } else {
                auto patch = std::make_unique<PatchC2>(vertices, event.patchIndices, event.gridIndices);
                auto result = appState.patchC2Set.emplace(patch->id, std::move(patch));
                result.first->second->patchCountX = event.patchCountX;
                result.first->second->patchCountY = event.patchCountY;
                result.first->second->wrapped = event.wrapped;
                ref = result.first->second;
                appState.lastIdCreated = result.first->second->id;
            }

            for(auto &pointId : event.controlPoints) {
                auto &point = *appState.pointSet[pointId];
                std::visit(overloaded{
                    [&](std::unique_ptr<PatchC0> &patch) {
                        patch->controlPoints.emplace_back(point.id, point);
                    },
                    [&](std::unique_ptr<PatchC2> &patch) {
                        patch->controlPoints.emplace_back(point.id, point);
                    },
                    [&](std::monostate _){}
                }, ref);
            }
        });

        eventPublisher.subscribe([&](const CreateGregoryPatchEvent &event) {
            auto gregory = std::make_unique<GregoryPatch>(
                    appState,
                    event.creator.patchSides,
                    event.creator.p3is,
                    event.creator.p2is,
                    event.creator.p1is,
                    event.creator.p0,
                    event.creator.fiSide,
                    event.creator.fiMiddle,
                    event.creator.fiLast
                    );
            appState.lastIdCreated = gregory->id;
            appState.gregoryPatchSet.emplace(gregory->id, std::move(gregory));
        });

        appState.eventPublisher.subscribe([&](const CreateIntersectionEvent &event) {
            // TODO DEBUG
            // for(int i = 0; i < event.intersectionPoints.size(); i++) {
                // float ratio = static_cast<float>(i) / event.intersectionPoints.size();
                // appState.eventPublisher.publish(CreatePointEvent{event.intersectionPoints[i]});
                // appState.pointSet[appState.lastIdCreated]->color = glm::vec4(ratio, ratio, ratio, 1);
            // }
            auto intersection = std::make_unique<Intersection>(event.intersectionPoints, event.surfaces, event.wrapped);
            appState.lastIdCreated = intersection->id;
            appState.intersectionSet.emplace(intersection->id, std::move(intersection));
        });
    }
}

#endif //OPENGL_SANDBOX_CREATEEVENTSHANDLER_H
