#ifndef TAILLIGHTCONFIG_H
#define TAILLIGHTCONFIG_H

#include <network.h>
#include <identity.h>

#define OTA_SSID "phoxlight"
#define OTA_PASS "phoxlight"
#define OTA_HOSTNAME "phoxlightota"

#define DB_VER 3
#define EVENT_VER 2
#define NUM_PX 16
#define TAIL_PIN 2
#define EVENT_PORT 6767
#define BUTTON_PIN 14
#define STATUS_PIN 2
// TODO - get BIN_VERSION from VERSION file
#define BIN_VERSION 12

// number of components that can be registered
// for a controller
#define MAX_COMPONENTS 10

#define PUBLIC_SSID "phoxlight"
#define PUBLIC_PASS "phoxlight"

// TODO - generate these
#define PRIVATE_SSID "phoxlightpriv"
#define PRIVATE_PASS "phoxlightpriv"

typedef struct TailLightConfig {
    char ssid[SSID_MAX];
    char pass[PASS_MAX];
    char hostname[HOSTNAME_MAX];
    int currentPreset;
    int offset;
    NetworkMode networkMode;
    Identity components[MAX_COMPONENTS];
} TailLightConfig;

TailLightConfig * getConfig();
Identity * getIdentity();

int loadConfig();
int writeConfig(TailLightConfig * c);
void logConfig(TailLightConfig * c);
int writeDefaultConfig();

int registerComponent(Identity * component);
// TODO - removeComponent

#endif
