#include <Esp.h>
#include <alltransforms.h>
#include <loop.h>
#include <animator.h>
#include <lightstrip.h>
#include <statuslight.h>
#include <network.h>
#include <ota.h>
#include <digitalbutton.h>
#include <eventReceiver.h>
#include <eventSender.h>
#include <eventRegistry.h>
#include <objstore.h>
#include <event.h>
#include <taillight.h>
#include <taillightconfig.h>
#include "status.h"

#define DEV_MODE 0

void asplode(char * err){
    Serial.printf("ERROR: %s\n", err);
    delay(1000);
    ESP.restart();
}

TailLight tailLight;
StatusLight status;
TailLightConfig * config = getConfig();
Identity * id = getIdentity();

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
    config->currentPreset = presetIndex;
    writeConfig(config);
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

void startTurnSignalRight(Event * e, Request * r){
    signalOn = true;
    tailLightLayerStart(tailLight, SIGNAL_RIGHT);
    tailLightLayerStop(tailLight, RUNNING);
}
void stopTurnSignalRight(Event * e, Request * r){
    signalOn = false;
    tailLightLayerStop(tailLight, SIGNAL_RIGHT);
    if(!signalOn && !brakeOn){
        tailLightLayerStart(tailLight, RUNNING);
    }
}
void startTurnSignalLeft(Event * e, Request * r){
    signalOn = true;
    tailLightLayerStart(tailLight, SIGNAL_LEFT);
    tailLightLayerStop(tailLight, RUNNING);
}
void stopTurnSignalLeft(Event * e, Request * r){
    signalOn = false;
    tailLightLayerStop(tailLight, SIGNAL_LEFT);
    if(!signalOn && !brakeOn){
        tailLightLayerStart(tailLight, RUNNING);
    }
}
void startBrakeLayer(Event * e, Request * r){
    brakeOn = true;
    tailLightLayerStart(tailLight, BRAKE);
    tailLightLayerStop(tailLight, RUNNING);
}
void stopBrakeLayer(Event * e, Request * r){
    brakeOn = false;
    tailLightLayerStop(tailLight, BRAKE);
    if(!signalOn && !brakeOn){
        tailLightLayerStart(tailLight, RUNNING);
    }
}

void setNextPreset(Event * e, Request * r){
    nextPreset();
}

void setTaillightOffset(Event * e, Request * r){
    int newOffset = (config->offset+1) % NUM_PX;
    Serial.printf("setting taillight offset to %i\n", newOffset); 
    tailLightSetOffset(tailLight, newOffset);
    config->offset = newOffset;
    writeConfig(config);
}

void setNetworkMode(Event * e, Request * r){
    int mode = (e->body[1] << 8) + e->body[0];
    Serial.printf("setting network mode to %i\n", mode); 
    config->networkMode = (NetworkMode)mode;
    writeConfig(config);
    delay(100);
    flash();
}

void restoreDefaultConfig(Event * e, Request * r){
    Serial.println("restoring default taillight config");
    if(!writeDefaultConfig()){
        Serial.println("could not restore default config");
        return;
    }
    Serial.println("restored default config");
    delay(100);
    flash();
}

void ping(Event * e, Request * r){
    Serial.printf("got ping with requestId %i. I should respond\n", e->header->requestId);
    Serial.printf("responding to %s:%i\n", r->remoteIP.toString().c_str(), r->remotePort);
    if(!eventSendC(r->client, EVENT_VER, PONG, 0, NULL, NULL)){
        Serial.printf("ruhroh");
        return;
    }
    flash();
}

//int eventSendC(WiFiClient * client, int version, int opCode, int length, void * body, int responseId){
void who(Event * e, Request * r){
    Serial.printf("someone wants to know who i am\n");
    if(!eventSendC(r->client, EVENT_VER, WHO, sizeof(Identity), (void*)&id, NULL)){
        Serial.printf("ruhroh");
        return;
    }
}

void requestRegisterComponent(Event * e, Request * r){
    Serial.printf("someone wants me to register a thing\n");
    // TODO - i suspect some sort of sanitization
    // and bounds checking should occur here
    Identity * component = (Identity*)e->body;
    if(!registerComponent(component)){
        Serial.printf("failed to register component\n");
        return;
    }

    // persist new config to disk
    if(!writeConfig(config)){
        Serial.printf("failed to write config to disk\n");
    }

    PrivateNetworkCreds creds = getPrivateCreds();
    Serial.printf("ssid: %s, pass: %s\n", creds.ssid, creds.pass);
    if(!eventSendC(r->client, EVENT_VER, REGISTER_CONFIRM, sizeof(PrivateNetworkCreds), (void*)&creds, NULL)){
        Serial.printf("failed to respond to registration request\n");
        return;
    }
    flash();
}

void pauseTailLight(Event * e, Request * r){
    if(!tailLightPause(tailLight)){
        Serial.println("couldn't pause the taillight");
    }
}

void resumeTailLight(Event * e, Request * r){
    if(!tailLightStart(tailLight)){
        Serial.println("couldn't resume the taillight");
    }
}

void generateNetworkCreds(Event * e, Request * r){
    if(!generatePrivateNetworkCreds()){
        Serial.println("couldn't generate new network creds");
    }
}

typedef struct Pixel {
    uint16_t x;
    uint8_t r;
    uint8_t g;
    uint8_t b;
} Pixel;

void setPixel(Event * e, Request * r){
    Pixel * p = (Pixel*)e->body;
    byte rgb[3] = {p->r, p->g, p->b};
    // TODO - make sure x is within bounds
    tailLightSetPixel(tailLight, p->x, rgb);
}

// events to listen for in run mode
int startRunListeners(){
    int ok = eventListen(EVENT_VER, EVENT_PORT);
    if(ok){
        eventRegister(SIGNAL_R_ON, startTurnSignalRight);
        eventRegister(SIGNAL_R_OFF, stopTurnSignalRight);
        eventRegister(SIGNAL_L_ON, startTurnSignalLeft);
        eventRegister(SIGNAL_L_OFF, stopTurnSignalLeft);
        eventRegister(BRAKE_ON, startBrakeLayer);
        eventRegister(BRAKE_OFF, stopBrakeLayer);
        eventRegister(PING, ping);
        eventRegister(WHO, who);
        eventRegister(NEXT_PRESET, setNextPreset);

        eventRegister(PAUSE_TAILLIGHT, pauseTailLight);
        eventRegister(RESUME_TAILLIGHT, resumeTailLight);
        eventRegister(SET_PIXEL, setPixel);

        Serial.printf("Listening for events with EVENT_VER: %i, eventPort: %i\n",
            EVENT_VER, EVENT_PORT);
    }
    return ok;
}

// events to listen for in sync mode
int startSyncListeners(){
    int ok = eventListen(EVENT_VER, EVENT_PORT);
    if(ok){
        eventRegister(SET_TAILLIGHT_OFFSET, setTaillightOffset);
        eventRegister(SET_DEFAULT_CONFIG, restoreDefaultConfig);
        eventRegister(SET_NETWORK_MODE, setNetworkMode);
        eventRegister(REGISTER_COMPONENT, requestRegisterComponent);
        eventRegister(GENERATE_NETWORK_CREDS, generateNetworkCreds);
        eventRegister(PING, ping);
        eventRegister(WHO, who);

        Serial.printf("Listening for events with EVENT_VER: %i, eventPort: %i\n",
            EVENT_VER, EVENT_PORT);
    }
    return ok;
}

int shouldEnterSyncMode(){
    int buttonPosition;
    pinMode(BUTTON_PIN, INPUT_PULLUP);
    digitalWrite(BUTTON_PIN, HIGH);

    for(int i = 0; i < 5; i++){
        buttonPosition = digitalRead(BUTTON_PIN);
        if(buttonPosition == HIGH){
            return 0;
        }
        delay(200);
    }

    buttonPosition = digitalRead(BUTTON_PIN);
    if(buttonPosition == HIGH){
        return 0;
    }

    return 1;
}

void enterSyncMode(){
    Serial.println("entering sync mode");

    // stop taillight
    tailLightStop(tailLight);

    setEnterSyncStatusLight(status);
    // keep flashing sync status light
    // till user releases button
    pinMode(BUTTON_PIN, INPUT_PULLUP);
    digitalWrite(BUTTON_PIN, HIGH);
    while(digitalRead(BUTTON_PIN) == LOW){
       delay(200); 
    }

    setNetworkConnectStatusLight(status);
    // stop network so it can be restarted in
    // appropriate mode
    if(!networkStop()){
        Serial.println("couldn't stop network");
    }

    // start network
    if(config->networkMode == CONNECT){
        Serial.printf("OTA attempting to connect to ssid: %s, pass: %s\n",
            PUBLIC_SSID, PUBLIC_PASS);
        if(!networkConnect(PUBLIC_SSID, PUBLIC_PASS)){
            Serial.println("couldnt bring up network");
            flashFailStatusLight(status);
            return;
        }
    } else {
        Serial.printf("OTA attempting to create ssid: %s, pass: %s\n",
            PUBLIC_SSID, PUBLIC_PASS);
        if(!networkCreate(PUBLIC_SSID, PUBLIC_PASS, IPAddress(SERVER_IP_UINT32))){
            Serial.println("couldnt create network");
            flashFailStatusLight(status);
            return;
        }
    }

    networkAdvertise(OTA_HOSTNAME);
    Serial.printf("OTA advertising hostname: %s\n", OTA_HOSTNAME);

    if(!startSyncListeners()){
        Serial.println("couldnt start listening for events");
    }

    // ota
    otaOnStart(&otaStarted);
    otaOnProgress(&otaProgress);
    otaOnError(&otaError);
    otaOnEnd(&otaEnd);
    otaStart();

    byte green[3] = {0,40,0};
    int pattern2[] = {3000,50,0};

    flashSuccessStatusLight(status);
    setIdleStatusLight(status);
}

void setup(){
    Serial.begin(115200);
    Serial.println("\n");

    setupStartHeap = ESP.getFreeHeap();
    Serial.printf("setupStartHeap: %i\n", setupStartHeap);

    status = statusLightCreate(STATUS_PIN, 16);
    setFSWriteStatusLight(status);

    // load config from fs
    loadConfig();
    logConfig(config);

    setMiscStatusLight(status);

    // start up taillight
    tailLight = tailLightCreate(TAIL_PIN, NUM_PX, 1.0, config->offset);
    if(tailLight == NULL){
        asplode("couldnt create taillight");
    }
    if(!tailLightStart(tailLight)){
        asplode("couldnt start taillight");
    }

    if(shouldEnterSyncMode()){
        Serial.println("going to sync mode");
        enterSyncMode();
        return;
    }

    setNetworkConnectStatusLight(status);

    // start network
    switch(config->networkMode){
        case CONNECT:
            if(!networkConnect(config->ssid, config->pass)){
                Serial.println("couldnt bring up network");
            }
            networkAdvertise(config->hostname);
            break;
        case CREATE:
            if(!networkCreate(config->ssid, config->pass, IPAddress(SERVER_IP_UINT32))){
                Serial.println("couldnt create up network");
            }
            networkAdvertise(config->hostname);
            break;
        case OFF:
            Serial.println("turning network off");
            if(!networkOff()){
                Serial.println("couldnt turn off network");
            }
            break;
        default:
            Serial.println("couldnt load network mode, defaulting to CONNECT");
            if(!networkConnect(config->ssid, config->pass)){
                Serial.println("couldnt bring up network");
            }
            networkAdvertise(config->hostname);
            break;
    }

    if(!startRunListeners()){
        Serial.println("couldnt start listening for events");
    }

    // NOTE - this stuff is unsafe for run mode! make sure
    // DEV_MODE is off in production!
    if(DEV_MODE){
        if(!startSyncListeners()){
            Serial.println("couldnt start sync mode listeners");
        }
        otaOnStart(&otaStarted);
        otaOnProgress(&otaProgress);
        otaOnError(&otaError);
        otaOnEnd(&otaEnd);
        otaStart();
    }

    setFSWriteStatusLight(status);

    // load last selected preset
    tailLightLoadPreset(tailLight, config->currentPreset);

    statusLightStop(status);

    // switch presets
    DigitalButton btn = buttonCreate(BUTTON_PIN, 50);
    buttonOnTap(btn, nextPreset);

    // debug log heap usage so i can keep an eye out for leaks
    setupEndHeap = ESP.getFreeHeap();
    Serial.printf("setupEndHeap: %i, delta: %i\n", setupEndHeap, setupStartHeap - setupEndHeap);
    loopAttach(logHeapUsage, 5000, NULL);
}

void loop(){
    loopTick();
}
