#ifndef JOYSTICK_H
#define JOYSTICK_H

// ky-023 joystick, 2 potentiometers on adc1
void calibrateJoystick();  // stick released: average 50 readings = centre
void readJoystick(int& x, int& y);  // centred, /20, dead zone of 4. y + = stick up

#endif
