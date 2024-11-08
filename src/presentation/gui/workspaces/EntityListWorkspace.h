//
// Created by faliszewskii on 03.03.24.
//

#ifndef OPENGL_SANDBOX_ENTITYLISTWORKSPACE_H
#define OPENGL_SANDBOX_ENTITYLISTWORKSPACE_H

#include <any>

#include "../../../logic/state/AppState.h"
#include "../../../logic/concepts/has_name.h"
#include "../../../logic/events/selection/SelectEntityEvent.h"
#include "../../../logic/events/create/CreateBezierC0Event.h"
#include "../../../logic/events/point/PointMovedEvent.h"
#include "../../../logic/geometry/InterpolatedC2.h"
#include <glm/gtx/euler_angles.hpp>
#include <variant>

#include "../../../logic/algorithms/FloodFill.h"

namespace EntityListWorkspace {

    void renderWorkspaceTorus(Torus &torus, AppState &appState);
    bool renderWorkspaceTransform(Transformation &transform);
    void renderWorkspacePoint(Point &point, AppState &appState);
    void renderWorkspaceBezierC0(BezierC0 &bezier, AppState &appState);
    void renderWorkspaceBezierC2(BezierC2 &bezier, AppState &appState);
    void renderWorkspaceInterpolatedC2(InterpolatedC2 &interpolated, AppState &appState);
    void renderWorkspacePatchC0(PatchC0 &patch, AppState &appState);
    void renderWorkspacePatchC2(PatchC2 &patch, AppState &appState);
    void renderWorkspaceGregory(GregoryPatch &patch, AppState &appState);
    void renderWorkspaceIntersection(Intersection &intersection, AppState &appState);
    void renderWorkspaceMultiple(AppState &appState);
    void collapsePoint(AppState &appState);
    template<typename T>
    void swapPoint(Point &oldPoint, Point &newPoint, std::map<int, std::unique_ptr<T>> &map);

    template<typename T> requires has_name<T>
    void renderNameInput(T &el);
    template<typename T> requires has_name<T> && has_id<T>
    void renderListing(std::map<int, std::unique_ptr<T>> &list, AppState &appState, int contextLevel);
    void renderWorkspaceSelected(AppState &appState);

    void renderDeleteButton(AppState &appState);

    inline void render(AppState &appState) {
        if (ImGui::Button("Add Torus"))
            appState.eventPublisher.publish(CreateTorusEvent{appState.cursorPosition});
        ImGui::SameLine();
        if (ImGui::Button("Add Point"))
            appState.eventPublisher.publish(CreatePointEvent{appState.cursorPosition});
        ImGui::SameLine();
        if (ImGui::Button("Add Bezier C0"))
            appState.eventPublisher.publish(CreateBezierC0Event{});
        if (ImGui::Button("Add Bezier C2"))
            appState.eventPublisher.publish(CreateBezierC2Event{});
        ImGui::SameLine();
        if (ImGui::Button("Add Interpolated C2"))
            appState.eventPublisher.publish(CreateInterpolatedC2Event{});;
        if (ImGui::Button("Create Patch")) {
            appState.bezierCreatorOpen = true;
            appState.bezierPatchCreator.reset();
        }

        renderDeleteButton(appState);

        if(ImGui::CollapsingHeader("Scene objects", ImGuiTreeNodeFlags_DefaultOpen)) {
            if (ImGui::BeginListBox("Objects#Workspace", ImVec2(-FLT_MIN, 0))) {
                renderListing(appState.torusSet, appState, 0);
                renderListing(appState.pointSet, appState, 0);
                renderListing(appState.bezierC0Set, appState, 0);
                renderListing(appState.bezierC2Set, appState, 0);
                renderListing(appState.interpolatedC2Set, appState, 0);
                renderListing(appState.patchC0Set, appState, 0);
                renderListing(appState.patchC2Set, appState, 0);
                renderListing(appState.gregoryPatchSet, appState, 0);
                renderListing(appState.intersectionSet, appState, 0);
                ImGui::EndListBox();
            }
        }
        renderWorkspaceSelected(appState);
    }

    inline void renderDeleteButton(AppState &appState) {
        bool disabled = appState.selectedEntities.empty();
        for(auto &el : appState.selectedEntities) {
            std::visit(overloaded{
                    [&](std::reference_wrapper<Point> &point) {
                        for (auto &patch: appState.patchC2Set) {
                            for (int i = 0; i < patch.second->controlPoints.size(); i++)
                                if (patch.second->controlPoints[i].first == point.get().id)
                                    disabled = true;
                        }
                        for (auto &patch: appState.patchC0Set) {
                            for (int i = 0; i < patch.second->controlPoints.size(); i++)
                                if (patch.second->controlPoints[i].first == point.get().id)
                                    disabled = true;
                        }
                        },
                        [&](auto &_) {}
                    }, el.second);
        }
        ImGui::BeginDisabled(disabled);
        if (ImGui::Button("Delete")) {
            for(auto &el : appState.selectedEntities) {
                std::visit(overloaded{
                       [&](Torus &torus) { appState.torusSet.erase(appState.torusSet.find(torus.id)); },
                       [&](Point &point) {
                           int id = point.id;
                           appState.pointSet.erase(appState.pointSet.find(id));
                           appState.eventPublisher.publish(PointDeletedEvent{id});
                       },
                       [&](BezierC0 &bezier) { appState.bezierC0Set.erase(appState.bezierC0Set.find(bezier.id)); },
                       [&](BezierC2 &bezier) { appState.bezierC2Set.erase(appState.bezierC2Set.find(bezier.id)); },
                       [&](InterpolatedC2 &interpolated) { appState.interpolatedC2Set.erase(appState.interpolatedC2Set.find(interpolated.id)); },
                       [&](PatchC0 &patch) {
//                           std::unordered_set<int> pointToDelete;
//                           for(auto &point : patch.controlPoints) {
//                               pointToDelete.insert(point.second.get().id);
//                           }
//                           for(auto &id: pointToDelete) {
//                               appState.pointSet.erase(appState.pointSet.find(id));
//                               appState.eventPublisher.publish(PointDeletedEvent{id});
//                           }

                           std::vector<int> gregories;
                           for(auto &gregory : appState.gregoryPatchSet) {
                               bool toDelete = false;
                               for(auto &side : gregory.second->patchSides)
                                   for(auto &v : side)
                                       toDelete |= v.patchId == patch.id;
                               if(toDelete) {
                                   gregories.emplace_back(gregory.first);
                               }
                           }
                           for(auto &g : gregories) {
                               appState.gregoryPatchSet.erase(appState.gregoryPatchSet.find(g));
                               appState.gregoryPatchCreator.reset();
                           }
                           appState.patchC0Set.erase(appState.patchC0Set.find(patch.id));
                       },
                       [&](PatchC2 &patch) {
//                           for(auto &point : patch.controlPoints) {
//                               int id = point.second.get().id;
//                               appState.pointSet.erase(appState.pointSet.find(id));
//                               appState.eventPublisher.publish(PointDeletedEvent{id});
//                           }
                           appState.patchC2Set.erase(appState.patchC2Set.find(patch.id));
                       },
                       [&](GregoryPatch &patch) {
                           appState.gregoryPatchSet.erase(appState.gregoryPatchSet.find(patch.id));
                       },
                        [&](Intersection &intersection) {
                            for(auto &pSurface : intersection.surfaces) {
                                IntersectableSurface surface = pSurface.first;
                                std::visit(overloaded{
                                    [&](auto &s) {
                                        s.get().clearMask();
                                        for(auto &pOtherIntersection : appState.intersectionSet) {
                                            if(pOtherIntersection.first == intersection.id)  continue;
                                            Intersection &otherIntersection = *pOtherIntersection.second;
                                            if(std::visit(overloaded{[&](auto &e){ return e.get().id == s.get().id;}}, otherIntersection.surfaces[0].first))
                                                appState.surfaceIntersection.addToMask(surface, otherIntersection, 0);
                                            if(std::visit(overloaded{[&](auto &e){ return e.get().id == s.get().id;}}, otherIntersection.surfaces[1].first))
                                                appState.surfaceIntersection.addToMask(surface, otherIntersection, 1);
                                        }
                                    }
                                }, surface);
                            }
                            appState.intersectionSet.erase(appState.intersectionSet.find(intersection.id));
                        }
                }, el.second);
            }
            appState.selectedEntities.clear(); // TODO Move to events.
        }
        ImGui::EndDisabled();
    }

    inline void renderWorkspaces(AppState &appState, EntityType element) {
        visit(overloaded{
                [&](Torus &torus) { renderWorkspaceTorus(torus, appState); },
                [&](Point &point) { renderWorkspacePoint(point, appState); },
                [&](BezierC0 &bezier) { renderWorkspaceBezierC0(bezier, appState); },
                [&](BezierC2 &bezier) { renderWorkspaceBezierC2(bezier, appState); },
                [&](InterpolatedC2 &interpolated) { renderWorkspaceInterpolatedC2(interpolated, appState); },
                [&](PatchC0 &patch) { renderWorkspacePatchC0(patch, appState); },
                [&](PatchC2 &patch) { renderWorkspacePatchC2(patch, appState); },
                [&](GregoryPatch &patch) { renderWorkspaceGregory(patch, appState); },
                [&](Intersection &intersection) { renderWorkspaceIntersection(intersection, appState); }
        }, element);
    }

    inline void renderWorkspaceSelected(AppState &appState) {
        if(ImGui::BeginChild("##SelectedEntities")) {
            // Render workspaces for context.
            for(auto &context : appState.selectionContext)
                renderWorkspaces(appState, context.second);
            auto &selected = appState.selectedEntities;
            // Flavour text for empty selection.
            if (selected.empty())
                ImGui::SeparatorText("Select entities");
            else if (selected.size() == 1) {
                // Render workspace for one object.
                renderWorkspaces(appState, selected.begin()->second);
            } else {
                // Render workspace for multiple selection/
                renderWorkspaceMultiple(appState);
            }
            ImGui::EndChild();
        }
    }

    inline void renderWorkspaceBezierC0(BezierC0 &bezier, AppState &appState) {
        int idCounter = 0;
        ImGui::SeparatorText("Bezier C0");
        renderNameInput(bezier);

        ImGui::DragInt("Adaptation Multiplier", &bezier.adaptationMultiplier);
        ImGui::Checkbox("Draw Polyline", &bezier.drawPolyline);
        ImGui::SeparatorText("Control Points");
        if (ImGui::BeginListBox("Control points#Workspace", ImVec2(-FLT_MIN, 0))) {
            for(auto &pPoint : bezier.controlPoints) {
                Point &point = pPoint.second;
                auto &entities = appState.selectedEntities;
                if (ImGui::Selectable((point.name + "##" + std::to_string(idCounter++)).c_str(), entities.end() != std::find_if(entities.begin(), entities.end(), [&](auto &e){ return e.first == point.id;}))) {
                    appState.eventPublisher.publish(SelectEntityEvent{point, 1});
                }
            }
            ImGui::EndListBox();
        }
    }

    inline void renderWorkspaceBezierC2(BezierC2 &bezier, AppState &appState) { // TODO Extreme code duplication from above
        int idCounter=0;
        ImGui::SeparatorText("Bezier C2");
        renderNameInput(bezier);

        ImGui::DragInt("Adaptation Multiplier", &bezier.adaptationMultiplier);
        ImGui::Checkbox("Draw Polyline", &bezier.drawPolyline);
        ImGui::SameLine();
        ImGui::Checkbox("Draw Bernstein", &bezier.drawBernstein);
        ImGui::SeparatorText("Control Points");
        if (ImGui::BeginListBox("Control points#Workspace", ImVec2(-FLT_MIN, 0))) {
            for(auto &pPoint : bezier.controlPoints) {
                Point &point = pPoint.second;
                auto &entities = appState.selectedEntities;
                if (ImGui::Selectable((point.name + "##" + std::to_string(idCounter++)).c_str(), entities.end() != std::find_if(entities.begin(), entities.end(), [&](auto &e){ return e.first == point.id;}))) {
                    appState.eventPublisher.publish(SelectEntityEvent{point, 1});
                }
            }
            ImGui::EndListBox();
        }
        if(bezier.drawBernstein) {
            ImGui::SeparatorText("Bernstein Points");
            if (ImGui::BeginListBox("Bernstein points#Workspace", ImVec2(-FLT_MIN, 0))) {
                for(auto &pPoint : bezier.bernsteinPoints) {
                    Point &point = *pPoint;
                    auto &entities = appState.selectedEntities;
                    if (ImGui::Selectable((point.name + "##" + std::to_string(idCounter++)).c_str(), entities.end() != std::find_if(entities.begin(), entities.end(), [&](auto &e){ return e.first == point.id;}))) {
                        appState.eventPublisher.publish(SelectEntityEvent{point, 1});
                    }
                }
                ImGui::EndListBox();
            }
        }
    }

    inline void renderWorkspaceInterpolatedC2(InterpolatedC2 &interpolated, AppState &appState) {
        int idCounter = 0;
        ImGui::SeparatorText("Interpolated C0");
        renderNameInput(interpolated);

        ImGui::DragInt("Adaptation Multiplier", &interpolated.adaptationMultiplier);
        ImGui::SeparatorText("Control Points");
        if (ImGui::BeginListBox("Control points#Workspace", ImVec2(-FLT_MIN, 0))) {
            for(auto &pPoint : interpolated.controlPoints) {
                Point &point = pPoint.second;
                auto &entities = appState.selectedEntities;
                if (ImGui::Selectable((point.name + "##" + std::to_string(idCounter++)).c_str(), entities.end() != std::find_if(entities.begin(), entities.end(), [&](auto &e){ return e.first == point.id;}))) {
                    appState.eventPublisher.publish(SelectEntityEvent{point, 1});
                }
            }
            ImGui::EndListBox();
        }
    }

    inline void renderWorkspacePatchC0(PatchC0 &patch, AppState &appState) {
        int idCounter = 0;
        ImGui::SeparatorText("Patch C0");
        renderNameInput(patch);
        ImGui::Text(("Wrapped: " + std::to_string(patch.wrapped)).c_str());
        ImGui::SeparatorText("Visualization");
        ImGui::Checkbox("Draw Bezier Grid", &patch.drawBezierGrid);
        bool modified = false;
        modified = ImGui::InputInt("Grid Count Width", &patch.bezierPatchGridWidth);
        if(modified && patch.bezierPatchGridWidth < 1) patch.bezierPatchGridWidth = 1;
        modified = ImGui::InputInt("Grid Count Length", &patch.bezierPatchGridLength);
        if(modified && patch.bezierPatchGridLength < 1) patch.bezierPatchGridLength = 1;

        ImGui::SeparatorText("Control Points");
        if (ImGui::BeginListBox("Control points#Workspace", ImVec2(-FLT_MIN, 0))) {
            for(auto &pPoint : patch.controlPoints) {
                Point &point = pPoint.second;
                auto &entities = appState.selectedEntities;
                if (ImGui::Selectable((point.name + "##" + std::to_string(idCounter++)).c_str(), entities.end() != std::find_if(entities.begin(), entities.end(), [&](auto &e){ return e.first == point.id;}))) {
                    appState.eventPublisher.publish(SelectEntityEvent{point, 1});
                }
            }
            ImGui::EndListBox();
        }
    }

    inline void renderWorkspacePatchC2(PatchC2 &patch, AppState &appState) {
        int idCounter = 0;
        ImGui::SeparatorText("Patch C2");
        renderNameInput(patch);

        ImGui::Text(("Wrapped: " + std::to_string(patch.wrapped)).c_str());
        ImGui::SeparatorText("Visualization");
        ImGui::Checkbox("Draw Bezier Grid", &patch.drawBezierGrid);
        bool modified = false;
        modified = ImGui::InputInt("Grid Count Width", &patch.bezierPatchGridWidth);
        if(modified && patch.bezierPatchGridWidth < 1) patch.bezierPatchGridWidth = 1;
        modified = ImGui::InputInt("Grid Count Length", &patch.bezierPatchGridLength);
        if(modified && patch.bezierPatchGridLength < 1) patch.bezierPatchGridLength = 1;

        ImGui::SeparatorText("Control Points");
        if (ImGui::BeginListBox("Control points#Workspace", ImVec2(-FLT_MIN, 0))) {
            for(auto &pPoint : patch.controlPoints) {
                Point &point = pPoint.second;
                auto &entities = appState.selectedEntities;
                if (ImGui::Selectable((point.name + "##" + std::to_string(idCounter++)).c_str(), entities.end() != std::find_if(entities.begin(), entities.end(), [&](auto &e){ return e.first == point.id;}))) {
                    appState.eventPublisher.publish(SelectEntityEvent{point, 1});
                }
            }
            ImGui::EndListBox();
        }
    }

    inline void renderWorkspaceGregory(GregoryPatch &patch, AppState &appState) {
        ImGui::SeparatorText("Gregory Patch");
        renderNameInput(patch);

        ImGui::InputInt("Grid Count Width", &patch.patchGridWidth);
        if(patch.patchGridWidth < 1) patch.patchGridWidth = 1;
        ImGui::InputInt("Grid Count Length", &patch.patchGridLength);
        if(patch.patchGridLength < 1) patch.patchGridLength = 1;

        ImGui::Checkbox("Draw Vectors", &patch.drawVectors);
    }

    inline void renderMaskImage(int textureId, std::array<std::array<glm::vec3, 256>, 256> &mask, ImVec2 size, AppState &appState, IntersectableSurface &s) {
        if(ImGui::ImageButton((void*)(intptr_t)textureId, size,  ImVec2(0, 0),  ImVec2(1, 1), 0)) {
            glm::vec2 coords =  {size.y - (ImGui::GetCursorScreenPos().y - ImGui::GetMousePos().y), ImGui::GetMousePos().x - ImGui::GetCursorScreenPos().x};
            coords.x *= mask.size() / size.x;
            coords.y *= mask[0].size() / size.y;
            glm::vec3 sourceColor = mask[coords.x][coords.y];
            if(sourceColor != glm::vec3(0) && sourceColor != glm::vec3(1)) return;
            glm::vec3 destColor = glm::abs(mask[coords.x][coords.y] - glm::vec3(1));
            floodFill(coords.x, coords.y, 0, mask.size(), 0, mask[0].size(),
                [&](int x, int y) {mask[x][y] = destColor;},
                [&](int x, int y) {return mask[x][y] == sourceColor;});
            std::visit([&](auto &el){ return el.get().setMask(mask);}, s);
        }
    }

    inline void renderWorkspaceIntersection(Intersection &intersection, AppState &appState) {
        ImGui::SeparatorText("Intersection");
        renderNameInput(intersection);
        if(ImGui::Button("Convert to interpolated curve")) {
            appState.eventPublisher.publish(CreateInterpolatedC2Event{});
            InterpolatedC2 &interpolated = *appState.interpolatedC2Set[appState.lastIdCreated];
            for(auto &point : intersection.points) {
                appState.eventPublisher.publish(CreatePointEvent{point});
                Point &newPoint = *appState.pointSet[appState.lastIdCreated];
                interpolated.addPoint(newPoint);
            }
            if(intersection.wrapped) {
                appState.eventPublisher.publish(CreatePointEvent{intersection.points.front()});
                Point &newPoint = *appState.pointSet[appState.lastIdCreated];
                interpolated.addPoint(newPoint);
            }
        }
        if(intersection.surfaces.size() == 1)
            std::visit([&](auto &e) {ImGui::Text("Self-intersection of %s", e.get().name);}, intersection.surfaces.front().first);
        else
            std::visit([&](auto &e1, auto &e2) {ImGui::Text("Intersection between %s and %s", e1.get().name.c_str(), e2.get().name.c_str());}, intersection.surfaces[0].first, intersection.surfaces[1].first);
        // TODO Self-intersection
        auto maskA = std::visit([&](auto &el){ return el.get().getMask();}, intersection.surfaces[0].first);
        auto maskB = std::visit([&](auto &el){ return el.get().getMask();}, intersection.surfaces[1].first);
        renderMaskImage(maskA.first.get().id, maskA.second.get(), ImVec2(256, 256), appState, intersection.surfaces[0].first);
        renderMaskImage(maskB.first.get().id, maskB.second.get(), ImVec2(256, 256), appState, intersection.surfaces[1].first);
    }

    template<typename T>
    void addPointToCurve(AppState &appState);

    inline void renderWorkspaceMultiple(AppState &appState) {
        ImGui::SeparatorText(("Selected " + std::to_string(appState.selectedEntities.size()) + " entities").c_str());
        bool modified = false;
        auto &centerTransform = appState.centerOfMassTransformation;

        collapsePoint(appState);
        // Because first element is Bézier curve we can add following points to it
        addPointToCurve<BezierC0>(appState);
        addPointToCurve<BezierC2>(appState);

        auto position = glm::vec3(centerTransform.getTranslationRef());
        ImGui::Text("Position:");
        modified |= ImGui::DragScalar("x##position", ImGuiDataType_Float, glm::value_ptr(position) + 0, 0.01f);
        modified |= ImGui::DragScalar("y##position", ImGuiDataType_Float, glm::value_ptr(position) + 1, 0.01f);
        modified |= ImGui::DragScalar("z##position", ImGuiDataType_Float, glm::value_ptr(position) + 2, 0.01f);

        auto angle = glm::vec3(centerTransform.getRotationAngles());
        ImGui::Text("Rotation:");
        modified |= ImGui::DragScalar("x##orientation", ImGuiDataType_Float, glm::value_ptr(angle) + 0, 0.01f);
        modified |= ImGui::DragScalar("y##orientation", ImGuiDataType_Float, glm::value_ptr(angle) + 1, 0.01f);
        modified |= ImGui::DragScalar("z##orientation", ImGuiDataType_Float, glm::value_ptr(angle) + 2, 0.01f);

        auto scale = glm::vec3(centerTransform.getScaleRef());
        ImGui::Text("Scale:");
        modified |= ImGui::DragScalar("x##scale", ImGuiDataType_Float, glm::value_ptr(scale) + 0, 0.001f);
        modified |= ImGui::DragScalar("y##scale", ImGuiDataType_Float, glm::value_ptr(scale) + 1, 0.001f);
        modified |= ImGui::DragScalar("z##scale", ImGuiDataType_Float, glm::value_ptr(scale) + 2, 0.001f);

        // TODO Podzielić na różne modified
        // TODO Event center of mass translated, rotated etc.
        if(modified) {
            auto translationDiff = position - centerTransform.getTranslationRef();
            auto angleDiff = angle - centerTransform.getRotationAngles();
            auto scaleRatio = scale / centerTransform.getScaleRef();

            centerTransform.setTranslation(position);
            centerTransform.setRotation(angle);
            centerTransform.setScale(scale);

            std::set<int> moved;
            for(auto &el : appState.selectedEntities) {
                std::visit(overloaded{
                        [&](Torus &torus) {
                            moved.emplace(torus.id); // Add addTranslation
                            torus.transform.setTranslation(torus.transform.translation + translationDiff);

                            torus.transform.setOrientation(glm::quat(angleDiff) * torus.transform.getOrientationRef());
                            torus.transform.setTranslation(centerTransform.translation +
                                glm::vec3(glm::eulerAngleX(angleDiff.x) * glm::vec4(torus.transform.translation-centerTransform.translation, 1)));
                            torus.transform.setTranslation(centerTransform.translation +
                                glm::vec3(glm::eulerAngleY(angleDiff.y) * glm::vec4(torus.transform.translation-centerTransform.translation, 1)));
                            torus.transform.setTranslation(centerTransform.translation +
                                glm::vec3(glm::eulerAngleZ(angleDiff.z) * glm::vec4(torus.transform.translation-centerTransform.translation, 1)));

                            torus.transform.setScale(torus.transform.scale * scaleRatio);
                            torus.transform.setTranslation(torus.transform.translation +
                                                           (torus.transform.translation - centerTransform.translation) * (scaleRatio - glm::vec3(1)));

                            for(auto &pIntersection : appState.intersectionSet) {
                                Intersection &intersection = *pIntersection.second;
                                if(std::visit(overloaded{[&](auto &e){ return e.get().id == torus.id;}}, intersection.surfaces[0].first) ||
                                    std::visit(overloaded{[&](auto &e){ return e.get().id == torus.id;}}, intersection.surfaces[1].first)) {
                                    for(auto &pSurface : intersection.surfaces) {
                                        IntersectableSurface surface = pSurface.first;
                                        std::visit(overloaded{
                                            [&](auto &s) {
                                                s.get().clearMask();
                                                for(auto &pOtherIntersection : appState.intersectionSet) {
                                                    if(pOtherIntersection.first == intersection.id)  continue;
                                                    Intersection &otherIntersection = *pOtherIntersection.second;
                                                    if(std::visit(overloaded{[&](auto &e){ return e.get().id == s.get().id;}}, otherIntersection.surfaces[0].first))
                                                        appState.surfaceIntersection.addToMask(surface, otherIntersection, 0);
                                                    if(std::visit(overloaded{[&](auto &e){ return e.get().id == s.get().id;}}, otherIntersection.surfaces[1].first))
                                                        appState.surfaceIntersection.addToMask(surface, otherIntersection, 1);
                                                }
                                            }
                                        }, surface);
                                    }
                                    appState.intersectionSet.erase(appState.intersectionSet.find(intersection.id));
                                }
                            }
                        },
                        [&](Point &point) {
                            if(moved.contains(point.id)) return;
                            moved.emplace(point.id);
                            glm::vec3 prevPosition = point.position;
                            point.position += translationDiff;
                            point.position += (point.position - centerTransform.translation) * (scaleRatio - glm::vec3(1));
                            point.position = centerTransform.translation + glm::vec3(glm::eulerAngleX(angleDiff.x) * glm::vec4(point.position-centerTransform.translation, 1));
                            point.position = centerTransform.translation + glm::vec3(glm::eulerAngleY(angleDiff.y) * glm::vec4(point.position-centerTransform.translation, 1));
                            point.position = centerTransform.translation + glm::vec3(glm::eulerAngleZ(angleDiff.z) * glm::vec4(point.position-centerTransform.translation, 1));
                            appState.eventPublisher.publish(PointMovedEvent{point, point.position - prevPosition});
                        },
                        [&](BezierC0 &_) { /* ignore */ },
                        [&](BezierC2 &_) { /* ignore */ },
                        [&](InterpolatedC2 &_) { /* ignore */ },
                        [&](PatchC0 &_) { /* ignore */ },
                        [&](PatchC2 &_) { /* ignore */ },
                        [&](GregoryPatch &_) { /* ignore */},
                        [&](Intersection &intersection) {
                            std::cout<<"test";
                        }

                }, el.second);
            }
        }
    }

    template<typename T>
    void addPointToCurve(AppState &appState) {
        if(std::holds_alternative<std::reference_wrapper<T>>(appState.selectedEntities.begin()->second)) {
            if(ImGui::Button("Add points to curve")) { // TODO Create Event Handler
                T &curve = std::get<std::reference_wrapper<T>>(appState.selectedEntities.begin()->second);
                for(auto &el : appState.selectedEntities)
                    if(std::holds_alternative<std::reference_wrapper<Point>>(el.second))
                        curve.addPoint(std::get<std::reference_wrapper<Point>>(el.second));
            }
        }
    }

    inline void collapsePoint(AppState &appState) {
        bool allPoint = std::all_of(appState.selectedEntities.begin(), appState.selectedEntities.end(),[](auto &el) {
            return std::holds_alternative<std::reference_wrapper<Point>>(el.second);
        });
        if(allPoint){
            if(ImGui::Button("Collapse points")) {
                appState.gregoryPatchCreator.reset();
                appState.eventPublisher.publish(CreatePointEvent{appState.centerOfMassTransformation.translation});
                auto &point = *appState.pointSet[appState.lastIdCreated];

                std::vector<int> toDelete;
                for(auto &gregory : appState.gregoryPatchSet) {
                    bool present = false;
                    for(auto &pOld : appState.selectedEntities) {
                        auto &old = std::get<std::reference_wrapper<Point>>(pOld.second).get();
                        for(auto &side : gregory.second->patchSides)
                            for(auto &v : side)
                                present |= v.pointId == old.id || v.outgoingBernstein[0] == old.id;
                    }
                    if(present)
                        toDelete.emplace_back(gregory.first);
                }
                for(auto &g : toDelete) {
                    appState.gregoryPatchCreator.reset();
                    appState.gregoryPatchSet.erase(appState.gregoryPatchSet.find(g));
                }

                for(auto &pOld : appState.selectedEntities) {
                    auto &old = std::get<std::reference_wrapper<Point>>(pOld.second).get();

                    swapPoint<BezierC0>(old, point, appState.bezierC0Set);
                    swapPoint<BezierC2>(old, point, appState.bezierC2Set);
                    swapPoint<InterpolatedC2>(old, point, appState.interpolatedC2Set);
                    swapPoint<PatchC0>(old, point, appState.patchC0Set);
                    swapPoint<PatchC2>(old, point, appState.patchC2Set);

                    appState.pointSet.erase(appState.pointSet.find(old.id));
                    appState.eventPublisher.publish(PointDeletedEvent{old.id});
                }

                appState.selectedEntities.clear();
                appState.eventPublisher.publish(PointMovedEvent{point, {}});
            }
        }
    }

    template<typename T>
    inline void swapPoint(Point &oldPoint, Point &newPoint, std::map<int, std::unique_ptr<T>> &map) {
        for(auto &el : map) {
            for(int i = 0; i < el.second->controlPoints.size(); i++)
                if(el.second->controlPoints[i].first == oldPoint.id) {
                    el.second->controlPoints[i] = {newPoint.id, newPoint};
                }
        }
    }

    inline void renderWorkspaceTorus(Torus &torus, AppState &appState) {
        ImGui::SeparatorText(torus.name.c_str());
        renderNameInput(torus);

        if(renderWorkspaceTransform(torus.transform)) {
            for(auto &pIntersection : appState.intersectionSet) {
                Intersection &intersection = *pIntersection.second;
                if(std::visit(overloaded{[&](auto &e){ return e.get().id == torus.id;}}, intersection.surfaces[0].first) ||
                    std::visit(overloaded{[&](auto &e){ return e.get().id == torus.id;}}, intersection.surfaces[1].first)) {
                    for(auto &pSurface : intersection.surfaces) {
                        IntersectableSurface surface = pSurface.first;
                        std::visit(overloaded{
                            [&](auto &s) {
                                s.get().clearMask();
                                for(auto &pOtherIntersection : appState.intersectionSet) {
                                    if(pOtherIntersection.first == intersection.id)  continue;
                                    Intersection &otherIntersection = *pOtherIntersection.second;
                                    if(std::visit(overloaded{[&](auto &e){ return e.get().id == s.get().id;}}, otherIntersection.surfaces[0].first))
                                        appState.surfaceIntersection.addToMask(surface, otherIntersection, 0);
                                    if(std::visit(overloaded{[&](auto &e){ return e.get().id == s.get().id;}}, otherIntersection.surfaces[1].first))
                                        appState.surfaceIntersection.addToMask(surface, otherIntersection, 1);
                                }
                            }
                        }, surface);
                    }
                    appState.intersectionSet.erase(appState.intersectionSet.find(intersection.id));
                    }
            }
        }


        ImGui::SeparatorText("Parameters");
        bool modified = false;
        modified |= ImGui::DragFloat("Radius", &torus.radius, 0.01, torus.thickness < 0.001 ? 0.001: torus.thickness, FLT_MAX);
        modified |= ImGui::DragFloat("Thickness", &torus.thickness, 0.01, 0.0009, torus.radius);
        modified |= ImGui::DragInt("Radial Resolution", &torus.radialResolution, 1, 3, INT_MAX);
        modified |= ImGui::DragInt("Tubular Resolution", &torus.tubularResolution, 1, 3, INT_MAX);
        if(modified) torus.generate();
    }

    template<typename T> requires has_name<T>
    void renderNameInput(T &el) {
        const static int bufSize = 1024;
        static char nameBuffer[bufSize] = "\0";
        el.name.copy(nameBuffer, el.name.size());
        nameBuffer[el.name.size()] = '\0';
        if (ImGui::InputText("Name", nameBuffer, bufSize))
            el.name = std::string(nameBuffer);
    }

    inline void renderWorkspacePoint(Point &point, AppState &appState) {
        int idCounter = 0;
        ImGui::SeparatorText(point.name.c_str());
        renderNameInput(point);

        for(auto &bezier : appState.bezierC0Set) {
            for(int i = 0; i < bezier.second->controlPoints.size(); i++)
                if(bezier.second->controlPoints[i].first == point.id)
                    if(ImGui::Button(("Remove point from " + bezier.second->name + "##" + std::to_string(idCounter++)).c_str())) {
                        bezier.second->removePoint(i);
                    }
        }
        for(auto &bezier : appState.bezierC2Set) {
            for(int i = 0; i < bezier.second->controlPoints.size(); i++)
                if(bezier.second->controlPoints[i].first == point.id)
                    if(ImGui::Button(("Remove point from " + bezier.second->name + "##" + std::to_string(idCounter++)).c_str())) {
                        bezier.second->removePoint(i);
                    }
        }
        for(auto &interpolated : appState.interpolatedC2Set) {
            for(int i = 0; i < interpolated.second->controlPoints.size(); i++)
                if(interpolated.second->controlPoints[i].first == point.id)
                    if(ImGui::Button(("Remove point from " + interpolated.second->name + "##" + std::to_string(idCounter++)).c_str())) {
                        interpolated.second->removePoint(i);
                    }
        }
        // TODO Refactor all above. It's stupid

        ImGui::Text("Position:");
        bool pointMoved = false;
        glm::vec3 prevPosition = point.position;
        pointMoved |= ImGui::DragFloat(("x##position"+std::to_string(point.id)).c_str(), static_cast<float *>(glm::value_ptr(point.position)) + 0, 0.01f);
        pointMoved |= ImGui::DragFloat(("y##position"+std::to_string(point.id)).c_str(), static_cast<float *>(glm::value_ptr(point.position)) + 1, 0.01f);
        pointMoved |= ImGui::DragFloat(("z##position"+std::to_string(point.id)).c_str(), static_cast<float *>(glm::value_ptr(point.position)) + 2, 0.01f);

        // TODO Maybe changes in point position also should be done through events?
        if(pointMoved) appState.eventPublisher.publish(PointMovedEvent{point, point.position - prevPosition});
    }

    inline bool renderWorkspaceTransform(Transformation &transform) {
        ImGui::SeparatorText("Transform");
        bool modified = false;

        auto position = static_cast<float *>(glm::value_ptr(transform.getTranslationRef()));
        ImGui::Text("Position:");
        modified |= ImGui::DragScalar("x##position", ImGuiDataType_Float, position + 0, 0.01f);
        modified |= ImGui::DragScalar("y##position", ImGuiDataType_Float, position + 1, 0.01f);
        modified |= ImGui::DragScalar("z##position", ImGuiDataType_Float, position + 2, 0.01f);

        auto oldAngle = transform.getRotationAngles();
        auto newAngle = glm::vec3(oldAngle);
        auto angleRef = static_cast<float *>(glm::value_ptr(newAngle));
        ImGui::Text("Rotation:");
        if(ImGui::DragScalar("x##orientation", ImGuiDataType_Float, angleRef + 0, 0.01f)) {
            modified |= true;
            transform.addRotation(glm::vec3(newAngle.x - oldAngle.x, 0, 0));
        }
        if(ImGui::DragScalar("y##orientation", ImGuiDataType_Float, angleRef + 1, 0.01f)) {
            modified |= true;
            transform.addRotation(glm::vec3(0, newAngle.y - oldAngle.y, 0));
        }
        if(ImGui::DragScalar("z##orientation", ImGuiDataType_Float, angleRef + 2, 0.01f)) {
            modified |= true;
            transform.addRotation(glm::vec3(0, 0, newAngle.z - oldAngle.z));
        }

        auto scale = static_cast<float *>(glm::value_ptr(transform.getScaleRef()));
        ImGui::Text("Scale:");
        modified |= ImGui::DragFloat("x##scale", scale + 0, 0.001f, 0.0001f, 1000, nullptr,ImGuiSliderFlags_AlwaysClamp);
        modified |= ImGui::DragFloat("y##scale", scale + 1, 0.001f, 0.0001f, 1000, nullptr,ImGuiSliderFlags_AlwaysClamp);
        modified |= ImGui::DragFloat("z##scale", scale + 2, 0.001f, 0.0001f, 1000, nullptr,ImGuiSliderFlags_AlwaysClamp);
        return modified;
    }

    template<typename T> requires has_name<T> && has_id<T>
    inline void renderListing(std::map<int, std::unique_ptr<T>> &list, AppState &appState, int contextLevel) {
        int idCounter=0;
        auto &entities = appState.selectedEntities;
        for (auto &el: std::views::values(list)) {
            if (ImGui::Selectable((el->name + "##" + std::to_string(idCounter++)).c_str(), entities.end() != std::find_if(entities.begin(), entities.end(), [&](auto &e){ return e.first == el->id;}))) {
                appState.eventPublisher.publish(SelectEntityEvent{*el, contextLevel});
            }
        }
    }
}
#endif //OPENGL_SANDBOX_ENTITYLISTWORKSPACE_H
