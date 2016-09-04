#include <Arduino.h>
#include "presets.h"
#include "objstore.h"
#include "defaultPresets.h"
#include "blendingmode.h"

#define ARRAY_SIZE(x) ((sizeof x) / (sizeof *x))

/*
 * When adding new default presets, make sure any transform configs
 * are added to defaultPresets.h, an entry is added to the transformFunction
 * enum, and handled in taillight.cpp:loadStoredPreset
 * TODO - some sort of registration to more elegantly handle this
 */

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

static int writeFirePreset(){

    byte bitmapData[] = {
        255, 100, 0, 255,
        255, 100, 0, 255,
        255, 100, 0, 255,
        255, 100, 0, 255,
        255, 100, 0, 255,
        255, 100, 0, 255,
        255, 100, 0, 255,
        255, 100, 0, 255,
        255, 100, 0, 255,
        255, 100, 0, 255,
        255, 100, 0, 255,
        255, 100, 0, 255,
        255, 100, 0, 255,
        255, 100, 0, 255,
        255, 100, 0, 255,
        255, 100, 0, 255,
    };

    BitmapConfig bitmap = {
        16, 1, {.ptr=(void*)&bitmapData}
    };

    TransformFireConfig transformConfig = {
        2, 4
    };

    TransformConfig transform = {
        FIRE,
        {.ptr=(void*)&transformConfig},
        sizeof(TransformFireConfig)
    };

    PresetConfig preset = {
        // layer
        {{
            // keyframes
            {
                {
                    // duration
                    -1,
                    // bitmap
                    {.ptr=(void*)&bitmap},
                    // transforms
                    { 
                        {.ptr=(void*)&transform},
                    }
                }
            }
        }}
    };

    int presetId = presetWrite(&preset);    
    if(!presetId){
        Serial.printf("couldnt write default fire preset\n");
        return 0;
    }

    return presetId;
}

static int writeFlashyPreset(){

    byte bitmapData[] = {
        255, 0, 0, 255,
        255, 0, 0, 255,
        255, 0, 0, 255,
        255, 0, 0, 255,
        255, 0, 0, 255,
        255, 0, 0, 255,
        255, 0, 0, 255,
        255, 0, 0, 255,
        255, 0, 0, 255,
        255, 0, 0, 255,
        255, 0, 0, 255,
        255, 0, 0, 255,
        255, 0, 0, 255,
        255, 0, 0, 255,
        255, 0, 0, 255,
        255, 0, 0, 255,
    };

    BitmapConfig bitmap = {
        16, 1, {.ptr=(void*)&bitmapData}
    };

    TransformAlphaConfig tcfg1 = { 1.0, 1.0 };
    TransformConfig trans1 = {
        ALPHA_TRANS,
        {.ptr=(void*)&tcfg1},
        sizeof(TransformAlphaConfig)
    };
    KeyframeConfig key1 = {
        // duration
        1,
        // bitmap
        {.ptr=(void*)&bitmap},
        // transforms
        {{.ptr=(void*)&trans1}},
    };

    TransformAlphaConfig tcfg2 = { 0.0, 0.0 };
    TransformConfig trans2 = {
        ALPHA_TRANS,
        {.ptr=(void*)&tcfg2},
        sizeof(TransformAlphaConfig)
    };
    KeyframeConfig key2 = {
        // duration
        1,
        // bitmap
        {0},
        // transforms
        {{.ptr=(void*)&trans2}}
    };

    TransformAlphaConfig tcfg3 = { 1.0, 1.0 };
    TransformConfig trans3 = {
        ALPHA_TRANS,
        {.ptr=(void*)&tcfg3},
        sizeof(TransformAlphaConfig)
    };
    KeyframeConfig key3 = {
        // duration
        1,
        // bitmap
        {0},
        // transforms
        {{.ptr=(void*)&trans3}},
    };

    TransformAlphaConfig tcfg4 = { 0.0, 0.0 };
    TransformConfig trans4 = {
        ALPHA_TRANS,
        {.ptr=(void*)&tcfg4},
        sizeof(TransformAlphaConfig)
    };
    KeyframeConfig key4 = {
        // duration
        1,
        // bitmap
        {0},
        // transforms
        {{.ptr=(void*)&trans4}}
    };

    TransformAlphaConfig tcfg5 = { 1.0, 1.0 };
    TransformConfig trans5 = {
        ALPHA_TRANS,
        {.ptr=(void*)&tcfg5},
        sizeof(TransformAlphaConfig)
    };
    KeyframeConfig key5 = {
        // duration
        1,
        // bitmap
        {0},
        // transforms
        {{.ptr=(void*)&trans5}},
    };

    TransformAlphaConfig tcfg6 = { 0.0, 0.0 };
    TransformConfig trans6 = {
        ALPHA_TRANS,
        {.ptr=(void*)&tcfg6},
        sizeof(TransformAlphaConfig)
    };
    KeyframeConfig key6 = {
        // duration
        10,
        // bitmap
        {0},
        // transforms
        {{.ptr=(void*)&trans6}}
    };

    PresetConfig preset = {
        // layer
        {{
            // keyframes
            { key1, key2, key3, key4, key5, key6 }
        }}
    };

    int presetId = presetWrite(&preset);    
    if(!presetId){
        Serial.printf("couldnt write default flashy preset\n");
        return 0;
    }

    return presetId;
}

static int writeTestyPreset(){

    byte bitmapData[] = {
        255, 0, 0, 255,
        0,0,0,0,
        0,0,0,0,
        0,0,0,0,
        255, 255, 0, 255,
        0,0,0,0,
        0,0,0,0,
        0,0,0,0,
        0, 255, 0, 255,
        0,0,0,0,
        0,0,0,0,
        0,0,0,0,
        0, 255, 255, 255,
        0,0,0,0,
        0,0,0,0,
        0,0,0,0
    };

    BitmapConfig bitmap = {
        16, 1, {.ptr=(void*)&bitmapData}
    };

    PresetConfig preset = {
        // layer
        {{
            // keyframes
            {
                {
                    // duration
                    -1,
                    // bitmap
                    {.ptr=(void*)&bitmap},
                    // transforms
                    {}
                }
            }
        }}
    };

    int presetId = presetWrite(&preset);    
    if(!presetId){
        Serial.printf("couldnt write default testy preset\n");
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
    int firePresetId = writeFirePreset();
    int flashyPresetId = writeFlashyPreset();
    int testyPresetId = writeTestyPreset();

    Serial.printf("wrote spinny preset %i, fire preset %i, flashy preset %i\n", 
            spinnyPresetId, firePresetId, flashyPresetId);

    // return number of default presets created
    return 4;
}
