#pragma once

#include <vector>
#include <map>
#include "Controller/Physics/RigidBody.hpp"
#include "Model/Components/Transform.hpp"
#include "Controller/Physics/ReactMath.hpp"
#include "Controller/Physics/Shapes/ReactTerrainShape.hpp"
#include "Controller/Physics/Shapes/ReactSphereShape.hpp"
#include "Controller/Physics/Shapes/ReactBoxShape.hpp"
#include "Controller/Physics/Shapes/ReactConcaveShape.hpp"
#include "Controller/Export.hpp"
/**
 * @class PhysicsSystem
 * @brief Singleton that Manages physics
 */
class YOKAI_API PhysicsSystem
{
public:
    /**
     * @brief Returns this instance of Physics manager
     * @return PhysicsSystem&
     */
    static PhysicsSystem& getInstance();

    /**
     * @brief Initialises Physics Manager
     */
    void Init();

    ///Deleted copy constructor
    PhysicsSystem(PhysicsSystem const&) = delete;

    ///Deleted = operator
    void operator=(PhysicsSystem const&) = delete;
    /**
     * @brief Updates the simulation
     * @param float - dt
     */
    void update(float dt) const;


    // easy to implement if needed
    //int addBoundingSphere(glm::vec3 *position, double radius);
    /**
     * @brief Adds a bounding box
     * @param unsigned int - ID
     * @param Transform* - transform
     * @param float - width
     * @param float - length
     * @param float - height
     * @return ReactBoxShape*
     */
    unsigned int addAABB(unsigned int ID, Transform* transform, float width, float height, float length);
    /**
     * @brief Returns a collider given an ID
     * @param int - colliderID
     * @return RigidBody
     */
    RigidBody * getRigidBody(int colliderID);
    /**
     * @brief Deletes a rigid Body
     * @param int - ID
     */
    void deleteRigidBody(int ID);
    /**
     * @brief Adds a sphere body to the scene
     * @param ID
     * @param transform
     * @param radius
     * @return unsigned int bodyID
     */
    unsigned int addSphere(unsigned int ID, Transform* transform,float radius);

    unsigned int addConcaveShape(unsigned int ID, Transform* transform,unsigned int modelID);

    unsigned int addTerrainShape(unsigned int ID, Transform* transform,std::vector<std::vector<float>> heightvals);

    void RendererUpdate();

    void Draw();
    /**
     * @brief DeInitialises the physics simulation
     */
    void DeInit();
    ///object to create physics shapes
    reactphysics3d::PhysicsCommon physicsCommon;
    ///physics world for simulation
    reactphysics3d::PhysicsWorld* physicsWorld;
private:
    ///Privatised Constructor
    PhysicsSystem() = default;
    std::string path1;
    std::string path2;
    ///Privatised destructor
    //Test Renderer Values
    unsigned int l_vbo_ = 0, l_vao_ = 0;
    unsigned int t_vbo_ = 0, t_vao_ = 0;
    unsigned int line_num_ = 0, triag_num_ = 0;

    /// The shared pointer to draw react objects,
    Shader* shader_ = nullptr;

    ///count of map
    int m_mapCount;
    ///map of colliders
    std::map<int, RigidBody> m_colliders;	//TODO: make colliders so can add spheres
    ///Collision callback listener
};