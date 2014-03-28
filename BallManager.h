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
  static const Uint32 HOST_MASK = 0xFFFF;

  Ball *globalBall;
  std::vector<Ball *> playerBalls;
  std::vector<Ball *> ballList;
  std::vector<Ball *> mainBalls;

  BallManager(TileSimulator *sim);
  virtual ~BallManager();

  bool initBallManager();
  void initMultiplayer(int nPlayers);
  void setGlobalBall(Ball *ball, Uint32 host);
  void setPlayerBall(Ball *ball, int idx, Uint32 host);
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
  void moveOrAddBall(int id, Ogre::SceneNode* nodepc, Ogre::Entity* ballmeshpc, Ogre::Vector3 velocity);
  void moveOrAddPlayerBall(int id, Ogre::SceneNode* nodepc, Ogre::Entity* ballmeshpc, Ogre::Vector3 velocity);

  TileSimulator* getSimulator();
  Uint32 popScoringHost();

  bool checkCollisions(btRigidBody *aTile, void *body0, void *body1);

private:
  std::vector<bool> playerBallsActive;
  TileSimulator *sim;
  bool globalBallActive;
  int ballCollisions;
  Uint32 scoringHost;
};

#endif /* BALLMANAGER_H_ */
