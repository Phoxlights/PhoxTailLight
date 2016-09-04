#ifndef PRESETS_H
#define PRESETS_H

#include "constants.h"

// use id when storing on disk, use ptr after
// loading object into memory
typedef union IdOrPointer {
    int id;
    void * ptr;
} IdOrPointer;

typedef struct BitmapConfig {
    int width;
    int height;
    IdOrPointer bitmapData;
} BitmapConfig;

typedef struct TransformConfig {
    int fn;
    IdOrPointer transformArgs;
    int transformArgsLength;
} TransformConfig;

typedef struct KeyframeConfig {
    int duration;
    IdOrPointer bitmap;
    IdOrPointer transforms[MAX_TRANSFORMS];
} KeyframeConfig;

typedef struct LayerConfig {
    KeyframeConfig keyframes[MAX_KEYFRAMES];
} LayerConfig;

typedef struct PresetConfig {
    // TODO - multiple layers
    LayerConfig layers[1];
} PresetConfig;

PresetConfig * presetLoad(int id);
int presetWrite(PresetConfig * preset);
int presetLog(PresetConfig * preset, bool idsArePointers);

#endif
