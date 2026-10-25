#ifndef RADIO_H
#define RADIO_H

#include <Arduino.h>

// esp-now in broadcast mode, no pairing
void initRadio();
void radioSend(const void* data, size_t len);

#endif
