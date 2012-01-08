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

class PlayStreamThread {
public:
  static void init();
  static void shutdown();

  static bool play(int index);
  static Sound * load(const char * filename);
  static SoundList Sounds;
private:
  PlayStreamThread() { }
  ~PlayStreamThread() { }
};

#endif