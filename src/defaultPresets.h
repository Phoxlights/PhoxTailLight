#ifndef DEFAULTPRESETS_H
#define DEFAULTPRESETS_H

#include "animator.h"
    
struct PresetConfig;

typedef enum TransformFunction {
    TRANSLATE_X = 0,
    RGB = 1,
    FIRE = 2,
    ALPHA_TRANS = 3,
} TransformFunction;

typedef struct TransformTranslateXConfig {
    int start;
    int end;
    bool wrap;
} TransformTranslateXConfig;

typedef struct TransformRGBConfig {
    byte begin[3];
    byte end[3];
    int mode;
} TransformRGBConfig;

typedef struct TransformFireConfig {
    int frequency;
    int flickerCount;
} TransformFireConfig;

typedef struct TransformAlphaConfig {
    float begin;
    float end;
} TransformAlphaConfig;

// dynamic presets (load/save to fs)
int defaultPresetsWrite();

// static presets
void addPulseRedLayer(AnimatorLayer l, int numPx);
void addPulseGreenLayer(AnimatorLayer l, int numPx);
void addPulseBlueLayer(AnimatorLayer l, int numPx);
void addPulseYellowLayer(AnimatorLayer l, int numPx);
void addPulseWhiteLayer(AnimatorLayer l, int numPx);
void addSolidRedLayer(AnimatorLayer l, int numPx);
void addSolidGreenLayer(AnimatorLayer l, int numPx);
void addSolidBlueLayer(AnimatorLayer l, int numPx);
void addSolidYellowLayer(AnimatorLayer l, int numPx);
void addSolidWhiteLayer(AnimatorLayer l, int numPx);
void addStrobeRedLayer(AnimatorLayer l, int numPx);
void addStrobeGreenLayer(AnimatorLayer l, int numPx);
void addStrobeBlueLayer(AnimatorLayer l, int numPx);
void addStrobeYellowLayer(AnimatorLayer l, int numPx);
void addStrobeWhiteLayer(AnimatorLayer l, int numPx);
void addColoryLayer(AnimatorLayer l, int numPx);
void addPulsyColoryLayer(AnimatorLayer l, int numPx);
void addFlameLayer(AnimatorLayer l, int numPx);

#endif
