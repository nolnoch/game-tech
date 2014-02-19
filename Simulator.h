#include <btBulletDynamicsCommon.h>
#include <vector>

#include "Ball.h"

using std::vector;

class Simulator
{
    btBroadphaseInterface* broadphase;
    btDefaultCollisionConfiguration* collisionConfiguration;
    btCollisionDispatcher* dispatcher;
    btSequentialImpulseConstraintSolver* solver;
    btDiscreteDynamicsWorld* dynamicsWorld;
    Ogre::SceneManager* sceneMgr;

    vector<Ball*> balls;

  private:


  public:

    Simulator(Ogre::SceneManager* sceneMgrPtr)
    {
        sceneMgr = sceneMgrPtr;
        broadphase = new btDbvtBroadphase();
        collisionConfiguration = new btDefaultCollisionConfiguration();
        dispatcher = new btCollisionDispatcher(collisionConfiguration);
        solver = new btSequentialImpulseConstraintSolver();
        dynamicsWorld = new btDiscreteDynamicsWorld(dispatcher, broadphase, solver, collisionConfiguration);
        dynamicsWorld->setGravity(btVector3(0, -980, 0));

       
    }

    void addBall(Ball* ball)
    {
        ball->addToWorld(dynamicsWorld);
        balls.push_back(ball);
    }

    
    void addPlane(int x, int y, int z, int d)
    {
        btCollisionShape* groundShape = new btStaticPlaneShape(btVector3(x, y, z), d);
        btDefaultMotionState* groundMotionState = new btDefaultMotionState(btTransform(btQuaternion(0, 0, 0, 1), btVector3(0, -1, 0)));
        btRigidBody::btRigidBodyConstructionInfo groundRigidBodyCI(0, groundMotionState, groundShape, btVector3(0, 0, 0));
        btRigidBody* groundRigidBody = new btRigidBody(groundRigidBodyCI);
        groundRigidBody->setRestitution(1.0);
        dynamicsWorld->addRigidBody(groundRigidBody);
    }
    

    void simulateStep(double delay)
    {
        dynamicsWorld->stepSimulation((1/60.f) - delay, 10);
    }
};