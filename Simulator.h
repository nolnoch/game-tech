#include <btBulletDynamicsCommon.h>
#include <vector>
#include "MinimalOgre.h"


#include "Ball.h"


 const static int WALL_SIZE = 2400;
    const static int PLANE_DIST = WALL_SIZE / 2; //the initial offset from the center
    const static int NUM_TILES_ROW = 5; // number of tiles in each row of a wall.
    const static int NUM_TILES_WALL = NUM_TILES_ROW * NUM_TILES_ROW; //number of total tiles on a wall.
    const static int TILE_WIDTH = WALL_SIZE / NUM_TILES_ROW;

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


    void addTile(Ogre::SceneNode* node) {

        btCollisionShape* groundShape = new btBoxShape(btVector3(240,240,240));
        btDefaultMotionState* groundMotionState = new btDefaultMotionState(btTransform(btQuaternion(0, 0, 0, 1),   
            btVector3(node->getPosition().x, node->getPosition().y, node->getPosition().z - PLANE_DIST - 239)));
        btRigidBody::btRigidBodyConstructionInfo groundRigidBodyCI(0, groundMotionState, groundShape, btVector3(0, 0, 0));
        btRigidBody* groundRigidBody = new btRigidBody(groundRigidBodyCI);
        groundRigidBody->setRestitution(1.0);
        dynamicsWorld->addRigidBody(groundRigidBody);
    }

};
