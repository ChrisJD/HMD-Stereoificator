Please note that this project is no longer in active development. Please check out Vireio, TriDef or VorpX as alternatives.



Get current binary downloads from: http://stereoificator.com/

---------------------------------
////    HMD-Stereoificator, a fork of VIREIO PERCEPTION    ////
---------------------------------

BUILD INSTRUCTIONS:
---------------------------------
Created using Visual C++ 2010 Express (may work with newer versions) on Windows 7 64-bit.
http://www.microsoft.com/visualstudio/eng/downloads#d-2010-express 

Built using the Microsoft DirectX SDK (June 2010) (may work with newer versions).
http://www.microsoft.com/en-au/download/details.aspx?id=6812

Download libfreespace (libfreespace-0.6rc0-win32-vs2010.zip) from here:
http://libfreespace.hillcrestlabs.com/content/download 
Extract it to a folder

Download Oculus SDK from
https://developer.oculusvr.com/
Extract and copy LibOVR to project directory.

Click Start, Control Panel, System (in System and Security), Advanced System Settings, Environment Variables
Create environment variables:
	FREESPACE : The folder for libfreespace (Hillcrest Labs SDK).
	DXSDK_DIR : Your DirectX SDK folder (already set automatically by the June 2010 DirectX SDK)

Open the HMD-Stereoificator.sln solution file. It contains all the projects with their dependencies set correctly. 
Choose either Debug or Release. 
Either Run or Build the solution.
It should just work.
(There should be only two compiler warnings about the output directories not being the same as the target directory. That's deliberate.)

When you want to publish it, just build in Release mode and zip the contents of the Release folder. 
But be careful because when running in Release mode, changing the separation, convergence or other settings in game will affect the files you will release.
The Debug folder is automatically created from the files in the Release folder whenever you compile in Debug mode. 

If you want to view the debug prints from hooking the game, download and run DebugView. 
http://technet.microsoft.com/en-au/sysinternals/bb896647.aspx
Sometimes DebugView becomes unresponsive, in which case just continue what you were doing and look at it later.

It makes sense to work with the DxProxy project mainly. The DxProxy project is the main meat of the application. Basically what it does is it proxies DirectX so the game loads this modified "d3d9.dll" file instead of the real "d3d9.dll". 

The DxHijack project helps us to inject our modified dll into games, without having to place the proxy "d3d9.dll" file in the game folder (though some games don't work with this method, so we still have to copy the dll over as a last resort). 

The Stereoificator project is the GUI interface for the app. You probably will not need to touch this unless you are adding new options to the menu (ie a new tracking device).

Bug reports and questions: https://github.com/ChrisJD/HMD-Stereoificator/issues
