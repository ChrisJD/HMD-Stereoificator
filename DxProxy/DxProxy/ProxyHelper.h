/********************************************************************
Vireio Perception: Open-Source Stereoscopic 3D Driver
Copyright (C) 2012 Andres Hernandez

Stereoificator
Modifications Copyright (C) 2013 Chris Drain


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

#ifndef PROXYHELPER_H_INCLUDED
#define PROXYHELPER_H_INCLUDED

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <windows.h>
#include "pugixml.hpp"
#include "Stereoificator.h"

class ProxyHelper
{
public:
	ProxyHelper();

	struct ProxyConfig
	{
		ProxyConfig() :
			gameName("Invalid game name"),
			game_type(10),
			stereo_mode(26),
			tracker_mode(0),
			separationAdjustment(0.0f),
			aspect_multiplier(1.0f),
			swap_eyes(true),
			yaw_multiplier(25.0f),
			pitch_multiplier(25.0f),
			roll_multiplier(1.0f),
			worldScaleFactor(1.0f),
			rollEnabled(false),
			shaderRulePath(""),
			horizontalGameFov(110.0f),
			ipd(IPD_DEFAULT),
			hudScale(1.0f),
			hudDistance(1.0f),
			debugMode(0), // 0 == off
			forceAdapterNumber(-1), // -1 == disabled
			duplicationRules(0) 
		{}


		std::string gameName;
		int game_type;
		int stereo_mode;
		int tracker_mode;
		float separationAdjustment;
		float aspect_multiplier;
		bool swap_eyes;
		float yaw_multiplier;
		float pitch_multiplier;
		float roll_multiplier;

		float worldScaleFactor; // mm * worldScaleFactor = mm in game units
		bool rollEnabled;
		std::string shaderRulePath; // full path of shader rules for this game
		int duplicationRules;
		float horizontalGameFov; // The horizontal FoV in degrees that the game is set to render

		float ipd; // in mm

		float hudScale;
		float hudDistance;

		int debugMode;
		int forceAdapterNumber;
		int hudDistanceMode;
	};


	bool LoadConfig(ProxyConfig& config);
	char* GetBaseDir();
	char* GetTargetExe();
	void GetPath(char* newFolder, char* path);
	bool baseDirLoaded; 
	char* baseDir;
	char* targetExe;
	bool HasProfile(char* name);
	bool SaveConfig(ProxyConfig& config);
	bool SaveConfig(int mode = -1, float aspect = -1.0f);
	bool SaveConfig2(int mode = -1);
	
	bool GetConfig(int& mode, int& mode2);
	//bool SaveProfile(float sepAdjustment = 0.0f, bool swap_eyes = false, float yaw = 25.0f, float pitch = 25.0f, float roll = 1.0f, float worldScale = 1.0f);

	// Checks if users.xml exists. If it doesn't the file is created and populated. Returns true if file exists or was created, false if there was a problem
	bool CheckUsersXml();
	

	
	
	bool LoadUserConfig(ProxyConfig& config, bool forceDefault = false);

private:
	// Loads user config from specified userSettings node. Uses defaultUserSettingsNode as a fallback if userSettings doesn't conain a value for an attribute. (and hardcoded fallback if neither are available)
	void UserConfigFromNode(ProxyConfig& cfg, pugi::xml_node& userSettingsNode, pugi::xml_node& defaultUserSettingsNode);

	// Loads attribute from specified node. Uses defaultNode as a fallback if specificNode doesn't contain the specified attribute.
	pugi::xml_attribute GetAttributeWithFallback(pugi::xml_node specificNode, pugi::xml_node defaultNode, pugi::char_t* attributeName);

	// Loads user and game nodes from a user config file for the specified game name.
	// Returns true if document was parsed successfully. This value does not reflect whether the user or game nodes are valid.
	bool GetUserAndGameNodes(pugi::char_t* userConfigPath, std::string gameName, pugi::xml_document* document, pugi::xml_node& user, pugi::xml_node& game);

	bool SaveUserConfig(ProxyConfig& cfg);

};

#endif