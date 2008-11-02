Introduction
------------

JGE++ (Jas Game Engine++) is a hardware accelerated 2D game SDK for PSP. It supports cross-platform development under MS Windows. The entire game development process, including coding, debugging and testing, can be done in Windows. (However, it's still recommended to test your game on PSP from time to time to make sure everything is fine.)

You can use JGE++ to make Windows games but the primary platform is PSP.


Features

    * 1.xx and 3.xx firmware support on PSP.
    * Hardware accelerated 2D rendering including scaling, rotations and colour blending.
    * Animated sprites.
    * Geometry shapes rendering, including rectangle, circle, polygons and thick lines.
    * Loading PNG, JPEG and GIF.
    * Spline.
    * Advanced multiple emitter and key frame based particle system.
    * Bitmap fonts.
    * Chinese GBK fonts.
    * True Type fonts.
    * Stereo WAV playback.
    * Hardware MP3 decoding on PSP.
    * Resource manager.
    * Zipped resource support.
    * Frame based animation system using XML scripts.
    * Basic 3D functions, including support of rendering textured triangles, Quake 2 (MD2) model and OBJ model.
    * Port of HGE helper classes: hgeParticleSystem, hgeDistortionMesh and hgeFont.
    * Input system for English and Chinese.



Project page:
-------------
http://jge.googlecode.com/



Official forums:
----------------
http://jge.khors.com/



To create a new JGE++ project:
------------------------------
Change to "JGE\Tools" in a command prompt and type:

newproject project_name project_description

A project called "project_name" will be created in "JGE\Projects". You can find a VC2005 Express solution file there and you can open it up and start working on your own cool PSP game.



To build JGE++ for PSP:
-----------------------
Change to the JGE directory (say, in a cygwin prompt). 

Do either:

(1) type in "make" or "make 1xx" to build for 1.xx firmware, 
	OR
(2) type in "make 3xx" to build for 3.xx firmware.



To build a project for PSP:
---------------------------
Change to your project directory (say, in a cygwin prompt). Do either:

(1) type in "make" or "make 1xx" to build for 1.xx firmware, 
	OR
(2) type in "make 3xx" to build for 3.xx firmware.



Key mapping on Windows:
-----------------------

PSP                     WINDOWS
==================      ==================
UP/DOWN/LEFT/RIGHT      W/S/A/D
ANALOG CONTROL          UP/DOWN/LEFT/RIGHT
TRIANGLE                I
SQUARE                  J
CIRCLE                  L
CROSS                   K
SELECT                  CTRL
START                   ENTER
HOME                    F1
HOLD                    F2
NOTE                    F3
L                       Q
R                       E



Note for testing on PSP:
------------------------
Please remember to copy the entire folder "bin\Res" into the appropriate folder on PSP.



License:
--------
JGE++ is Licensed under the BSD license, see LICENSE in root folder for details.



Credits:
--------
The JGE++ Team is: 

- James Hui (a.k.a. Dr.Watson)
- Sijiu Duan (a.k.a. Chi80)



Special thanks to:
------------------
- Cheese (WAV playback code)
- Cooleyes (MP3 hardware decoder)
- Fan990099
- Firenonsuch
- Newcreat
- Subelf
- Youmentang
- Jasmine



Contact:
--------
Bugs and comments can be forwarded to jhkhui@gmail.com or sijiu49@gmail.com.
 


History:
--------

[2007.10.22]
- v1.0 released.

[2006.03.26]
- v0.2b released. 
- First version that enabled cross-platform development on Windows by using HGE
- Bundled with SkyAnimator and source code of StarBugz and Mario Demo.

[2006.03.12]
- first public release.
- PSP only.

________________________________________________________________________________________

Copyright (C) 2007 James Hui <jhkhui@gmail.com>
Copyright (C) 2007 Sijiu Duan <sijiu49@gmail.com>

