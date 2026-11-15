#ifndef ULTRASONIC_H
#define ULTRASONIC_H

// two hc-sr04 at the front, echo through a 1k/2k divider on each:
// left trig 39 / echo 10, right trig 11 / echo 12
void initUltrasonic();
void updateUltrasonic();  // every 50 ms: read the last ping, fire the other sensor; both sent to con at 10 Hz

#endif
