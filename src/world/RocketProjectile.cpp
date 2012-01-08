#include "Projectile.h"

#include "Map.h"
#include "GLSprite.h"
#include "RobotRocketEffect.h"

RocketProjectile::RocketProjectile(GLvector2f pos, GLvector2f velocity, Map * map)
{
  mInitialized = false;
  mMap = map;
  mPos = pos;
  mVelocity = velocity;
}

RocketProjectile::~RocketProjectile()
{
  if(!mInitialized) return;
  delete mSprite;
  delete mRocketEffect;
}

void RocketProjectile::init()
{
  if(mInitialized) return;
  mSprite = new GLSprite("data\\rocketprojectile.png");
  mRocketEffect = new RobotRocketEffect(mPos.x, mPos.y, 14.0f, 18.0f, 2, 60);
  mLight = new LightSource(mPos, GLvector3f(1.0f, 1.0f, 0.0f), 0.1, glpLight);
  mMap->LightSources().push_back(mLight);
  mWidth = mSprite->w();
  mHeight = mSprite->h();
  mInitialized = true;
}

bool RocketProjectile::collide(GLvector2f n, GLfloat distance)
{
  if(!mInitialized) return true;
  if(mPos.x < (GL_SCREEN_FWIDTH * GL_SCREEN_FACTOR) - GL_MAP_THRESHOLD &&
     mPos.x > GL_MAP_THRESHOLD &&
     mPos.y < (GL_SCREEN_FHEIGHT * GL_SCREEN_FACTOR) - GL_MAP_THRESHOLD &&
     mPos.y > GL_MAP_THRESHOLD) mMap->addCirclePolygon(mPos, 100.0f);
  mMap->LightSources().remove(mLight);
  mMap->playAnimation(new GLAnimatedSprite("data\\explode.png", mPos, 64, 64));
  delete mLight;
  return false;
}

void RocketProjectile::draw()
{
  if(!mInitialized) return;
  GLfloat angle = mVelocity.angle() * 360.0f / (2.0f * M_PI);
  
  mLight->pos = mPos;
  mLight->angle = angle;
  mSprite->moveTo(mPos.x, mPos.y);
  mRocketEffect->moveTo(mPos.x - 7.0f, mPos.y - 3.0f);
  mSprite->setRotation(mSprite->pos().x, mSprite->pos().y, angle);
  mRocketEffect->setRotation(mSprite->pos().x, mSprite->pos().y, angle - 90.0);
  mSprite->draw();
  mRocketEffect->draw();
  mVelocity *= 1.02f;
}