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

    void addPlane(int x, int y, int z, int d)
    {
        btCollisionShape* groundShape = new btStaticPlaneShape(btVector3(x, y, z), d);
        btDefaultMotionState* groundMotionState = new btDefaultMotionState(btTransform(btQuaternion(0, 0, 0, 1), btVector3(0, -1, 0)));
        btRigidBody::btRigidBodyConstructionInfo groundRigidBodyCI(0, groundMotionState, groundShape, btVector3(0, 0, 0));
        btRigidBody* groundRigidBody = new btRigidBody(groundRigidBodyCI);
        dynamicsWorld->addRigidBody(groundRigidBody);
    }

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

        addPlane(0, 1, 0, -800);
        addPlane(0, -1, 0, -800);
        addPlane(1, 0, 0, -800);
        addPlane(-1, 0, 0, -800);
        addPlane(0, 0, 1, -800);
        addPlane(0, 0, -1, -800);
    }

    void addBall(Ball* ball)
    {
        ball->addToWorld(dynamicsWorld);
        balls.push_back(ball);
    }

    void simulateStep()
    {
        dynamicsWorld->stepSimulation(1/60.f, 10);
    }
};
