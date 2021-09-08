#include "BoxCollider.hpp"

BoxCollider::BoxCollider(GameObject* parent) : Component(parent){}

void BoxCollider::Start()
{
    extents = glm::vec3(1,1,1);
    if(m_parent->GetComponent<Transform>() == nullptr)
    {
        m_parent->AddComponent<Transform>();
    }
    colliderID = PhysicsSystem::getInstance().addAABB(m_parent->GetObjectID(),m_parent->GetComponent<Transform>().get(),extents.x,extents.y,extents.z);
}
void BoxCollider::SetExtents(glm::vec3 extent)
{
   extents = extent;
}
void BoxCollider::SetExtents(float x,float y, float z)
{
   extents = glm::vec3(x,y,z);
}

void BoxCollider::Update(float deltaTime)
{
 
}