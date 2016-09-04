#ifndef TAILLIGHT_H
#define TAILLIGHT_H

#include "animator.h"

typedef enum TailLightLayer {
    RUNNING = 0,
    RUNNING2 = 1,
    BRAKE = 2,
    SIGNAL_RIGHT = 3,
    SIGNAL_LEFT = 4,
    OTA = 5
};

typedef struct TailLightState* TailLight;

TailLight tailLightCreate(int pin, int numPx, float brightness, int offset);
int tailLightStart(TailLight state);
int tailLightStop(TailLight state);
int tailLightPause(TailLight state);

int tailLightLayerStart(TailLight state, TailLightLayer layerIndex);
int tailLightLayerStop(TailLight state, TailLightLayer layerIndex);
int tailLightLayerStopAll(TailLight state);
// NOTE - this is solely exposed as a workaround for
// needing to manually tick taillight during OTA
void tailLightTick(void * s);

int tailLightSetPixel(TailLight state, int x, byte rgb[3]);

// update progress bar values
int tailLightUpdateProgressBar(TailLight state, unsigned int progress, unsigned int total);

int tailLightNextPreset(TailLight state);
int tailLightLoadPreset(TailLight state, int presetIndex);
int tailLightGetPresetIndex(TailLight state);

int tailLightSetOffset(TailLight state, int offset);

// TODO - create layers/keyframes/transforms from config struct
// TODO - preset registration with cleanup function

#endif
