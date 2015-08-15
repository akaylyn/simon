
#ifndef TouchTests_h
#define TouchTests_h

#include <Arduino.h>
#include <Metro.h> // timers
#include "Touch.h"

class TouchTests {
    public:
        void testChanged(int electrode);
        void testPressed(int electrode);
        void testBongoMode();
        void testWhatPressed();
        void testAnyColorPressed();
        void testAnyButtonPressed();

        // unit test for touch sensor
        void touchUnitTest(unsigned long timeout=(-1)); // unsigned.  will flip to maximum time.

        static const int TIMEOUT = 5000;
};

#endif

