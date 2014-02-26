
#include "Simulator.h"

btRigidBody* Simulator::activetile;
vector<Ball*> Simulator::mainballs;
bool Simulator::targethit;



Simulator::Simulator(Ogre::SceneManager* sceneMgrPtr)
{
    sceneMgr = sceneMgrPtr;
    broadphase = new btDbvtBroadphase();
    collisionConfiguration = new btDefaultCollisionConfiguration();
    dispatcher = new btCollisionDispatcher(collisionConfiguration);
    solver = new btSequentialImpulseConstraintSolver();
    dynamicsWorld = new btDiscreteDynamicsWorld(dispatcher, broadphase, solver, collisionConfiguration);
    dynamicsWorld->setGravity(btVector3(0, 0, 0));

    targethit = false;
}

void Simulator::addBall(Ball* ball)
{
    ball->addToWorld(dynamicsWorld);
    balls.push_back(ball);
}

void Simulator::addMainBall(Ball* ball)
{
    addBall(ball);
    mainballs.push_back(ball);
}

void Simulator::addPlane(int x, int y, int z, int d)
{
    btCollisionShape* groundShape = new btStaticPlaneShape(btVector3(x, y, z), d);
    btDefaultMotionState* groundMotionState = new btDefaultMotionState(btTransform(btQuaternion(0, 0, 0, 1), btVector3(0, -1, 0)));
    btRigidBody::btRigidBodyConstructionInfo groundRigidBodyCI(0, groundMotionState, groundShape, btVector3(0, 0, 0));
    btRigidBody* groundRigidBody = new btRigidBody(groundRigidBodyCI);
    groundRigidBody->setRestitution(1.0);
    groundRigidBody->setCollisionFlags(groundRigidBody->getCollisionFlags() ^ btCollisionObject::CF_CUSTOM_MATERIAL_CALLBACK);
    dynamicsWorld->addRigidBody(groundRigidBody);
}

void Simulator::addTile(Ogre::SceneNode* node, int xsize, int ysize, int zsize) 
{
    btCollisionShape* groundShape = new btBoxShape(btVector3(xsize, ysize, zsize));
    btDefaultMotionState* groundMotionState = new btDefaultMotionState(btTransform(btQuaternion(0, 0, 0, 1),   
        btVector3(node->_getDerivedPosition().x, node->_getDerivedPosition().y, node->_getDerivedPosition().z)));
    btRigidBody::btRigidBodyConstructionInfo groundRigidBodyCI(0, groundMotionState, groundShape, btVector3(0, 0, 0));
    btRigidBody* groundRigidBody = new btRigidBody(groundRigidBodyCI);
    activetile = groundRigidBody;
    gContactProcessedCallback = foo;
    groundRigidBody->setRestitution(1.0);
    dynamicsWorld->addRigidBody(groundRigidBody);

    tiles.push_back(groundRigidBody);
}

bool Simulator::simulateStep(double delay)
{
    targethit = false;
    dynamicsWorld->stepSimulation((1/60.f) - delay, 10);
    if(targethit)
    {
        if(tiles.size() > 0)
        {
            tiles.pop_back();
            if(tiles.size() > 0)
                activetile = tiles.back();
            else
                activetile = NULL;
        }
    }
    return targethit;
}

void Simulator::removeBall(Ball* ball) {
    btRigidBody* body = ball->getRigidBody();
    dynamicsWorld->removeRigidBody(body);
    delete body->getMotionState();
    delete body;
    delete ball->node;
}

