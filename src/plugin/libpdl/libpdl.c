/*=============================================================================
 Copyright (C) 2010 WebOS Internals <http://www.webos-internals.org/>
 Copyright (C) 2010 Ryan Hope <rmh3093@gmail.com>
 Copyright (C) 2010 mdklein <???>

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation; either version 2
 of the License, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 =============================================================================*/

#include <PDL.h>

PDL_Err PDL_ServiceCall(const char *uri, const char *payload){return 0;}
PDL_Err PDL_ServiceCallWithCallback(const char *uri, const char *payload, PDL_ServiceCallbackFunc callback, void *user, PDL_bool removeAfterResponse){return 0;}
PDL_Err PDL_UnregisterServiceCallback(PDL_ServiceCallbackFunc callback){return 0;}
PDL_Err PDL_RegisterFunction(const char *functionName, const char *schema, PDL_ProviderCallbackFunc function){return 0;}
PDL_Err PDL_ServiceRegistrationComplete(const char *suiteName){return 0;}
PDL_bool        PDL_ParamExists(PDL_ServiceParameters *parms, const char *name){return 0;}
void            PDL_GetParamString(PDL_ServiceParameters *parms, const char *name, char *buffer, int bufferLen){return;}
int         PDL_GetParamInt(PDL_ServiceParameters *parms, const char *name){return 0;}
double      PDL_GetParamDouble(PDL_ServiceParameters *parms, const char *name){return 0;}
PDL_Err PDL_ProviderReply(PDL_ServiceParameters *parms, const char *reply){return 0;}
PDL_Err PDL_RegisterJSHandler(const char *functionName, PDL_JSHandlerFunc function){return 0;}
PDL_Err PDL_JSRegistrationComplete(){return 0;}
int PDL_GetNumJSParams(PDL_JSParameters *parms){return 0;}
const char *PDL_GetJSParamString(PDL_JSParameters *parms, int paramNum){return 0;}
int PDL_GetJSParamInt(PDL_JSParameters *parms, int paramNum){return 0;}
double PDL_GetJSParamDouble(PDL_JSParameters *parms, int paramNum){return 0;}
PDL_Err PDL_JSReply(PDL_JSParameters *parms, const char *reply){return 0;}
PDL_Err PDL_JSException(PDL_JSParameters *parms, const char *reply){return 0;}
PDL_Err PDLNet_Get_Info(const char * _interface, NETinfo * interfaceInfo){return 0;}
PDL_Err PDL_CheckLicense(void){return 0;}
PDL_Err PDL_ScreenTimeoutEnable(PDL_bool Enable){return 0;}
PDL_Err PDL_Init(unsigned int flags){return 0;}
PDL_Err PDL_LaunchEmail(const char* Subject, const char* Body){return 0;}
PDL_Err PDL_LaunchBrowser(const char* Url){return 0;}
char*   PDL_GetKeyName(PDL_key Key){return 0;}
PDL_Err PDL_GetLanguage(char *buffer, int bufferLen){return 0;}
PDL_Err PDL_GetNetInfo(const char *interfaceName, PDL_NetInfo * interfaceInfo){return 0;}
PDL_Err PDL_SetOrientation(PDL_Orientation orientation){return 0;}
PDL_Err PDL_BannerMessagesEnable(PDL_bool Enable){return 0;}
PDL_Err PDL_GesturesEnable(PDL_bool Enable){return 0;}
PDL_Err PDL_CustomPauseUiEnable(PDL_bool Enable){return 0;}
PDL_Err PDL_NotifyMusicPlaying(PDL_bool MusicPlaying){return 0;}
PDL_Err PDL_SetFirewallPortStatus(int port, PDL_bool Open){return 0;}
PDL_Err PDL_GetUniqueID(char *buffer, int bufferLen){return 0;}
PDL_Err PDL_GetDeviceName(char *buffer, int bufferLen){return 0;}
PDL_Err PDL_GetCallingPath(char *buffer, int bufferLen){return 0;}
PDL_Err PDL_GetDataFilePath(const char *dataFileName, char *buffer, int bufferLen){return 0;}
PDL_Err PDL_GetAppinfoValue(const char *name, char *buffer, int bufferLen){return 0;};
PDL_Err PDL_EnableLocationTracking(PDL_bool activate){return 0;}
PDL_Err PDL_GetLocation(PDL_Location *location){return 0;}
PDL_bool PDL_IsPlugin(void){return 0;}
void    PDL_Quit(){return;}
PDL_Err PDL_CallJS(const char *functionName, const char **params, int numParams){return 0;}
const char *PDL_GetHardware(void){return 0;}
int PDL_GetHardwareID(void){return 0;}
int PDL_GetPDKVersion(void){return 0;}
PDL_Err PDL_GetRegionCountryCode(char *buffer, int bufferLen){return 0;}
PDL_Err PDL_GetRegionCountryName(char *buffer, int bufferLen){return 0;}
PDL_Err PDL_GetScreenMetrics(PDL_ScreenMetrics *outMetrics){return 0;}
PDL_Err PDL_GetOSVersion(PDL_OSVersion *version){return 0;}
const char *PDL_GetParamJson(PDL_ServiceParameters *parms){return 0;}
int PDL_isAppLicensedForDevice(const char *appid){return 0;}
PDL_Err PDL_Minimize(void){return 0;}
PDL_Err PDL_SetTouchAggression(PDL_TouchAggression aggression){return 0;}
PDL_Err PDL_Vibrate(int periodMS, int durationMS){return 0;}

PDL_Err PDL_EnableCompass(PDL_bool activate){return 0;}
PDL_Err PDL_GetCompass(PDL_Compass *compass){return 0;}
PDL_Err PDL_SetKeyboardState(PDL_bool bVisible){return 0;}

int SDL_WaitEvent(SDL_Event *event) {return 0;}
int SDL_Init(Uint32 flags) {return 0;}
void SDL_Quit(void) {}
