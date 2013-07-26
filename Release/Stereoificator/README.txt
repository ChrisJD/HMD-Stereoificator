---------------------------------
////    Stereoificator, a fork of VIREIO PERCEPTION    ////
---------------------------------
ALPHA quality software. Expect many bugs and problems. There are many things that are in worse shape than the current Vireio release. For example, Skyrim doesn't work at all at the moment with this fork.

*** DO NOT USE ONLINE with any game that has cheat prevention (VAC, Punkbuster, any mmo, etc) since the driver could be detected as a hack which may get you banned. ***

Nothing about this is plug and play, make sure you read the documentation. I repeat this software is NOT plug and play.
See: https://github.com/ChrisJD/HMD-Stereoificator/wiki for the most up-to-date documentation.


What's in it
---------------------------------
The core change in this fork is a near total rewrite of the rendering system so that both eyes are rendered every frame, this is significantly more complicated and there are still plenty of bugs to fix. The performance hit for this varies between hardware and games, it can be worse than 50% (of non-stereo frame rate) and it can be better. For example I get 65-70% of mono performance in HL2, but experimenting with Dead Rising 2 I was getting 35-40% of mono fps. Very little optimisation has been done so far and more isn't likely to happen until a lot more bugs are squashed.

This fork is also focusing on Head Mounted Display (HMD) support and all other stereo modes except side-by-side (which remains for testing purposes only and will not function well as a 3D mode as it is missing needed adjustments) have been removed.

A lot of the manual configuration of settings has been rendered unnecessary by updating the calculations to match the hardware optics in the same way the LibOVR SDK from Oculus works. All that should be required, as far as view adjustment is concerned, after the games settings are configured ('correct' fov is very important) is to adjust the eye separation using the f2/f3 keys while playing to somewhere that "feels right". This is still not ideal but it's workable and simpler to use. The quality of the default values varies quite a bit depending on what can be found in-game to be used to determine the approximate world scale.


HOT-KEYS:
---------------------------------
F2 / F3 : Adjust Separation. Hold shift to adjust 10x faster, hold ctrl to adjust 10x slower.
F4 / F5 : Adjust HUD distance from player (only in some games)
Ctrl-F4 / Ctrl-F5 - Adjust HUD Scale (only in some games) 

F6 : Swap Eyes.

Shift-F6 : Resets all user settings for the game you are playing to defaults. 

F8 / F9 : Adjust Yaw rate.
SHIFT + F8 / F9 : Adjust Pitch rate.
CONTROL + F8 / F9 : Adjust Roll rate.


Supported Games (all experimental) https://github.com/ChrisJD/HMD-Stereoificator/wiki/Game-Configuration