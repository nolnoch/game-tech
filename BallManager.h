/*
 * BallManager.h
 *
 *  Created on: Mar 4, 2014
 *      Author: nolnoch
 */

#ifndef BALLMANAGER_H_
#define BALLMANAGER_H_

#include <vector>

#include "TileSimulator.h"
#include "Ball.h"

class TileSimulator;

class BallManager {
public:
  static const unsigned int HOST_MASK = 0xFFFF;

  Ball *globalBall;

  Ogre::Vector3 collisionPosition;

  std::vector<Ball *> playerBalls;
  std::vector<Ball *> ballList;
  std::vector<Ball *> mainBalls;


  BallManager(TileSimulator *sim);
  virtual ~BallManager();

  bool initBallManager();
  void initMultiplayer(int nPlayers);
  void setGlobalBall(Ball *ball, unsigned int host);
  void setPlayerBall(Ball *ball, int idx, unsigned int host);
  Ball* addBall(Ogre::SceneNode* n, int x, int y, int z, int r);
  Ball* addMainBall(Ogre::SceneNode* n, int x, int y, int z, int r);
  void enableGravity();
  void removeBall(Ball* rmBall);
  void removeGlobalBall();
  void removePlayerBall(int idx);
  bool isGlobalBall();
  bool isPlayerBall(int idx);
  void clearBalls();
  int getNumberBallCollisions();
  Ogre::Vector3 getCollisionPosition();
  void moveOrAddBall(int id, Ogre::SceneNode* nodepc, Ogre::Entity* ballmeshpc);

  TileSimulator* getSimulator();
  unsigned int popScoringHost();

  bool checkCollisions(btRigidBody *aTile, void *body0, void *body1);

private:
  std::vector<bool> playerBallsActive;
  TileSimulator *sim;
  bool globalBallActive;
  int ballCollisions;
  unsigned int scoringHost;
};

#endif /* BALLMANAGER_H_ */
