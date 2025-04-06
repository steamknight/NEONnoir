# List all the Blitz Basic files to be merged in order they should appear
# in the output file

$bb2_files = @(
    "./src/NN_build.bb2",
    "./src/NN_startup.bb2",
    "./src/DB_log.bb2",
    "./src/NN_core.bb2",
    "./src/NN_utils.bb2",
    "./src/MPSH.bb2"
    "./src/MS_music.bb2",
    "./src/TX_core.bb2",
    "./src/NN_game.bb2",
    "./src/SC_scene.bb2"
    "./src/SH_shapes.bb2"
    "./src/DG_dialogue.bb2",
    "./src/NS_opcodes.bb2",
    "./src/NS_script.bb2",
    "./src/NN_main.bb2"
)

$data_files = @(
    "./data/title.iff",
    "./data/blank.iff",
    "./data/locations/loc_alleyway1.iff",
    "./data/locations/loc_alley_alleycat.iff",
    "./data/locations/loc_alleyway3.iff",
    "./data/locations/loc_alleyway_atm.iff",
    "./data/locations/loc_subway_entrance.iff",
    "./data/locations/loc_subway_interior.iff",
    "./data/locations/loc_alleycat_bar.iff",
    "./data/locations/loc_alleycat_entrance.iff",
    "./data/locations/loc_apartment_bedroom.iff",
    "./data/locations/loc_apartment_interior2.iff",
    "./data/locations/loc_blake_apartment.iff",
    "./data/locations/loc_laundry_exterior.iff",
    "./data/locations/loc_laundry_interior.iff",
    "./data/locations/loc_laundry_interior_fight.iff",
    "./data/locations/loc_b19_entrance.iff",
    "./data/locations/loc_b19_office.iff",
    "./data/locations/loc_b19_office2.iff",
    "./data/locations/loc_b19_storage.iff",
    "./data/locations/loc_b19_catwalk.iff",
    "./data/locations/loc_segs_place.iff",
    "./data/locations/loc_xantech_entrance.iff",
    "./data/locations/loc_xantech_lobby.iff",
    "./data/locations/loc_xantech_medical.iff",
    "./data/locations/loc_xantech_cafe.iff",
    "./data/locations/loc_xantech_corridor.iff",
    "./data/locations/loc_xantech_servers.iff",
    "./data/cutscenes/intro_01.iff",
    "./data/cutscenes/intro_02.iff",
    "./data/cutscenes/intro_03.iff",
    "./data/cutscenes/intro_04.iff",
    "./data/cutscenes/end_act_2.iff",
    "./data/cutscenes/end_act_1.iff",
    "./data/cutscenes/danni1.iff",
    "./data/cutscenes/danni2.iff",
    "./data/cutscenes/danni3.iff",
    "./data/cutscenes/b19.iff",
    "./data/cutscenes/xancon1.iff",
    "./data/cutscenes/xancon2.iff",
    "./data/cutscenes/xancon3.iff",
    "./data/cutscenes/end.iff",
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
    "./data/music/detective.mod",
    "./data/music/crystal.mod",
    "./data/music/to_industry.mod",
    "./data/mpg.iff",
    "./data/lang_selection.iff",
    "./data/flags.shapes",
    "./data/menu.shapes",
    "./data/people.mpsh",
    "./data/ui.pal",
    "./data/lang/en.noir",
    "./data/lang/it.noir",
    "./data/lang/de.noir",
    "./data/lang/latin-1.shapes"
)

function Set-ToolPath($path) {
    $ENV:Path = "$path\external;$path\external\NConvert;" + $ENV:Path
}

function Copy-GameData {
    $data_dir = "$ENV:NEONnoir_path"

    # Copy all the data files
    ForEach ($file in $data_files) {
        $dest_path = Join-Path -Path $data_dir -ChildPath $file

        if (!(Test-Path -Path $dest_path)) {
            $directory = Split-Path -Path $dest_path -Parent
            New-Item -ItemType Directory -Force -Path $directory
        }

        Write-Host "Copying game data to $dest_path..."
        Copy-Item -Path $file -Destination $dest_path
    }
}

function Copy-Game {
    $output_file = "$ENV:NEONnoir_path\neonnoir.bb2"

    $file_in = "BUILD_NUM"
    $file_out = "./src/NN_build.bb2"
    $date = Get-Date -Format "yyMM"
    $fileVersion = [version](Get-Content $file_in | Select-Object -First 1)
    $newVersion = "{0}.{1}.{2}.{3}" -f $fileVersion.Major, $fileVersion.Minor, $date, ($fileVersion.Revision + 1)
    $newVersion | Set-Content $file_in
    "NN_version$ = `"$newVersion`"" | Set-Content $file_out

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
    Invoke-Expression "& external/ConvertEOL unix $file $ENV:NEONnoir_path\$file"
}

function Archive-Game {
    Read-Host "Press 'Enter' once the project has been built in Blitz\"

    if (Test-Path -Path "./build/") {
        Write-Host "Cleaning up 'build' folder..."
        Remove-Item -Path "./build/" -Recurse
    }

    $null = New-Item -ItemType Directory -Path "./build/parts" -Force

    Write-Host "Copying files..."
    Copy-Item -Path ("$ENV:NEONnoir_path\build\NEONnoir.lha") -Destination "./build/"
    Copy-Item -Path ("$ENV:NEONnoir_path\build\parts\NN*.lha") -Destination "./build/parts/"

    $count = 1
    Get-ChildItem ".\build\parts" | ForEach-Object {
        $name = $_.BaseName
        $volume = "NN{0:d2}" -f $count

        Invoke-Expression "& xdftool .\build\$volume.adf format 'NEONnoir Disk $count' + write .\build\parts\$name.lha"
        $count += 1
    }
    Invoke-Expression "& xdftool .\build\NN01.adf write .\scripts\install_neonnoir"
    Invoke-Expression "& xdftool .\build\NN01.adf write .\scripts\Install_NEONnoir.info"
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

# Make sure we have a valid output directory.
# Save it to the user's environment variables.
if ($null -eq $ENV:NEONnoir_path) {
    Write-Host "Initializing build environment."
    Write-Host

    Add-Type -AssemblyName 'System.Windows.Forms'

    while ($null -eq $ENV:NEONnoir_path) {
        Write-Host "Please select an output directory."
        $dialog = New-Object System.Windows.Forms.FolderBrowserDialog
        if ($dialog.ShowDialog() -eq [System.Windows.Forms.DialogResult]::OK) {
            $directory_name = $dialog.SelectedPath
            Write-Host "Writing output directory $directory_name to NEONnoir_path environment variable."
            $ENV:NEONnoir_path = $directory_name
        }
    }
}

# Add the python scripts to the path so we can use xdftool
if (-1 -eq $ENV:PATH.IndexOf("$ENV:APPDATA\Python\Python37\Scripts")) {
    $Env:PATH = "$ENV:APPDATA\Python\Python37\Scripts\;" + $Env:PATH
}
if (-1 -eq $ENV:PATH.IndexOf("$ENV:LOCALAPPDATA\Packages\PythonSoftwareFoundation.Python.3.13_qbz5n2kfra8p0\LocalCache\local-packages\Python313\Scripts")) {
    $Env:PATH = "$ENV:LOCALAPPDATA\Packages\PythonSoftwareFoundation.Python.3.13_qbz5n2kfra8p0\LocalCache\local-packages\Python313\Scripts;" + $Env:PATH
}

function CreateUSB {
    param (
        [Parameter(Mandatory = $true)]
        [string]$sourceDirectory,

        [Parameter(Mandatory = $true)]
        [char]$driveLetter,

        [Parameter(Mandatory = $true)]
        [string]$newName
    )

    # Copy files
    Copy-Item -Path $sourceDirectory -Destination "$($driveLetter):\" -Recurse -Force

    # Rename drive
    Set-Volume -DriveLetter $driveLetter -NewFileSystemLabel $newName

    # Eject drive
    $driveEject = New-Object -comObject Shell.Application
    $driveEject.Namespace(17).ParseName("$($driveLetter):\").InvokeVerb("Eject")
}

function Prepare-USB {
    if (Test-Path -Path "./output/") {
        Write-Host "Cleaning up 'output' folder..."
        Remove-Item -Path "./output/" -Recurse
    }

    $null = New-Item -ItemType Directory -Path "./output/ADF" -Force
    $null = New-Item -ItemType Directory -Path "./output/manuals" -Force

    Copy-Item -Path "./manuals/*.pdf" -Destination "./output/manuals" -Recurse -Force
    Copy-Item -Path "./build/*.adf" -Destination "./output/adf" -Recurse -Force
    Copy-Item -Path "./build/NEONnoir.lha" -Destination "./output" -Force

    Compress-Archive -Path "./output/*" -DestinationPath "./output/NEONnoir.zip" -Force
}
