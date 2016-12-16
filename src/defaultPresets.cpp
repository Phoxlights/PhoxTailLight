#include <Arduino.h>
#include <ease.h>
#include "presets.h"
#include "objstore.h"
#include "defaultPresets.h"
#include "animator.h"
#include "blendingmode.h"
#include "alltransforms.h"

/*
 * When adding new default presets, make sure any transform configs
 * are added to defaultPresets.h, an entry is added to the transformFunction
 * enum, and handled in taillight.cpp:loadStoredPreset
 * TODO - some sort of registration to more elegantly handle this
 */

// presets loaded from filesystem
static int writeSpinnyPreset(){

    byte bitmapData[] = {
        255, 0, 0, 255,
        255, 0, 0, 127,
        255, 0, 0, 64,
        255, 0, 0, 32,
        255, 0, 0, 16,
        255, 0, 0, 32,
        255, 0, 0, 64,
        255, 0, 0, 127,
        255, 0, 0, 255,
        255, 0, 0, 127,
        255, 0, 0, 64,
        255, 0, 0, 32,
        255, 0, 0, 16,
        255, 0, 0, 32,
        255, 0, 0, 64,
        255, 0, 0, 127,
    };

    BitmapConfig bitmap = {
        16, 1, {.ptr=(void*)&bitmapData}
    };

    // keyframe 1
    TransformRGBConfig tcfg1 = {
        {255,0,0}, {0,255,0}, REPLACE
    };

    TransformConfig trans1 = {
        RGB,
        {.ptr=(void*)&tcfg1},
        sizeof(TransformRGBConfig)
    };

    TransformTranslateXConfig tcfg2 = {
        0, 15, true
    };

    TransformConfig trans2 = {
        TRANSLATE_X,
        {.ptr=(void*)&tcfg2},
        sizeof(TransformTranslateXConfig)
    };

    KeyframeConfig key1 = {
        // duration
        30,
        // bitmap
        {.ptr=(void*)&bitmap},
        // transforms
        { 
            {.ptr=(void*)&trans1},
            {.ptr=(void*)&trans2},
        }
    };

    // keyframe 2
    TransformRGBConfig tcfg3 = {
        {0,255,0}, {0,0,255}, REPLACE
    };

    TransformConfig trans3 = {
        RGB,
        {.ptr=(void*)&tcfg3},
        sizeof(TransformRGBConfig)
    };

    TransformTranslateXConfig tcfg4 = {
        0, 15, true
    };

    TransformConfig trans4 = {
        TRANSLATE_X,
        {.ptr=(void*)&tcfg4},
        sizeof(TransformTranslateXConfig)
    };

    KeyframeConfig key2 = {
        // duration
        30,
        // bitmap
        {0},
        // transforms
        { 
            {.ptr=(void*)&trans3},
            {.ptr=(void*)&trans4},
        }
    };

    // keyframe 3
    TransformRGBConfig tcfg5 = {
        {0,0,255}, {255,0,0}, REPLACE
    };

    TransformConfig trans5 = {
        RGB,
        {.ptr=(void*)&tcfg5},
        sizeof(TransformRGBConfig)
    };

    TransformTranslateXConfig tcfg6 = {
        0, 15, true
    };

    TransformConfig trans6 = {
        TRANSLATE_X,
        {.ptr=(void*)&tcfg6},
        sizeof(TransformTranslateXConfig)
    };

    KeyframeConfig key3 = {
        // duration
        30,
        // bitmap
        {0},
        // transforms
        { 
            {.ptr=(void*)&trans5},
            {.ptr=(void*)&trans6},
        }
    };

    PresetConfig preset = {
        // layer
        {{
            // keyframes
            { key1, key2, key3 }
        }}
    };

    int presetId = presetWrite(&preset);    
    if(!presetId){
        Serial.printf("couldnt write default spinny preset\n");
        return 0;
    }

    return presetId;
}

int defaultPresetsWrite(){
    // wipe all the stuff out real good
    objStoreWipe("bitmapdata");
    objStoreWipe("bitmapconfig");
    objStoreWipe("transform");
    objStoreWipe("transformArgs");
    objStoreWipe("presets");

    int spinnyPresetId = writeSpinnyPreset();

    Serial.printf("wrote spinny preset %i", spinnyPresetId);
    return 1;
}


// TODO - put these someplace and share em
static byte RED[] = {255,0,0,255};
static byte ORANGE[] = {255,255,0,255};
static byte GREEN[] = {0,255,0,255};
static byte BLUE[] = {0,0,255,255};
static byte YELLOW[] = {255,255,0,255};
static byte PURPLE[] = {255,0,255,255};
static byte WHITE[] = {255,255,255,255};
static byte BLACK[] = {0,0,0,255};

// hardcoded presets
static void addPulseLayer(AnimatorLayer l, int numPx, byte * color){
    Bitmap * bmp = Bitmap_create(numPx, 1);
    Bitmap_fill(bmp, color);

    AnimatorKeyframe k1 = animatorKeyframeCreate(l, 20, bmp);
    animatorKeyframeAddTransform(k1, createTransformPulse(3, 11, 3, 3));
}
void addPulseRedLayer(AnimatorLayer l, int numPx){
    addPulseLayer(l, numPx, RED);
}
void addPulseGreenLayer(AnimatorLayer l, int numPx){
    addPulseLayer(l, numPx, GREEN);
}
void addPulseBlueLayer(AnimatorLayer l, int numPx){
    addPulseLayer(l, numPx, BLUE);
}
void addPulseYellowLayer(AnimatorLayer l, int numPx){
    addPulseLayer(l, numPx, YELLOW);
}
void addPulseWhiteLayer(AnimatorLayer l, int numPx){
    addPulseLayer(l, numPx, WHITE);
}

static void addSolidLayer(AnimatorLayer l, int numPx, byte * color){
    Bitmap * bmp = Bitmap_create(numPx, 1);
    Bitmap_fill(bmp, color);

    AnimatorKeyframe k1 = animatorKeyframeCreate(l, 75, bmp);
}
void addSolidRedLayer(AnimatorLayer l, int numPx){
    addSolidLayer(l, numPx, RED);
}
void addSolidGreenLayer(AnimatorLayer l, int numPx){
    addSolidLayer(l, numPx, GREEN);
}
void addSolidBlueLayer(AnimatorLayer l, int numPx){
    addSolidLayer(l, numPx, BLUE);
}
void addSolidYellowLayer(AnimatorLayer l, int numPx){
    addSolidLayer(l, numPx, YELLOW);
}
void addSolidWhiteLayer(AnimatorLayer l, int numPx){
    addSolidLayer(l, numPx, WHITE);
}

static void addStrobeLayer(AnimatorLayer l, int numPx, byte * color){
    Bitmap * bmp = Bitmap_create(numPx, 1);
    Bitmap_fill(bmp, color);

    AnimatorKeyframe k1 = animatorKeyframeCreate(l, 1, bmp);
    animatorKeyframeAddTransform(k1, createTransformAlpha(0.0, 0.0));
    AnimatorKeyframe k2 = animatorKeyframeCreate(l, 1, bmp);
    animatorKeyframeAddTransform(k2, createTransformAlpha(1.0, 1.0));
    AnimatorKeyframe k3 = animatorKeyframeCreate(l, 1, bmp);
    animatorKeyframeAddTransform(k3, createTransformAlpha(0.0, 0.0));
    AnimatorKeyframe k4 = animatorKeyframeCreate(l, 1, bmp);
    animatorKeyframeAddTransform(k4, createTransformAlpha(1.0, 1.0));
    AnimatorKeyframe k5 = animatorKeyframeCreate(l, 1, bmp);
    animatorKeyframeAddTransform(k5, createTransformAlpha(0.0, 0.0));
    AnimatorKeyframe k6 = animatorKeyframeCreate(l, 1, bmp);
    animatorKeyframeAddTransform(k6, createTransformAlpha(1.0, 1.0));
    AnimatorKeyframe k7 = animatorKeyframeCreate(l, 10, bmp);
    animatorKeyframeAddTransform(k7, createTransformAlpha(0.0, 0.0));
}
void addStrobeRedLayer(AnimatorLayer l, int numPx){
    addStrobeLayer(l, numPx, RED);
}
void addStrobeGreenLayer(AnimatorLayer l, int numPx){
    addStrobeLayer(l, numPx, GREEN);
}
void addStrobeBlueLayer(AnimatorLayer l, int numPx){
    addStrobeLayer(l, numPx, BLUE);
}
void addStrobeYellowLayer(AnimatorLayer l, int numPx){
    addStrobeLayer(l, numPx, YELLOW);
}
void addStrobeWhiteLayer(AnimatorLayer l, int numPx){
    addStrobeLayer(l, numPx, WHITE);
}

void addColoryLayer(AnimatorLayer l, int numPx){
    Bitmap * bmp = Bitmap_create(numPx, 1);
    Bitmap_fill(bmp, BLUE);

    AnimatorKeyframe k1 = animatorKeyframeCreate(l, 30, bmp);
    animatorKeyframeAddTransform(k1, createTransformRGB(RED, GREEN, REPLACE));
    AnimatorKeyframe k2 = animatorKeyframeCreate(l, 30, bmp);
    animatorKeyframeAddTransform(k2, createTransformRGB(GREEN, BLUE, REPLACE));
    AnimatorKeyframe k3 = animatorKeyframeCreate(l, 30, bmp);
    animatorKeyframeAddTransform(k3, createTransformRGB(BLUE, RED, REPLACE));
}

void addPulsyColoryLayer(AnimatorLayer l, int numPx){
    Bitmap * bmp = Bitmap_create(numPx, 1);
    Bitmap_fill(bmp, BLUE);

    AnimatorKeyframe k1 = animatorKeyframeCreate(l, 16, bmp);
    animatorKeyframeAddTransform(k1, createTransformRGB(RED, GREEN, REPLACE));
    animatorKeyframeAddTransform(k1, createTransformPulse(5,1,5,5));
    AnimatorKeyframe k2 = animatorKeyframeCreate(l, 16, bmp);
    animatorKeyframeAddTransform(k2, createTransformRGB(GREEN, BLUE, REPLACE));
    animatorKeyframeAddTransform(k2, createTransformPulse(5,1,5,5));
    AnimatorKeyframe k3 = animatorKeyframeCreate(l, 16, bmp);
    animatorKeyframeAddTransform(k3, createTransformRGB(BLUE, RED, REPLACE));
    animatorKeyframeAddTransform(k3, createTransformPulse(5,1,5,5));
}

void addFlameLayer(AnimatorLayer l, int numPx){
    Bitmap * bmp = Bitmap_create(numPx, 1);
    byte orange[] = {255, 100, 0, 255};
    Bitmap_fill(bmp, orange);

    AnimatorKeyframe k1 = animatorKeyframeCreate(l, 255, bmp);
    animatorKeyframeAddTransform(k1, createTransformFire(2, 4));
}

void addWhiteAcross(AnimatorLayer l, int numPx){
    Bitmap * bmp1 = Bitmap_create(numPx, 1);
    GradientStop * stops[MAX_STOPS] = {
        (GradientStop*)&BLACK,
        (GradientStop*)&WHITE,
        (GradientStop*)&BLACK,
    };
    Bitmap_gradient(bmp1, stops, 3);

    AnimatorKeyframe k1 = animatorKeyframeCreate(l, 50, bmp1);
    animatorKeyframeAddTransform(k1, createTransformTranslateX(0, 47, true, EASE_IN_OUT));
    animatorKeyframeAddTransform(k1, createTransformMirrorX(true));
    // rotate 45deg
    animatorKeyframeAddTransform(k1, createTransformTranslateX(12, 12, true, LINEAR));
}

void addRedBlueAcross(AnimatorLayer l, int numPx){
    Bitmap * bmp1 = Bitmap_create(numPx, 1);
    GradientStop * stops[MAX_STOPS] = {
        (GradientStop*)&RED,
        (GradientStop*)&BLUE,
        (GradientStop*)&RED,
    };
    Bitmap_gradient(bmp1, stops, 3);

    AnimatorKeyframe k1 = animatorKeyframeCreate(l, 50, bmp1);
    animatorKeyframeAddTransform(k1, createTransformTranslateX(0, 15, true, LINEAR));
    animatorKeyframeAddTransform(k1, createTransformMirrorX(true));
    // rotate 45deg
    animatorKeyframeAddTransform(k1, createTransformTranslateX(12, 12, true, LINEAR));
}

void addSpinnyLayer(AnimatorLayer l, int numPx){
    byte almost[] = {0,0,0,100};
    Bitmap * bmp1 = Bitmap_create(numPx, 1);
    GradientStop * stops[MAX_STOPS] = {
        (GradientStop*)&almost,
        (GradientStop*)&BLACK,
        (GradientStop*)&almost,
        (GradientStop*)&BLACK,
        (GradientStop*)&almost,
    };
    Bitmap_gradient(bmp1, stops, 5);

    AnimatorKeyframe k1 = animatorKeyframeCreate(l, 30, bmp1);
    animatorKeyframeAddTransform(k1, createTransformRGB(RED, GREEN, REPLACE));
    animatorKeyframeAddTransform(k1, createTransformTranslateX(0, 15, true, LINEAR));

    AnimatorKeyframe k2 = animatorKeyframeCreate(l, 30, bmp1);
    animatorKeyframeAddTransform(k2, createTransformRGB(GREEN, BLUE, REPLACE));
    animatorKeyframeAddTransform(k2, createTransformTranslateX(0, 15, true, LINEAR));

    AnimatorKeyframe k3 = animatorKeyframeCreate(l, 30, bmp1);
    animatorKeyframeAddTransform(k3, createTransformRGB(BLUE, RED, REPLACE));
    animatorKeyframeAddTransform(k3, createTransformTranslateX(0, 15, true, LINEAR));
}

void addSpinny2(AnimatorLayer l, int numPx){
    byte almost[] = {0,0,0,100};
    Bitmap * bmp1 = Bitmap_create(numPx, 1);
    GradientStop * stops[MAX_STOPS] = {
        (GradientStop*)&almost,
        (GradientStop*)&BLACK,
        (GradientStop*)&almost,
        (GradientStop*)&BLACK,
        (GradientStop*)&almost,
    };
    Bitmap_gradient(bmp1, stops, 5);

    AnimatorKeyframe k1 = animatorKeyframeCreate(l, 35, bmp1);
    animatorKeyframeAddTransform(k1, createTransformRGB(RED, GREEN, REPLACE));
    animatorKeyframeAddTransform(k1, createTransformTranslateX(0, 31, true, EASE_IN_OUT));

    AnimatorKeyframe k2 = animatorKeyframeCreate(l, 35, bmp1);
    animatorKeyframeAddTransform(k2, createTransformRGB(GREEN, BLUE, REPLACE));
    animatorKeyframeAddTransform(k2, createTransformTranslateX(0, 31, true, EASE_IN_OUT));

    AnimatorKeyframe k3 = animatorKeyframeCreate(l, 35, bmp1);
    animatorKeyframeAddTransform(k3, createTransformRGB(BLUE, RED, REPLACE));
    animatorKeyframeAddTransform(k3, createTransformTranslateX(0, 31, true, EASE_IN_OUT));
}

void addTightRedPulse(AnimatorLayer l, int numPx){
    Bitmap * bmp1 = Bitmap_create(numPx, 1);
    GradientStop * stops[MAX_STOPS] = {
        (GradientStop*)&RED,
        (GradientStop*)&BLACK,
        (GradientStop*)&RED,
    };
    Bitmap_gradient(bmp1, stops, 3);

    AnimatorKeyframe k1 = animatorKeyframeCreate(l, 30, bmp1);
    animatorKeyframeAddTransform(k1, createTransformTranslateX(0, 15, true, LINEAR));
    animatorKeyframeAddTransform(k1, createTransformMirrorX(false));
}

