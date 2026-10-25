#ifndef ENCODERS_H
#define ENCODERS_H

// lm393 slot sensors, one per wheel
void initEncoders();
void sendSpeed();  // every 100 ms, wheel speeds in mm/s to the controller

#endif
