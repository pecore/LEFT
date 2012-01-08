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

  mWidth = mSprite->w();
  mHeight = mSprite->h();

  mInitialized = true;
}

bool RocketProjectile::collide(GLvector2f n, GLfloat distance)
{
  if(!mInitialized) return true;

  mMap->addCirclePolygon(mPos, 150.0f);
  return false;
}

void RocketProjectile::draw()
{
  if(!mInitialized) return;

  GLfloat angle = mVelocity.angle() * 360.0f / (2.0f * M_PI);

  mSprite->moveTo(mPos.x, mPos.y);
  mRocketEffect->moveTo(mPos.x - 7.0f, mPos.y - 3.0f);
  mSprite->setRotation(mSprite->pos().x, mSprite->pos().y, angle);
  mRocketEffect->setRotation(mSprite->pos().x, mSprite->pos().y, angle - 90.0);
  mSprite->draw();
  mRocketEffect->draw();
}
