

class CameraMan : public OgreBites::SdkCameraMan
{
  public:
    CameraMan(Ogre::Camera* c ) : OgreBites::SdkCameraMan(c)
    {
        
    }

    virtual bool frameRenderingQueued(const Ogre::FrameEvent& evt)
    {
        //if (mStyle == CS_FREELOOK)
        {
            // build our acceleration vector based on keyboard input composite
            Ogre::Vector3 accel = Ogre::Vector3::ZERO;
            if (mGoingForward) accel += mCamera->getDirection();
            if (mGoingBack) accel -= mCamera->getDirection();
            if (mGoingRight) accel += mCamera->getRight();
            if (mGoingLeft) accel -= mCamera->getRight();
            if (mGoingUp) accel += mCamera->getUp();
            if (mGoingDown) accel -= mCamera->getUp();

            // if accelerating, try to reach top speed in a certain time
            Ogre::Real topSpeed = mFastMove ? mTopSpeed * 5 : mTopSpeed;
            if (accel.squaredLength() != 0)
            {
                accel.normalise();
                mVelocity += accel * topSpeed * evt.timeSinceLastFrame * 10;
            }
            // if not accelerating, try to stop in a certain time
            else 
                mVelocity -= mVelocity * evt.timeSinceLastFrame * 10;

            // keep camera velocity below top speed and above zero
            if (mVelocity.squaredLength() > topSpeed * topSpeed)
            {
                mVelocity.normalise();
                mVelocity *= topSpeed;
            }
            else if (mVelocity.squaredLength() < 0.1) 
                mVelocity = Ogre::Vector3::ZERO;

            if (mVelocity != Ogre::Vector3::ZERO)
                mCamera->move(mVelocity * evt.timeSinceLastFrame);

            int minbound = -1100.0;
            int maxbound = 1100.0;

            if(mCamera->getPosition().x > maxbound)
                mCamera->setPosition(maxbound, mCamera->getPosition().y, mCamera->getPosition().z);
            else if(mCamera->getPosition().x < minbound)
                mCamera->setPosition(minbound, mCamera->getPosition().y, mCamera->getPosition().z);

            if(mCamera->getPosition().y > maxbound)
                mCamera->setPosition(mCamera->getPosition().x, maxbound, mCamera->getPosition().z);
            else if(mCamera->getPosition().y < minbound)
                mCamera->setPosition(mCamera->getPosition().x, minbound, mCamera->getPosition().z);

            if(mCamera->getPosition().z > maxbound)
                mCamera->setPosition(mCamera->getPosition().x, mCamera->getPosition().y, maxbound);
            else if(mCamera->getPosition().z < minbound)
                mCamera->setPosition(mCamera->getPosition().x, mCamera->getPosition().y, minbound);
        }

        return true;
    }

};
