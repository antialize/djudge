#include "globals.hh"
#include "langsupport.hh"

std::string proxyPath;
std::string entriesPath;
int         droneUser;
int         droneGroup;
int         nobodyUser;
int         nobodyGroup;
std::multimap<float, LangSupport *> langByRank;
std::map<std::string, LangSupport *> langByName;

