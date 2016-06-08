#ifndef MF_SETTINGS_HH_
#define MF_SETTINGS_HH_

#include <fstream>
#include <string>
#include <map>
#include <pthread.h>

using namespace std;

// implemented as a non-thread-safe singleton class

CLICK_DECLS

const string ROUTER_SETTING_FILE_PATH = "/root/scripts/exprmt/mfrouter.settings";

class MF_Settings {
private:
  inline MF_Settings() {
  }

  inline void readSettingsFile() {
    ifstream sfile(ROUTER_SETTING_FILE_PATH.c_str());  
    if (sfile.is_open()) {
      string line;
      while (getline(sfile, line)) {
        // process line 
        uint32_t i = 0;
        uint32_t len = line.size();
        while (i<len && line[i]==' ') ++i;
        if (i==len || line[i] == '#') continue;

        uint32_t right = i;
        while (right<len && line[right] != '=' && line[right] != ' ') 
          ++right;
        string key = line.substr(i, right-i);

        i = line.find_first_of('=', i);
        string value = line.substr(i+1, len-i-1);
        if (key == "" || value == "") continue;
        _kv[key] = value;
        fprintf(stderr, "settings: inserted key: %s, value: %s\n", key.c_str(), value.c_str());
      }
      sfile.close();
    } else {
      fprintf(stderr, "ERROR: settings: unable to open file: %s\n", ROUTER_SETTING_FILE_PATH.c_str());
    }
  }
  static MF_Settings *_instance;
  static pthread_mutex_t _lock;
  map<string, string> _kv;

public:
  static inline MF_Settings* instance() {
    pthread_mutex_lock(&_lock);
    if (!_instance) {
      _instance = new MF_Settings();
      _instance->readSettingsFile();
    }
    pthread_mutex_unlock(&_lock);
    return _instance;
  }
  
  inline string lookup(string &key) {
    map<string, string>::iterator it = _kv.find(key);
    if (it == _kv.end()) {
      fprintf(stderr, "ERROR: settings: value for key: %s not found\n", key.c_str());
      return "";
    }
    return it->second;
  }
};
MF_Settings* MF_Settings::_instance = NULL;
pthread_mutex_t MF_Settings::_lock = PTHREAD_MUTEX_INITIALIZER;

#endif
