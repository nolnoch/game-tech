/*
-----------------------------------------------------------------------------
Filename:    BaseGame.cpp
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
#include "BaseGame.h"

//-------------------------------------------------------------------------------------
BaseGame::BaseGame(void)
: mRoot(0),
  mCamera(0),
  mSceneMgr(0),
  mWindow(0),
  mResourcesCfg(Ogre::StringUtil::BLANK),
  mPluginsCfg(Ogre::StringUtil::BLANK),
  mTrayMgr(0),
  mCameraMan(0),
  mDetailsPanel(0),
  mCursorWasVisible(false),
  mShutDown(false),
  mInputManager(0),
  mMouse(0),
  mKeyboard(0)
{
}

//-------------------------------------------------------------------------------------
BaseGame::~BaseGame(void)
{
  if (mTrayMgr) delete mTrayMgr;
  if (mCameraMan) delete mCameraMan;

  //Remove ourself as a Window listener
  Ogre::WindowEventUtilities::removeWindowEventListener(mWindow, this);
  windowClosed(mWindow);
  delete mRoot;
}

//-------------------------------------------------------------------------------------
bool BaseGame::configure(void)
{
  // Show the configuration dialog and initialise the system
  // You can skip this and use root.restoreConfig() to load configuration
  // settings if you were sure there are valid ones saved in ogre.cfg
  if(mRoot->showConfigDialog())
  {
    // If returned true, user clicked OK so initialise
    // Here we choose to let the system create a default rendering window by passing 'true'
    mWindow = mRoot->initialise(true, "TutorialApplication Render Window");

    return true;
  }
  else
  {
    return false;
  }
}
//-------------------------------------------------------------------------------------
void BaseGame::chooseSceneManager(void)
{
  // Get the SceneManager, in this case a generic one
  mSceneMgr = mRoot->createSceneManager(Ogre::ST_GENERIC);
  mSceneMgr->setShadowTechnique(Ogre::SHADOWTYPE_TEXTURE_MODULATIVE);
  mSceneMgr->setShadowFarDistance(2500.0);
}
//-------------------------------------------------------------------------------------
void BaseGame::createCamera(void)
{
  // Create the camera
  mCamera = mSceneMgr->createCamera("PlayerCam");
  mCamera->setFOVy(Ogre::Radian(1.50));

  // Position it at 500 in Z direction
  mCamera->setPosition(Ogre::Vector3(2000,0,2000));
  // Look back along -Z
  mCamera->lookAt(Ogre::Vector3::ZERO);
  mCamera->setNearClipDistance(5);
}
//-------------------------------------------------------------------------------------
void BaseGame::createFrameListener(void)
{
  Ogre::LogManager::getSingletonPtr()->logMessage("*** Initializing OIS ***");
  OIS::ParamList pl;
  size_t windowHnd = 0;
  std::ostringstream windowHndStr;

  mWindow->getCustomAttribute("WINDOW", &windowHnd);
  windowHndStr << windowHnd;
  pl.insert(std::make_pair(std::string("WINDOW"), windowHndStr.str()));

  mInputManager = OIS::InputManager::createInputSystem( pl );

  mKeyboard = static_cast<OIS::Keyboard*>(mInputManager->createInputObject( OIS::OISKeyboard, true ));
  mMouse = static_cast<OIS::Mouse*>(mInputManager->createInputObject( OIS::OISMouse, true ));

  mMouse->setEventCallback(this);
  mKeyboard->setEventCallback(this);

  //Set initial mouse clipping size
  windowResized(mWindow);

  //Register as a Window listener
  Ogre::WindowEventUtilities::addWindowEventListener(mWindow, this);

  mTrayMgr = new OgreBites::SdkTrayManager("InterfaceName", mWindow, mMouse, this);
  mTrayMgr->hideFrameStats();
  mTrayMgr->hideLogo();
  mTrayMgr->hideCursor();

  Ogre::FontManager::getSingleton().getByName("SdkTrays/Caption")->load();

  // create a params panel for displaying sample details
  Ogre::StringVector items;
  items.push_back("cam.pX");
  items.push_back("cam.pY");
  items.push_back("cam.pZ");
  items.push_back("");
  items.push_back("cam.oW");
  items.push_back("cam.oX");
  items.push_back("cam.oY");
  items.push_back("cam.oZ");
  items.push_back("");
  items.push_back("Filtering");
  items.push_back("Poly Mode");

  mDetailsPanel = mTrayMgr->createParamsPanel(OgreBites::TL_NONE, "DetailsPanel", 200, items);
  mDetailsPanel->setParamValue(9, "Bilinear");
  mDetailsPanel->setParamValue(10, "Solid");
  mDetailsPanel->hide();

  mRoot->addFrameListener(this);
}
//-------------------------------------------------------------------------------------
void BaseGame::destroyScene(void)
{
}
//-------------------------------------------------------------------------------------
void BaseGame::createViewports(void)
{
  // Create one viewport, entire window
  Ogre::Viewport* vp = mWindow->addViewport(mCamera);
  vp->setBackgroundColour(Ogre::ColourValue(0,0,0));

  // Alter the camera aspect ratio to match the viewport
  mCamera->setAspectRatio(
      Ogre::Real(vp->getActualWidth()) / Ogre::Real(vp->getActualHeight()));
}
//-------------------------------------------------------------------------------------
void BaseGame::setupResources(void)
{
  // Load resource paths from config file
  Ogre::ConfigFile cf;
  cf.load(mResourcesCfg);

  // Go through all sections & settings in the file
  Ogre::ConfigFile::SectionIterator seci = cf.getSectionIterator();

  Ogre::String secName, typeName, archName;
  while (seci.hasMoreElements())
  {
    secName = seci.peekNextKey();
    Ogre::ConfigFile::SettingsMultiMap *settings = seci.getNext();
    Ogre::ConfigFile::SettingsMultiMap::iterator i;
    for (i = settings->begin(); i != settings->end(); ++i)
    {
      typeName = i->first;
      archName = i->second;
#if OGRE_PLATFORM == OGRE_PLATFORM_APPLE
      // OS X does not set the working directory relative to the app,
      // In order to make things portable on OS X we need to provide
      // the loading with it's own bundle path location
      if (!Ogre::StringUtil::startsWith(archName, "/", false)) // only adjust relative dirs
        archName = Ogre::String(Ogre::macBundlePath() + "/Contents/Resources/" + archName);
#endif
      Ogre::ResourceGroupManager::getSingleton().addResourceLocation(
          archName, typeName, secName);
    }
  }
}
//-------------------------------------------------------------------------------------
void BaseGame::createResourceListener(void)
{

}
//-------------------------------------------------------------------------------------
void BaseGame::loadResources(void)
{
  Ogre::ResourceGroupManager::getSingleton().initialiseAllResourceGroups();
}
//-------------------------------------------------------------------------------------
void BaseGame::go(void)
{
#ifdef _DEBUG
  mResourcesCfg = "resources_d.cfg";
  mPluginsCfg = "plugins_d.cfg";
#else
  mResourcesCfg = "resources.cfg";
  mPluginsCfg = "plugins.cfg";
#endif

#if OGRE_PLATFORM == OGRE_PLATFORM_APPLE_IOS || OGRE_PLATFORM == OGRE_PLATFORM_APPLE
  Ogre::String workingDir = Ogre::macBundlePath()+"/Contents/Resources/";
  mResourcesCfg = workingDir + mResourcesCfg;
  mPluginsCfg = workingDir + mPluginsCfg;
#endif

  if (!setup())
    return;

  mRoot->startRendering();

  // clean up
  destroyScene();
}
//-------------------------------------------------------------------------------------
bool BaseGame::setup(void)
{
  mRoot = new Ogre::Root(mPluginsCfg);

  setupResources();

  bool carryOn = configure();
  if (!carryOn) return false;

  chooseSceneManager();
  createCamera();
  createViewports();

  // Set default mipmap level (NB some APIs ignore this)
  Ogre::TextureManager::getSingleton().setDefaultNumMipmaps(5);

  // Create any resource listeners (for loading screens)
  createResourceListener();
  // Load resources
  loadResources();

  // Create the scene
  createScene();

  createFrameListener();

  return true;
};
//-------------------------------------------------------------------------------------
bool BaseGame::frameRenderingQueued(const Ogre::FrameEvent& evt)
{
  if(mWindow->isClosed())
    return false;

  if(mShutDown)
    return false;

  //Need to capture/update each device
  mKeyboard->capture();
  mMouse->capture();

  mTrayMgr->frameRenderingQueued(evt);

  if (!mTrayMgr->isDialogVisible())
  {
    mCameraMan->frameRenderingQueued(evt);   // if dialog isn't up, then update the camera
    if (mDetailsPanel->isVisible())   // if details panel is visible, then update its contents
    {
      mDetailsPanel->setParamValue(0, Ogre::StringConverter::toString(mCamera->getDerivedPosition().x));
      mDetailsPanel->setParamValue(1, Ogre::StringConverter::toString(mCamera->getDerivedPosition().y));
      mDetailsPanel->setParamValue(2, Ogre::StringConverter::toString(mCamera->getDerivedPosition().z));
      mDetailsPanel->setParamValue(4, Ogre::StringConverter::toString(mCamera->getDerivedOrientation().w));
      mDetailsPanel->setParamValue(5, Ogre::StringConverter::toString(mCamera->getDerivedOrientation().x));
      mDetailsPanel->setParamValue(6, Ogre::StringConverter::toString(mCamera->getDerivedOrientation().y));
      mDetailsPanel->setParamValue(7, Ogre::StringConverter::toString(mCamera->getDerivedOrientation().z));
    }
  }

  return true;
}
//-------------------------------------------------------------------------------------
bool BaseGame::keyPressed( const OIS::KeyEvent &arg )
{
  if (mTrayMgr->isDialogVisible()) return true;   // don't process any more keys if dialog is up

  if (arg.key == OIS::KC_F)   // toggle visibility of advanced frame stats
  {
    mTrayMgr->toggleAdvancedFrameStats();
  }
  else if (arg.key == OIS::KC_G)   // toggle visibility of even rarer debugging details
  {
    if (mDetailsPanel->getTrayLocation() == OgreBites::TL_NONE)
    {
      mTrayMgr->moveWidgetToTray(mDetailsPanel, OgreBites::TL_TOPRIGHT, 0);
      mDetailsPanel->show();
    }
    else
    {
      mTrayMgr->removeWidgetFromTray(mDetailsPanel);
      mDetailsPanel->hide();
    }
  }
  else if (arg.key == OIS::KC_T)   // cycle polygon rendering mode
  {
    Ogre::String newVal;
    Ogre::TextureFilterOptions tfo;
    unsigned int aniso;

    switch (mDetailsPanel->getParamValue(9).asUTF8()[0])
    {
      case 'B':
        newVal = "Trilinear";
        tfo = Ogre::TFO_TRILINEAR;
        aniso = 1;
        break;
      case 'T':
        newVal = "Anisotropic";
        tfo = Ogre::TFO_ANISOTROPIC;
        aniso = 8;
        break;
      case 'A':
        newVal = "None";
        tfo = Ogre::TFO_NONE;
        aniso = 1;
        break;
      default:
        newVal = "Bilinear";
        tfo = Ogre::TFO_BILINEAR;
        aniso = 1;
        break;
    }

    Ogre::MaterialManager::getSingleton().setDefaultTextureFiltering(tfo);
    Ogre::MaterialManager::getSingleton().setDefaultAnisotropy(aniso);
    mDetailsPanel->setParamValue(9, newVal);
  }
  else if (arg.key == OIS::KC_R)   // cycle polygon rendering mode
  {
    Ogre::String newVal;
    Ogre::PolygonMode pm;

    switch (mCamera->getPolygonMode())
    {
      case Ogre::PM_SOLID:
        newVal = "Wireframe";
        pm = Ogre::PM_WIREFRAME;
        break;
      case Ogre::PM_WIREFRAME:
        newVal = "Points";
        pm = Ogre::PM_POINTS;
        break;
      default:
        newVal = "Solid";
        pm = Ogre::PM_SOLID;
        break;
    }

    mCamera->setPolygonMode(pm);
    mDetailsPanel->setParamValue(10, newVal);
  }
  else if(arg.key == OIS::KC_F5)   // refresh all textures
  {
    Ogre::TextureManager::getSingleton().reloadAll();
  }
  else if (arg.key == OIS::KC_SYSRQ)   // take a screenshot
  {
    mWindow->writeContentsToTimestampedFile("screenshot", ".jpg");
  }
  else if (arg.key == OIS::KC_ESCAPE)
  {
    mShutDown = true;
  }

  mCameraMan->injectKeyDown(arg);
  return true;
}

bool BaseGame::keyReleased( const OIS::KeyEvent &arg )
{
  mCameraMan->injectKeyUp(arg);
  return true;
}

bool BaseGame::mouseMoved( const OIS::MouseEvent &arg )
{
  if (mTrayMgr->injectMouseMove(arg)) return true;
  mCameraMan->injectMouseMove(arg);
  return true;
}

bool BaseGame::mousePressed( const OIS::MouseEvent &arg, OIS::MouseButtonID id )
{
  if (mTrayMgr->injectMouseDown(arg, id)) return true;
  mCameraMan->injectMouseDown(arg, id);
  return true;
}

bool BaseGame::mouseReleased( const OIS::MouseEvent &arg, OIS::MouseButtonID id )
{
  if (mTrayMgr->injectMouseUp(arg, id)) return true;
  mCameraMan->injectMouseUp(arg, id);
  return true;
}

//Adjust mouse clipping area
void BaseGame::windowResized(Ogre::RenderWindow* rw)
{
  unsigned int width, height, depth;
  int left, top;
  rw->getMetrics(width, height, depth, left, top);

  const OIS::MouseState &ms = mMouse->getMouseState();
  ms.width = width;
  ms.height = height;
}

//Unattach OIS before window shutdown (very important under Linux)
void BaseGame::windowClosed(Ogre::RenderWindow* rw)
{
  //Only close for window that created OIS (the main window in these demos)
  if( rw == mWindow )
  {
    if( mInputManager )
    {
      mInputManager->destroyInputObject( mMouse );
      mInputManager->destroyInputObject( mKeyboard );

      OIS::InputManager::destroyInputSystem(mInputManager);
      mInputManager = 0;
    }
  }
}
