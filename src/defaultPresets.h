#ifndef DEFAULTPRESETS_H
#define DEFAULTPRESETS_H
    
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

int defaultPresetsWrite();

#endif
