//
// Created by charl on 19/05/2021.
//
#pragma once
#include <iostream>
#include <reactphysics3d/reactphysics3d.h>
#include "Model/Components/Transform.hpp"
class ReactShape {
    public:
    /**
     * @brief Default Constructor
     */
    ReactShape() = default;
    /**
     * @brief Returns a pointer to the collision shape
     * @return CollisionShape*
     */
    virtual reactphysics3d::CollisionShape* getCollisionShape(){return shape;}
    /**
     * @brief Virtual function to delete a shape
     * @param physicsCommon
     */
    virtual void DeleteShape(reactphysics3d::PhysicsCommon &physicsCommon);
    /**
     * @brief Returns a shapeID
     * @return shapeID
     */
    unsigned int getShapeID() const{return shapeID;}
    /**
     * @brief Returns a pointer to the collision shape
     * @return CollisionShape*
     */
    reactphysics3d::CollisionShape *getShape();

protected:
    ///Shape Pointer
    reactphysics3d::CollisionShape* shape;
    ///ShapeID
    unsigned int shapeID;
};
