#ifndef _ROBOTROCKETEFFECT_H_
#define _ROBOTROCKETEFFECT_H_

#include "GLDefines.h"
#include "GLParticleEffect.h"

#define FIRE_PARTICLE_COUNT 50

class RobotRocketEffect : public GLParticleEffect {
public:
  RobotRocketEffect(GLfloat x, GLfloat y, GLfloat width, GLfloat height);
  ~RobotRocketEffect();

  void moveTo(GLfloat x, GLfloat y);
  void setVelocity2f(GLfloat x, GLfloat y) { mVelocity.x = x; mVelocity.y = y; };
  void setVelocity(GLvector2f & v) { mVelocity = v; };
  void setRotation(GLfloat x, GLfloat y, GLfloat angle);

  GLvector2f direction() { return (mdestMid - msrcMid); };

  void draw();

private:
  GLvector2f mPos;
  GLvector2f mVelocity;
  GLvector2f mRotation;
  GLfloat mAngle;

  void recalculateVectors();
  void spawn(GLParticleDummy * p, int i);

  GLfloat mMaxDistance;
  GLvector3f mColor[3];
  int mLifeTime[FIRE_PARTICLE_COUNT];

  GLfloat mWidth;
  GLfloat mHeight;

  GLvector2f msrcA; // line between 2 points
  GLvector2f msrcB;
  GLvector2f msrcAB;
  GLvector2f msrcMid;

  GLvector2f mdestA; // line between 2 points
  GLvector2f mdestB;
  GLvector2f mdestAB;
  GLvector2f mdestMid;
};

#endif
