//
// Created by User on 14/03/2025.
//

#ifndef GAMEOBJECT_H
#define GAMEOBJECT_H

#include <string>
#include <utility>
#include "components/RigidBody.h"
#include "Corrade/Containers/GrowableArray.h"
#include "Magnum/Math/Color.h"
#include "Magnum/Platform/GlfwApplication.h"
#include "Magnum/SceneGraph/Drawable.h"
#include "systems/levels/Level_1.h"
#include "../systems/network/Serializable.h"

struct InstanceData;

class GameObject : public Serializable {
    public:
        std::string _name;
        btVector3 _location;
        btQuaternion _rotation;
        float _mass;
        RigidBody* _rigidBody;

        GameObject(std::string name, float m) : _name(std::move(name)), _mass(m), _rigidBody(nullptr) {}

        virtual void setMass(float mass) = 0;
        void serialize(std::ostream &ostr) const override = 0;
        void unserialize(std::istream &istr) override = 0;

    protected:
        Level_1* _app;
        void giveDefaultName();
};

class ColoredDrawable: public SceneGraph::Drawable3D {
    public:
        explicit ColoredDrawable(Object3D& object, Containers::Array<InstanceData>& instanceData, const Color3& color, const Matrix4& primitiveTransformation, SceneGraph::DrawableGroup3D* drawables): SceneGraph::Drawable3D{object, drawables}, _instanceData(instanceData), _color{color}, _primitiveTransformation{primitiveTransformation} {}

    private:
        void draw(const Matrix4& transformation, SceneGraph::Camera3D&) override {
            const Matrix4 t = transformation*_primitiveTransformation;
            arrayAppend(_instanceData, InPlaceInit, t, t.normalMatrix(), _color);
        }

        Containers::Array<InstanceData>& _instanceData;
        Color3 _color;
        Matrix4 _primitiveTransformation;
};


#endif //GAMEOBJECT_H
