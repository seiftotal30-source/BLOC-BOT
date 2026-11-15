#ifndef BENCH_H
#define BENCH_H

// serial commands for bench tests (115200 baud), h = help
void printHelp();
void handleSerial();  // also keeps a serial command running for 2 s

#endif
