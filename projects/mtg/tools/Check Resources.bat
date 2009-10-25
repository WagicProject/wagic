@SetLocal EnableDelayedExpansion
@echo PSP resource updating script. 
@echo This script copies resources from PC to PSP.
@echo It must be run from the tools directory.
@echo.
@echo To avoid prompts, please edit the path variable wthpath in this file.
@echo.
@echo This script does not copy SFX or profiles.
@REM Set the following two lines to the proper paths
@echo.
@echo What is your PSP WTH path (K:/PSP/Game5XX/WTH)
@set /P wthpath=Path: 
@echo Copying files to %wthpath%

xcopy "../bin/res/sets" "%wthpath%/Res/sets" /E /D /Y
xcopy "../bin/res/lang" "%wthpath%/Res/lang" /E /D /Y
xcopy "../bin/res/ai" "%wthpath%/Res/ai" /E /D /Y
xcopy "../bin/res/graphics" "%wthpath%/Res/graphics" /E /D /Y
xcopy "../bin/Res/themes" "%wthpath%/Res/themes" /E /D /Y
pause