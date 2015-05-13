// Main Console

#ifndef Console_h
#define Console_h

#include <Arduino.h>

void gamePlayModeLoop(boolean performStartup);
void bongoModeLoop(boolean performStartup);
void proximityModeLoop(boolean performStartup);
void lightsTestModeLoop(boolean performStartup);
void fireTestModeLoop(boolean performStartup);
void proximityResetModeLoop(boolean performStartup);

#endif
