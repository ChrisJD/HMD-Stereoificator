/********************************************************************
Vireio Perception: Open-Source Stereoscopic 3D Driver
Copyright (C) 2012 Andres Hernandez

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
********************************************************************/

#include "ProxyHelper.h"


using namespace pugi;

HRESULT RegGetString(HKEY hKey, LPCTSTR szValueName, LPTSTR * lpszResult);

ProxyHelper::ProxyHelper()
	: baseDirLoaded(false)
{
}

char* ProxyHelper::GetBaseDir()
{
	if (baseDirLoaded == true){
		OutputDebugString("PxHelp: Already have base value.\n");
		return baseDir;
	}

	HKEY hKey;
	LPCTSTR sk = TEXT("SOFTWARE\\Stereoificator\\Stereoificator");

	LONG openRes = RegOpenKeyEx(HKEY_CURRENT_USER, sk, 0, KEY_QUERY_VALUE , &hKey);

	if (openRes==ERROR_SUCCESS) 
	{
		OutputDebugString("PxHelp: Success opening key.\n");
	} 
	else 
	{
		OutputDebugString("PxHelp: Error opening key.\n");
		return "";
	}

	HRESULT hr = RegGetString(hKey, TEXT("BasePath"), &baseDir);
	if (FAILED(hr)) 
	{
		OutputDebugString("PxHelp: Error with GetString.\n");
		return "";
	} 
	else 
	{
		OutputDebugString("PxHelp: Success with GetString.\n");
		//strcpy_s(baseDir, sizeof(szVal), szVal);
		OutputDebugString(baseDir);
		OutputDebugString("\n");
		baseDirLoaded = true;
	}

	return baseDir;
}

char* ProxyHelper::GetTargetExe()
{
	HKEY hKey;
	LPCTSTR sk = TEXT("SOFTWARE\\Stereoificator\\Stereoificator");

	LONG openRes = RegOpenKeyEx(HKEY_CURRENT_USER, sk, 0, KEY_QUERY_VALUE , &hKey);

	if (openRes==ERROR_SUCCESS) 
	{
		OutputDebugString("PxHelp TE: Success opening key.\n");
	} 
	else
	{
		OutputDebugString("PxHelp TE: Error opening key.\n");
		return "";
	}
	 
	HRESULT hr = RegGetString(hKey, TEXT("TargetExe"), &targetExe);
	if (FAILED(hr)) 
	{
		OutputDebugString("PxHelp TE: Error with GetString.\n");
		return "";
	} 
	else 
	{
		OutputDebugString("PxHelp TE: Success with GetString.\n");
		OutputDebugString(targetExe);
		OutputDebugString("\n");
	}

	return targetExe;
}

void ProxyHelper::GetPath(char* newFolder, char* path)
{
	strcpy_s(newFolder, 512, GetBaseDir());
	strcat_s(newFolder, 512, path);
}

bool FileExists(char* path)
{
  DWORD attrib = GetFileAttributes(path);

  return ((attrib != INVALID_FILE_ATTRIBUTES) && !(attrib & FILE_ATTRIBUTE_DIRECTORY));
}


bool ProxyHelper::LoadConfig(ProxyConfig& config)
{
	bool fileFound = false;
	bool profileFound = false;
	bool userCfgFound = false;

	// set defaults
	config.game_type = 0;
	config.stereo_mode = 0;
	config.tracker_mode = 0;
	config.separationAdjustment = 0.0f;
	config.swap_eyes = false;
	config.aspect_multiplier = 1.0f;

	// load the base dir for the app
	GetBaseDir();
	OutputDebugString("Got base dir as: ");
	OutputDebugString(baseDir);
	OutputDebugString("\n");

	// get global config
	char configPath[512];
	GetPath(configPath, "cfg\\config.xml");
	OutputDebugString(configPath);
	OutputDebugString("\n");

	xml_document docConfig;
	xml_parse_result resultConfig = docConfig.load_file(configPath);

	if(resultConfig.status == status_ok)
	{
		xml_node xml_config = docConfig.child("config");

		config.stereo_mode = xml_config.attribute("stereo_mode").as_int();
		config.aspect_multiplier = xml_config.attribute("aspect_multiplier").as_float();
		config.tracker_mode = xml_config.attribute("tracker_mode").as_int();

		fileFound = true;
	}

	
	// get the target exe
	GetTargetExe();
	OutputDebugString("Got target exe as: ");
	OutputDebugString(targetExe);
	OutputDebugString("\n");

	// get the profile
	char profilePath[512];
	GetPath(profilePath, "cfg\\profiles.xml");
	OutputDebugString(profilePath);
	OutputDebugString("\n");

	xml_document docProfiles;
	xml_parse_result resultProfiles = docProfiles.load_file(profilePath);
	xml_node profile;
	xml_node gameProfile;

	if(resultProfiles.status == status_ok)
	{
		xml_node xml_profiles = docProfiles.child("profiles");

		for (xml_node profile = xml_profiles.child("profile"); profile; profile = profile.next_sibling("profile"))
		{
			if(strcmp(targetExe, profile.attribute("game_exe").value()) == 0)
			{
				OutputDebugString("Load the specific profile!!!\n");
				gameProfile = profile;
				profileFound = true;
				break;
			}
		}
	}

	if(resultProfiles.status == status_ok && profileFound && gameProfile)
	{
		OutputDebugString("Set the config to profile!!!\n");
		config.game_type = gameProfile.attribute("game_type").as_int(10);
		config.gameName = gameProfile.attribute("game_name").as_string();

		OutputDebugString(config.gameName.c_str());
		OutputDebugString("\n");

		char buf[32];
		LPCSTR psz = NULL;
		wsprintf(buf,"gameType: %d", config.game_type);
		psz = buf;
		OutputDebugString(psz);
		OutputDebugString("\n");

		config.rollEnabled = gameProfile.attribute("rollEnabled").as_bool(false);
		config.worldScaleFactor = gameProfile.attribute("worldScaleFactor").as_float(1.0f);
		
		// get file name
		std::string shaderRulesFileName = gameProfile.attribute("shaderModRules").as_string("");

		if (!shaderRulesFileName.empty()) {
			std::stringstream sstm;
			sstm << GetBaseDir() << "cfg\\shader_rules\\" << shaderRulesFileName;
			config.shaderRulePath = sstm.str();
		}
		else {
			config.shaderRulePath = "";
		}


		userCfgFound = LoadUserConfig(config);
	}

	if (!(fileFound && profileFound && userCfgFound)) {
		OutputDebugString("[ERROR] Could not load part/all of proxy config");
	}

	
	return fileFound && profileFound && userCfgFound;
}

bool ProxyHelper::HasProfile(char* name)
{
	// get the profile
	bool profileFound = false;
	char profilePath[512];
	GetPath(profilePath, "cfg\\profiles.xml");

	xml_document docProfiles;
	xml_parse_result resultProfiles = docProfiles.load_file(profilePath);
	xml_node profile;

	if(resultProfiles.status == status_ok)
	{
		xml_node xml_profiles = docProfiles.child("profiles");

		for (xml_node profile = xml_profiles.child("profile"); profile; profile = profile.next_sibling("profile"))
		{
			if(strcmp(name, profile.attribute("game_exe").value()) == 0)
			{
				OutputDebugString("Found a profile!!!\n");
				profileFound = true;
				break;
			}
		}
	}

	return profileFound;
}


HRESULT RegGetString(HKEY hKey, LPCTSTR szValueName, LPTSTR * lpszResult) {
 
    // Given a HKEY and value name returns a string from the registry.
    // Upon successful return the string should be freed using free()
    // eg. RegGetString(hKey, TEXT("my value"), &szString);
 
    DWORD dwType=0, dwDataSize=0, dwBufSize=0;
    LONG lResult;
 
    // Incase we fail set the return string to null...
    if (lpszResult != NULL) *lpszResult = NULL;
 
    // Check input parameters...
    if (hKey == NULL || lpszResult == NULL) return E_INVALIDARG;
 
    // Get the length of the string in bytes (placed in dwDataSize)...
    lResult = RegQueryValueEx(hKey, szValueName, 0, &dwType, NULL, &dwDataSize );
 
    // Check result and make sure the registry value is a string(REG_SZ)...
    if (lResult != ERROR_SUCCESS) return HRESULT_FROM_WIN32(lResult);
    else if (dwType != REG_SZ)    return DISP_E_TYPEMISMATCH;
 
    // Allocate memory for string - We add space for a null terminating character...
    dwBufSize = dwDataSize + (1 * sizeof(TCHAR));
    *lpszResult = (LPTSTR)malloc(dwBufSize);
 
    if (*lpszResult == NULL) return E_OUTOFMEMORY;
 
    // Now get the actual string from the registry...
    lResult = RegQueryValueEx(hKey, szValueName, 0, &dwType, (LPBYTE) *lpszResult, &dwDataSize );
 
    // Check result and type again.
    // If we fail here we must free the memory we allocated...
    if (lResult != ERROR_SUCCESS) { free(*lpszResult); return HRESULT_FROM_WIN32(lResult); }
    else if (dwType != REG_SZ)    { free(*lpszResult); return DISP_E_TYPEMISMATCH; }
 
    // We are not guaranteed a null terminated string from RegQueryValueEx.
    // Explicitly null terminate the returned string...
    (*lpszResult)[(dwBufSize / sizeof(TCHAR)) - 1] = TEXT('\0');
 
    return NOERROR;
}


bool ProxyHelper::SaveConfig(ProxyConfig& cfg)
{
	return SaveUserConfig(cfg);
}


bool ProxyHelper::SaveConfig(int mode, float aspect)
{
	// load the base dir for the app
	GetBaseDir();
	OutputDebugString(baseDir);
	OutputDebugString("\n");

	// get global config
	char configPath[512];
	GetPath(configPath, "cfg\\config.xml");

	xml_document docConfig;
	xml_parse_result resultConfig = docConfig.load_file(configPath);

	if(resultConfig.status == status_ok)
	{
		xml_node xml_config = docConfig.child("config");

		if(mode >= 0)
			xml_config.attribute("stereo_mode") = mode;

		if(aspect >= 0.0f)
			xml_config.attribute("aspect_multiplier") = aspect;

		docConfig.save_file(configPath);

		return true;
	}
	
	return false;
}

bool ProxyHelper::SaveConfig2(int mode)
{
	// load the base dir for the app
	GetBaseDir();
	OutputDebugString(baseDir);
	OutputDebugString("\n");

	// get global config
	char configPath[512];
	GetPath(configPath, "cfg\\config.xml");

	xml_document docConfig;
	xml_parse_result resultConfig = docConfig.load_file(configPath);

	if(resultConfig.status == status_ok)
	{
		xml_node xml_config = docConfig.child("config");

		if(mode >= 0)
			xml_config.attribute("tracker_mode") = mode;

		docConfig.save_file(configPath);

		return true;
	}
	
	return false;
}



bool ProxyHelper::LoadUserConfig(ProxyConfig& config)
{
	// get the user_profile
	bool userFound = false;
	bool settingsFound = false;
	char usersPath[512];

	if (CheckUsersXml()) {

		GetPath(usersPath, "cfg\\users.xml");
		OutputDebugString(usersPath);
		OutputDebugString("\n");
	}
	else {
		GetPath(usersPath, "cfg\\defaults.users.xml");
		OutputDebugString(usersPath);
		OutputDebugString("\n");
	}

	xml_document docUsers;
	xml_parse_result resultUsers = docUsers.load_file(usersPath);	

	if(resultUsers.status == status_ok)
	{
		xml_node user_profile = docUsers.child("users").child("user");

		config.ipd = user_profile.attribute("ipd").as_float(IPD_DEFAULT);
		vireio::clamp(&config.ipd, 1.0f, 100.0f);


		
		xml_node gameSettings = user_profile.child("gamesettings");
		

		for (xml_node settingsEntry = gameSettings.child("game"); settingsEntry; settingsEntry = settingsEntry.next_sibling("game"))
		{
			if( config.gameName.compare(settingsEntry.attribute("game_name").as_string()) == 0)
			{
				OutputDebugString("User specific profile settings found.\n");
				
				config.separationAdjustment = settingsEntry.attribute("separationAdjustment").as_float(0.0f);
				config.swap_eyes = settingsEntry.attribute("swap_eyes").as_bool(true);
				config.yaw_multiplier = settingsEntry.attribute("yaw_multiplier").as_float(25.0f);
				config.pitch_multiplier = settingsEntry.attribute("pitch_multiplier").as_float(25.0f);
				config.roll_multiplier = settingsEntry.attribute("roll_multiplier").as_float(1.0f);
				config.horizontalGameFov = settingsEntry.attribute("horizontalFoV").as_float(110.0f);

				config.hudScale = settingsEntry.attribute("hudScale").as_float(1.0f);
				config.hudDistance= settingsEntry.attribute("hudDistance").as_float(1.0f);
		
				if(config.yaw_multiplier <= 0.0f) config.yaw_multiplier = 25.0f;
				if(config.pitch_multiplier <= 0.0f) config.pitch_multiplier = 25.0f;
				if(config.roll_multiplier <= 0.0f) config.roll_multiplier = 1.0f;

				break;
			}
		}
		
		
	}

	if (!(userFound && settingsFound)) {
		OutputDebugString("Error loading user settings for game.");
	}

	return userFound && settingsFound;
}

bool ProxyHelper::SaveUserConfig(ProxyConfig& cfg)
{
	// get the profile
	bool profileSaved = false;
	char profilePath[512];
	GetPath(profilePath, "cfg\\users.xml");
	OutputDebugString(profilePath);
	OutputDebugString("\n");

	xml_document userProfiles;
	xml_parse_result userProfilesResult = userProfiles.load_file(profilePath);

	if(userProfilesResult.status == status_ok)
	{
		xml_node user_profile = userProfiles.child("users").child("user");

		if (user_profile.type() != node_null) {

			user_profile.attribute("ipd") = cfg.ipd;

			xml_node gameSettings = user_profile.child("gamesettings");
			for (xml_node settingsEntry = gameSettings.child("game"); settingsEntry; settingsEntry = settingsEntry.next_sibling("game"))
			{
				if( cfg.gameName.compare(settingsEntry.attribute("game_name").as_string()) == 0)
				{

					settingsEntry.attribute("separationAdjustment") = cfg.separationAdjustment;
					settingsEntry.attribute("swap_eyes") = cfg.swap_eyes;
					settingsEntry.attribute("yaw_multiplier") = cfg.yaw_multiplier;
					settingsEntry.attribute("pitch_multiplier") = cfg.pitch_multiplier;
					settingsEntry.attribute("roll_multiplier") = cfg.roll_multiplier;
					settingsEntry.attribute("horizontalFoV") = cfg.horizontalGameFov;

					settingsEntry.attribute("hudScale") = cfg.hudScale;
					settingsEntry.attribute("hudDistance")= cfg.hudDistance;

					OutputDebugString("Saving the user settings to users.xml!!!\n");
					userProfiles.save_file(profilePath);
					profileSaved = true;

					break;
				}
			}
		}
	}

	if (!profileSaved) {
		OutputDebugString("[ERROR] Setting NOT saved to users.xml");
	}

	return profileSaved;
}


bool ProxyHelper::GetConfig(int& mode, int& mode2)
{
	// load the base dir for the app
	GetBaseDir();
	OutputDebugString(baseDir);
	OutputDebugString("\n");

	// get global config
	char configPath[512];
	GetPath(configPath, "cfg\\config.xml");

	xml_document docConfig;
	xml_parse_result resultConfig = docConfig.load_file(configPath);

	if(resultConfig.status == status_ok)
	{
		xml_node xml_config = docConfig.child("config");

		mode = xml_config.attribute("stereo_mode").as_int();
		mode2 = xml_config.attribute("tracker_mode").as_int();

		return true;
	}
	
	return false;
}


bool ProxyHelper::CheckUsersXml()
{
	char usersPath[512];
	GetPath(usersPath, "cfg\\users.xml");
	bool usersXMLExists = FileExists(usersPath);

	if (usersXMLExists)
		return true;

	char defaultUsersPath[512];
	GetPath(defaultUsersPath, "cfg\\defaults.users.xml");
	if (!FileExists(defaultUsersPath)) {
		OutputDebugString("[ERROR] default.users.xml could not be found.");
		return false;
	}

	xml_document docDefaultUsers;
	xml_parse_result resultProfiles = docDefaultUsers.load_file(defaultUsersPath);

	// save the file as users.xml
	if(resultProfiles.status == status_ok)
	{
		docDefaultUsers.save_file(usersPath);

		return true;
	}

	return false;
}