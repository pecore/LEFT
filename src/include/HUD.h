/*
    Copyright (c) 2011   LEFT PROJECT
    All rights reserved.

    file authors:
    Jan Christian Meyer
*/

#ifndef _HUD_H_
#define _HUD_H_

#include "GLDefines.h"
#define MAP_COLLIDABLES_MAX 2048

#include "Collidable.h"
#include "Projectile.h"
#include "GLParticle.h"
#include "GLFont.h"

class HUDButton {
public:
  HUDButton(const char * finactive, const char * factive) {
    inactive = new GLSprite(finactive);
    active = new GLSprite(factive);
  }
  ~HUDButton() {
    delete inactive;
    delete active;
  }
  GLSprite * inactive;
  GLSprite * active;
};

class HUD {
public:
  HUD() {
    mButtons.push_back(new HUDButton("data\\btn_rocket_inactive.png", "data\\btn_rocket_active.png"));
    mButtons.push_back(new HUDButton("data\\btn_shotgun_inactive.png", "data\\btn_shotgun_active.png"));
    mButtons.push_back(new HUDButton("data\\btn_grenade_inactive.png", "data\\btn_grenade_active.png"));
    mTurboIcon = new HUDButton("data\\turbo_loading.png", "data\\turbo.png");
    mTurboOpacity = 1.0f;
    mTurboLoading = false;
    mFont = gResources->getFont("data\\euphemia.fnt")->font;
    mHealth = 100.0f;
    mActive = 0;
  }
  ~HUD() {
    HUDButton * b = 0;
    foreach(std::list<HUDButton *>, b, mButtons) {
      delete b;
    }
    delete mTurboIcon;
  }

  void setTurboOpacity(GLfloat o) { mTurboOpacity = o; }
  void setTurboLoading(bool loading) { mTurboLoading = loading; }

  void setActive(unsigned int a) { mActive = (a >= 1 && a <= 3) ? a : mActive; }
  void nextActive() { mActive++; if(mActive > 3) mActive = 1; }
  unsigned int getActive() { return mActive; }
  
  void setHealth(GLfloat health) { mHealth = health; }
  void addHealth(GLfloat delta) { mHealth += delta; }
  GLfloat getHealth() { return mHealth; }

  void draw(GLfloat * opacity) {
    GLfloat offset = 450.0f;
    HUDButton * b = 0;
    unsigned int i = 1;
    foreach(std::list<HUDButton *>, b, mButtons) {
      GLSprite * s = (i == mActive) ? b->active : b->inactive;
      s->moveTo(GL_SCREEN_BOTTOMLEFT.x + offset, GL_SCREEN_BOTTOMLEFT.y + s->h() / 2.0f);
      offset += s->w() + 10.0f;
      s->setAlpha(opacity[i-1]);
      s->draw();
      i++;
    }
    glColor3f(0.3f, 0.6f, 0.9f);
    glFontPrint(mFont, GLvector2f(20.0f, 20.0f), "%3.0f", mHealth);

    GLSprite * s = mTurboLoading ? mTurboIcon->inactive : mTurboIcon->active;
    s->moveTo(GL_SCREEN_BOTTOMLEFT.x + 115.0f, GL_SCREEN_BOTTOMLEFT.y + 30.0f);
    s->setAlpha(mTurboOpacity);
    s->draw();
  }

private:
  std::list<HUDButton *> mButtons;
  unsigned int mActive;
  bm_font * mFont;

  GLfloat mHealth;
  HUDButton * mTurboIcon;
  GLfloat mTurboOpacity;
  bool mTurboLoading;
};

#endif
