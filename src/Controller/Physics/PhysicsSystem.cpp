//PhysicsSystem.cpp - manages physics

#include "PhysicsSystem.hpp"
#include "Controller/GameObjectManager.hpp"
#include <glm/gtx/string_cast.hpp>
void PhysicsSystem::Init()
{
    // Create the world settings
    reactphysics3d::PhysicsWorld::WorldSettings settings;
    //settings.defaultVelocitySolverNbIterations = 20;
    //settings.isSleepingEnabled = false;
    settings.gravity = reactphysics3d::Vector3(0, -35, 0);

    physicsWorld = physicsCommon.createPhysicsWorld(settings);
    //physicsWorld->setEventListener(&listener);
}
void PhysicsSystem::DeInit()
{
    for(auto& collider : m_colliders)
    {
        collider.second.DeleteBody(physicsWorld,physicsCommon);
    }
    m_colliders.clear();
    physicsCommon.destroyPhysicsWorld(physicsWorld);
}

PhysicsSystem& PhysicsSystem::getInstance()
{
    static PhysicsSystem instance;
    return instance;
}

void PhysicsSystem::update(float dt) const
{

    physicsWorld->update(static_cast<rp3d::decimal>(dt));

}

unsigned int PhysicsSystem::addSphere(unsigned int ID,Transform *transform, float radius)
{
    RigidBody object;
    auto* sphere = new ReactSphereShape();
    object.CreateBody(ID,physicsWorld,transform->getPosition(),transform->getRotation());
    sphere->CreateSphereShape(radius,physicsCommon);
    object.AddCollisionShape(sphere);
    unsigned int temp = object.getColliderID();
    m_colliders.emplace(object.getColliderID(),object);
    return temp;
}

unsigned int PhysicsSystem::addConcaveShape(unsigned int ID, Transform* transform,unsigned int modelID)
{
    RigidBody object;
    auto* shape = new ReactConcaveShape();
    glm::vec3 newPos = glm::vec3(transform->getPosition().x,transform->getPosition().y,transform->getPosition().z);
    object.CreateBody(ID,physicsWorld,newPos,transform->getRotation());
    shape->CreateConcaveShape(physicsCommon,modelID);
    object.AddCollisionShape(shape);
    object.SetBodyType(rp3d::BodyType::STATIC);
    unsigned int temp = object.getColliderID();
    m_colliders.emplace(object.getColliderID(),object);
    return temp;
}

unsigned int PhysicsSystem::addTerrainShape(unsigned int ID, Transform* transform,std::vector<std::vector<float>> heightvals)
{
    RigidBody terrain;
    ReactTerrainShape* terrShape = new ReactTerrainShape();
    glm::vec3 position(100, 123,100);
    auto orientation = glm::identity<glm::quat>();
    terrain.CreateBody(-2,physicsWorld,position,orientation);
    terrShape->CreateTerrainShape(physicsCommon,heightvals);
    terrain.AddCollisionShape(terrShape);
    terrain.SetBounciness(0.0);
    terrain.SetRollingResistance(1.0);
    terrain.SetBodyType(rp3d::BodyType::STATIC);
    unsigned int temp = terrain.getColliderID();
    m_colliders.emplace(terrain.getColliderID(),terrain);
    return temp;
}

unsigned int PhysicsSystem::addAABB(unsigned int ID,Transform* transform, float width, float height, float length)
{
    RigidBody object;
    auto* box = new ReactBoxShape();
    glm::vec3 newPos = glm::vec3(transform->getPosition().x,transform->getPosition().y,transform->getPosition().z);
    object.CreateBody(ID,physicsWorld,newPos,transform->getRotation());
    box->CreateBoxShape(glm::vec3(width,height,length),physicsCommon);
    object.AddCollisionShape(box);
    unsigned int temp = object.getColliderID();
    m_colliders.emplace(object.getColliderID(),object);

    return temp;
}

RigidBody * PhysicsSystem::getRigidBody(int colliderID)
{
    try{
        return &m_colliders.at(colliderID);
    } catch (std::exception &e) {
        std::cout << colliderID <<std::endl;
        std::cout << e.what() <<std::endl;
        return nullptr;
    }
}

void PhysicsSystem::deleteRigidBody(int ID)
{
    m_colliders.at(ID).DeleteBody(physicsWorld,physicsCommon);
    m_colliders.erase(ID);
}

