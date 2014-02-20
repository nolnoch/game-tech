#include <btBulletDynamicsCommon.h>
#include <vector>

#include "Ball.h"


 const static int WALL_SIZE = 2400;
    const static int PLANE_DIST = WALL_SIZE / 2; //the initial offset from the center
    const static int NUM_TILES_ROW = 5; // number of tiles in each row of a wall.
    const static int NUM_TILES_WALL = NUM_TILES_ROW * NUM_TILES_ROW; //number of total tiles on a wall.
    const static int TILE_WIDTH = WALL_SIZE / NUM_TILES_ROW;

using std::vector;



class Simulator
{
  private:
    btBroadphaseInterface* broadphase;
    btDefaultCollisionConfiguration* collisionConfiguration;
    btCollisionDispatcher* dispatcher;
    btSequentialImpulseConstraintSolver* solver;
    btDiscreteDynamicsWorld* dynamicsWorld;
    Ogre::SceneManager* sceneMgr;
    vector<Ball*> balls;
    std::deque<btRigidBody*> tiles;

    static btRigidBody* activetile;
    static Ball* mainball;
    static bool targethit;

  public:

    static bool foo(btManifoldPoint& cp, void* body0, void* body1)
    {
        for(int i = 0; i < 2; i++)
        {
            if(activetile == body0 && mainball->checkRigidBody((btRigidBody*)body1))
            {
                targethit = true;
            }
            else if(activetile == body1 && mainball->checkRigidBody((btRigidBody*)body0))
            {
                targethit = true;
            }
        }
    }

    Simulator(Ogre::SceneManager* sceneMgrPtr);

    void addBall(Ball* ball);

    void addMainBall(Ball* ball);

    void addPlane(int x, int y, int z, int d);
    
    bool simulateStep(double delay);

    void addTile(Ogre::SceneNode* node, int xsize, int ysize, int zsize);
};
