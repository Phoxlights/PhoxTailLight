#include <Arduino.h>
#include "constants.h"
#include "objstore.h"
#include "presets.h"
// TODO - dont include lightstrip just for STRIDE constant :/
#include "lightstrip.h"
#include "bitmap.h"

int presetLog(PresetConfig * preset, bool idsArePointers){

    Serial.printf("preset\n");
    Serial.printf("  layers[]\n");
    Serial.printf("    layer[0]\n");

    KeyframeConfig * keyframes = preset->layers[0].keyframes;
    KeyframeConfig keyframe;
    Serial.printf("      keyframes[]\n");
    for(int i = 0; i < MAX_KEYFRAMES; i++){
        keyframe = keyframes[i];

        // empty keyframe, means were all done here
        if(!keyframe.duration && !keyframe.bitmap.id && !keyframe.transforms[0].id){
            break;
        }

        Serial.printf("        keyframe[%i]\n", i);
        Serial.printf("          duration: %i\n", keyframe.duration);

        // if a bitmap is on this keyframe, load it up
        if(keyframe.bitmap.id){
            if(idsArePointers){
                BitmapConfig * bitmapConfig = (BitmapConfig*)keyframe.bitmap.ptr;
                Serial.printf("          bitmap\n");
                Serial.printf("            width: %i\n", bitmapConfig->width);
                Serial.printf("            height: %i\n", bitmapConfig->height);
                Serial.printf("            bitmapData[]\n");
                byte * bitmapData = (byte*)bitmapConfig->bitmapData.ptr;
                for(int j = 0, offset = 0, numPx = bitmapConfig->width * bitmapConfig->height; j < numPx; j++){
                    offset = j * STRIDE;
                    Serial.printf("              %i, %i, %i, %i\n",
                        bitmapData[offset],
                        bitmapData[offset+1],
                        bitmapData[offset+2],
                        bitmapData[offset+3]);
                }
            } else {
                Serial.printf("          bitmap.id: %i\n", keyframe.bitmap.id);
            }
        }

        Serial.printf("          transforms[]\n");
        if(keyframe.transforms[0].id){
            for(int j = 0; j < MAX_TRANSFORMS; j++){
                // empty transform id, means were all done here
                if(!keyframe.transforms[j].id){
                    break;
                }

                if(idsArePointers){
                    TransformConfig * transformConfig = (TransformConfig*)keyframe.transforms[j].ptr;

                    Serial.printf("            transform[%i]\n", j);
                    Serial.printf("              fn: %i\n", transformConfig->fn);

                    void * transformArgs = transformConfig->transformArgs.ptr;

                    // TODO - pass this off to a function or something?
                    /*
                    switch(transformConfig->fn){
                        case TRANSLATE_X:
                            {
                            TransformTranslateXConfig * transformArgs = (TransformTranslateXConfig*)transformConfig->transformArgs.ptr;
                            Serial.printf("              transformArgs\n");
                            Serial.printf("                start: %i\n", transformArgs->start);
                            Serial.printf("                end: %i\n", transformArgs->end);
                            Serial.printf("                wrap: %s\n", transformArgs->wrap ? "true" : "false");
                            }
                            break;

                        default:
                            break;
                    }
                    */
                } else {
                    Serial.printf("            transform[%i]\n", j);
                    Serial.printf("              id: %i\n", keyframe.transforms[j].id);
                }
            }
        }
    }
    return 1;
}

PresetConfig * presetLoad(int id){
    PresetConfig * preset = (PresetConfig*)malloc(sizeof(PresetConfig));

    if(!objStoreGet("presets", id, preset, sizeof(PresetConfig))){
        Serial.printf("unable to load preset %i", id);
        free(preset);
        return NULL;
    }

    KeyframeConfig * keyframes = preset->layers[0].keyframes;
    KeyframeConfig * keyframe;

    int i;
    for(i = 0; i < MAX_KEYFRAMES; i++){
        keyframe = &keyframes[i];

        // empty keyframe, means were all done here
        if(!keyframe->duration && !keyframe->bitmap.id && !keyframe->transforms[0].id){
            break;
        }

        // load bitmap
        if(keyframe->bitmap.id){
            // load bitmap config
            int bitmapId = keyframe->bitmap.id;
            BitmapConfig * bitmapConfig = (BitmapConfig*)malloc(sizeof(BitmapConfig));
            if(!objStoreGet("bitmapconfig", bitmapId, bitmapConfig, sizeof(BitmapConfig))){
                // TODO - should error out?
                Serial.printf("unable to load bitmap config %i\n", bitmapId);
                free(bitmapConfig);
                free(preset);
                return NULL;
            }
            keyframe->bitmap.ptr = bitmapConfig;

            // load bitmap data
            int bitmapDataId = bitmapConfig->bitmapData.id;
            int bmpSize = bitmapConfig->width * bitmapConfig->height * STRIDE;
            byte * bitmapData = (byte*)malloc(bmpSize);
            if(!objStoreGet("bitmapdata", bitmapId, bitmapData, bmpSize)){
                // TODO - should error out?
                Serial.printf("unable to load bitmap data %i\n", bitmapDataId);
                free(bitmapData);
                free(bitmapConfig);
                free(preset);
                return NULL;
            }
            bitmapConfig->bitmapData.ptr = bitmapData;
        }

        // load transforms
        if(keyframe->transforms[0].id){

            int j;
            for(j = 0; j < MAX_TRANSFORMS; j++){
                // empty transform id, means were all done here
                if(!keyframe->transforms[j].id){
                    break;
                }

                // load transform config
                int transformId = keyframe->transforms[j].id;
                TransformConfig * transformConfig = (TransformConfig*)malloc(sizeof(TransformConfig));
                if(!objStoreGet("transform", transformId, transformConfig, sizeof(TransformConfig))){
                    // TODO - should error out?
                    Serial.printf("unable to load transform config %i\n", transformId);
                    free(transformConfig);
                    free(preset);
                    return NULL;
                }
                keyframe->transforms[j].ptr = transformConfig;
                
                // load transform args
                void * transformArgs = malloc(transformConfig->transformArgsLength);
                if(!objStoreGet("transformArgs", transformConfig->transformArgs.id, transformArgs, transformConfig->transformArgsLength)){
                    // TODO - should error out?
                    Serial.printf("unable to load transform args %i\n", transformConfig->transformArgs.id);
                    free(transformArgs);
                    free(transformConfig);
                    free(preset);
                    return NULL;
                }
                transformConfig->transformArgs.ptr = transformArgs;
            }
            Serial.printf("loaded %i transforms\n", j);
        }
    }
    Serial.printf("loaded %i keyframes\n", i);

    return preset;
}

// NOTE - in the process of writing a preset, the preset's bitmaps
// and transforms are modified in such a way that they cannot be
// written again :(
// TODO - make it possible to reuse transforms or bitmaps when
// writing a preset (Eg, a preset could have keyframes {key1, key2, key1, key2}
int presetWrite(PresetConfig * preset){
    KeyframeConfig * keyframes = preset->layers[0].keyframes;
    KeyframeConfig * keyframe;
    for(int i = 0; i < MAX_KEYFRAMES; i++){
        keyframe = &keyframes[i];

        // empty keyframe, means were all done here
        if(!keyframe->duration && !keyframe->bitmap.ptr && !keyframe->transforms[0].ptr){
            break;
        }

        // if a bitmap is on this keyframe, write it
        if(keyframe->bitmap.ptr){
            BitmapConfig * bitmapConfig = (BitmapConfig*)keyframe->bitmap.ptr;

            // write bitmap data
            byte * bitmapData = (byte*)bitmapConfig->bitmapData.ptr;
            int bmpSize = bitmapConfig->width * bitmapConfig->height * STRIDE;
            int id = objStoreCreate("bitmapdata", bitmapData, bmpSize);
            if(!id){
                Serial.println("couldnt write bitmap data");
                return 0;
            }
            bitmapConfig->bitmapData.id = id;

            // write bitmap config
            int cId = objStoreCreate("bitmapconfig", bitmapConfig, sizeof(BitmapConfig));
            if(!cId){
                Serial.println("couldnt write bitmap config");
                return 0;
            }
            keyframe->bitmap.id = cId;
        }

        // write transforms
        if(keyframe->transforms[0].ptr){
            for(int j = 0; j < MAX_TRANSFORMS; j++){
                // empty transform id, means were all done here
                if(!keyframe->transforms[j].ptr){
                    break;
                }

                TransformConfig * transformConfig = (TransformConfig*)keyframe->transforms[j].ptr;

                void * transformArgs = transformConfig->transformArgs.ptr;
                int id = objStoreCreate("transformArgs", transformArgs, transformConfig->transformArgsLength);
                if(!id){
                    Serial.println("couldnt write transform args");
                    return 0;
                }
                transformConfig->transformArgs.id = id;

                int transId = objStoreCreate("transform", transformConfig, sizeof(TransformConfig));
                if(!transId){
                    Serial.println("couldnt write transform config");
                    return 0;
                }
                keyframe->transforms[j].id = transId;
            }
        }
    }

    int id = objStoreCreate("presets", preset, sizeof(PresetConfig));
    if(!id){
        Serial.println("couldnt write preset");
        return 0;
    }

    return id;
}
