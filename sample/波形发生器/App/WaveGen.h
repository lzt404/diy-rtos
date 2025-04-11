#ifndef WAVEGEN_H
#define WAVEGEN_H

#include <stdint.h>

typedef enum {
    WaveSquare,
    WaveUnknown,
}WaveType;

void WaveGenInit (void);
uint32_t WaveSelectType (WaveType type);
uint32_t WaveSquareSet (uint32_t highMs, uint32_t lowMs);
uint32_t WaveStartOutput (void);
void WaveStopOutput (void);

#endif //WAVEGEN_H
