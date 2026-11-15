#ifndef ULTRASONIC_H
#define ULTRASONIC_H

// hc-sr04 at the front (trig 11, echo 12 through a 1k/2k divider)
void initUltrasonic();
void updateUltrasonic();  // every 100 ms: send the last distance, fire the next ping

#endif
