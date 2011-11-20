/*
    Copyright (c) 2011   LEFT PROJECT
    All rights reserved.

    file authors:
    Jan Christian Meyer
*/

#ifndef _RIGIDBODY_H_
#define _RIGIDBODY_H_

#include "GLDefines.h"
#include "Collidable.h"

class RigidBody : public Collidable {
public:
  RigidBody() { };
  ~RigidBody() { };
  
  GLvector2f pos() { return mPos; };

  virtual void integrate(GLfloat dt) { };
  virtual void collide(GLvector2f base, GLvector2f dir) { };
    
  void update(GLfloat dt) {
    if(mMass > 0.0f) {
      mVelocity += gravity * dt;
    }
  }

protected:
  GLvector2f mPos;
  GLvector2f mVelocity;
  GLfloat mMass;

private:
};

#endif
