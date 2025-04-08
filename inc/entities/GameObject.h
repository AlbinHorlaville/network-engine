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
#include "systems/levels/Engine.h"
#include "../systems/network/Serializable.h"

struct InstanceData;

enum ObjectType {
    CUBE,
    SPHERE,
};

class GameObject : public Serializable {
    public:
        uint32_t _id = 0;
        ObjectType _type;
        std::string _name;
        Object3D* _parent = nullptr;
        btVector3 _location;
        btVector3 _linearVelocity;
        btVector3 _angularVelocity;
        btQuaternion _rotation;
        float _mass;
        RigidBody* _rigidBody;

        GameObject(std::string name, float m) : _name(std::move(name)), _mass(m), _rigidBody(nullptr) {}
        ~GameObject() {
            delete _rigidBody;
            _app = nullptr;
        }

        virtual void setMass(float mass) = 0;
        virtual void updateDataFromBullet();
        void serialize(std::ostream &ostr) const override = 0;
        void unserialize(std::istream &istr) override = 0;

    protected:
        Engine* _app;
        void giveDefaultName();
};

class ColoredDrawable: public SceneGraph::Drawable3D {
    public:
        explicit ColoredDrawable(Object3D& object, Containers::Array<InstanceData>& instanceData, const Color3& color, const Matrix4& primitiveTransformation, SceneGraph::DrawableGroup3D* drawables):
            SceneGraph::Drawable3D{object, drawables}, _instanceData(instanceData), _color{color}, _primitiveTransformation{primitiveTransformation} {}

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
