/*
 * AUTO-GENERATED _componentMain.c for the sensorsComponent component.

 * Don't bother hand-editing this file.
 */

#include "legato.h"
#include "../liblegato/eventLoop.h"
#include "../liblegato/log.h"

#ifdef __cplusplus
extern "C" {
#endif

extern const char* _sensorsComponent_le_adc_ServiceInstanceName;
const char** le_adc_ServiceInstanceNamePtr = &_sensorsComponent_le_adc_ServiceInstanceName;
void le_adc_ConnectService(void);
// Component log session variables.
le_log_SessionRef_t sensorsComponent_LogSession;
le_log_Level_t* sensorsComponent_LogLevelFilterPtr;

// Component initialization function (COMPONENT_INIT).
void _sensorsComponent_COMPONENT_INIT(void);

// Library initialization function.
// Will be called by the dynamic linker loader when the library is loaded.
__attribute__((constructor)) void _sensorsComponent_Init(void)
{
    LE_DEBUG("Initializing sensorsComponent component library.");

    // Connect client-side IPC interfaces.
    le_adc_ConnectService();

    // Register the component with the Log Daemon.
    sensorsComponent_LogSession = log_RegComponent("sensorsComponent", &sensorsComponent_LogLevelFilterPtr);

    //Queue the COMPONENT_INIT function to be called by the event loop
    event_QueueComponentInit(_sensorsComponent_COMPONENT_INIT);
}


#ifdef __cplusplus
}
#endif
