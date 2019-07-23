#include "legato.h"
#include "interfaces.h"
#include "accelerometer.h"


// [DeclareVariables]
#define APP_RUNNING_DURATION_SEC 600        //run this app for 10min

//Device configuration path
#define DEVICE_CONFIG_SET_RES    "/deviceConfig"
#define MAX_RESOURCES           20


// [AssetDataPath]
//-------------------------------------------------------------------------------------------------
/**
 * Declare asset data path
 */
//-------------------------------------------------------------------------------------------------

/* variables */
// float - Latitude, longtitude and Accuracy
#define LATITUDE "lwm2m.6.0.0"

#define LONGTITUDE "lwm2m.6.0.1"

#define ACCURACY "lwm2m.6.0.3"

#define Accel_X "Acceleration X"

#define Accel_Y "Acceleration Y"

#define Accel_Z "Acceleration Z"

#define Gyro_X "Gyro X"

#define Gyro_Y "Gyro Y"

#define Gyro_Z "Gyro Z"
// [AssetDataPath]
//-------------------------------------------------------------------------------------------------
/**
 * AVC related variable and update timer
 */
//-------------------------------------------------------------------------------------------------
// reference timer for app session
le_timer_Ref_t sessionTimer;
//reference to AVC event handler
le_avdata_SessionStateHandlerRef_t  avcEventHandlerRef = NULL;
//reference to AVC Session handler
le_avdata_RequestSessionObjRef_t sessionRef = NULL;
//reference to location update timer
le_timer_Ref_t locUpdateTimerRef = NULL;
le_timer_Ref_t accelUpdateTimerRef = NULL;
//reference to push asset data timer
le_timer_Ref_t serverUpdateTimerRef = NULL;
//reference to record timer
le_timer_Ref_t recordUpdateTimerRef = NULL;
//reference to record
static le_avdata_RecordRef_t recordRef;
//-------------------------------------------------------------------------------------------------
/**
 * Target location related declarations.
 */
//-------------------------------------------------------------------------------------------------
static double lat = 2.0;
static double longt = 2.0;
static double hAccu = 0.0;
static double AcceX = 0.0;
static double AcceY = 0.0;
static double AcceZ = 0.0;
static double GyroX = 0.0;
static double GyroY = 0.0;
static double GyroZ = 0.0;


static uint64_t GetCurrentTimestamp(void)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    uint64_t utcMilliSec = (uint64_t)(tv.tv_sec) * 1000 + (uint64_t)(tv.tv_usec) / 1000;
    return utcMilliSec;
}
//-------------------------------------------------------------------------------------------------
/**
 * Device Config setting data handler.
 * This function is returned whenever AirVantage performs a write on /deviceConfig
 */
//-------------------------------------------------------------------------------------------------
static void DeviceConfigHandler
(
    const char* path,
    le_avdata_AccessType_t accessType,
    le_avdata_ArgumentListRef_t argumentList,
    void* contextPtr
)
{
    int newValue;

    // User has to append the app name to asset data path while writing from server side.
    // Hence use global name space for accessing the value written to this path.
    le_avdata_SetNamespace(LE_AVDATA_NAMESPACE_GLOBAL);
    le_result_t resultGetInt = le_avdata_GetInt(path, &newValue);
    le_avdata_SetNamespace(LE_AVDATA_NAMESPACE_APPLICATION);

    if (LE_OK == resultGetInt)
    {
        LE_INFO("%s set to %d", path, newValue);
    }
    else
    {
        LE_ERROR("Error in getting setting %s - Error = %d", path, resultGetInt);
    }
}

// [PushCallbackHandler]
//-------------------------------------------------------------------------------------------------
/**
 * Push ack callback handler
 * This function is called whenever push has been performed successfully in AirVantage server
 */
//-------------------------------------------------------------------------------------------------
static void PushCallbackHandler
(
    le_avdata_PushStatus_t status,
    void* contextPtr
)
{
    switch (status)
    {
        case LE_AVDATA_PUSH_SUCCESS:
            LE_INFO("Legato assetdata push successfully");
            break;
        case LE_AVDATA_PUSH_FAILED:
            LE_INFO("Legato assetdata push failed");
            break;
    }
}
//-------------------------------------------------------------------------------------------------
/**
 * Push ack callback handler
 * This function is called every 10 seconds to push the data and update data in AirVantage server
 */
//-------------------------------------------------------------------------------------------------
/*void PushResources(le_timer_Ref_t  timerRef)
{
	// if session is still open, push the values
    if (NULL != avcEventHandlerRef)
    {
        le_result_t resultPushLatitude;
        resultPushLatitude = le_avdata_Push(LATITUDE, PushCallbackHandler, NULL);
        if (LE_FAULT == resultPushLatitude)
        {
            LE_ERROR("Error pushing LATITUDE");
        }
        le_result_t resultPushLongtitude;
        resultPushLongtitude = le_avdata_Push(LONGTITUDE, PushCallbackHandler, NULL);
        if (LE_FAULT == resultPushLongtitude)
        {
            LE_ERROR("Error pushing LONGTITUDE");
        }
        le_result_t resultPushAccuracy;
        resultPushAccuracy = le_avdata_Push(ACCURACY, PushCallbackHandler, NULL);
        if (LE_FAULT == resultPushAccuracy)
        {
            LE_ERROR("Error pushing ACCURACY");
        }
        le_result_t resultPushAccelX;
        resultPushAccelX = le_avdata_Push(Accel_X, PushCallbackHandler, NULL);
        if (LE_FAULT == resultPushAccelX)
        {
            LE_ERROR("Error pushing AccelX");
        }
        le_result_t resultPushAccelY;
        resultPushAccelY = le_avdata_Push(Accel_Y, PushCallbackHandler, NULL);
        if (LE_FAULT == resultPushAccelY)
        {
            LE_ERROR("Error pushing AccelY");
        }
        le_result_t resultPushAccelZ;
        resultPushAccelZ = le_avdata_Push(Accel_Z, PushCallbackHandler, NULL);
        if (LE_FAULT == resultPushAccelZ)
        {
            LE_ERROR("Error pushing AccelZ");
        }
        le_result_t resultPushGyroX;
        resultPushGyroX = le_avdata_Push(Gyro_X, PushCallbackHandler, NULL);
        if (LE_FAULT == resultPushGyroX)
        {
            LE_ERROR("Error pushing GyroX");
        }
        le_result_t resultPushGyroY;
        resultPushGyroY = le_avdata_Push(Gyro_Y, PushCallbackHandler, NULL);
        if (LE_FAULT == resultPushGyroY)
        {
            LE_ERROR("Error pushing GyroY");
        }
        le_result_t resultPushGyroZ;
        resultPushGyroZ = le_avdata_Push(Gyro_Z, PushCallbackHandler, NULL);
        if (LE_FAULT == resultPushGyroZ)
        {
            LE_ERROR("Error pushing GyroZ");
        }
    }
}
*/
//-------------------------------------------------------------------------------------------------
/**
 * Function relevant to AirVantage server connection
 */
//-------------------------------------------------------------------------------------------------
static void sig_appTermination_cbh(int sigNum)
{
    LE_INFO("Close AVC session");
    le_avdata_ReleaseSession(sessionRef);
    if (NULL != avcEventHandlerRef)
    {
        //unregister the handler
        LE_INFO("Unregister the session handler");
        le_avdata_RemoveSessionStateHandler(avcEventHandlerRef);
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * Get 2D location function
 */
//-------------------------------------------------------------------------------------------------
void getLocation()
{

	int32_t la=10, lo=10, acc=10;
	le_result_t result;
	result = le_pos_Get2DLocation(&la,&lo,&acc);
	LE_INFO("result = %d \n", result);
	LE_INFO("Latitude= %d, Longtitude= %d, Accuracy= %d",la,lo,acc);
    lat= (double)la / 1000000.0;
    longt= (double)lo / 1000000.0;
	hAccu= (double)acc;
	le_result_t resultSetLatitude = le_avdata_SetFloat(LATITUDE, lat);
    if (LE_FAULT == resultSetLatitude)
    {
        LE_ERROR("Error in setting LATITUDE");
    }
    le_result_t resultSetLongtitude = le_avdata_SetFloat(LONGTITUDE, longt);
    if (LE_FAULT == resultSetLongtitude)
    {
        LE_ERROR("Error in setting LONGTITUDE");
    }
    le_result_t resultSetAccuracy = le_avdata_SetInt(ACCURACY, hAccu);
    if (LE_FAULT == resultSetAccuracy)
    {
        LE_ERROR("Error in setting ACCURACY");
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * Get accelerometer
 */
//-------------------------------------------------------------------------------------------------
void getAccelerometer()
{
	le_result_t result;
	result = mangOH_ReadAccelerometer(&AcceX,&AcceY,&AcceZ);
	if(result==LE_OK)
		LE_INFO("AcceX = %f, AcceY = %f, AcceZ = %f",AcceX,AcceY,AcceZ);
	else
		LE_INFO("Can't get Accel");
	result = mangOH_ReadGyro(&GyroX,&GyroY,&GyroZ);
	if(result==LE_OK)
		LE_INFO("GyroX = %f, GyroY = %f, GyroZ = %f",GyroX,GyroY,GyroZ);
	else
		LE_INFO("Can't get Gyro");
	le_result_t resultSetAcceX = le_avdata_SetFloat(Accel_X,AcceX);
	if(LE_FAULT==resultSetAcceX)
		LE_ERROR("Error in setting Accel X");
	le_result_t resultSetAcceY = le_avdata_SetFloat(Accel_Y,AcceY);
	if(LE_FAULT==resultSetAcceY)
		LE_ERROR("Error in setting Accel Y");
	le_result_t resultSetAcceZ = le_avdata_SetFloat(Accel_Z,AcceZ);
	if(LE_FAULT==resultSetAcceZ)
		LE_ERROR("Error in setting Accel Z");
	le_result_t resultSetGyroX = le_avdata_SetFloat(Gyro_X,GyroX);
	if(LE_FAULT==resultSetGyroX)
		LE_ERROR("Error in setting Gyro X");
	le_result_t resultSetGyroY = le_avdata_SetFloat(Gyro_Y,GyroY);
	if(LE_FAULT==resultSetGyroY)
		LE_ERROR("Error in setting Gyro Y");
	le_result_t resultSetGyroZ = le_avdata_SetFloat(Gyro_Z,GyroZ);
	if(LE_FAULT==resultSetGyroZ)
		LE_ERROR("Error in setting Gyro Z");

}

//-------------------------------------------------------------------------------------------------
/**
 * record information
 */
//-------------------------------------------------------------------------------------------------
void recordInfo ()
{
	le_result_t result = le_avdata_RecordFloat(recordRef,LATITUDE,lat,GetCurrentTimestamp());
	if ( result !=LE_OK)
		LE_ERROR("Error in recording latitude");
	result = le_avdata_RecordFloat(recordRef,LONGTITUDE,longt,GetCurrentTimestamp());
	if ( result !=LE_OK)
		LE_ERROR("Error in recording longtitude");
	result = le_avdata_RecordFloat(recordRef,ACCURACY,hAccu,GetCurrentTimestamp());
	if ( result !=LE_OK)
		LE_ERROR("Error in recording accuracy");
	result = le_avdata_RecordFloat(recordRef,Accel_X,AcceX,GetCurrentTimestamp());
	if ( result !=LE_OK)
		LE_ERROR("Error in recording accelerometer X");
	result = le_avdata_RecordFloat(recordRef,Accel_Y,AcceY,GetCurrentTimestamp());
	if ( result !=LE_OK)
		LE_ERROR("Error in recording accelerometer Y");
	result = le_avdata_RecordFloat(recordRef,Accel_Z,AcceZ,GetCurrentTimestamp());
	if ( result !=LE_OK)
		LE_ERROR("Error in recording accelerometer Z");
	result = le_avdata_RecordFloat(recordRef,Gyro_X,GyroX,GetCurrentTimestamp());
	if ( result !=LE_OK)
		LE_ERROR("Error in recording Gyro X");
	result = le_avdata_RecordFloat(recordRef,Gyro_Y,GyroY,GetCurrentTimestamp());
	if ( result !=LE_OK)
		LE_ERROR("Error in recording Gyro Y");
	result = le_avdata_RecordFloat(recordRef,Gyro_Z,GyroZ,GetCurrentTimestamp());
	if ( result !=LE_OK)
		LE_ERROR("Error in recording Gyro Z");
	result = le_avdata_PushRecord(recordRef,PushCallbackHandler,NULL);

}

//-------------------------------------------------------------------------------------------------
/**
 * Status handler for avcService updates
 */
//-------------------------------------------------------------------------------------------------
static void avcStatusHandler
(
    le_avdata_SessionState_t updateStatus,
    void* contextPtr
)
{
    switch (updateStatus)
    {
        case LE_AVDATA_SESSION_STARTED:
            LE_INFO("Legato session started successfully");
            break;
        case LE_AVDATA_SESSION_STOPPED:
            LE_INFO("Legato session stopped");
            break;
    }
}

static void timerExpiredHandler(le_timer_Ref_t  timerRef)
{
    sig_appTermination_cbh(0);

    LE_INFO("Legato AssetDataApp Ended");

    //Quit the app
    exit(EXIT_SUCCESS);
}


COMPONENT_INIT
{
	LE_INFO("Start Legato AssetDataApp");
    le_sig_Block(SIGTERM);
    le_sig_SetEventHandler(SIGTERM, sig_appTermination_cbh);
     le_posCtrl_Request();

    //Start AVC Session
    //Register AVC handler
    avcEventHandlerRef = le_avdata_AddSessionStateHandler(avcStatusHandler, NULL);
    //Request AVC session. Note: AVC handler must be registered prior starting a session
    le_avdata_RequestSessionObjRef_t sessionRequestRef = le_avdata_RequestSession();
    if (NULL == sessionRequestRef)
    {
    	LE_ERROR("AirVantage Connection Controller does not start.");
    }else{
    	sessionRef=sessionRequestRef;
    	LE_INFO("AirVantage Connection Controller started.");
    }
    // [StartAVCSession]
    // [CreateTimer]
    LE_INFO("Started LWM2M session with AirVantage");
    sessionTimer = le_timer_Create("AssetDataAppSessionTimer");
    le_clk_Time_t avcInterval = {APP_RUNNING_DURATION_SEC, 0};
    le_timer_SetInterval(sessionTimer, avcInterval);
    le_timer_SetRepeat(sessionTimer, 1);
    le_timer_SetHandler(sessionTimer, timerExpiredHandler);
    le_timer_Start(sessionTimer);
    // [CreateTimer]
    // [CreateResources]
    LE_INFO("Create instances AssetData ");
    le_result_t resultCreateLatitude;
    resultCreateLatitude = le_avdata_CreateResource(LATITUDE,LE_AVDATA_ACCESS_VARIABLE);
    if (LE_FAULT == resultCreateLatitude)
    {
        LE_ERROR("Error in creating LATITUDE");
    }
    le_result_t resultCreateLongtitude;
    resultCreateLongtitude = le_avdata_CreateResource(LONGTITUDE,LE_AVDATA_ACCESS_VARIABLE);
    if (LE_FAULT == resultCreateLongtitude)
    {
        LE_ERROR("Error in creating LONGTITUDE");
    }
    le_result_t resultCreateAccuracy;
    resultCreateAccuracy = le_avdata_CreateResource(ACCURACY,LE_AVDATA_ACCESS_VARIABLE);
    if (LE_FAULT == resultCreateAccuracy)
    {
        LE_ERROR("Error in creating Accuracy");
    }
    le_result_t resultCreateAccelX;
    resultCreateAccelX = le_avdata_CreateResource(Accel_X,LE_AVDATA_ACCESS_VARIABLE);
    if (LE_FAULT == resultCreateAccelX)
    {
        LE_ERROR("Error in creating AccelX");
    }
    le_result_t resultCreateAccelY;
    resultCreateAccelY = le_avdata_CreateResource(Accel_Y,LE_AVDATA_ACCESS_VARIABLE);
    if (LE_FAULT == resultCreateAccelY)
    {
        LE_ERROR("Error in creating AccelY");
    }
    le_result_t resultCreateAccelZ;
    resultCreateAccelZ = le_avdata_CreateResource(Accel_Z,LE_AVDATA_ACCESS_VARIABLE);
    if (LE_FAULT == resultCreateAccelZ)
    {
        LE_ERROR("Error in creating AccelZ");
    }
    le_result_t resultCreateGyroX;
    resultCreateGyroX = le_avdata_CreateResource(Gyro_X,LE_AVDATA_ACCESS_VARIABLE);
    if (LE_FAULT == resultCreateGyroX)
    {
        LE_ERROR("Error in creating GyroX");
    }
    le_result_t resultCreateGyroY;
    resultCreateGyroY = le_avdata_CreateResource(Gyro_Y,LE_AVDATA_ACCESS_VARIABLE);
    if (LE_FAULT == resultCreateGyroY)
    {
        LE_ERROR("Error in creating GyroY");
    }
    le_result_t resultCreateGyroZ;
    resultCreateGyroZ = le_avdata_CreateResource(Gyro_Z,LE_AVDATA_ACCESS_VARIABLE);
    if (LE_FAULT == resultCreateGyroZ)
    {
        LE_ERROR("Error in creating GyroZ");
    }
    // [CreateResources]
    // [AssignValues]
    getLocation();
    le_result_t resultSetLatitude = le_avdata_SetFloat(LATITUDE, lat);
    if (LE_FAULT == resultSetLatitude)
    {
        LE_ERROR("Error in setting LATITUDE");
    }
    le_result_t resultSetLongtitude = le_avdata_SetFloat(LONGTITUDE, longt);
    if (LE_FAULT == resultSetLongtitude)
    {
        LE_ERROR("Error in setting LONGTITUDE");
    }
    le_result_t resultSetAccuracy = le_avdata_SetInt(ACCURACY, hAccu);
    if (LE_FAULT == resultSetAccuracy)
    {
        LE_ERROR("Error in setting ACCURACY");
    }
    // [AssignValues]
    // [SetTimer]
    //Set timer to update temperature on a regular basis
    locUpdateTimerRef = le_timer_Create("locUpdateTimer");     //create timer
    le_clk_Time_t locUpdateInterval = { 10, 0 };            //update temperature every 20 seconds
    le_timer_SetInterval(locUpdateTimerRef, locUpdateInterval);
    le_timer_SetRepeat(locUpdateTimerRef, 0);                   //set repeat to always
    //set callback function to handle timer expiration event
    le_timer_SetHandler(locUpdateTimerRef, getLocation);
    //start timer
    le_timer_Start(locUpdateTimerRef);
    // [SetTimer]
    //Set timer to update accelerometer on a regular basis
    accelUpdateTimerRef = le_timer_Create("accelUpdateTimer");     //create timer
    le_clk_Time_t accelUpdateInterval = { 10, 0 };            //update temperature every 20 seconds
    le_timer_SetInterval(accelUpdateTimerRef, accelUpdateInterval);
    le_timer_SetRepeat(accelUpdateTimerRef, 0);                   //set repeat to always
    //set callback function to handle timer expiration event
    le_timer_SetHandler(accelUpdateTimerRef, getAccelerometer);
    //start timer
    le_timer_Start(accelUpdateTimerRef);
    // [SetTimer]
    recordRef =le_avdata_CreateRecord();
    recordUpdateTimerRef = le_timer_Create("recUpdateTimer");
    le_timer_SetMsInterval(recordUpdateTimerRef, 10000 );
    le_timer_SetRepeat(recordUpdateTimerRef, 0);                   //set repeat to always
    //set callback function to handle timer expiration event
    le_timer_SetHandler(recordUpdateTimerRef, recordInfo);
    le_timer_Start(recordUpdateTimerRef);



    /* [PushTimer]
    //Set timer to update on server on a regular basis
    serverUpdateTimerRef = le_timer_Create("serverUpdateTimer");     //create timer
    le_clk_Time_t serverUpdateInterval = { 10, 0 };            //update server every 10 seconds
    le_timer_SetInterval(serverUpdateTimerRef, serverUpdateInterval);
    le_timer_SetRepeat(serverUpdateTimerRef, 0);                   //set repeat to always
    //set callback function to handle timer expiration event
    le_timer_SetHandler(serverUpdateTimerRef, PushResources);
    //start timer
    le_timer_Start(serverUpdateTimerRef);
    // [PushTimer]*/
    le_result_t resultDeviceConfig;
    //
    //le_posCtrl_Release(posCtrlRef);

        char path[LE_AVDATA_PATH_NAME_BYTES];
        int i;

        // Create device config resources
        for (i = 0; i < MAX_RESOURCES; i++)
        {
            snprintf(path, sizeof(path), "%s/%d", DEVICE_CONFIG_SET_RES, i);

            LE_INFO("Creating asset %s", path);

            resultDeviceConfig = le_avdata_CreateResource(path, LE_AVDATA_ACCESS_SETTING);
            if (LE_FAULT == resultDeviceConfig)
            {
               LE_ERROR("Error in creating DEVICE_CONFIG_SET_RES");
            }
        }

        // Add resource handler at the device config (root)
        LE_INFO("Add resource event handler");
        le_avdata_AddResourceEventHandler(DEVICE_CONFIG_SET_RES, DeviceConfigHandler, NULL);
}
