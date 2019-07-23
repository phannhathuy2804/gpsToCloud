/*
 * AUTO-GENERATED _componentMain.c for the gpsToCloudComponent component.

 * Don't bother hand-editing this file.
 */

#include "legato.h"
#include "../liblegato/eventLoop.h"
#include "../liblegato/log.h"

#ifdef __cplusplus
extern "C" {
#endif

extern const char* _gpsToCloudComponent_le_pos_ServiceInstanceName;
const char** le_pos_ServiceInstanceNamePtr = &_gpsToCloudComponent_le_pos_ServiceInstanceName;
void le_pos_ConnectService(void);
extern const char* _gpsToCloudComponent_le_posCtrl_ServiceInstanceName;
const char** le_posCtrl_ServiceInstanceNamePtr = &_gpsToCloudComponent_le_posCtrl_ServiceInstanceName;
void le_posCtrl_ConnectService(void);
extern const char* _gpsToCloudComponent_le_avdata_ServiceInstanceName;
const char** le_avdata_ServiceInstanceNamePtr = &_gpsToCloudComponent_le_avdata_ServiceInstanceName;
void le_avdata_ConnectService(void);
// Component log session variables.
le_log_SessionRef_t gpsToCloudComponent_LogSession;
le_log_Level_t* gpsToCloudComponent_LogLevelFilterPtr;

// Component initialization function (COMPONENT_INIT).
void _gpsToCloudComponent_COMPONENT_INIT(void);

// Library initialization function.
// Will be called by the dynamic linker loader when the library is loaded.
__attribute__((constructor)) void _gpsToCloudComponent_Init(void)
{
    LE_DEBUG("Initializing gpsToCloudComponent component library.");

    // Connect client-side IPC interfaces.
    le_pos_ConnectService();
    le_posCtrl_ConnectService();
    le_avdata_ConnectService();

    // Register the component with the Log Daemon.
    gpsToCloudComponent_LogSession = log_RegComponent("gpsToCloudComponent", &gpsToCloudComponent_LogLevelFilterPtr);

    //Queue the COMPONENT_INIT function to be called by the event loop
    event_QueueComponentInit(_gpsToCloudComponent_COMPONENT_INIT);
}


#ifdef __cplusplus
}
#endif
