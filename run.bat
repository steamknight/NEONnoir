:: --- Blitz Basic 2.1 build batch script ---
:: ---   by MickGyver (Mikael Norrgard)   ---

:: Leave the following two lines as is
@echo off
setlocal EnableDelayedExpansion

:: -----------------------------------------------------------------------------
:: -- CONFIGURE THE FOLLOWING VARIABLES ----------------------------------------
:: -----------------------------------------------------------------------------

:: Set the project path as seen by WorkBench in the emulated Amiga instance.
:: (This is a Windows folder set up as a virtual hdd in WinUAE.)
:: The project path must end with a forward slash (/).
set project_path_on_amiga=Develop:/JamProj/

set destination_path=C:\Users\mass\OneDrive\Amiga\hdf\Development\NEONnoir\

:: Add all project files, first file must be the main file of the project.
:: All project filenames must end with "_asc" and have the extension ".bb2".
set project_files[0]=location_tester_asc.bb2
::set project_files[0]=main_asc.bb2
set project_files[1]=utils_asc.bb2
set project_files[2]=state_logo_asc.bb2
set project_files[3]=state_game_asc.bb2
set project_files[4]=IV_inventory_asc.bb2
set project_files[5]=NN_game_asc.bb2
set project_files[6]=NN_string_table_asc.bb2
set project_files[7]=NS_script_asc.bb2
set project_files[8]=SC_scene_asc.bb2
set project_files[9]=SH_shapes_asc.bb2
set project_files[10]=NS_opcodes_asc.bb2
:: set project_files[1]=include1_asc.bb2
:: set project_files[2]=include2_asc.bb2

:: Set the WinUAE folder.
:: WinUAE folder must end with a backslash (\)
set winuae_folder=C:\Users\mass\OneDrive\Amiga\

:: Set the WinUAE executable name (winuae64.exe for 64bit, winuae.exe for 32bit)
set winuae_exe=winuae64.exe

:: Set the config file to use in WinUAE
set winuae_config="C:\Users\mass\OneDrive\Amiga\a1200 - BetterWB.uae"

:: Set time (seconds) to wait for WorkBench to start before starting BB2
:: (Used when WinUAE isn't already running.)
set /a wb_delay=7

:: -----------------------------------------------------------------------------
:: -- END OF VARIABLES ---------------------------------------------------------
:: -----------------------------------------------------------------------------

:: -----------------------------------------------------------------------------
:: YOU SHOULDN'T NEED TO EDIT ANYTHING BELOW THIS LINE
:: -----------------------------------------------------------------------------

:main

:: WinUAE 64-bit: winuae64.exe, WinUAE 32-bit: winuae.exe
tasklist /FI "IMAGENAME eq %winuae_exe%" 2>NUL | find /I /N "%winuae_exe%">NUL

:: If WinUAE is not running, then run it!
if %ERRORLEVEL%==1 goto winuaenotrunning

:: Get the number of source files in project
echo -----------------------------------
set /a x=0
:arrayLength
if defined project_files[%x%] (
   call echo %%project_files[%x%]%%
   set /a x+=1
   GOTO :arrayLength
)

:: Reduce project file count by one for loops
set /a count=%x%-1

:: Run ConvertEOL for every *_asc.bb2 file in the project
for /l %%n in (0,1,%count%) do (
  ConvertEOL unix src\!project_files[%%n]! "%destination_path%!project_files[%%n]:~0,-8!.bb2"
)

:: Build the arguments for the WinUAEArexx command
set filelist=!project_path_on_amiga!!project_files[0]:~0,-8!.bb2
for /l %%n in (1,1,%count%) do (
  set filelist=!filelist! !project_path_on_amiga!!project_files[%%n]:~0,-8!.bb2
)

:: Run the ARexx script
WinUAEArexx blitzbasic2 1000 !filelist!

:: Bring WinUAE to front
BringToFront "WinUAE"

:: Go to end of script
goto end

:winuaenotrunning

echo -------------------------------
echo Starting WinUAE and SuperTED
echo -------------------------------
start "" "%winuae_folder%%winuae_exe%" -f "%winuae_config%"
timeout %wb_delay%
goto main

:end

timeout 5
exit
