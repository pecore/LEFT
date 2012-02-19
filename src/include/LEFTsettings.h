/*
    Copyright (c) 2011   LEFT PROJECT
    All rights reserved.

    file authors:
    Jan Christian Meyer
*/

#ifndef _LEFTSETTINGS_H_
#define _LEFTSETTINGS_H_

#include <map>
#include <list>
#include <string>

enum SettingsType {
  stNone = 0,
  stString,
  stInt, 
  stFloat,
};

class SettingsValue {
public:
  SettingsValue() 
  { 
    mType = stNone;
  }
  
  SettingsType type() { return mType; }

#define GETTER(name, t, st, v, def) t get ## name () { if(mType == (st)) { return v; } else { return def; } }
  GETTER(s, std::string, stString, s, "")
  GETTER(i, int        , stInt   , i, 0)
  GETTER(f, float      , stFloat , f, 0.0f)

  SettingsValue(std::string value) 
  {
    mType = stString;
    s = value;
  }
  
  SettingsValue(int value) 
  {
    mType = stInt;
    i = value;
  }

  SettingsValue(float value)
  {
    mType = stFloat;
    f = value;
  }

private: 
  SettingsType mType;
  std::string s;
  int         i;
  float       f;
};

typedef std::map<std::string, SettingsValue> SettingsMap;

class Settings {
public:
  Settings() { init(); }

  SettingsValue get(std::string key) { return container[key]; }
  
  std::string gets(std::string key) { return container[key].gets(); }
  int         geti(std::string key) { return container[key].geti(); }
  float       getf(std::string key) { return container[key].getf(); }

  void set(std::string key, int         i) { container[key] = i; }
  void set(std::string key, float       f) { container[key] = f; }
  void set(std::string key, std::string s) { container[key] = s; }
  
  void getkeys(std::list<std::string> & keys) {
    SettingsMap::iterator iter = container.begin();
    for(; iter != container.end(); iter++) {
      keys.push_back(iter->first);
    }
  }

private:
  SettingsMap container;

  void init()
  {
#if 1
    set("r_xsize", 1066);
    set("r_ysize",  600);
    set("r_fullscreen", 0);
#else 
    set("r_xsize", 1920);
    set("r_ysize",  1080);
    set("r_fullscreen", 1);
#endif
    set("r_volume", 0.01f);

    set("p_name", "pecore");
    //set("p_robot", "robotv3");

    set("net_host", "192.168.2.100");
    set("net_port", "40155");
  }

};

extern Settings * gSettings;

#endif
