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
    }

    void addToWorld(btDynamicsWorld* world)
    {
        world->addRigidBody(rigidBody);
    }
};
