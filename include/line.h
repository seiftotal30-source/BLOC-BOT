#ifndef LINE_H
#define LINE_H

// 5-channel line sensor bar under the front (gpio 2, 4, 5, 6, 7)
void initLine();
void updateLine();  // every 50 ms: read the 5 sensors, send them to the controller

#endif
