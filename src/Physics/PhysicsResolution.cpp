﻿#include "PhysicsResolution.hpp"
#include "glm/gtx/string_cast.hpp"
#include "glm/matrix.hpp"

void PhysicsResolution::onContact(const rp3d::CollisionCallback::CallbackData &callbackData) 
{   
    for (int p = 0; p < callbackData.getNbContactPairs(); p++) 
    {
        // Get the contact pair
        CollisionCallback::ContactPair contactPair = callbackData.getContactPair(p);
        int body1 = contactPair.getCollider1()->getEntity().id;
        int body2 = contactPair.getCollider2()->getEntity().id;

        if ((!PhysicsSystem::getInstance().getPhysicsBody(body1)->getIsStaticObject() || !PhysicsSystem::getInstance().getPhysicsBody(body2)->getIsStaticObject())) 
        {

            for (int c = 0; c < contactPair.getNbContactPoints(); c++) 
            {
                // Get the contact point
                CollisionCallback::ContactPoint contactPoint = contactPair.getContactPoint(c);
                double penetration = contactPoint.getPenetrationDepth();
                glm::dvec3 contactNormal = ReactMath::rp3dVecToGlm(const_cast<rp3d::Vector3&>(contactPoint.getWorldNormal()));

                // Get the contact point on the first collider and convert it in world-space
                glm::dvec3 body1ContactPoint = ReactMath::rp3dVecToGlm(contactPair.getCollider1()->getLocalToWorldTransform() * contactPoint.getLocalPointOnCollider1());
                glm::dvec3 body2ContactPoint = ReactMath::rp3dVecToGlm(contactPair.getCollider1()->getLocalToWorldTransform() * contactPoint.getLocalPointOnCollider2());

                //std::cout << "b1 col - (" << body1ContactPoint.x << ", " << body1ContactPoint.y << ", " << body1ContactPoint.z << ")" << std::endl;
                //std::cout << "b2 col - (" << body2ContactPoint.x << ", " << body2ContactPoint.y << ", " << body2ContactPoint.z << ")" << std::endl;

                ResolveClipping(body1, body2, penetration, contactNormal);
                CollisionResolution(body1, body2, penetration, contactNormal, body1ContactPoint, body2ContactPoint);
            }
        }
    }
}

void PhysicsResolution::ResolveClipping(int body1, int body2, double penetration, glm::dvec3 contactNormal) 
{
    if (!PhysicsSystem::getInstance().getPhysicsBody(body1)->getIsStaticObject()) 
    {
        PhysicsSystem::getInstance().getPhysicsBody(body1)->SetPosition(PhysicsSystem::getInstance().getPhysicsBody(body1)->GetPosition() + (contactNormal * (-(penetration / 2))));
    }

    if (!PhysicsSystem::getInstance().getPhysicsBody(body2)->getIsStaticObject()) 
    {
        PhysicsSystem::getInstance().getPhysicsBody(body2)->SetPosition(PhysicsSystem::getInstance().getPhysicsBody(body2)->GetPosition() - (contactNormal * (-(penetration / 2))));
    }
}

void PhysicsResolution::CollisionResolution(int body1, int body2, double penetration, glm::dvec3 contactNormal, glm::dvec3 body1ContactPoint, glm::dvec3 body2ContactPoint) 
{
    double coefficientOfRestitution = 0.5;
    glm::dvec3 linearVelocity1 = PhysicsSystem::getInstance().getPhysicsBody(body1)->getLinearVelocity();
    glm::dvec3 angularVelocity1 = PhysicsSystem::getInstance().getPhysicsBody(body1)->getAngularVelocity();
    glm::dvec3 linearVelocity2 = PhysicsSystem::getInstance().getPhysicsBody(body2)->getLinearVelocity();
    glm::dvec3 angularVelocity2 = PhysicsSystem::getInstance().getPhysicsBody(body2)->getAngularVelocity();
    glm::dvec3 r1 = body1ContactPoint - (PhysicsSystem::getInstance().getPhysicsBody(body1)->getCentreOfMass());
    glm::dvec3 r2 = body2ContactPoint - (PhysicsSystem::getInstance().getPhysicsBody(body2)->getCentreOfMass());

    double restitution = -(1.0 + coefficientOfRestitution);

    glm::dvec3 relativeVelocity = linearVelocity1 - linearVelocity2;

    glm::dvec3 r1cross = glm::cross(r1, contactNormal);
    glm::dvec3 r2cross = glm::cross(r2, contactNormal);

    double combinedInverseMass = PhysicsSystem::getInstance().getPhysicsBody(body1)->getInverseMass() + PhysicsSystem::getInstance().getPhysicsBody(body2)->getInverseMass();

    double numerator = restitution * (glm::dot(contactNormal, relativeVelocity) + glm::dot(angularVelocity1, r1cross) - glm::dot(angularVelocity2, r2cross));
    glm::dvec3 denominator = combinedInverseMass + ((r1cross * PhysicsSystem::getInstance().getPhysicsBody(body1)->getInverseInertiaTensor() * r1cross) + (r2cross * PhysicsSystem::getInstance().getPhysicsBody(body2)->getInverseInertiaTensor() * r2cross));
    
    glm::dvec3 lambda = numerator / denominator;
	glm::dvec3 linearImpulse = ((numerator * contactNormal) / denominator);
    
    //TESTING
    //std::cout << "R1 - (" << r1.x << ", " << r1.y << ", " << r1.z << ")" << std::endl;
    //std::cout << "R2 - (" << r2.x << ", " << r2.y << ", " << r2.z << ")" << std::endl;
    //std::cout << restitution << std::endl;
    //std::cout << "RV - (" << relativeVelocity.x << ", " << relativeVelocity.y << ", " << relativeVelocity.z << ")" << std::endl;
    //std::cout << "R1cross - (" << r1cross.x << ", " << r1cross.y << ", " << r1cross.z << ")" << std::endl;
    //std::cout << "R2cross - (" << r2cross.x << ", " << r2cross.y << ", " << r2cross.z << ")" << std::endl; std::cout <<
    //std::cout << combinedInverseMass << std::endl;
    //std::cout << numerator << std::endl;
    //std::cout << denominator << std::endl;

    linearVelocity1 += linearImpulse / PhysicsSystem::getInstance().getPhysicsBody(body1)->getMass();
    linearVelocity2 -= linearImpulse / PhysicsSystem::getInstance().getPhysicsBody(body2)->getMass();

    angularVelocity1 = angularVelocity1 + (lambda * PhysicsSystem::getInstance().getPhysicsBody(body1)->getInverseInertiaTensor()) * r1cross;
    angularVelocity2 = angularVelocity2 - (lambda * PhysicsSystem::getInstance().getPhysicsBody(body2)->getInverseInertiaTensor()) * r2cross;

    //std::cout << angularVelocity1.x << ", " << angularVelocity1.y << ", " << angularVelocity1.z << std::endl;
    //std::cout << angularVelocity2.x << ", " << angularVelocity2.y << ", " << angularVelocity2.z << std::endl;

    if (!PhysicsSystem::getInstance().getPhysicsBody(body1)->getIsStaticObject()) 
    {
        PhysicsSystem::getInstance().SubmitLinearVelocity(body1, linearVelocity1);
        PhysicsSystem::getInstance().SubmitAngularVelocity(body1, angularVelocity1);
    }

     if (!PhysicsSystem::getInstance().getPhysicsBody(body2)->getIsStaticObject()) 
     {
        PhysicsSystem::getInstance().SubmitLinearVelocity(body2, linearVelocity2);
        PhysicsSystem::getInstance().SubmitAngularVelocity(body2, angularVelocity2);
    }
}