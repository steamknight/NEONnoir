# List all the Blitz Basic files to be merged in order they should appear
# in the output file

$bb2_files = @(
    "./src/NN_build.bb2",
    "./src/NN_startup.bb2",
    "./src/DB_log.bb2",
    "./src/NN_core.bb2",
    "./src/NN_utils.bb2",
    "./src/MS_music.bb2",
    "./src/NN_game.bb2",
    "./src/TX_core.bb2",
    "./src/SC_scene.bb2"
    "./src/SH_shapes.bb2"
    "./src/DG_dialogue.bb2",
    "./src/NS_opcodes.bb2",
    "./src/NS_script.bb2",
    "./src/NN_main.bb2"
    #"./src/location_tester.bb2"
)

$data_files = @(
    "./data/title.iff",
    "./data/intro01.iff",
    "./data/intro02.iff",
    "./data/intro03.iff",
    "./data/intro04.iff",
    "./data/blank.iff",
    "./data/alleyway1a.iff",
    "./data/locations/loc_alley_alleycat.iff",
    "./data/alleyway3a.iff",
    "./data/alleyway_trash.iff",
    "./data/alleyway_atm.iff",
    "./data/subway_entrance.iff",
    "./data/locations/loc_subway_interior.iff",
    "./data/alleycat1.iff",
    "./data/locations/loc_alleycat_bar.iff",
    "./data/locations/loc_alleycat_entrance.iff",
    "./data/locations/loc_apartment_bedroom.iff",
    "./data/locations/loc_apartment_interior2.iff",
    "./data/locations/loc_blake_apartment.iff",
    "./data/locations/loc_laundry_exterior.iff",
    "./data/locations/loc_laundry_interior.iff",
    "./data/locations/loc_b19_entrance.iff",
    "./data/locations/loc_b19_office.iff",
    "./data/locations/loc_b19_office2.iff",
    "./data/locations/loc_b19_storage.iff",
    "./data/locations/loc_b19_catwalk.iff",
    "./data/locations/loc_segs_place.iff",
    "./data/cutscenes/end_act_2.iff",
    "./data/cutscenes/end_act_1.iff",
    "./data/cutscenes/danni1.iff",
    "./data/cutscenes/danni2.iff",
    "./data/cutscenes/danni3.iff",
    "./data/cutscenes/b19.iff",
    "./data/display.font",
    "./data/frame.shape",
    "./data/pointers.shape",
    "./data/gutter.neon",
    "./data/music/theme.mod",
    "./data/music/ambient.mod",
    "./data/music/dance.mod",
    "./data/music/intrigue.mod",
    "./data/music/boss_intro.mod",
    "./data/music/boss_theme.mod",
    "./data/mpg.iff",
    "./data/1.shapes",
    "./data/2.shapes",
    "./data/3.shapes",
    "./data/4.shapes",
    "./data/5.shapes",
    "./data/6.shapes",
    "./data/7.shapes",
    "./data/8.shapes",
    "./data/menu.shapes"
)

# Location where the built file will reside
# $output_dir = "C:/Users/mass/tools/WinUAE/HardDrives/Develop/NEONnoir/"
# $data_dir = "C:/Users/mass/tools/WinUAE/HardDrives/Develop/NEONnoir/data"
$output_dir = "C:/Users/mass/OneDrive/Amiga/hdf/Development/NEONnoir/"
$data_dir = "C:/Users/mass/OneDrive/Amiga/hdf/Development/NEONnoir/data"

$Env:PATH = "C:\Users\mass\AppData\Roaming\Python\Python37\Scripts\;" + $Env:PATH

function Set-ToolPath($path) {
    $ENV:Path = "$path\external;$path\external\NConvert;" + $ENV:Path
}

function Copy-GameData {
    # Copy all the data files
    Copy-Item -Path $data_files -Destination $data_dir
}

function Copy-Game {
    $output_file = $output_dir + "neonnoir.bb2"

    $file_in = "BUILD_NUM"
    $file_out = "./src/NN_build.bb2"
    $date = Get-Date -Format "yyMM"
    $fileVersion = [version](Get-Content $file_in | Select-Object -First 1)
    $newVersion = "{0}.{1}.{2}.{3}" -f $fileVersion.Major, $fileVersion.Minor, $date, ($fileVersion.Revision + 1)
    $newVersion | Set-Content $file_in
    "; $newVersion" | Set-Content $file_out

    # Create the file
    Get-Content $bb2_files | Out-File $output_file
    
    # Convert line endings
    Invoke-Expression "& external/ConvertEOL unix $output_file $output_file"
}

function Publish-Game {
    Copy-Game
    Copy-GameData
}

function Convert-EOL($file) {
    Invoke-Expression "& ConvertEOL unix $file $output_dir$file"
}

function Get-GeneratedShapes {
    Copy-Item -Path "$output_dir\\assetgen\\*.shapes" -Destination ".\data"
}

function Archive-Game {
    Read-Host "Press 'Enter' once the project has been built in Blitz\"

    if (Test-Path -Path "./build/") {
        Write-Host "Cleaning up 'build' folder..."
        Remove-Item -Path "./build/" -Recurse
    }

    $null = New-Item -ItemType Directory -Path "./build/parts" -Force

    Write-Host "Copying files..."
    Copy-Item -Path ($output_dir + "build/NEONnoir.lha") -Destination "./build/"

    Split-File -Filename "./build/NEONnoir.lha" -Prefix "./build/parts/NEONnoir" -Size (800 * 1024)

    $count = 1
    Get-ChildItem ".\build\parts" | ForEach-Object {
        $name = $_.BaseName
        $volume = "NN{0:d2}" -f $count

        Invoke-Expression "& xdftool .\build\$volume.adf format '$volume' + write .\build\parts\$name"
        $count += 1
    }
    Invoke-Expression "& xdftool .\build\NN01.adf write .\src\install.sh"

    $drive = Read-Host "Enter the drive to copy the ADFs to"
    Copy-Item -Path "./build/*.adf" -Destination $drive
}

# https://stackoverflow.com/questions/4533570/in-powershell-how-do-i-split-a-large-binary-file
function Split-File {
    param (
        $Filename, $Prefix, [Int32] $Size
    )
    $stream = [System.IO.File]::OpenRead($Filename)
    $chunkNum = 1
    $bytes = New-Object byte[] $Size

    while ($bytesRead = $stream.Read($bytes, 0, $Size)) {
        $outFile = "$Prefix{0:d2}" -f $chunkNum
        $ostream = [System.IO.File]::OpenWrite($outFile)
        $ostream.Write($bytes, 0, $bytesRead)
        $ostream.Close()

        Write-Host "Writing $outFile"
        $chunkNum += 1
    }
}

