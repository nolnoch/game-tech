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
    static vector<Ball*> mainballs;
    static bool targethit;

  public:

    static bool foo(btManifoldPoint& cp, void* body0, void* body1)
    {
        if(activetile == NULL)
            return true;
        for(int i = 0; i < mainballs.size(); i++)
        {
            Ball* mball = mainballs[i];
            if(activetile == body0 && mball->checkRigidBody((btRigidBody*)body1))
            {
                targethit = true;
                mball->lockPosition();
                return true;
            }
            else if(activetile == body1 && mball->checkRigidBody((btRigidBody*)body0))
            {
                targethit = true;
                mball->lockPosition();
                return true;
            }
        }
    }

    Simulator(Ogre::SceneManager* sceneMgrPtr);

    void addBall(Ball* ball);

    void addMainBall(Ball* ball);

    void addPlane(int x, int y, int z, int d);
    
    bool simulateStep(double delay);

    void addTile(Ogre::SceneNode* node, int xsize, int ysize, int zsize);

    
    void removeBall(Ball* ball);
};
