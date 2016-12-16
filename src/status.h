#ifndef STATUS_H
#define STATUS_H

#include <statuslight.h>

int setIdleStatusLight(StatusLight status);
int setEnterSyncStatusLight(StatusLight status);
int setNetworkConnectStatusLight(StatusLight status);
int setFSWriteStatusLight(StatusLight status);
int setMiscStatusLight(StatusLight status);
int setBusyStatusLight(StatusLight status);
int setFailStatusLight(StatusLight status);
int setSuccessStatusLight(StatusLight status);
void flashFailStatusLight(StatusLight status);
void flashSuccessStatusLight(StatusLight status);
void flashEnterSyncStatusLight(StatusLight status);

#endif
