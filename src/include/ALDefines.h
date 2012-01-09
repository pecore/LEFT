#ifndef _ALDEFINES_H_
#define _ALDEFINES_H_

#include "al.h"
#include <list>

struct Sound {
  void * data;
  unsigned long size;
  unsigned long format;
  unsigned long freq;
};
typedef std::list<Sound *> SoundList;

class SoundPlayer {
public:
  static void init();
  static void clear();

  static bool play(int index);
  static Sound * load(const char * filename);
  static SoundList Sounds;
private:
  SoundPlayer() { }
  ~SoundPlayer() { }
};

#endif
