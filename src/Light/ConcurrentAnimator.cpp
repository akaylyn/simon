#include "ConcurrentAnimator.h"

void ConcurrentAnimator::animate(AnimateFunc animate, AnimationConfig &config) {
    calculateAnimation(
            animate,
            config.strip,
            config.color,
            config.position,
            config.ready
            );
    push(config.strip, config.timer, config.ready);
    config.position++;
}

void ConcurrentAnimator::calculateAnimation(
        AnimateFunc animate,
        Adafruit_NeoPixel *strip,
        RgbColor color,
        int position,
        bool &ready
        ) {
    if (!ready) {
        //Serial << "calcAnimation !! Not Ready !! " << (*strip).numPixels() << endl;
        return;
    }
    //Serial << "calcAnimation" << endl;
    (*animate)((*strip), color.red, color.green, color.blue, position);
    ready = false;
}

void ConcurrentAnimator::push(Adafruit_NeoPixel *strip, Metro timer, bool &ready) {
    if (!timer.check()) return;
    (*strip).show();
    ready = true;
}

