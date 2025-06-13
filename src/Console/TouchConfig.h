
#ifndef TouchConfig_h
#define TouchConfig_h

#include <MPR121.h>

class TouchConfig {
    public:
        void setup();
        void setConfig(int configSet);

    private:
        MPR121_settings_t config1;
        MPR121_settings_t config2;
        MPR121_settings_t config3;
};

extern TouchConfig touchConfig;

#endif