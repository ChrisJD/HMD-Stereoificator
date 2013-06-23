---------------------------------
////    HMD-Stereoificator, a fork of VIREIO PERCEPTION    ////
---------------------------------
This ALPHA quality software. Expect many bugs and problems. There are many things that are in worse shape than the current Vireio release. For example, Skyrim doesn't work at all currently with this fork.

- DO NOT USE ONLINE with any game that has cheat prevention (VAC, Punkbuster, any mmo, etc) since the driver could be detected as a hack which may get you banned.


What's in it
---------------------------------
The core change in this fork is a near total rewrite of the rendering system so that both eyes are rendered every frame, this is significantly more complicated and there are still plenty of bugs to fix. The performance hit for this varies between hardware and games, it can be worse than 50% (of non-stereo frame rate) and it can be better. For example I get 65-70% of mono performance in HL2, but experimenting with Dead Rising 2 I was getting 35-40% of mono fps.
This fork is also focusing on Head Mounted Display (HMD) support and all other stereo modes except side-by-side (which remains for testing purposes only and will not function well as a 3D mode as it is missing needed adjustments) have been removed.
A lot of the manual configuration of settings has been rendered unnecessary by updating the calculations to match the hardware optics in the same way the LibOVR SDK from Oculus works. 
All that should be required after the games settings are configured ('correct' fov is very important) is to adjust the eye separation using the f2/f3 keys while playing to somewhere that "feels right". This is still not ideal but it's workable and simpler to use.
The quality of the default values (which are intended for 64mm IPD) varies quite a bit depending on what can be found in-game to be used to determine the approximate world scale.


HOT-KEYS:
---------------------------------
F1 : Save Screenshot. Saves the original left and right images (for testing/debugging) and the final image to game folder (will overwrite images from previous sessions).

F2 / F3 : Adjust Separation. Hold shift to adjust 10x faster, hold ctrl to adjust 10x slower.

F6 : Swap Eyes.

F8 / F9 : Adjust Tracking Yaw.
SHIFT + F8 / F9 : Adjust Tracking Pitch.
CONTROL + F8 / F9 : Adjust Tracking Roll.


NOTES:
---------------------------------
- DO NOT USE ONLINE with any game that has cheat prevention (VAC, Punkbuster, any mmo, etc) since the driver could be detected as a hack which may get you banned.

- Set the game FoV. This varies depending on the game and can be a pain in the ass.  If the game takes horizontal FoV use 92** degrees, if it takes vertical FoV and you are using a 16:10 resolution use 66 degrees and if it takes vertical FoV and you are using a 16:9 resolution use 61 degrees.
- Enable vertical sync to avoid tearing.
- In general dynamic shadows don't work very well at the moment, turn them down and if that doesn't fix the problem then turn them off.
- Open HMD-Stereoificator and leave it running in the background while playing.

- It is better to adjust all game settings before playing with the driver, as changing certain settings may cause the game to crash (i.e. changing the resolution - although this is a bug if it happens and reposting of the issue would be appreciated).
- The mouse icon may not be located where the cursor actually is. This is a known issue, click where the object you want to click on would be if the game was running normally without Stereoficator running.
- Bug reports and questions: https://github.com/ChrisJD/HMD-Stereoificator/issues


Experimental Game support. (Hint, it's all experimental)
Be sure to read the general notes above and game specific notes below, feedback with regard to which issues are the most disruptive to gameplay is appreciated to help prioritise tasks.
---------------------------------
If it isn't possbile for you to get the game world looking reasonable adjusting just the settings please provide as descriptive a description of the problem as possible. Especially with regards to GRID, I have a feeling the car internals are getting scaled differently to the world but I can't pin down what's wrong looking at it on a monitor.

EGO Engine:
F1 2010
Dirt 2	
Dirt 3
GRID

UT3/UDK Engine:
Mirror's Edge
The Ball
Borderlands
Borderlands 2

Source Engine (bug fixes for these are very low priority given how good valves support of vr in source has been, flashlights are far worse than Vireio):
Half-Life 2 	
Left4Dead	
Left4Dead 2	
Dear Esther	
Portal
Portal 2

Other:
AaaaAAAAA!!!!!	





GAME NOTES:
---------------------------------

If the notes say "Copy dlls", then copy 'd3d9.dll' and 'libfreespace.dll' from the bin directory of HMD-Stereoificator to the same folder as the games executable.
If a Steam game won't launch via your normal method; 'run from Steam' or 'run the exe directly', then try the other.

Half-Life 2: Set Shadow Detail to Medium. (optional: Enable developers console. **Use "fov_desired 92".)
DeadEsther: **Use "fov_desired 92".
Portal 2: Use "r_shadows 0", "viewmodel_offset_y 8", **"cl_fov 92"

Mirror's Edge: 
Copy dlls.
Won't launch unless I start through Steam (your mileage may vary). 

The Ball: Tilde(~) for console then fov 92**.

Borderlands: 
Copy dlls.
Default FoV is way to narrow so a change is needed. FoV (set it to 92) change is easiest with the utility in this http://forums.gearboxsoftware.com/showpost.php?p=3863369&postcount=388 thread. There is a working link in that post. FoV resets a lot in Borderlands so I always use the set FoV on forward option.
HUD should probably be disabled, http://borderlands.wikia.com/wiki/Borderlands_PC_Tweaks#Show.2FHide_HUD (haven't tested this)

Borderlands 2:
Disable dynamic shadows (Instructions: http://forums.gearboxsoftware.com/showthread.php?t=150107)
Enable console (Instructions: http://forums.gearboxsoftware.com/showthread.php?p=2763900)
Then use console to set fov with "fov 92"
Disable HUD in console using "ToggleHUD"
Launch game via Borderlands2.exe (don't launch through Steam)


EGO Engine games:
The FoV needs to be changed in the car configuration files. The easiest way to make the change is to go and get the fov changer utility from http://www.nogripracing.com/forum/showpost.php?p=1382564&postcount=18 (you'll need to register with the forum to be able to download it). You probably want to make a backup of the car files (easiest just to backup the entire game directory) before making any changes. I only change the cockpit fov as that's the only place I want to play from.
DirectX9 mode needs to be forced if you have a 10/11 card for all EGO games (except possibly GRID?). DirectX9 is forced in the configuration file "My Documents\My Games\GAME_NAME\hardwaresettings\hardware_settings_config.xml" where GAME_NAME is DiRT2, DiRT3, FormulaOne
Shadows need to be on the lowest setting without turning them off (all though you can turn them off if you want).
Disable all HUD elements.

DiRT 2: Run via dirt2.exe
GRID: Run via GRID.exe

DiRT 3 and
F1 2010:
Copy dlls.
For me they won't launch unless I start them through Steam and sometimes it takes 2 or 3 tries before they work without a securerom error.


AaaaAAAA!!!!: Copy dlls.



** Most Source games (and plenty of others) default to 90, this is probably close enough at the moment for most people who don't want to do to much configuration