#include <btBulletDynamicsCommon.h>
#include "OgreMotionState.h"

class Ball
{
    double x, y, z;
    double dx, dy, dz;
    int radius;
    btCollisionShape* collisionShape;
    btRigidBody* rigidBody;
    btScalar mass;
    Ogre::SceneNode* node;
    OgreMotionState* motionState;

  public:
    
    Ball(Ogre::SceneNode* newnode, int nx, int ny, int nz, int nr)
    {
        x = nx;
        y = ny;
        z = nz;
        radius = nr;
        node = newnode;
        node->setPosition(x, y, z);

        motionState = new OgreMotionState(btTransform(btQuaternion(0, 0, 0, 1.0), btVector3(x, y, z)), node);
        collisionShape = new btSphereShape(radius);
        mass = 1;
        btVector3 sphereInertia(0, 0, 0);
        collisionShape->calculateLocalInertia(mass, sphereInertia);
        btRigidBody::btRigidBodyConstructionInfo ballCI(mass, motionState, collisionShape, sphereInertia);
        rigidBody = new btRigidBody(ballCI);
        rigidBody->setCollisionFlags(rigidBody->getCollisionFlags() ^ btCollisionObject::CF_CUSTOM_MATERIAL_CALLBACK);

        rigidBody->setRestitution(0.3);
    }

    void addToWorld(btDynamicsWorld* world)
    {
        world->addRigidBody(rigidBody);
    }

    void removeGravity()
    {
        rigidBody->setGravity(btVector3(0, 0, 0));
        rigidBody->setRestitution(0.6);
        rigidBody->applyDamping(1);
    }

    void setPosition(int x, int y, int z)
    {
        node->setPosition(x, y, z);
        rigidBody->setMassProps(0, btVector3(0, 0, 0));
        motionState->setWorldTransform(btTransform(btQuaternion(0, 0, 0, 1), btVector3(x, y, z)));
    }

    void applyForce(double dx, double dy, double dz)
    {
        rigidBody->activate(true);
        rigidBody->applyCentralImpulse(btVector3(dx, dy, dz));
    }

    bool checkRigidBody(btRigidBody* ptr)
    {
        return ptr == rigidBody;
    }
};
