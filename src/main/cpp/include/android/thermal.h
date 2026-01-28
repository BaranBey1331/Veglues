#ifndef ANDROID_THERMAL_H
#define ANDROID_THERMAL_H

#ifdef __cplusplus
extern "C" {
#endif

enum AThermalStatus {
    ATHERMAL_STATUS_NONE = 0,
    ATHERMAL_STATUS_LIGHT = 1,
    ATHERMAL_STATUS_MODERATE = 2,
    ATHERMAL_STATUS_SEVERE = 3,
    ATHERMAL_STATUS_CRITICAL = 4,
    ATHERMAL_STATUS_EMERGENCY = 5,
    ATHERMAL_STATUS_SHUTDOWN = 6
};

struct AThermalManager;
typedef struct AThermalManager AThermalManager;

typedef void (*AThermalStatusListener)(void *data, AThermalStatus status);

// Function prototypes - strictly available API 30+
AThermalManager* AThermal_acquireManager();
void AThermal_releaseManager(AThermalManager *manager);
AThermalStatus AThermal_getCurrentThermalStatus(AThermalManager *manager);
int AThermal_registerThermalStatusListener(AThermalManager *manager, AThermalStatusListener listener, void *data);
int AThermal_unregisterThermalStatusListener(AThermalManager *manager, AThermalStatusListener listener, void *data);

#ifdef __cplusplus
}
#endif

#endif
