//
// Created by faliszewskii on 10.01.24.
//

#ifndef OPENGL_SANDBOX_PROPERTYVISITOR_H
#define OPENGL_SANDBOX_PROPERTYVISITOR_H

class TransformationProperty;
class PointLightProperty;

class PropertyVisitor {
public:
    virtual void visitTransformationProperty(TransformationProperty& transformation) {};
    virtual void visitPointLightProperty(PointLightProperty& transformation) {};
protected:
    PropertyVisitor() = default;
};


#endif //OPENGL_SANDBOX_PROPERTYVISITOR_H