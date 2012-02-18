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
    mButtons.push_back(new HUDButton("data\\btn_bfg_inactive.png", "data\\btn_bfg_active.png"));
    mTurboIcon = new HUDButton("data\\turbo_loading.png", "data\\turbo.png");
    mTurboBar = new GLParticle(35, 100, 0.0f, 0.8f, 0.3f, 1.0f, glpSolid);
    mTurboOpacity = 1.0f;
    mTurboLoading = false;
    mFont = gResources->getFont("data\\euphemia.fnt")->font;
    mHealth = 100.0f;
    mActive = 2;
  }
  ~HUD() {
    HUDButton * b = 0;
    foreach(std::list<HUDButton *>, b, mButtons) {
      delete b;
    }
    delete mTurboIcon;
  }

  GLParticle * mTurboBar;

  unsigned int count() { return mButtons.size(); }

  void setTurboOpacity(GLfloat o) { mTurboBar->setSize(20.0f, 100 * o); }
  void setTurboLoading(bool loading) { mTurboLoading = loading; }

  void setActive(unsigned int a) { mActive = (a >= 1 && a <= mButtons.size()) ? a : mActive; }
  void nextActive() { mActive++; if(mActive > mButtons.size()) mActive = 1; }
  unsigned int getActive() { return mActive; }
  
  void setHealth(GLfloat health) { mHealth = health; }
  void addHealth(GLfloat delta) { mHealth += delta; }
  GLfloat getHealth() { return mHealth; }

  void draw(GLfloat * opacity) {
    GLfloat offset = 100.0f;
    HUDButton * b = 0;
    unsigned int i = 1;
    foreach(std::list<HUDButton *>, b, mButtons) {
      if((i == mActive)) {
        GLvector3f turbocolor(0.0f, 0.8f, 0.3f);
        if(mTurboLoading) {
          turbocolor = GLvector3f(0.6f, 0.0f, 0.0f);
        }
        mTurboBar->setColor(turbocolor, 1.0f);
        mTurboBar->moveTo(GL_SCREEN_BOTTOMLEFT.x + 15.0f, GL_SCREEN_BOTTOMLEFT.y + offset + b->active->h() / 2.0f + mTurboBar->h() / 2.0f);
        mTurboBar->draw();
        b->active->moveTo(GL_SCREEN_BOTTOMLEFT.x + b->active->w() / 2.0f, GL_SCREEN_BOTTOMLEFT.y + offset);
        b->active->draw();
      }
      i++;
    }
    glColor3f(0.3f, 0.6f, 0.9f);
    glFontPrint(mFont, GLvector2f(20.0f, 20.0f), "%3.0f", mHealth);

#if 0
    mTurboIcon->inactive->moveTo(GL_SCREEN_BOTTOMLEFT.x + 50.0f, GL_SCREEN_BOTTOMLEFT.y + 85.0f);
    mTurboIcon->inactive->draw();
    mTurboIcon->active->setAlpha(mTurboOpacity);
    mTurboIcon->active->moveTo(GL_SCREEN_BOTTOMLEFT.x + 50.0f, GL_SCREEN_BOTTOMLEFT.y + 85.0f);
    mTurboIcon->active->draw();
#endif
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
