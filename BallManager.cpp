/*
 * BallManager.cpp
 *
 *  Created on: Mar 4, 2014
 *      Author: nolnoch
 */

#include "BallManager.h"

BallManager::BallManager(TileSimulator *sim):
sim(sim),
globalBall(0),
ballCollisions(0),
globalBallActive(false)
{
  scoringHost = 0;
}

BallManager::~BallManager() {
  clearBalls();
}

bool BallManager::initBallManager() {
  sim->setBallManager(this);

  return true;
}

void BallManager::initMultiplayer(int nPlayers) {
  playerBalls.assign(nPlayers, NULL);
  playerBallsActive.assign(nPlayers, false);
}

void BallManager::setGlobalBall(Ball *ball, unsigned int host) {
  ball->host = (host & HOST_MASK) >> 16;
  globalBall = ball;
  globalBallActive = true;
}

void BallManager::setPlayerBall(Ball *ball, int idx, unsigned int host) {
  ball->host = (host & HOST_MASK) >> 16;
  ball->shot = true;
  playerBalls[idx] = ball;
  playerBallsActive[idx] = true;
}

Ball* BallManager::addBall(Ogre::SceneNode* n, int x, int y, int z, int r) {
  n->setPosition(x, y, z);

  btRigidBody *body = sim->addBallShape(n, r);
  Ball *ball = new Ball(body, n, x, y, z);
  ballList.push_back(ball);

  return ball;
}

Ball* BallManager::addMainBall(Ogre::SceneNode* n, int x, int y, int z, int r) {
  Ball *mainBall = addBall(n, x, y, z, r);
  mainBalls.push_back(mainBall);

  return mainBall;
}

void BallManager::enableGravity() {
  std::vector<Ball *>::iterator it;

  for (it = ballList.begin(); it != ballList.end(); it++) {
    (*it)->enableGravity();
  }
}

void BallManager::removeBall(Ball* rmBall) {
  bool found = false;

  std::vector<Ball *>::iterator it;
  for (it = ballList.begin(); it != ballList.end() && !found; it++) {
    if ((*it) == rmBall) {
      ballList.erase(it);
      found = true;
    }
  }

  btRigidBody* ballBody = rmBall->getRigidBody();
  sim->getDynamicsWorld().removeRigidBody(ballBody);
  delete ballBody->getMotionState();
  delete rmBall;
}

void BallManager::removeGlobalBall() {
  removeBall(globalBall);
  globalBall = NULL;
  globalBallActive = false;
}

void BallManager::removePlayerBall(int idx) {
  removeBall(playerBalls[idx]);
  playerBalls[idx] = NULL;
  playerBallsActive[idx] = false;
}


void BallManager::moveOrAddBall(int id, Ogre::SceneNode* nodepc, Ogre::Entity* ballmeshpc)
{
  double x = nodepc->getPosition().x;
  double y = nodepc->getPosition().y;
  double z = nodepc->getPosition().z;

  if (id < mainBalls.size()) {
    Ball* rmBall = mainBalls[id];

    removeBall(rmBall);

    mainBalls[id] = new Ball(sim->addBallShape(nodepc, 100), nodepc, x, y, z);
  } else {
    mainBalls.push_back(new Ball(sim->addBallShape(nodepc, 100), nodepc, x, y, z));
  }
  ballList.push_back(mainBalls[id]);
}

bool BallManager::isGlobalBall() {
  return globalBallActive;
}

bool BallManager::isPlayerBall(int idx) {
  return playerBallsActive[idx];
}

void BallManager::clearBalls() {
  std::vector<Ball *>::iterator it;
  int nMainBalls = mainBalls.size();
  int nBallList = ballList.size();

  for (it = ballList.begin(); it != ballList.end(); it++) {
    btRigidBody* ballBody = (*it)->getRigidBody();
    sim->getDynamicsWorld().removeRigidBody(ballBody);
    delete ballBody->getMotionState();
    delete (*it);
  }

  globalBall = NULL;
  globalBallActive = false;
  ballList.clear();
  mainBalls.clear();
  playerBalls.assign(playerBalls.size(), NULL);
  playerBallsActive.assign(playerBallsActive.size(), false);
}

TileSimulator* BallManager::getSimulator() {
  return sim;
}

unsigned int BallManager::popScoringHost() {
  unsigned int ret = scoringHost;
  scoringHost = 0;

  return ret;
}

bool BallManager::checkCollisions(btRigidBody *aTile, void *body0, void *body1) {
  bool hit = false;

  std::vector<Ball *>::iterator it;
  for (it = mainBalls.begin(); it != mainBalls.end() && !hit; it++) {
    Ball* mball = (*it);

    if ((aTile == body0 && mball->checkRigidBody((btRigidBody*)body1)) ||
        (aTile == body1 && mball->checkRigidBody((btRigidBody*)body0))) {
      scoringHost = mball->host;
      mball->lockPosition();
      hit = true;
      collisionPosition = (mball->getSceneNode())->_getDerivedPosition();
    } else if (mball->checkRigidBody((btRigidBody*)body0)) {
      std::vector<Ball *>::iterator it2;

      for (it2 = ballList.begin(); it2 != ballList.end(); it2++) {

        if((*it2) != mball && (*it2)->checkRigidBody((btRigidBody*)body1)) {
          if (mball->host || (*it2)->host) {
            if (mball->host && (*it2)->host) {
              if (mball->getRigidBody()->getLinearVelocity() >
              (*it2)->getRigidBody()->getLinearVelocity()) {
                (*it2)->host = mball->host;
              } else {
                mball->host = (*it2)->host;
              }
            } else if (mball->host) {
              (*it2)->host = mball->host;
            } else {
              mball->host = (*it2)->host;
            }
          }
          ballCollisions++;
        }
      }
    }
  }

  return hit;
}

int BallManager::getNumberBallCollisions() {
  int buf = ballCollisions;
  ballCollisions = 0;
  return buf;
}

Ogre::Vector3 BallManager::getCollisionPosition() {
  return collisionPosition;
}



