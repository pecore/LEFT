/*
    Copyright (c) 2011   LEFT PROJECT
    All rights reserved.

    file authors:
    Jan Christian Meyer
*/

#ifndef _SoundPlayer_H_
#define _SoundPlayer_H_

#include "al.h"
#include <list>

struct Sound {
  unsigned char * data;
  unsigned long size;
  unsigned long format;
  unsigned long freq;
  float volume;
};
typedef std::list<Sound *> SoundList;

class SoundPlayer {
public:
  static void init();
  static void clear();

  static bool play(Sound * s);
  static Sound * load(const char * filename);
private:
  SoundPlayer() { }
  ~SoundPlayer() { }
  static bool initOpenAL();
};

#endif
