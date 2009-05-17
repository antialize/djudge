//-*- mode: c++; tab-width: 4; indent-tabs-mode: t; c-file-style: "stroustrup"; -*-
#ifndef __apparmor_hh__
#define __apparmor_hh__
void appArmorLoadProfile(const char * profile, ...);
void appArmorRemoveProfile(const char * path);
#endif //__apparmor_hh__
