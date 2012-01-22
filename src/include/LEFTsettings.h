/*
    Copyright (c) 2011   LEFT PROJECT
    All rights reserved.

    file authors:
    Jan Christian Meyer
*/

#ifndef _LEFTSETTINGS_H_
#define _LEFTSETTINGS_H_
#include "GLDefines.h"

enum SettingsType {
  stPointer = 0,
  stInt, 
  stFloat,
};

class SettingsValue {
public:
  SettingsValue() 
  { 
    mType = stInt;
    mData.i = 0;
  }
  
  ~SettingsValue() 
  {
    if(mType == stPointer && mData.p) {
      delete mData.p;
    }
  }

#define GETTER(name, t, st, v, def) t get ## name () { if(mType == (st)) { return (t) mData. ## v; } else { return def; } }

  GETTER(s, char *, stPointer, p, 0)
  GETTER(i, int   , stInt    , i, 0)
  GETTER(f, float , stFloat  , f, 0.0f)

  SettingsValue(char * value) 
  {
    mType = stPointer;
    unsigned int size = strlen(value); 
    mData.p = new char[size];
    if(mData.p) {
      strcpy((char *) mData.p, value);
    }
  }
  
  SettingsValue(int value) 
  {
    mType = stInt;
    mData.i = value;
  }

  SettingsValue(float value)
  {
    mType = stFloat;
    mData.f = value;
  }

private: 
  SettingsType mType;
  union {
    void * p;
    int    i;
    float  f;
  } mData;
};

#include <map>
typedef std::map<std::string, SettingsValue> SettingsMap;

class Settings {
public:
  Settings() { initdefault(); }

  char * gets(std::string key) { return container[key].gets(); }
  int    geti(std::string key) { return container[key].geti(); }
  float  getf(std::string key) { return container[key].getf(); }

  void set(std::string key, int    i) { container[key] = i; }
  void set(std::string key, float  f) { container[key] = f; }
  void set(std::string key, char * s) { container[key] = s; }
  
  void getkeys(std::list<std::string> & keys) {
    SettingsMap::iterator iter = container.begin();
    for(; iter != container.end(); iter++) {
      keys.push_back(iter->first);
    }
  }

private:
  SettingsMap container;

  void initdefault()
  {
    set("r_xsize", 1280);
    set("r_ysize",  720);

    set("p_name", "pecore");

    set("net_host", "127.0.0.1");
    set("net_port", "40155");
  }

};

#endif
