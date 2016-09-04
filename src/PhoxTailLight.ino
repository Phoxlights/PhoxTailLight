#include "loop.h"
#include "taillight.h"
#include "animator.h"
#include "lightstrip.h"
#include "statuslight.h"
#include "alltransforms.h"
#include "network.h"
#include "ota.h"
#include "digitalbutton.h"
#include "eventReceiver.h"
#include "eventRegistry.h"
#include "objstore.h"
#include "event.h"

#define OTA_SSID "phoxlight"
#define OTA_PASS "phoxlight"
#define OTA_HOSTNAME "phoxlightota"

void asplode(char * err){
    Serial.printf("ERROR: %s\n", err);
    delay(1000);
    ESP.restart();
}

typedef struct TailLightConfig {
    int numPx;
    int tailPin;
    char ssid[SSID_MAX];
    char pass[PASS_MAX];
    char hostname[HOSTNAME_MAX];
    int eventPort;
    int eventVer;
    int currentPreset;
    int offset;
    int buttonPin;
    int statusPin;
    NetworkMode networkMode;
} TailLightConfig;

// default config values
TailLightConfig defaultConfig = {
    16,
    2,
    "phoxlight",
    "phoxlight",
    "phoxlight",
    6767,
    1,
    0,
    0,
    0, // pin 14 for production unit
    2,
    CONNECT
};

int configId = 1;
TailLightConfig config;
TailLight tailLight;

StatusLight status;

int writeDefaultConfig(){
    // delete existing taillight config
    Serial.println("wiping taillight config");
    objStoreWipe("taillight");

    int id = objStoreCreate("taillight", &defaultConfig, sizeof(TailLightConfig));
    if(!id){
        Serial.println("failed to write default config");
        return 0;
    }
    Serial.printf("wrote default taillight config, with id %i\n", id);
    return 1;
}

int writeCurrentConfig(){
    if(!objStoreUpdate("taillight", configId, &config, sizeof(TailLightConfig))){
        Serial.println("failed to write config");
        return 0;
    }
    return 1;
}

int loadConfig(){
    // load from fs
    objStoreInit(1);
    if(!objStoreGet("taillight", configId, &config, sizeof(TailLightConfig))){
        // store defaults
        Serial.println("taillight config not found, storing defaults");
        if(!writeDefaultConfig()){
            asplode("couldnt store default taillight config. prepare your butt.");
        }
        // reload config now that we have defaults
        loadConfig();
    }
    return 1;
}

void logConfig(){
    Serial.printf("\
config: {\n\
    numPx: %i,\n\
    tailPin: %i,\n\
    ssid: %s,\n\
    hostname: %s,\n\
    eventPort: %i,\n\
    eventVer: %i,\n\
    currentPreset: %i,\n\
    offset: %i,\n\
    buttonPin: %i,\n\
    statusPin: %i,\n\
    networkMode: %s,\n\
}\n", 
    config.numPx, config.tailPin, config.ssid,
    config.hostname, config.eventPort, config.eventVer,
    config.currentPreset, config.offset, config.buttonPin,
    config.statusPin, 
    config.networkMode == 0 ? "CONNECT" : config.networkMode == 1 ? "CREATE" : "OFF");
}

void otaStarted(){
    Serial.println("ota start");
    // NOTE - OTA turns off the main loop, so we need to manually
    // tick the taillight
    tailLightStop(tailLight);
    statusLightStop(status);
    if(!tailLightLayerStart(tailLight, OTA)){
        Serial.println("couldn't start ota layer");
    }
}

void otaProgress(unsigned int progress, unsigned int total){
    Serial.print("ota progress");
    tailLightUpdateProgressBar(tailLight, progress, total);
    // NOTE - OTA turns off the main loop, so we need to manually
    // tick the taillight
    tailLightTick(tailLight);
}

void otaError(ota_error_t err){
    Serial.println("ota err");
    // TODO - restart taillight?
    //tailLightLayerStopAll(tailLight);
    //tailLightLayerStart(tailLight, RUNNING);
}

void otaEnd(){
    Serial.println("ota end");
    tailLightStop(tailLight);
}

void nextPreset(){
    tailLightNextPreset(tailLight);
    int presetIndex = tailLightGetPresetIndex(tailLight);
    config.currentPreset = presetIndex;
    writeCurrentConfig();
}

int setupStartHeap, setupEndHeap, prevHeap;
void logHeapUsage(void * state){
    int currHeap = ESP.getFreeHeap();
    int delta = setupEndHeap - currHeap;
    Serial.printf("currHeap: %i, delta: %i\n", currHeap, delta);
    prevHeap = currHeap;
}

// flashes status light real quick
void flash(){
    byte white[3] = {50,50,50};
    int pattern[] = {50, 100, 0};
    if(!statusLightSetPattern(status, white, pattern)){
        Serial.println("couldnt flash status light");
    }
    delay(50);
    statusLightStop(status);
}

// keep track of extra layers to determine
// if running layer should be started or
// stopped
bool signalOn = false;
bool brakeOn = false;

void startTurnSignalRight(Event * e){
    signalOn = true;
    tailLightLayerStart(tailLight, SIGNAL_RIGHT);
    tailLightLayerStop(tailLight, RUNNING);
}
void stopTurnSignalRight(Event * e){
    signalOn = false;
    tailLightLayerStop(tailLight, SIGNAL_RIGHT);
    if(!signalOn && !brakeOn){
        tailLightLayerStart(tailLight, RUNNING);
    }
}
void startTurnSignalLeft(Event * e){
    signalOn = true;
    tailLightLayerStart(tailLight, SIGNAL_LEFT);
    tailLightLayerStop(tailLight, RUNNING);
}
void stopTurnSignalLeft(Event * e){
    signalOn = false;
    tailLightLayerStop(tailLight, SIGNAL_LEFT);
    if(!signalOn && !brakeOn){
        tailLightLayerStart(tailLight, RUNNING);
    }
}
void startBrakeLayer(Event * e){
    brakeOn = true;
    tailLightLayerStart(tailLight, BRAKE);
    tailLightLayerStop(tailLight, RUNNING);
}
void stopBrakeLayer(Event * e){
    brakeOn = false;
    tailLightLayerStop(tailLight, BRAKE);
    if(!signalOn && !brakeOn){
        tailLightLayerStart(tailLight, RUNNING);
    }
}

void setTaillightOffset(Event * e){
    int newOffset = (config.offset+1) % config.numPx;
    Serial.printf("setting taillight offset to %i\n", newOffset); 
    tailLightSetOffset(tailLight, newOffset);
    config.offset = newOffset;
    writeCurrentConfig();
}

void setButtonPin(Event * e){
    int pin = (e->body[1] << 8) + e->body[0];
    Serial.printf("setting button pin to %i\n", pin); 
    config.buttonPin = pin;
    writeCurrentConfig();
    delay(100);
    flash();
}

void setTaillightPin(Event * e){
    int pin = (e->body[1] << 8) + e->body[0];
    Serial.printf("setting tail light pin to %i\n", pin); 
    config.tailPin = pin;
    writeCurrentConfig();
    delay(100);
    flash();
}

void setStatusPin(Event * e){
    int pin = (e->body[1] << 8) + e->body[0];
    Serial.printf("setting status pin to %i\n", pin); 
    config.statusPin = pin;
    writeCurrentConfig();
    delay(100);
    flash();
}

void setNetworkMode(Event * e){
    int mode = (e->body[1] << 8) + e->body[0];
    Serial.printf("setting network mode to %i\n", mode); 
    config.networkMode = (NetworkMode)mode;
    writeCurrentConfig();
    delay(100);
    flash();
}

void restoreDefaultConfig(Event * e){
    Serial.println("restoring default taillight config");
    if(!writeDefaultConfig()){
        Serial.println("could not restore default config");
        return;
    }
    Serial.println("restored default config");
    delay(100);
    flash();
}

void ping(Event * e){
    flash();
}

void pauseTailLight(Event * e){
    if(!tailLightPause(tailLight)){
        Serial.println("couldn't pause the taillight");
    }
}

void resumeTailLight(Event * e){
    if(!tailLightStart(tailLight)){
        Serial.println("couldn't resume the taillight");
    }
}

typedef struct Pixel {
    uint16_t x;
    uint8_t r;
    uint8_t g;
    uint8_t b;
} Pixel;

void setPixel(Event * e){
    Pixel * p = (Pixel*)e->body;
    byte rgb[3] = {p->r, p->g, p->b};
    // TODO - make sure x is within bounds
    tailLightSetPixel(tailLight, p->x, rgb);
}

bool canOTA = true;
void neverOTAEver(){
    // button was released after boot, 
    // so don't allow OTA mode to happen
    canOTA = false;
}
void enterOTAMode(){
    Serial.println("entering OTA mode");

    // stop taillight
    tailLightStop(tailLight);

    // status light
    byte blue[3] = {0,0,40};
    byte red[3] = {40,0,0};
    int pattern[] = {500,50,0};
    if(!statusLightSetPattern(status, blue, pattern)){
        Serial.println("couldnt setup status light");
    }

    // stop network so it can be restarted in
    // connect mode
    if(!networkStop()){
        Serial.println("couldn't stop network");
    }

    Serial.printf("OTA attempting to connect to ssid: %s, pass: %s\n",
        OTA_SSID, OTA_PASS);

    if(!networkConnect(OTA_SSID, OTA_PASS)){
        Serial.println("couldnt connect to ota network");
        statusLightSetPattern(status, red, pattern);
        return;
    }
    networkAdvertise(OTA_HOSTNAME);
    Serial.printf("OTA advertising hostname: %s\n", OTA_HOSTNAME);

    // enable SET_NETWORK_MODE endpoint just in case it isnt,
    // this way a device with NETWORK_MODE off will be able to
    // be turned back on
    eventReceiverStart(config.eventVer, config.eventPort);
    eventReceiverRegister(SET_NETWORK_MODE, setNetworkMode);
    Serial.printf("Listening for SET_NETWORK_MODE with eventVer: %i, eventPort: %i\n",
        config.eventVer, config.eventPort);

    // ota
    otaOnStart(&otaStarted);
    otaOnProgress(&otaProgress);
    otaOnError(&otaError);
    otaOnEnd(&otaEnd);
    otaStart();

    byte green[3] = {0,40,0};
    int pattern2[] = {3000,50,0};
    if(!statusLightSetPattern(status, green, pattern2)){
        Serial.println("couldnt setup status light");
    }
}

void setup(){
    Serial.begin(115200);
    delay(1000);
    Serial.println("\n");

    setupStartHeap = ESP.getFreeHeap();
    Serial.printf("setupStartHeap: %i\n", setupStartHeap);

    // NOTE - config.statusPin is NOT used here
    // because this needs to be guaranteed to work
    status = statusLightCreate(2, 16);

    byte purple[3] = {20,0,20};
    int pattern[] = {1000,50,0};
    if(!statusLightSetPattern(status, purple, pattern)){
        Serial.println("couldnt setup status light");
    }

    // load config from fs
    loadConfig();
    logConfig();

    // status light
    byte blue[3] = {0,0,40};
    if(!statusLightSetPattern(status, blue, pattern)){
        Serial.println("couldnt setup status light");
    }

    // start network
    switch(config.networkMode){
        case CONNECT:
            if(!networkConnect(config.ssid, config.pass)){
                Serial.println("couldnt bring up network");
            }
            networkAdvertise(config.hostname);
            break;
        case CREATE:
            if(!networkCreate(config.ssid, config.pass, IPAddress(192,168,4,1))){
                Serial.println("couldnt create up network");
            }
            networkAdvertise(config.hostname);
            break;
        case OFF:
            Serial.println("turning network off");
            if(!networkOff()){
                Serial.println("couldnt turn off network");
            }
            break;
        default:
            Serial.println("couldnt load network mode, defaulting to CONNECT");
            if(!networkConnect(config.ssid, config.pass)){
                Serial.println("couldnt bring up network");
            }
            networkAdvertise(config.hostname);
            break;
    }

    byte green[3] = {0,40,0};
    if(!statusLightSetPattern(status, green, pattern)){
        Serial.println("couldnt setup status light");
    }

    byte yellow[3] = {20,20,20};
    if(!statusLightSetPattern(status, yellow, pattern)){
        Serial.println("couldnt setup status light");
    }

    if(eventReceiverStart(config.eventVer, config.eventPort)){
        eventReceiverRegister(SIGNAL_R_ON, startTurnSignalRight);
        eventReceiverRegister(SIGNAL_R_OFF, stopTurnSignalRight);
        eventReceiverRegister(SIGNAL_L_ON, startTurnSignalLeft);
        eventReceiverRegister(SIGNAL_L_OFF, stopTurnSignalLeft);
        eventReceiverRegister(BRAKE_ON, startBrakeLayer);
        eventReceiverRegister(BRAKE_OFF, stopBrakeLayer);
        eventReceiverRegister(PING, ping);

        // these should eventually go to a safer API
        // (maybe in OTA mode only or something)
        eventReceiverRegister(SET_TAILLIGHT_OFFSET, setTaillightOffset);
        eventReceiverRegister(SET_TAILLIGHT_PIN, setTaillightPin);
        eventReceiverRegister(SET_STATUS_PIN, setStatusPin);
        eventReceiverRegister(SET_BUTTON_PIN, setButtonPin);
        eventReceiverRegister(SET_DEFAULT_CONFIG, restoreDefaultConfig);
        eventReceiverRegister(SET_NETWORK_MODE, setNetworkMode);

        eventReceiverRegister(PAUSE_TAILLIGHT, pauseTailLight);
        eventReceiverRegister(RESUME_TAILLIGHT, resumeTailLight);
        eventReceiverRegister(SET_PIXEL, setPixel);
    }

    byte orange[3] = {20,20,0};
    if(!statusLightSetPattern(status, orange, pattern)){
        Serial.println("couldnt setup status light");
    }

    // start up taillight
    tailLight = tailLightCreate(config.tailPin, config.numPx, 1.0, config.offset);
    if(tailLight == NULL){
        asplode("couldnt create taillight");
    }
    if(!tailLightStart(tailLight)){
        asplode("couldnt start taillight");
    }

    // load last selected preset
    tailLightLoadPreset(tailLight, config.currentPreset);

    statusLightStop(status);

    // switch presets
    DigitalButton btn = buttonCreate(config.buttonPin, 200);
    buttonOnTap(btn, nextPreset);
    // OTA mode
    buttonOnUp(btn, neverOTAEver);
    buttonOnHold(btn, enterOTAMode, 4000);

    // debug log heap usage so i can keep an eye out for leaks
    setupEndHeap = ESP.getFreeHeap();
    Serial.printf("setupEndHeap: %i, delta: %i\n", setupEndHeap, setupStartHeap - setupEndHeap);
    loopAttach(logHeapUsage, 5000, NULL);
}

void loop(){
    loopTick();
}
