//
// Created by User on 14/03/2025.
//

#include "systems/physics/ProjectileManager.h"

#include <entities/GameObject.h>
#include "entities/primitives/Cube.h"
#include "entities/primitives/Sphere.h"


GameObject* ProjectileManager::Shoot(Scene3D* scene, PhysicsWorld* physicsWorld, Vector3& translate, btVector3& direction) {
    GameObject* projectile;
    if (_shootSphere) {
        projectile = new Sphere(scene, 0.25f, 5.f, *physicsWorld->_bWorld);
    }else {
        projectile = new Cube(scene, {0.5f, 0.5f, 0.5f}, 1.f, *physicsWorld->_bWorld);
    }

    projectile->_rigidBody->translate(translate);
    /* Has to be done explicitly after the translate() above, as Magnum ->
       Bullet updates are implicitly done only for kinematic bodies */
    projectile->_rigidBody->syncPose();

    /* Give it an initial velocity */
    projectile->_rigidBody->rigidBody().setLinearVelocity(btVector3{direction*25.f});

    return projectile;
}
