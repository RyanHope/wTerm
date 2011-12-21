/*=============================================================================
 Copyright (C) 2010 WebOS Internals <http://www.webos-internals.org/>
 Copyright (C) 2010-2011 Ryan Hope <rmh3093@gmail.com>
 Copyright (C) 2010 mdklein <???>

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation; either version 2
 of the License, or (fat your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 =============================================================================*/

#ifndef LIBPDL_H_
#define LIBPDL_H_

#include <SDL.h>

typedef SDLKey      PDL_key;
#define PDLKey      PDL_key;

typedef enum {
	PDL_FALSE = 0,
	PDL_TRUE  = 1
} PDL_bool;

#define CALLBACK_PAYLOAD_MAX 1024

#define HARDWARE_UNKNOWN	-1
#define HARDWARE_PRE		101
#define HARDWARE_PRE_PLUS	102
#define HARDWARE_PIXI		201
#define HARDWARE_VEER		301
#define HARDWARE_PRE_2		401
#define HARDWARE_PRE_3		501
#define HARDWARE_TOUCHPAD	601

#define PDL_GPS_UPDATE 0xE100  

#define PDL_GPS_FAILURE 0xE101 

#define PDL_VERSION_STR_SIZE 256

typedef enum
{
    PDL_NOERROR = 0,
    PDL_EMEMORY,
    PDL_ECONNECTION,
    PDL_INVALIDINPUT,
    PDL_EOTHER,
    PDL_UNINIT,
    PDL_NOTALLOWED,
    PDL_LICENSEFAILURE,
    PDL_STRINGTOOSMALL,
} PDL_Err;

typedef enum
{
        PDL_ORIENTATION_0 = 0,
        PDL_ORIENTATION_90,
        PDL_ORIENTATION_180,
        PDL_ORIENTATION_270,
} PDL_Orientation;

typedef enum
{
        PDL_AGGRESSION_LESSTOUCHES = 0,
        PDL_AGGRESSION_MORETOUCHES
} PDL_TouchAggression;

enum
{
    PDLK_GESTURE_AREA       = 231,
    PDLK_GESTURE_BACK       = 27,
    PDLK_GESTURE_FORWARD    = 229,
};

struct _PDL_NetInfo
{
        Uint32 ipaddress;
        Uint32 netmask;
        Uint32 broadcast;
};
typedef struct _PDL_NetInfo PDL_NetInfo;

#define _NETinfo        _PDL_NetInfo
#define NETinfo         PDL_NetInfo

typedef struct
{
        double latitude;
        double longitude;
        double altitude;
        double horizontalAccuracy;
        double verticalAccuracy;
        double heading;
        double velocity;
} PDL_Location;

typedef struct
{
	int confidence;
	double magneticBearing;
	double trueBearing;
} PDL_Compass;

typedef struct
{
	int horizontalPixels;
	int verticalPixels;
	int horizontalDPI;
	int verticalDPI;
	double aspectRatio;
} PDL_ScreenMetrics;

typedef struct
{
	int majorVersion;
	int minorVersion;
	int revision;
	char* versionStr;
} PDL_OSVersion;

typedef struct PDL_ServiceParameters PDL_ServiceParameters;
typedef struct PDL_JSParameters PDL_JSParameters;
typedef PDL_bool (*PDL_ServiceCallbackFunc) (PDL_ServiceParameters *params, void *user);
typedef PDL_bool (*PDL_ProviderCallbackFunc) (PDL_ServiceParameters *params);
typedef PDL_bool (*PDL_JSHandlerFunc) (PDL_JSParameters *params);

PDL_Err PDL_ServiceCall(const char *uri, const char *payload);
PDL_Err PDL_ServiceCallWithCallback(const char *uri, const char *payload, PDL_ServiceCallbackFunc callback, void *user, PDL_bool removeAfterResponse);
PDL_Err PDL_UnregisterServiceCallback(PDL_ServiceCallbackFunc callback);
PDL_Err PDL_RegisterFunction(const char *functionName, const char *schema, PDL_ProviderCallbackFunc function);
PDL_Err PDL_ServiceRegistrationComplete(const char *suiteName);
PDL_bool        PDL_ParamExists(PDL_ServiceParameters *parms, const char *name);
void            PDL_GetParamString(PDL_ServiceParameters *parms, const char *name, char *buffer, int bufferLen);
int         PDL_GetParamInt(PDL_ServiceParameters *parms, const char *name);
double      PDL_GetParamDouble(PDL_ServiceParameters *parms, const char *name);
PDL_Err PDL_ProviderReply(PDL_ServiceParameters *parms, const char *reply);

PDL_Err PDL_RegisterJSHandler(const char *functionName, PDL_JSHandlerFunc function);

PDL_Err PDL_JSRegistrationComplete();
int PDL_GetNumJSParams(PDL_JSParameters *parms);
const char *PDL_GetJSParamString(PDL_JSParameters *parms, int paramNum);
int PDL_GetJSParamInt(PDL_JSParameters *parms, int paramNum);
double PDL_GetJSParamDouble(PDL_JSParameters *parms, int paramNum);
PDL_Err PDL_JSReply(PDL_JSParameters *parms, const char *reply);
PDL_Err PDL_JSException(PDL_JSParameters *parms, const char *reply);

PDL_Err PDLNet_Get_Info(const char * _interface, NETinfo * interfaceInfo);
PDL_Err PDL_CheckLicense(void);
PDL_Err PDL_ScreenTimeoutEnable(PDL_bool Enable);

PDL_Err PDL_Init(unsigned int flags);

PDL_Err PDL_LaunchEmail(const char* Subject, const char* Body);
PDL_Err PDL_LaunchBrowser(const char* Url);
char*   PDL_GetKeyName(PDL_key Key);
PDL_Err PDL_GetLanguage(char *buffer, int bufferLen);
PDL_Err PDL_GetNetInfo(const char *interfaceName, PDL_NetInfo * interfaceInfo);
PDL_Err PDL_SetOrientation(PDL_Orientation orientation);
PDL_Err PDL_BannerMessagesEnable(PDL_bool Enable);
PDL_Err PDL_GesturesEnable(PDL_bool Enable);
PDL_Err PDL_CustomPauseUiEnable(PDL_bool Enable);
PDL_Err PDL_NotifyMusicPlaying(PDL_bool MusicPlaying);
PDL_Err PDL_SetFirewallPortStatus(int port, PDL_bool Open);
PDL_Err PDL_GetUniqueID(char *buffer, int bufferLen);
PDL_Err PDL_GetDeviceName(char *buffer, int bufferLen);
PDL_Err PDL_GetCallingPath(char *buffer, int bufferLen);
PDL_Err PDL_GetDataFilePath(const char *dataFileName, char *buffer, int bufferLen);
PDL_Err PDL_GetAppinfoValue(const char *name, char *buffer, int bufferLen);
PDL_Err PDL_EnableLocationTracking(PDL_bool activate);
PDL_Err PDL_GetLocation(PDL_Location *location);
PDL_bool PDL_IsPlugin(void);
void    PDL_Quit();

PDL_Err PDL_CallJS(const char *functionName, const char **params, int numParams);
const char *PDL_GetHardware(void);
int PDL_GetHardwareID(void);
int PDL_GetPDKVersion(void);
PDL_Err PDL_GetRegionCountryCode(char *buffer, int bufferLen);
PDL_Err PDL_GetRegionCountryName(char *buffer, int bufferLen);
PDL_Err PDL_GetScreenMetrics(PDL_ScreenMetrics *outMetrics);
PDL_Err PDL_GetOSVersion(PDL_OSVersion *version);
const char *PDL_GetParamJson(PDL_ServiceParameters *parms);
int PDL_isAppLicensedForDevice(const char *appid);
PDL_Err PDL_Minimize(void);
PDL_Err PDL_SetTouchAggression(PDL_TouchAggression aggression);
PDL_Err PDL_Vibrate(int periodMS, int durationMS);

PDL_Err PDL_EnableCompass(PDL_bool activate);
PDL_Err PDL_GetCompass(PDL_Compass *compass);
PDL_Err PDL_SetKeyboardState(PDL_bool bVisible);

#endif /* LIBPDL_H_ */
