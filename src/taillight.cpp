#include <stdlib.h>
#include "loop.h"
#include "animator.h"
#include "lightstrip.h"
#include "alltransforms.h"
#include "taillight.h"
#include "presets.h"
#include "defaultPresets.h"
#include "transform.h"

/*******************************************
 * this is some dense, yucko code. be warned.
 *******************************************/

static void addSignalRight(TailLight tail);
static void addSignalLeft(TailLight tail);
static void addBrake(TailLight tail);
static void addOTA(TailLight tail);

// TODO - infer this from the presets array
static int presetCount = 23;

typedef struct TailLightState {
    int pin;
    int numPx;
    Animator animator;
    LightStrip strip;
    byte * buffer;
    float brightness;
    int offset;

    unsigned int progress[2];

    AnimatorLayer running;
    AnimatorLayer signalRight;
    AnimatorLayer signalLeft;
    AnimatorLayer brake;
    AnimatorLayer ota;

    LoopNode loopNode;
    Transform offsetTransform;

    int currentPreset;
    int presetCount;

    bool paused;
} TailLightState;

TailLight tailLightCreate(int pin, int numPx, float brightness, int offset){
    TailLightState * state = (TailLightState*)malloc(sizeof(TailLightState));
    state->pin = pin;
    state->numPx = numPx;
    state->animator = animatorCreate(numPx, 1);
    if(state->animator == NULL){
        Serial.println("couldnt create animator");
        return NULL;
    }
    state->buffer = animatorGetBuffer(state->animator)->data;
    state->strip = lightStripCreate(pin, numPx, brightness, state->buffer);
    if(state->strip == NULL){
        Serial.println("couldnt create strip");
        return NULL;
    }
    state->loopNode = NULL;
    state->brightness = brightness;
    state->offset = offset;
    state->offsetTransform = createTransformTranslateX(offset, offset, true, LINEAR);
    state->running = NULL;
    state->signalRight = NULL;
    state->signalLeft = NULL;
    state->brake = NULL;

    state->paused = false;

    // for ota progress bar [progress, total]
    state->progress[0] = 0;
    state->progress[1] = 0;

    // setup presets
    state->currentPreset = -1;

    // needed for cycling the presets array
    state->presetCount = presetCount;

    /*
    // TODO - reenable this once presets are being edited
    if(!defaultPresetsWrite()){
        Serial.println("couldnt write default presets, continuing");
    }
    */

    tailLightNextPreset(state);
    addSignalRight(state);
    addSignalLeft(state);
    addBrake(state);
    addOTA(state);

    return state;
}

void tailLightTick(void * s){
    TailLightState * state = (TailLightState*)s;
    if(!state->paused){
        animatorTick(state->animator);
        // apply offset
        int domain[] = {1,1};
        applyTransform(state->offsetTransform, animatorGetBuffer(state->animator), domain);
    }
    // even if were paused, we might need to update
    // the light strip with the pixel buffer
    lightStripTick(state->strip);
}

// commits buffer to animator layer so that animation
// will continue using the modified buffer
static int tailLightCommitPixels(AnimatorLayer layer, byte * buffer){
    if(!animatorLayerUpdateBitmap(layer, buffer)){
        Serial.println("could not commit buffer to layer");
        return 0;
    }
    return 1;
}

int tailLightSetPixel(TailLight state, int x, byte rgb[3]){
    // TODO - this may not work?
    if(x > state->numPx){
        Serial.printf("cannot set pixel %i; exceeds numPx %i\n", x, state->numPx);
        return 0;
    }
    int offset = x * STRIDE;
    state->buffer[offset] = rgb[0];
    state->buffer[offset+1] = rgb[1];
    state->buffer[offset+2] = rgb[2];
    // TODO - get alpha from user
    state->buffer[offset+3] = 255;

    // get the buffer with the taillight offset removed
    // (opposite of offset is onset? i dunno)
    int count = state->numPx * STRIDE;
    byte * onsetBuffer = (byte*)malloc(count);
    int buffOffset = state->offset * STRIDE;
    for(int i = 0; i < count; i++){
        onsetBuffer[i] = state->buffer[(i + buffOffset) % count];
    }

    AnimatorLayer l = animatorGetLayerAt(state->animator, RUNNING);
    if(!tailLightCommitPixels(l, onsetBuffer)){
        Serial.println("could not commit modified buffer to running layer");
        free(onsetBuffer);
        return 0;
    }
    free(onsetBuffer);
    return 1;
}

int tailLightSetOffset(TailLight state, int offset){
    state->offset = offset;
    if(state->offsetTransform != NULL){
        freeTransform(state->offsetTransform);
    }
    state->offsetTransform = createTransformTranslateX(offset, offset, true, LINEAR);
    return 1;
}

int tailLightStart(TailLight state){
    if(state->loopNode && !state->paused){
        Serial.println("cannot start tail light; tail light already started and not paused");
        return 0;
    }
    state->paused = false;
    // if we're resuming from a pause, loopNode
    // will still be set and tickin'
    if(state->loopNode == NULL){
        state->loopNode = loopAttach(tailLightTick, 30, (void*)state);
    }
    return 1;
}

int tailLightStop(TailLight state){
    if(!state->loopNode){
        Serial.println("cannot stop tail light; tail light not started");
        return 0;
    }
    loopDetach(state->loopNode);
    state->loopNode = NULL;
    state->paused = false;
    return 1;
}

int tailLightPause(TailLight state){
    if(!state->loopNode){
        Serial.println("cannot pause tail light; tail light not started");
        return 0;
    }
    state->paused = true;
    return 1;
}

int tailLightLayerStart(TailLight state, TailLightLayer layerIndex){
    AnimatorLayer l = animatorGetLayerAt(state->animator, layerIndex);
    if(l == NULL){
        Serial.println("could not start layer, layer doesn't exist");
        return 0;
    }

    animatorLayerStart(l);
    return 1;
}
int tailLightLayerStop(TailLight state, TailLightLayer layerIndex){
    AnimatorLayer l = animatorGetLayerAt(state->animator, layerIndex);
    if(l == NULL){
        Serial.println("could not stop layer, layer doesn't exist");
        return 0;
    }

    animatorLayerStop(l);
    return 1;
}

// frees any existing running layer, then creates and
// returns a new running layer for configuration
static AnimatorLayer tailLightCreateRunningLayer(TailLight state){
    state->running = animatorLayerCreate(state->animator, state->numPx, 1, RUNNING);
    if(state->running == NULL){
        Serial.println("problem creating running layer");
        return NULL;
    }

    return state->running;
}

static AnimatorLayer tailLightCreateSignalRightLayer(TailLight state){
    state->signalRight = animatorLayerCreate(state->animator, state->numPx, 1, SIGNAL_RIGHT);
    if(state->signalRight == NULL){
        Serial.println("problem creating signalRight layer");
        return NULL;
    }

    return state->signalRight;
}

static AnimatorLayer tailLightCreateSignalLeftLayer(TailLight state){
    state->signalLeft = animatorLayerCreate(state->animator, state->numPx, 1, SIGNAL_LEFT);
    if(state->signalLeft == NULL){
        Serial.println("problem creating signalLeft layer");
        return NULL;
    }

    return state->signalLeft;
}

static AnimatorLayer tailLightCreateBrakeLayer(TailLight state){
    state->brake = animatorLayerCreate(state->animator, state->numPx, 1, BRAKE);
    if(state->brake == NULL){
        Serial.println("problem creating brake layer");
        return NULL;
    }

    return state->brake;
}

static AnimatorLayer tailLightCreateOTALayer(TailLight state){
    state->ota = animatorLayerCreate(state->animator, state->numPx, 1, OTA);
    if(state->ota == NULL){
        Serial.println("problem creating ota layer");
        return NULL;
    }

    return state->ota;
}

static void loadStoredPreset(TailLight tail, PresetConfig * preset){
    AnimatorLayer layer = tailLightCreateRunningLayer(tail);

    KeyframeConfig * keyframes = preset->layers[0].keyframes;
    KeyframeConfig keyframe;
    for(int i = 0; i < MAX_KEYFRAMES; i++){
        keyframe = keyframes[i];

        // empty keyframe, means were all done here
        if(!keyframe.duration && !keyframe.bitmap.id && !keyframe.transforms[0].id){
            break;
        }

        Bitmap * bmp = NULL;

        // if a bitmap is on this keyframe, load it up
        if(keyframe.bitmap.ptr){
            BitmapConfig * bitmapConfig = (BitmapConfig*)keyframe.bitmap.ptr;
            bmp = Bitmap_create(bitmapConfig->width, bitmapConfig->height, (byte*)bitmapConfig->bitmapData.ptr);
        }

        // make that there keyframe
        AnimatorKeyframe k = animatorKeyframeCreate(layer, keyframe.duration, bmp);

        if(keyframe.transforms[0].ptr){
            for(int j = 0; j < MAX_TRANSFORMS; j++){
                // empty transform id, means were all done here
                if(!keyframe.transforms[j].id){
                    break;
                }

                TransformConfig * transformConfig = (TransformConfig*)keyframe.transforms[j].ptr;
                void * transformArgs = transformConfig->transformArgs.ptr;

                // TODO - break this out or something?
                switch(transformConfig->fn){
                    case TRANSLATE_X:
                        {
                        TransformTranslateXConfig * transformArgs = (TransformTranslateXConfig*)transformConfig->transformArgs.ptr;
                        animatorKeyframeAddTransform(k, 
                            createTransformTranslateX(transformArgs->start, transformArgs->end, transformArgs->wrap, LINEAR));
                        }
                        break;

                    case RGB:
                        {
                        TransformRGBConfig * transformArgs = (TransformRGBConfig*)transformConfig->transformArgs.ptr;
                        animatorKeyframeAddTransform(k, 
                            createTransformRGB(transformArgs->begin, transformArgs->end, (BlendingMode)transformArgs->mode));
                        }
                        break;

                    case FIRE:
                        {
                        TransformFireConfig * transformArgs = (TransformFireConfig*)transformConfig->transformArgs.ptr;
                        animatorKeyframeAddTransform(k, 
                            createTransformFire(transformArgs->frequency, transformArgs->flickerCount));
                        }
                        break;

                    case ALPHA_TRANS:
                        {
                        TransformAlphaConfig * transformArgs = (TransformAlphaConfig*)transformConfig->transformArgs.ptr;
                        animatorKeyframeAddTransform(k, 
                            createTransformAlpha(transformArgs->begin, transformArgs->end));
                        }
                        break;

                    default:
                        break;
                }
            }
        }
    }
}

// dynamic layers
// TODO - reenable these when presets are editable
/*
static void addSpinnyLayer(TailLight tail){
    // TODO - someone know that spinny preset is 1
    PresetConfig * preset = presetLoad(1);
    if(!preset){
        Serial.printf("error loading spinny preset from fs\n");
    }

    loadStoredPreset(tail, preset);
}
*/

// hardcoded brake and signals
static void addBrake(TailLight tail){
    AnimatorLayer layer = tailLightCreateBrakeLayer(tail);

    Bitmap * bmp = Bitmap_create(tail->numPx, 1);
    byte red[] = {255, 0, 0, 255};
    Bitmap_fill(bmp, red);

    AnimatorKeyframe k1 = animatorKeyframeCreate(layer, 255, bmp);

    // dont play till needed
    animatorLayerStop(layer);
}

static void addSignalRight(TailLight tail){
    AnimatorLayer layer = tailLightCreateSignalRightLayer(tail);

    byte bitmap_data[] = {
        0,0,0,0,
        0,0,0,0,
        0,0,0,0,
        0,0,0,0,
        0,0,0,0,
        0,0,0,0,
        0,0,0,0,
        0,0,0,0,
        0,0,0,0,
        0,0,0,0,
        255,255,0,67,
        255,255,0,150,
        255,255,0,255,
        255,255,0,255,
        255,255,0,150,
        255,255,0,67,
    };
    Bitmap * bmp1 = Bitmap_create(tail->numPx, 1, bitmap_data);

    // slide
    AnimatorKeyframe k;
    k = animatorKeyframeCreate(layer, 10, bmp1);
    animatorKeyframeAddTransform(k, createTransformTranslateX(0, 10, true, LINEAR));
    animatorKeyframeAddTransform(k, createTransformMirrorX(true));

    // hold
    k = animatorKeyframeCreate(layer, 2, bmp1);
    animatorKeyframeAddTransform(k, createTransformTranslateX(10, 10, true, LINEAR));
    animatorKeyframeAddTransform(k, createTransformMirrorX(true));

    // off
    k = animatorKeyframeCreate(layer, 5, bmp1);
    animatorKeyframeAddTransform(k, createTransformTranslateX(10, 10, true, LINEAR));
    animatorKeyframeAddTransform(k, createTransformMirrorX(true));
    animatorKeyframeAddTransform(k, createTransformAlpha(0.0, 0.0));

    // on
    k = animatorKeyframeCreate(layer, 5, bmp1);
    animatorKeyframeAddTransform(k, createTransformTranslateX(10, 10, true, LINEAR));
    animatorKeyframeAddTransform(k, createTransformMirrorX(true));

    // off
    k = animatorKeyframeCreate(layer, 5, bmp1);
    animatorKeyframeAddTransform(k, createTransformTranslateX(10, 10, true, LINEAR));
    animatorKeyframeAddTransform(k, createTransformMirrorX(true));
    animatorKeyframeAddTransform(k, createTransformAlpha(0.0, 0.0));

    // on
    k = animatorKeyframeCreate(layer, 5, bmp1);
    animatorKeyframeAddTransform(k, createTransformTranslateX(10, 10, true, LINEAR));
    animatorKeyframeAddTransform(k, createTransformMirrorX(true));

    // fade
    k = animatorKeyframeCreate(layer, 5, bmp1);
    animatorKeyframeAddTransform(k, createTransformTranslateX(10, 10, true, LINEAR));
    animatorKeyframeAddTransform(k, createTransformMirrorX(true));
    animatorKeyframeAddTransform(k, createTransformAlpha(1.0, 0.0));

    // dont play till needed
    animatorLayerStop(layer);
}

// TODO - optimize this. lotsa transforms for minimal effect
static void addSignalLeft(TailLight tail){
    AnimatorLayer layer = tailLightCreateSignalLeftLayer(tail);

    byte bitmap_data[] = {
        0,0,0,0,
        0,0,0,0,
        0,0,0,0,
        0,0,0,0,
        0,0,0,0,
        0,0,0,0,
        0,0,0,0,
        0,0,0,0,
        0,0,0,0,
        0,0,0,0,
        255,255,0,67,
        255,255,0,150,
        255,255,0,255,
        255,255,0,255,
        255,255,0,150,
        255,255,0,67,
    };
    Bitmap * bmp1 = Bitmap_create(tail->numPx, 1, bitmap_data);

    // slide
    AnimatorKeyframe k;
    k = animatorKeyframeCreate(layer, 10, bmp1);
    animatorKeyframeAddTransform(k, createTransformTranslateX(0, 10, true, LINEAR));
    animatorKeyframeAddTransform(k, createTransformMirrorX(true));
    animatorKeyframeAddTransform(k, createTransformTranslateX(8, 8, true, LINEAR));

    // hold
    k = animatorKeyframeCreate(layer, 2, bmp1);
    animatorKeyframeAddTransform(k, createTransformTranslateX(10, 10, true, LINEAR));
    animatorKeyframeAddTransform(k, createTransformMirrorX(true));
    animatorKeyframeAddTransform(k, createTransformTranslateX(8, 8, true, LINEAR));

    // off
    k = animatorKeyframeCreate(layer, 5, bmp1);
    animatorKeyframeAddTransform(k, createTransformTranslateX(10, 10, true, LINEAR));
    animatorKeyframeAddTransform(k, createTransformMirrorX(true));
    animatorKeyframeAddTransform(k, createTransformAlpha(0.0, 0.0));

    // on
    k = animatorKeyframeCreate(layer, 5, bmp1);
    animatorKeyframeAddTransform(k, createTransformTranslateX(10, 10, true, LINEAR));
    animatorKeyframeAddTransform(k, createTransformMirrorX(true));
    animatorKeyframeAddTransform(k, createTransformTranslateX(8, 8, true, LINEAR));

    // off
    k = animatorKeyframeCreate(layer, 5, bmp1);
    animatorKeyframeAddTransform(k, createTransformTranslateX(10, 10, true, LINEAR));
    animatorKeyframeAddTransform(k, createTransformMirrorX(true));
    animatorKeyframeAddTransform(k, createTransformAlpha(0.0, 0.0));

    // on
    k = animatorKeyframeCreate(layer, 5, bmp1);
    animatorKeyframeAddTransform(k, createTransformTranslateX(10, 10, true, LINEAR));
    animatorKeyframeAddTransform(k, createTransformMirrorX(true));
    animatorKeyframeAddTransform(k, createTransformTranslateX(8, 8, true, LINEAR));

    // fade
    k = animatorKeyframeCreate(layer, 5, bmp1);
    animatorKeyframeAddTransform(k, createTransformTranslateX(10, 10, true, LINEAR));
    animatorKeyframeAddTransform(k, createTransformMirrorX(true));
    animatorKeyframeAddTransform(k, createTransformAlpha(1.0, 0.0));
    animatorKeyframeAddTransform(k, createTransformTranslateX(8, 8, true, LINEAR));

    // dont play till needed
    animatorLayerStop(layer);
}

static void addOTA(TailLight tail){
    AnimatorLayer layer = tailLightCreateOTALayer(tail);

    Bitmap * bmp = Bitmap_create(tail->numPx, 1);
    byte black[] = {0, 0, 0, 255};
    Bitmap_fill(bmp, black);

    // duration of -1 means run forever
    AnimatorKeyframe k = animatorKeyframeCreate(layer, -1, bmp);
    animatorKeyframeAddTransform(k, createTransformProgressBar(&tail->progress[0], &tail->progress[1]));
    // dont play till needed
    animatorLayerStop(layer);
}

// hardcoded running presets
static void addPulseRedLayer(TailLight tail){
    AnimatorLayer l = tailLightCreateRunningLayer(tail);
    addPulseRedLayer(l, tail->numPx);
}
static void addPulseGreenLayer(TailLight tail){
    AnimatorLayer l = tailLightCreateRunningLayer(tail);
    addPulseGreenLayer(l, tail->numPx);
}
static void addPulseBlueLayer(TailLight tail){
    AnimatorLayer l = tailLightCreateRunningLayer(tail);
    addPulseBlueLayer(l, tail->numPx);
}
static void addPulseYellowLayer(TailLight tail){
    AnimatorLayer l = tailLightCreateRunningLayer(tail);
    addPulseYellowLayer(l, tail->numPx);
}
static void addPulseWhiteLayer(TailLight tail){
    AnimatorLayer l = tailLightCreateRunningLayer(tail);
    addPulseWhiteLayer(l, tail->numPx);
}
static void addSolidRedLayer(TailLight tail){
    AnimatorLayer l = tailLightCreateRunningLayer(tail);
    addSolidRedLayer(l, tail->numPx);
}
static void addSolidGreenLayer(TailLight tail){
    AnimatorLayer l = tailLightCreateRunningLayer(tail);
    addSolidGreenLayer(l, tail->numPx);
}
static void addSolidBlueLayer(TailLight tail){
    AnimatorLayer l = tailLightCreateRunningLayer(tail);
    addSolidBlueLayer(l, tail->numPx);
}
static void addSolidYellowLayer(TailLight tail){
    AnimatorLayer l = tailLightCreateRunningLayer(tail);
    addSolidYellowLayer(l, tail->numPx);
}
static void addSolidWhiteLayer(TailLight tail){
    AnimatorLayer l = tailLightCreateRunningLayer(tail);
    addSolidWhiteLayer(l, tail->numPx);
}
static void addStrobeRedLayer(TailLight tail){
    AnimatorLayer l = tailLightCreateRunningLayer(tail);
    addStrobeRedLayer(l, tail->numPx);
}
static void addStrobeGreenLayer(TailLight tail){
    AnimatorLayer l = tailLightCreateRunningLayer(tail);
    addStrobeGreenLayer(l, tail->numPx);
}
static void addStrobeBlueLayer(TailLight tail){
    AnimatorLayer l = tailLightCreateRunningLayer(tail);
    addStrobeBlueLayer(l, tail->numPx);
}
static void addStrobeYellowLayer(TailLight tail){
    AnimatorLayer l = tailLightCreateRunningLayer(tail);
    addStrobeYellowLayer(l, tail->numPx);
}
static void addStrobeWhiteLayer(TailLight tail){
    AnimatorLayer l = tailLightCreateRunningLayer(tail);
    addStrobeWhiteLayer(l, tail->numPx);
}
static void addColoryLayer(TailLight tail){
    AnimatorLayer l = tailLightCreateRunningLayer(tail);
    addColoryLayer(l, tail->numPx);
}
static void addPulsyColoryLayer(TailLight tail){
    AnimatorLayer l = tailLightCreateRunningLayer(tail);
    addPulsyColoryLayer(l, tail->numPx);
}
static void addFlameLayer(TailLight tail){
    AnimatorLayer l = tailLightCreateRunningLayer(tail);
    addFlameLayer(l, tail->numPx);
}
static void addSpinnyLayer(TailLight tail){
    AnimatorLayer l = tailLightCreateRunningLayer(tail);
    addSpinnyLayer(l, tail->numPx);
}
static void addSpinny2(TailLight tail){
    AnimatorLayer l = tailLightCreateRunningLayer(tail);
    addSpinny2(l, tail->numPx);
}
static void addTightRedPulse(TailLight tail){
    AnimatorLayer l = tailLightCreateRunningLayer(tail);
    addTightRedPulse(l, tail->numPx);
}
static void addWhiteAcross(TailLight tail){
    AnimatorLayer l = tailLightCreateRunningLayer(tail);
    addWhiteAcross(l, tail->numPx);
}
static void addRedBlueAcross(TailLight tail){
    AnimatorLayer l = tailLightCreateRunningLayer(tail);
    addRedBlueAcross(l, tail->numPx);
}

// TODO - this list of presets should probably be
// part of the TailLight struct
static void(*presets[])(TailLight tail) = {
    // TODO - turn this back on when presets
    // are editable
    // dynamic presets
    //addSpinnyLayer,

    // hardcoded presets
    // spinny
    addSpinnyLayer,
    addSpinny2,
    addTightRedPulse,

    // across
    addWhiteAcross,
    addRedBlueAcross,

    // misc
    addColoryLayer,
    addPulsyColoryLayer,
    addFlameLayer,

    // solid, pulse, strobe
    addStrobeRedLayer,
    addPulseRedLayer,
    addSolidRedLayer,
    addStrobeGreenLayer,
    addPulseGreenLayer,
    addSolidGreenLayer,
    addStrobeBlueLayer,
    addPulseBlueLayer,
    addSolidBlueLayer,
    addStrobeYellowLayer,
    addPulseYellowLayer,
    addSolidYellowLayer,
    addStrobeWhiteLayer,
    addPulseWhiteLayer,
    addSolidWhiteLayer,
};

int tailLightNextPreset(TailLight state){
    int preset = ((state->currentPreset) + 1) % state->presetCount;
    if(!tailLightLoadPreset(state, preset)){
        Serial.printf("failed to load preset %i", preset);
        return 0;
    }
    return 1;
}

int tailLightLoadPreset(TailLight state, int presetIndex){
    if(presetIndex > state->presetCount || presetIndex < 0){
        Serial.printf("cannot set preset index %i, out of bounds", presetIndex);
        return 0;
    }
    state->currentPreset = presetIndex;
    // TODO - free bitmaps allocated during layer creation,
    // have each preset require some cleanup function
    presets[state->currentPreset](state);
    return 1;
}

int tailLightGetPresetIndex(TailLight state){
    return state->currentPreset;
}

int tailLightLayerStopAll(TailLight state){
    // stop all layers
    for(int i = 0; i < MAX_LAYERS; i++){
        if(!tailLightLayerStop(state, (TailLightLayer)i)){
            // not a problem!
            Serial.printf("couldn't stop layer %i\n", i);
        }
    }
    return 1;
}

int tailLightUpdateProgressBar(TailLight state, unsigned int progress, unsigned int total){
    // these values are used by the progress bar transform
    // so next time it ticks, it will use the updated values
    state->progress[0] = progress;
    state->progress[1] = total;
    return 1;
}
