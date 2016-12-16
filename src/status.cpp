#include "status.h"
#include <statuslight.h>

int SUPER_SLOW_BLINK[] = {5000,150,0};
int SLOW_BLINK[] = {1000,150,0};
int BLINK[] = {250,150,0};
int FAST_BLINK[] = {100,100,0};
int SUPER_FAST_BLINK[] = {50,50,0};
int SOLID[] = {1,0};

byte RED[] = {100,0,0};
byte ORANGE[] = {70,30,0};
byte YELLOW[] = {100,70,0};
byte GREEN[] = {0,100,0};
byte BLUE2[] = {0,100,100};
byte BLUE[] = {0,0,100};
byte PURPLE[] = {70,0,130};
byte PINK[] = {130,0,70};
byte WHITE[] = {100,100,100};

int setIdleStatusLight(StatusLight status){
    statusLightSetPattern(status, BLUE, SUPER_SLOW_BLINK);
}

int setEnterSyncStatusLight(StatusLight status){
    statusLightSetPattern(status, WHITE, SUPER_FAST_BLINK);
}

int setNetworkConnectStatusLight(StatusLight status){
    statusLightSetPattern(status, PURPLE, FAST_BLINK);
}

int setFSWriteStatusLight(StatusLight status){
    statusLightSetPattern(status, YELLOW, FAST_BLINK);
}

int setMiscStatusLight(StatusLight status){
    statusLightSetPattern(status, ORANGE, FAST_BLINK);
}

int setBusyStatusLight(StatusLight status){
    statusLightSetPattern(status, BLUE, SUPER_FAST_BLINK);
}

int setFailStatusLight(StatusLight status){
    statusLightSetPattern(status, RED, SUPER_FAST_BLINK);
}

int setSuccessStatusLight(StatusLight status){
    statusLightSetPattern(status, GREEN, SUPER_FAST_BLINK);
}

void flashFailStatusLight(StatusLight status){
    setFailStatusLight(status);
    delay(1000);
    // TODO - return to previous status?
    setIdleStatusLight(status);
}

void flashSuccessStatusLight(StatusLight status){
    setSuccessStatusLight(status);
    delay(1000);
    // TODO - return to previous status?
    setIdleStatusLight(status);
}

void flashEnterSyncStatusLight(StatusLight status){
    setSuccessStatusLight(status);
    delay(1000);
    // TODO - return to previous status?
    setIdleStatusLight(status);
}

