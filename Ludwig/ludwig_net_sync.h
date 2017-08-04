#pragma once
#include "profile.h"

#ifdef NET_SYNC

typedef void(*fp_event_callback)(char*, int);
fp_event_callback friedrich_acts_table[_EVENT_COUNT_];
fp_event_callback alan_acts_table[_EVENT_COUNT_];

#ifdef _WINDLL
#define EXPORTS _declspec(dllexport)
#else
#define EXPORTS _declspec(dllimport)
#endif
extern "C" EXPORTS void friedrich_talking(int);
extern "C" EXPORTS void alan_talking(char*, int);
extern "C" EXPORTS void friedrich_hearing();
extern "C" EXPORTS void alan_hearing();
extern "C" EXPORTS void friedrich_quiet();
extern "C" EXPORTS void alan_quiet();
extern "C" EXPORTS void friedrich_acts(net_events, fp_event_callback);
extern "C" EXPORTS void alan_acts(net_events, fp_event_callback);
extern "C" EXPORTS void friedrich_says(net_events, char*, unsigned long);
extern "C" EXPORTS void alan_says(net_events, char*, unsigned long);

#endif