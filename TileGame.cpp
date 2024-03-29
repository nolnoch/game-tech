/*
-----------------------------------------------------------------------------
Filename:    TileGame.cpp
-----------------------------------------------------------------------------

This source file is part of the
   ___                 __    __ _ _    _ 
  /___\__ _ _ __ ___  / / /\ \ (_) | _(_)
 //  // _` | '__/ _ \ \ \/  \/ / | |/ / |
/ \_// (_| | | |  __/  \  /\  /| |   <| |
\___/ \__, |_|  \___|   \/  \/ |_|_|\_\_|
      |___/                              
      Tutorial Framework
      http://www.ogre3d.org/tikiwiki/
-----------------------------------------------------------------------------
 */
#include "TileGame.h"

#if OGRE_PLATFORM == OGRE_PLATFORM_APPLE_IOS || OGRE_PLATFORM == OGRE_PLATFORM_APPLE
#   include <macUtils.h>
#   include "AppDelegate.h"
#endif

//-------------------------------------------------------------------------------------
TileGame::TileGame(void) :
mDirection(Ogre::Vector3::ZERO),
headNode(0),
ballMgr(0),
soundMgr(0),
netMgr(0),
sim(0),
panelLight(0),
scorePanel(0),
congratsPanel(0),
winnerPanel(0),
chargePanel(0),
multiScorePanel(0),
clientAcceptDescPanel(0),
clientAcceptOptPanel(0),
serverStartPanel(0),
playersWaitingPanel(0),
crosshairOverlay(0),
boing(0),
music(0),
gong(0),
hit2(0),
noteSequence(5),
noteIndex(0)
{
  gameDone = animDone = isCharging = paused = connected = server = netActive =
      invitePending = inviteAccepted = multiplayerStarted = tileHit = false;
  gameStart = true;

  mSpeed = score = shotsFired = tileCounter = winTimer = chargeShot =
      slowdownval = currTile = nPlayers = ballsounddelay = wins = 0;
  currLevel = 1;

  initLocalData();

  mTimer = OGRE_NEW Ogre::Timer();
  mTimer->reset();

  netTimer = OGRE_NEW Ogre::Timer();
  netTimer->reset();
}
//-------------------------------------------------------------------------------------
TileGame::~TileGame(void) {
  delete soundMgr;
  delete ballMgr;
  delete netMgr;
  delete sim;
}
//-------------------------------------------------------------------------------------
bool TileGame::configure() {
  bool ret = BaseGame::configure();

  // Networking //
  netMgr = new NetManager();
  if (netMgr->initNetManager()) {
    netMgr->addNetworkInfo(PROTOCOL_UDP);
    netActive = netMgr->startServer();
  }

  // Physics //
  sim = new TileSimulator();
  sim->initSimulator();

  // Balls //
  ballMgr = new BallManager(sim);
  ballMgr->initBallManager();

  // Sound //
  soundMgr = new SoundManager();
  soundMgr->initSoundManager();

  boing = soundMgr->loadSound("hit.wav");
  gong = soundMgr->loadSound("gong.wav");
  music = soundMgr->loadMusic("ambient.wav");
  hit2 = soundMgr->loadSound("hit2.wav");

  noteSequence[0] = soundMgr->loadSound("note5.wav");
  noteSequence[1] = soundMgr->loadSound("note4.wav");
  noteSequence[2] = soundMgr->loadSound("note3.wav");
  noteSequence[3] = soundMgr->loadSound("note2.wav");
  noteSequence[4] = soundMgr->loadSound("note1.wav");

  soundMgr->playMusic();
  soundMgr->setVolume(.25);

  return ret;
}
//-------------------------------------------------------------------------------------
void TileGame::createCamera(void) {
  BaseGame::createCamera();
  mCameraMan = new CameraMan(mCamera);
}
//-------------------------------------------------------------------------------------
void TileGame::createScene(void) {
  //  std::string postFilter = "Gaussian Blur";
  //  Ogre::CompositorManager::getSingleton().addCompositor(mCamera->getViewport(), postFilter);
  //  Ogre::CompositorManager::getSingleton().setCompositorEnabled(mCamera->getViewport(), postFilter, true);
  //  Ogre::ColourValue fadeColour(0.1, 0.1, 0.1);
  //  mSceneMgr->setFog(Ogre::FOG_EXP, fadeColour, 0.0008);

  Ogre::Plane wallBack(Ogre::Vector3::NEGATIVE_UNIT_Z, 0);
  Ogre::Plane wallFront(Ogre::Vector3::UNIT_Z,0);
  Ogre::Plane wallDown(Ogre::Vector3::UNIT_Y,0);
  Ogre::Plane wallUp(Ogre::Vector3::NEGATIVE_UNIT_Y,0);
  Ogre::Plane wallLeft(Ogre::Vector3::UNIT_X, 0);
  Ogre::Plane wallRight(Ogre::Vector3::NEGATIVE_UNIT_X,0);

  std::string wallMaterialName = "Examples/wallBlend";
  float mapping = 2;
  // Use the planes from above to generate new meshes for walls.
  Ogre::MeshManager::getSingleton().createPlane("ground",
      Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, wallDown,
      WALL_SIZE, WALL_SIZE, 20, 20, true, 1, 1, 1, Ogre::Vector3::UNIT_Z);
  Ogre::Entity* entGround = mSceneMgr->createEntity("GroundEntity", "ground");
  entGround->setMaterialName("Examples/floor");
  entGround->setCastShadows(false);
  Ogre::SceneNode* nodeGround = mSceneMgr->getRootSceneNode()->createChildSceneNode();
  nodeGround->setPosition(0 , -PLANE_DIST, 0);
  nodeGround->attachObject(entGround);

  Ogre::MeshManager::getSingleton().createPlane("ceiling",
      Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, wallUp,
      WALL_SIZE, WALL_SIZE, 20, 20, true, 1, 2, 2, Ogre::Vector3::UNIT_Z);
  Ogre::Entity* entCeiling = mSceneMgr->createEntity("CeilingEntity", "ceiling");
  entCeiling->setMaterialName("Examples/CloudySky");
  entCeiling->setCastShadows(false);
  Ogre::SceneNode* nodeCeiling = mSceneMgr->getRootSceneNode()->createChildSceneNode();
  nodeCeiling->setPosition(0 , PLANE_DIST, 0);
  nodeCeiling->attachObject(entCeiling);

  Ogre::MeshManager::getSingleton().createPlane("back",
      Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, wallBack,
      WALL_SIZE, WALL_SIZE, 20, 20, true, 1, mapping, mapping, Ogre::Vector3::UNIT_Y);
  Ogre::Entity* entBack = mSceneMgr->createEntity("BackEntity", "back");
  entBack->setMaterialName(wallMaterialName);
  entBack->setCastShadows(false);
  Ogre::SceneNode* nodeBack = mSceneMgr->getRootSceneNode()->createChildSceneNode("backNode");
  nodeBack->setPosition(0 , 0, PLANE_DIST);
  nodeBack->attachObject(entBack);

  Ogre::MeshManager::getSingleton().createPlane("front",
      Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, wallFront,
      WALL_SIZE, WALL_SIZE, 20, 20, true, 1, mapping, mapping, Ogre::Vector3::UNIT_Y);
  Ogre::Entity* entFront = mSceneMgr->createEntity("FrontEntity", "front");
  entFront->setMaterialName(wallMaterialName);
  entFront->setCastShadows(false);
  Ogre::SceneNode* nodeFront = mSceneMgr->getRootSceneNode()->createChildSceneNode("frontNode");
  nodeFront->setPosition(0 , 0, -PLANE_DIST);
  nodeFront->attachObject(entFront);

  Ogre::MeshManager::getSingleton().createPlane("left",
      Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, wallLeft,
      WALL_SIZE, WALL_SIZE, 20, 20, true, 1, mapping, mapping, Ogre::Vector3::UNIT_Y);
  Ogre::Entity* entLeft = mSceneMgr->createEntity("LeftEntity", "left");
  entLeft->setMaterialName(wallMaterialName);
  entLeft->setCastShadows(false);
  Ogre::SceneNode* nodeLeft = mSceneMgr->getRootSceneNode()->createChildSceneNode("leftNode");
  nodeLeft->setPosition(-PLANE_DIST , 0, 0);
  nodeLeft->attachObject(entLeft);

  Ogre::MeshManager::getSingleton().createPlane("right",
      Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, wallRight,
      WALL_SIZE, WALL_SIZE, 20, 20, true, 1, mapping, mapping, Ogre::Vector3::UNIT_Y);
  Ogre::Entity* entRight = mSceneMgr->createEntity("RightEntity", "right");
  entRight->setMaterialName(wallMaterialName);
  entRight->setCastShadows(false);
  Ogre::SceneNode* nodeRight = mSceneMgr->getRootSceneNode()->createChildSceneNode("rightNode");
  nodeRight->setPosition(PLANE_DIST, 0, 0);
  nodeRight->attachObject(entRight);

  // Set ambient light
  mSceneMgr->setAmbientLight(Ogre::ColourValue(0.40, 0.40, 0.40));

  mSceneMgr->setShadowTechnique(Ogre::SHADOWTYPE_STENCIL_ADDITIVE);

  // Create a light
  Ogre::Light* lSun = mSceneMgr->createLight("SunLight");
  lSun->setType(Ogre::Light::LT_POINT);
  lSun->setDiffuseColour(0.95, 0.95, 1.00);
  lSun->setPosition(-500,1600,-500);
  lSun->setSpecularColour(1.0,1.0,1.0);
  //  lSun->setAttenuation(3250, 1.0, 0.0000000001, 0.000001);
  lSun->setAttenuation(5250, 1.0, 0.00000001, 0.00000010);

  sim->createBounds(PLANE_DIST);

  levelSetup(currLevel);
}
//-------------------------------------------------------------------------------------
void TileGame::createFrameListener(void) {
  BaseGame::createFrameListener();

  Ogre::StringVector scorelist;
  scorelist.push_back("Score");
  scorelist.push_back("Shots Fired");
  scorelist.push_back("Current Level");
  Ogre::StringVector playerCountTag;
  playerCountTag.push_back("Current Players:");

  scorePanel = mTrayMgr->createParamsPanel(OgreBites::TL_TOPLEFT,
      "ScorePanel", 200, scorelist);
  congratsPanel = mTrayMgr->createLabel(OgreBites::TL_TOP,
      "CongratsPanel", "NULL", 300);
  chargePanel = mTrayMgr->createLabel(OgreBites::TL_BOTTOM,
      "Chargepanel", "|", 300);
  clientAcceptDescPanel = mTrayMgr->createLabel(OgreBites::TL_TOPRIGHT,
      "ClientAcceptDescPanel", "Join multiplayer game?", 250);
  clientAcceptOptPanel = mTrayMgr->createLabel(OgreBites::TL_TOPRIGHT,
      "ClientAcceptOptPanel", "(Y)es or (N)o", 160);
  playersWaitingPanel = mTrayMgr->createParamsPanel(OgreBites::TL_BOTTOMRIGHT,
      "PlayersWaitingPanel", 200, playerCountTag);
  winnerPanel = mTrayMgr->createLabel(OgreBites::TL_CENTER,
      "WinnerPanel", "NULL", 300);

  mTrayMgr->getTrayContainer(OgreBites::TL_TOPRIGHT)->hide();
  mTrayMgr->getTrayContainer(OgreBites::TL_BOTTOMRIGHT)->hide();
  mTrayMgr->getTrayContainer(OgreBites::TL_CENTER)->hide();

  congratsPanel->hide();

  Ogre::OverlayManager& overlayMgr = Ogre::OverlayManager::getSingleton();
  crosshairOverlay = overlayMgr.create("Crosshair");

  Ogre::OverlayContainer* panel = static_cast<Ogre::OverlayContainer*>(
      overlayMgr.createOverlayElement("Panel", "PanelName"));
  panel->setPosition(0.488, 0.475);
  panel->setDimensions(0.025, 0.0375);
  panel->setMaterialName("Examples/Crosshair");
  crosshairOverlay->add2D(panel);
  crosshairOverlay->show();
}
//-------------------------------------------------------------------------------------
bool TileGame::frameRenderingQueued(const Ogre::FrameEvent& evt) {
  bool ret = BaseGame::frameRenderingQueued(evt);
  int i, j;

  // update the sounds
  Ogre::Vector3 direction = mCamera->getOrientation() * Ogre::Vector3::NEGATIVE_UNIT_Z;
  //soundMgr->updateSounds(mCamera->getPosition(), direction);
  soundMgr->updateSounds(mCamera);

  if (paused)
    slowdownval += 1/1800.f;
  else {
    if (server || !multiplayerStarted)
      tileHit = sim->simulateStep(slowdownval);

    if (tileHit && !gameDone) {
      // Sound and Graphic FX
      soundMgr->playSound(hit2, ballMgr->getCollisionPosition(), mCamera);

      if (!tileEntities.empty()) {
        // Tile Sound
        if (tileEntities.size() <= noteSequence.size()) {
          soundMgr->playSound(noteSequence[tileEntities.size() - 1],
              tileEntities.back()->getParentNode()->_getDerivedPosition(), mCamera);
        }
        // Tile Texture
        tileEntities.back()->setMaterialName("Examples/ancientTileOn"); //bumpy
        tileEntities.pop_back();
        tileSceneNodes.pop_back();
      }

      // Scoring
      if (server && multiplayerStarted) {
        Uint32 scoringHost = ballMgr->popScoringHost();

        if (scoringHost != ((netMgr->getIPnbo() & BallManager::HOST_MASK) >> 16)) {
          bool found = false;
          for (i = 0; i < nPlayers && !found; i++) {
            if (scoringHost == ((playerData[i]->host & BallManager::HOST_MASK) >> 16)) {
              playerOldData[i]->score += 1;
              found = true;
            }
          }
        } else {
          score++;
        }

        updatePlayers();
      } else if (!multiplayerStarted) {
        score++;
      }

      // End of Round
      if (tileEntities.empty()) {
          gameDone = true;
          winTimer = 0;

          if (server) {
            netMgr->messageClients(PROTOCOL_TCP, STR_NXLVL.c_str());
          }
      } else if (server) {
        netMgr->messageClients(PROTOCOL_TCP, STR_TLHIT.c_str());
      }

      tileHit = false;
    }
  }

  if (gameDone && !paused) {
    if (winTimer++ == 0) {
      std::ostringstream win;
      int winner;

      congratsPanel->show();
      ballMgr->enableGravity();

      if (multiplayerStarted) {
        if (gameDone) {
          winner = findRoundWinner();
          if (winner != -1) {
            if (winner < nPlayers) {
              if (server)
                playerOldData[winner]->wins += 1;

              win << "Player ";
              win << winner;
              win << " wins Round ";
              win << currLevel;
              win << "!";
            } else {
              if (server)
                wins++;

              win << "You win Round ";
              win << currLevel;
              win << "!";
            }
          } else {
            win << "It's a tie for Round ";
            win << currLevel;
            win << "!";
          }
          winnerPanel->setCaption(win.str());
          mTrayMgr->getTrayContainer(OgreBites::TL_CENTER)->show();
        }
      }

    } else if (winTimer++ > 320) {
      levelTearDown();
      levelSetup(currLevel);
      congratsPanel->hide();
      if (multiplayerStarted)
        mTrayMgr->getTrayContainer(OgreBites::TL_CENTER)->hide();
    }
  }

  if (!animDone)
    simonSaysAnim();

  if (isCharging && chargeShot < 10000)
    chargeShot += 70;

  /* Update SDKTrays panels. */
  if (!mTrayMgr->isDialogVisible()) {
    // ScorePanel
    if (!multiplayerStarted) {
      scorePanel->setParamValue(0, Ogre::StringConverter::toString(score));
      scorePanel->setParamValue(1, Ogre::StringConverter::toString(shotsFired));
      scorePanel->setParamValue(2, Ogre::StringConverter::toString(currLevel));
    } else {
      multiScorePanel->setParamValue(0, Ogre::StringConverter::toString(Ogre::Vector2(score, wins)));
      for (i = 0; i < nPlayers; i++) {
        multiScorePanel->setParamValue(
            i + 1,
            Ogre::StringConverter::toString(Ogre::Vector2(playerOldData[i]->score, playerOldData[i]->wins))
        );
      }
      multiScorePanel->setParamValue(nPlayers + 1, Ogre::StringConverter::toString(shotsFired));
      multiScorePanel->setParamValue(nPlayers + 2, Ogre::StringConverter::toString(currLevel));
    }

    // Level complete notification.
    std::stringstream grats;
    grats << "Moving to level ";
    grats << (currLevel + 1);
    grats << "...";
    congratsPanel->setCaption(grats.str());

    // Charge shot meter.
    std::stringstream scharge;
    scharge << "|";
    for (int i = 1000; i < chargeShot; i += 160)
      scharge << "|";
    chargePanel->setCaption(scharge.str());

    // Number of players in the game.
    playersWaitingPanel->setParamValue(0,
        Ogre::StringConverter::toString(nPlayers + 1));
  }

  if (ballsounddelay > 0)
    ballsounddelay--;
  else {
    int numCollisions = ballMgr->getNumberBallCollisions();
    if (numCollisions > 0) {
      soundMgr->playSound(hit2, ballMgr->getCollisionPosition(), mCamera);
      ballsounddelay = 5;
    }
  }


  /* ***********************************************************************
   * Multiplayer Code
   */


  int broad_ticks = (BROAD_MS / SWEEP_MS);

  if (multiplayerStarted) {
    // Update players' positions locally.
    movePlayers();
    if (!server) {
      moveBalls();
      movePlayerBalls();
    }
  }

  if (netActive && (netTimer->getMilliseconds() > SWEEP_MS)) {
    std::string cmd, cmdArgs;
    std::ostringstream test;
    Uint32 *data;
    int nUp;

    /*  Received an update!  */
    if ((nUp = netMgr->scanForActivity())) {

      if (!server) {  /* **************      CLIENT      ******************* */

        if (!connected) {                       /* Running as single player. */
          if (!invitePending) {
            // Accept only the first invitation received if spammed.
            if (netMgr->udpServerData[0].updated) {
              invite = std::string(netMgr->udpServerData[0].output);
              if (std::string::npos != invite.find(STR_OPEN)) {
                mTrayMgr->getTrayContainer(OgreBites::TL_TOPRIGHT)->show();
                mTrayMgr->getTrayContainer(OgreBites::TL_BOTTOMRIGHT)->show();
                invitePending = true;
              }
              netMgr->udpServerData[0].updated = false;
            }
          }
        } else {                           /* Connected and running in game. */

          // Process UDP messages.
          for (i = 0; i < nUp; i++) {
            if (netMgr->udpServerData[i].updated) {
              data = (Uint32 *) netMgr->udpServerData[i].output;

              if ((data[0] == UINT_ADDPL) && (data[1] != netMgr->getIPnbo())) {
                j = 0;
                while (j < nPlayers && (data[1] != playerData[j]->host))
                  j++;
                if (j == nPlayers) {
                  addPlayer(++data);
                  nPlayers = playerData.size();
                }
              } else if (data[0] == UINT_UPDPL) {
                if (data[1] != netMgr->getIPnbo()) {
                  for (j = 0; j < nPlayers; j++) {
                    if (data[1] == playerData[j]->host) {
                      modifyPlayer(j, ++data);
                    }
                  }
                } else {
                  modifyScore(++data);
                }
              } else if (data[0] == UINT_UPDBL) {
                modifyBalls(++data);
              } else if (data[0] == UINT_UPDPB) {
                modifyPlayerBalls(++data);
              }
              netMgr->udpServerData[i].updated = false;
            }
          }
          // Process TCP messages.
          if (netMgr->tcpServerData.updated) {
            cmd = std::string(netMgr->tcpServerData.output);
            if (0 == cmd.find(STR_BEGIN)) {
              mTrayMgr->destroyWidget("ServerStartPanel");
              mTrayMgr->getTrayContainer(OgreBites::TL_TOPRIGHT)->hide();
              mTrayMgr->getTrayContainer(OgreBites::TL_CENTER)->hide();
              startMultiplayer();
            } else if (0 == cmd.find(STR_TLHIT)) {
              tileHit = true;
            } else if (0 == cmd.find(STR_NXLVL)) {
              std::cout << "Received NXLVL." << std::endl;
              tileHit = true;
            }

            netMgr->tcpServerData.updated = false;
          }
        }
      } else {  /* ****************      SERVER      *********************** */

        if (!connected) {        /* Initiated a server, but no game started. */
          // Update player count.
          nPlayers = netMgr->getClients();

          // If new players, add to own list and notify clients.
          if (nPlayers > playerData.size()) {
            int newClients = nPlayers - playerData.size();

            for (i = 1; i <= newClients; i++) {
              data = (Uint32 *) netMgr->udpClientData[nPlayers-i]->output;
              if (data[0] == UINT_ADDPL) {
                addPlayer(++data);
                notifyPlayers();
                netMgr->udpClientData[nPlayers-i]->updated = false;
              }
            }
            serverStartPanel->setCaption("Press (B) to start when ready.");
          }

        } else {                      /* Hosting a running game as a server. */

          for (i = 0; i < nPlayers; i++) {
            // Process UDP messages.
            if (netMgr->udpClientData[i]->updated) {
              data = (Uint32 *) netMgr->udpClientData[i]->output;
              if ((data[0] == UINT_UPDSV) && (data[1] != netMgr->getIPnbo())) {
                for (j = 0; j < nPlayers; j++) {
                  if (data[1] == playerData[j]->host) {
                    modifyPlayer(j, ++data);
                  }
                }
              }
              netMgr->udpClientData[i]->updated = false;
            }

            // Process TCP messages.
            if (netMgr->tcpClientData[i]->updated) {
              cmd = std::string(netMgr->tcpClientData[i]->output);
              data = (Uint32 *) netMgr->tcpClientData[i]->output;

              if ((data[0] == UINT_BLSHT) && (data[1] != netMgr->getIPnbo())) {
                for (j = 0; j < nPlayers; j++) {
                  if (data[1] == playerData[j]->host) {
                    modifyPlayer(j, ++data);
                  }
                }
              }

              netMgr->tcpClientData[i]->updated = false;
            }
          }
        }
      }
    }

    /* Independent of TCP/UDP update, we do these constantly. */


    if (multiplayerStarted) {                      /* In a multiplayer game. */
      // Message clients or server with global positions.
      if (server) {
        updatePlayers();
        updateBalls();
      } else {
        updateServer();
      }



    } else {                               /* Not yet in a multiplayer game. */
      // Server will broadcast game invitation every 8 seconds until launch.
      if (server && !connected && (ticks++ > broad_ticks)) {
        if (!netMgr->broadcastUDPInvitation(16))
          std::cout << "Failed to send broadcast." << std::endl;
        ticks = 0;
      }

      // Client triggers this to connect to server.
      if (inviteAccepted && !connected) {
        connected = netMgr->joinMultiPlayer(invite);
        if (!connected) {
          std::cout << "TileGame: Failed to join server." << std::endl;
          netMgr->close();
          netActive = false;
          inviteAccepted = false;
        } else {
          notifyServer();
          serverStartPanel = mTrayMgr->createLabel(OgreBites::TL_TOP,
              "ServerStartPanel", "Waiting on server...", 300);
          mTrayMgr->getTrayContainer(OgreBites::TL_TOPRIGHT)->hide();
          mTrayMgr->getTrayContainer(OgreBites::TL_CENTER)->hide();
        }
      }
    }

    netTimer->reset();
  }

  return ret;
}
//-------------------------------------------------------------------------------------
bool TileGame::keyPressed( const OIS::KeyEvent &arg ) {
  if (arg.key == OIS::KC_ESCAPE) {
    mShutDown = true;
  }
  else if (arg.key == OIS::KC_SPACE) {
  }
  else if (arg.key == OIS::KC_Y) {
    if (invitePending) {
      inviteAccepted = true;
      invitePending = false;
      ticks = 0;
    }
  }
  else if (arg.key == OIS::KC_N) {
    if (invitePending) {
      mTrayMgr->getTrayContainer(OgreBites::TL_TOPRIGHT)->hide();
      mTrayMgr->getTrayContainer(OgreBites::TL_BOTTOMRIGHT)->hide();
      mTrayMgr->getTrayContainer(OgreBites::TL_CENTER)->hide();
      invitePending = false;
    }
  } else if (arg.key == OIS::KC_B) {
    if (server && !connected && nPlayers > 0) {
      connected = true;
      netMgr->messageClients(PROTOCOL_TCP, STR_BEGIN.c_str(), STR_BEGIN.length());
      netMgr->denyConnections();
      mTrayMgr->destroyWidget("ServerStartPanel");
      mTrayMgr->getTrayContainer(OgreBites::TL_TOPRIGHT)->hide();
      mTrayMgr->getTrayContainer(OgreBites::TL_CENTER)->hide();

      startMultiplayer();
    }
  } else if (arg.key == OIS::KC_P) {
    paused = !paused;
    slowdownval = 0.0;

    soundMgr->toggleSound();
  } else if (arg.key == OIS::KC_M) {
    soundMgr->toggleSound();
  } else if (arg.key == OIS::KC_I) {
    std::cout << netMgr->getIPstring() << std::endl;
  } else if (arg.key == OIS::KC_U) {
    int i;

    std::cout << "GlobalBall host: " << ballMgr->globalBall->host << std::endl;
    for (i = 0; i < nPlayers; i++) {
      std::cout << "PlayerBall " << i << " host: " << ballMgr->playerBalls[i]->host << std::endl;
    }
    for (i = 0; i < ballMgr->mainBalls.size(); i++) {
      std::cout << "MainBall " << i << " host: " << ballMgr->mainBalls[i]->host << std::endl;
    }
  } else if (arg.key == OIS::KC_O) {
    if (netActive) {
      if (!server) {
        if ((server = netMgr->multiPlayerInit(16))) {
          serverStartPanel = mTrayMgr->createLabel(OgreBites::TL_TOP,
              "ServerStartPanel", "Waiting for clients...", 300);
          mTrayMgr->getTrayContainer(OgreBites::TL_BOTTOMRIGHT)->show();
          mTrayMgr->getTrayContainer(OgreBites::TL_TOPRIGHT)->hide();
          mTrayMgr->getTrayContainer(OgreBites::TL_CENTER)->hide();
          netTimer->reset();
          ticks = 0;
        } else {
          std::cout << "TileGame: Failed to start multiplayer game. Resuming"
              " single player mode." << std::endl;
        }
      } else {
        if (!connected) {
          mTrayMgr->destroyWidget("ServerStartPanel");
          mTrayMgr->getTrayContainer(OgreBites::TL_TOPRIGHT)->hide();
          mTrayMgr->getTrayContainer(OgreBites::TL_BOTTOMRIGHT)->hide();
          mTrayMgr->getTrayContainer(OgreBites::TL_CENTER)->hide();
          netMgr->stopServer(PROTOCOL_TCP);
          server = false;
          std::cout << "TileGame: Canceling multiplayer game. Resuming single"
              " player mode." << std::endl;
        }
      }
    }
  }
  else if (arg.key == OIS::KC_Q) {
    if(currTile >= -1)
      tileEntities[currTile+1]->setMaterialName("Examples/ancientTile"); //chrome
    currTile = tileEntities.size() - 1;
    timer.reset();
    animDone = false;
  }
  else if (arg.key == OIS::KC_K) {
    soundMgr->lowerVolume();
  }
  else if (arg.key == OIS::KC_L) {
    soundMgr->raiseVolume();
  }
  else if(arg.key == OIS::KC_T) {
    // soundMgr->playSound(chirp, mCamera->getPosition(), Ogre::Vector3(0,0,0));
  }

  return BaseGame::keyPressed(arg);
}
//-------------------------------------------------------------------------------------
bool TileGame::mousePressed( const OIS::MouseEvent &arg, OIS::MouseButtonID id ) {
  isCharging = true;
  chargeShot = 0;

  return BaseGame::mousePressed(arg, id);
}
//-------------------------------------------------------------------------------------
bool TileGame::mouseReleased( const OIS::MouseEvent &arg, OIS::MouseButtonID id ) {
  isCharging = false;
  if (chargeShot >= 1000 && !gameDone) {
    Ogre::Vector3 direction = mCamera->getOrientation() * Ogre::Vector3::NEGATIVE_UNIT_Z;
    double force = chargeShot * 0.85f;
    chargeShot = 0;
    shotsFired++;

    if (multiplayerStarted && connected) {
      if (!server) {
        updateServer(force, direction);
        return BaseGame::mouseReleased(arg, id);
      } else {
        updatePlayers(force, direction);
      }
    }

    Ogre::SceneNode* nodepc = mSceneMgr->getRootSceneNode()->createChildSceneNode();
    Ogre::Entity* ballMeshpc = mSceneMgr->createEntity("Sphere.mesh");

    if (ballMgr->isGlobalBall())
      ballMgr->removeGlobalBall();

    int x = mCamera->getPosition().x;
    int y = mCamera->getPosition().y;
    int z = mCamera->getPosition().z;
    ballMeshpc->setMaterialName("blenderSphere");
    ballMeshpc->setCastShadows(true);

    float ballSize = 120;                   //diameter
    float meshSize =  ballSize / 1;       //1 is size of the mesh in diameter

    nodepc->attachObject(ballMeshpc);
    nodepc->setScale(Ogre::Vector3(meshSize, meshSize, meshSize));

    ballMgr->setGlobalBall(ballMgr->addBall(nodepc, x, y, z, ballSize), netMgr->getIPnbo());
    ballMgr->globalBall->shot = true;
    ballMgr->globalBall->applyForce(force, direction);
    shotsFired++;
  }

  return BaseGame::mouseReleased(arg, id);
}
//-------------------------------------------------------------------------------------


#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
#define WIN32_LEAN_AND_MEAN
#include "windows.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
INT WINAPI WinMain( HINSTANCE hInst, HINSTANCE, LPSTR strCmdLine, INT )
#else
int main(int argc, char *argv[])
#endif
{
#if defined(OGRE_IS_IOS)
  NSAutoreleasePool * pool = [[NSAutoreleasePool alloc] init];
  int retVal = UIApplicationMain(argc, argv, @"UIApplication", @"AppDelegate");
  [pool release];
  return retVal;
#elif (OGRE_PLATFORM == OGRE_PLATFORM_APPLE) && __LP64__
  NSAutoreleasePool * pool = [[NSAutoreleasePool alloc] init];

  mAppDelegate = [[AppDelegate alloc] init];
  [[NSApplication sharedApplication] setDelegate:mAppDelegate];
  int retVal = NSApplicationMain(argc, (const char **) argv);

  [pool release];

  return retVal;
#else
  // Create application object
  TileGame app;

  try {
    app.go();
  } catch( Ogre::Exception& e ) {
#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
    MessageBox( NULL, e.getFullDescription().c_str(), "An exception has occured!", MB_OK | MB_ICONERROR | MB_TASKMODAL);
#else
    std::cerr << "An exception has occured: " <<
        e.getFullDescription().c_str() << std::endl;
#endif
  }
#endif
  return 0;
}

#ifdef __cplusplus
}
#endif
