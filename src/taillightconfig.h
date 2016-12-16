#ifndef TAILLIGHTCONFIG_H
#define TAILLIGHTCONFIG_H

#include <network.h>
#include <identity.h>

#define DB_VER 3
#define EVENT_VER 2
#define NUM_PX 16
#define TAIL_PIN 2
#define EVENT_PORT 6767
#define BUTTON_PIN 14
#define STATUS_PIN 2
// TODO - get BIN_VERSION from VERSION file
#define BIN_VERSION 13

// number of components that can be registered
// for a controller
#define MAX_COMPONENTS 10

// 192.168.4.1 17082560
#define SERVER_IP_UINT32 17082560

#define HOSTNAME "phoxtail"
#define OTA_HOSTNAME "phoxlightota"

#define PUBLIC_SSID "phoxlight"
#define PUBLIC_PASS "phoxlight"

#define DEFAULT_PRIVATE_SSID "phoxlightpriv"
#define DEFAULT_PRIVATE_PASS "phoxlightpriv"

typedef struct TailLightConfig {
    char ssid[SSID_MAX];
    char pass[PASS_MAX];
    char hostname[HOSTNAME_MAX];
    int currentPreset;
    int offset;
    NetworkMode networkMode;
    Identity components[MAX_COMPONENTS];
} TailLightConfig;

typedef struct PrivateNetworkCreds {
    char ssid[SSID_MAX];
    char pass[PASS_MAX];
} PrivateNetworkCreds;

TailLightConfig * getConfig();
Identity * getIdentity();
PrivateNetworkCreds getPrivateCreds();

int loadConfig();
int writeConfig(TailLightConfig * c);
void logConfig(TailLightConfig * c);
int writeDefaultConfig();
int generatePrivateNetworkCreds();

int registerComponent(Identity * component);
// TODO - removeComponent

#endif
