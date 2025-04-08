//
// Created by User on 14/03/2025.
//

#include "systems/physics/ProjectileManager.h"

#include <entities/GameObject.h>
#include "entities/primitives/Cube.h"
#include "entities/primitives/Sphere.h"
#include "systems/levels/Engine.h"

GameObject* ProjectileManager::Shoot(Engine* app, Scene3D* scene, Vector3& translate, btVector3& direction) {
    GameObject* projectile;
    Color3 color = 0x1010ff_rgbf;
    if (_shootSphere) {
        projectile = new Sphere(app, scene, 0.25f, 5.f, color);
    }else {
        projectile = new Cube(app, scene, {0.5f, 0.5f, 0.5f}, 1.f, color);
    }


    projectile->_rigidBody->translate(translate);
    /* Has to be done explicitly after the translate() above, as Magnum ->
       Bullet updates are implicitly done only for kinematic bodies */
    projectile->_rigidBody->syncPose();

    /* Give it an initial velocity */
    projectile->_rigidBody->rigidBody().setLinearVelocity(btVector3{direction*25.f});

    return projectile;
}
