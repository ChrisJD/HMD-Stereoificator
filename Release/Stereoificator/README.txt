---------------------------------
////    Stereoificator, a fork of VIREIO PERCEPTION    ////
---------------------------------
ALPHA quality software. Expect many bugs and problems. There are many things that are in worse shape than the current Vireio release. For example, Skyrim doesn't work at all at the moment with this fork.

*** DO NOT USE ONLINE with any game that has cheat prevention (VAC, Punkbuster, any mmo, etc) since the driver could be detected as a hack which may get you banned. ***

See: https://github.com/ChrisJD/HMD-Stereoificator/wiki for the most up-to-date documentation.


What's in it
---------------------------------
The core change in this fork is a near total rewrite of the rendering system so that both eyes are rendered every frame, this is significantly more complicated and there are still plenty of bugs to fix. The performance hit for this varies between hardware and games, it can be worse than 50% (of non-stereo frame rate) and it can be better. For example I get 65-70% of mono performance in HL2, but experimenting with Dead Rising 2 I was getting 35-40% of mono fps. Very little optimisation has been done so far and more isn't likely to happen until a lot more bugs are squashed.

This fork is also focusing on Head Mounted Display (HMD) support and all other stereo modes except side-by-side (which remains for testing purposes only and will not function well as a 3D mode as it is missing needed adjustments) have been removed.

A lot of the manual configuration of settings has been rendered unnecessary by updating the calculations to match the hardware optics in the same way the LibOVR SDK from Oculus works. All that should be required, as far as view adjustment is concerned, after the games settings are configured ('correct' fov is very important) is to adjust the eye separation using the f2/f3 keys while playing to somewhere that "feels right". This is still not ideal but it's workable and simpler to use. The quality of the default values varies quite a bit depending on what can be found in-game to be used to determine the approximate world scale.


HOT-KEYS:
---------------------------------
F2 / F3 : Adjust Separation. Hold shift to adjust 10x faster, hold ctrl to adjust 10x slower.
F4 / F5 : Adjust HUD distance from player (Currently Source based games only)
Ctrl-F4 / Ctrl-F5 - Adjust HUD Scale (Currently Source based games only) 

F6 : Swap Eyes.

Shift-F6 : Resets all user settings for the game you are playing to defaults. 

F8 / F9 : Adjust Yaw rate.
SHIFT + F8 / F9 : Adjust Pitch rate.
CONTROL + F8 / F9 : Adjust Roll rate.




Experimental Game support. (Hint, it's all experimental)
Be sure to read the Wiki (https://github.com/ChrisJD/HMD-Stereoificator/wiki) for full configuration instructions.
---------------------------------
If it isn't possbile for you to get the game world looking reasonable adjusting the settings please provide as descriptive a description of the problem as possible. Especially with regards to GRID, I have a feeling the car internals are getting scaled differently to the world but I can't pin down what's wrong looking at it on a monitor.

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

Source Engine (bug fixes for these are very low priority given how good valves support of vr in source has been, flashlights in l4d are far worse than Vireio):

Half-Life 2 	
Left4Dead	
Left4Dead 2	
Dear Esther	
Portal
Portal 2

Other:

AaaaAAAAA!!!!!	
