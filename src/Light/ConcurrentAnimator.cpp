#include "ConcurrentAnimator.h"

void ConcurrentAnimator::animate(AnimateFunc animate, AnimationConfig &config) {
  calculateAnimation(animate, config);
  push(config);
}

void ConcurrentAnimator::calculateAnimation(AnimateFunc animate, AnimationConfig &config) {
  if (!config.ready) {
    return;
  }
  Serial << "---" << config.name << "---" << endl;

  (*animate)((*config.strip),
      config.color.red, config.color.green, config.color.blue, config.position);
  config.ready = false;
}

void ConcurrentAnimator::push(AnimationConfig &config) {
  if (!config.timer.check()) return;
  config.strip->show();
  config.ready = true;
}

