#ifndef ISS_CONFIG_H
#define ISS_CONFIG_H
#include "app.h"
void config_defaults(IssTrackerApp *app);
void config_load(IssTrackerApp *app);
void config_save_window(WORD left, WORD top, WORD width, WORD height);
#endif
