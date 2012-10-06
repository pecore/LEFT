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

#include "Map.h"
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
  HUD(Map * map) {
    mMap = map;
    mButtons.push_back(new HUDButton(GL_RESOURCE_DATAPATH "btn_rocket_inactive.png", "data\\btn_rocket_active.png"));
    mButtons.push_back(new HUDButton(GL_RESOURCE_DATAPATH "btn_shotgun_inactive.png", "data\\btn_shotgun_active.png"));
    mButtons.push_back(new HUDButton(GL_RESOURCE_DATAPATH "btn_grenade_inactive.png", "data\\btn_grenade_active.png"));
    mButtons.push_back(new HUDButton(GL_RESOURCE_DATAPATH "btn_bfg_inactive.png", "data\\btn_bfg_active.png"));
    mFont = gResources->getFont(GL_RESOURCE_DATAPATH "couriernew.fnt")->font;
    
    mTurboBar = new GLParticle(16, 16, 0.0f, 0.8f, 0.3f, 1.0f, glpSolid);
    mTurboCharge = 1.0f;
    mTurboLoading = false;
    
    mHealthBar = new GLParticle(16, 16, 0.0f, 0.8f, 0.3f, 1.0f, glpSolid);
    mHealthBar->setSize(30.0f, 80.0f);
    mHealth = 100.0f;

    mActive = 2;
  }
  ~HUD() {
    HUDButton * b = 0;
    foreach(std::list<HUDButton *>, b, mButtons) {
      delete b;
    }
    if(mTurboBar) {
      delete mTurboBar;
    }
    if(mHealthBar) {
      delete mHealthBar;
    }
  }

  unsigned int count() { return mButtons.size(); }

  void setTurboCharge(GLfloat o) { mTurboBar->setSize(30.0f, 80.0f * o); mTurboCharge = o; }
  void setTurboLoading(bool loading) { mTurboLoading = loading; }

  void setActive(unsigned int a) { mActive = (a >= 1 && a <= mButtons.size()) ? a : mActive; }
  void nextActive() { mActive++; if(mActive > mButtons.size()) mActive = 1; }
  unsigned int getActive() { return mActive; }
  
  void setHealth(GLfloat health) { mHealth = health; }
  void addHealth(GLfloat delta) { mHealth += delta; }
  GLfloat getHealth() { return mHealth; }

  void draw(GLfloat * opacity) {
    HUDButton * b = 0;
    unsigned int i = 1;
    foreach(std::list<HUDButton *>, b, mButtons) {
      GLfloat offset = 15.0f;
      switch(mActive) {
      case 1: offset = 0; break;
      case 3: offset = 12.0f; break;
      case 4: offset = 12.0f; break;
      }
      if((i == mActive)) {
        b->active->moveTo(GL_SCREEN_BOTTOMLEFT.x + 75.0f + b->active->w() / 2.0f, GL_SCREEN_BOTTOMLEFT.y + offset + b->active->h() / 2.0f );
        b->active->draw();

        GLvector3f turbocolor(0.0f, 0.8f, 0.3f);
        if(mTurboLoading) {
          turbocolor = GLvector3f(0.6f, 0.0f, 0.0f);
        }
        mTurboBar->setColor(turbocolor, 1.0f);
        mTurboBar->moveTo(GL_SCREEN_BOTTOMLEFT.x + mTurboBar->w() / 2.0f, GL_SCREEN_BOTTOMLEFT.y + mTurboBar->h() / 2.0f);
        mTurboBar->draw();
        glColor3f(0.0f, 0.0f, 0.0f);
        glFontPrint(mFont, GLvector2f(0.0f, 2.0f), "%03d", (int)(mTurboCharge * 100.0f));

        mHealthBar->setColor4f(0.3f, 0.6f, 0.9f, 1.0f);
        mHealthBar->moveTo(GL_SCREEN_BOTTOMLEFT.x + mHealthBar->w() * 1.5f, GL_SCREEN_BOTTOMLEFT.y + mHealthBar->h() / 2.0f);
        mHealthBar->draw();
        glColor3f(0.0f, 0.0f, 0.0f);
        glFontPrint(mFont, GLvector2f(0.0f + mHealthBar->w(), 2.0f), "%03d", (int)mHealth);
      }
      i++;
    }
    glColor3f(0.3f, 0.6f, 0.9f);
    
    mMap->drawMinimap();
  }

private:
  Map * mMap;

  std::list<HUDButton *> mButtons;
  GLParticle * mTurboBar;
  GLParticle * mHealthBar;
  bm_font * mFont;

  unsigned int mActive;  
  GLfloat mHealth;
  GLfloat mTurboCharge;
  bool mTurboLoading;
};

#endif
