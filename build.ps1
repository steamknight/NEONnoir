# List all the Blitz Basic files to be merged in order they should appear
# in the output file

$bb2_files = @(
    "./src/NN_startup.bb2",
    "./src/DB_log.bb2",
    "./src/NN_core.bb2",
    "./src/NN_utils.bb2",
    "./src/NN_game.bb2",
    "./src/TX_core.bb2",
    "./src/SC_scene.bb2"
    "./src/SH_shapes.bb2"
    "./src/IV_inventory.bb2"
    "./src/MS_music.bb2",
    "./src/NS_opcodes.bb2",
    "./src/NS_script.bb2",
    "./src/DG_dialogue.bb2",
    "./src/NN_main.bb2"
    #"./src/location_tester.bb2"
)

$data_files = @(
    "./data/title.iff",
    "./data/intro01.iff",
    "./data/intro02.iff",
    "./data/intro03.iff",
    "./data/alleyway1.iff",
    "./data/alleyway2.iff",
    "./data/alleyway3.iff",
    "./data/display.font",
    "./data/frame.shape",
    "./data/pointers.shape",
    "./data/gutter.neon",
    "./data/theme.mod"
)

# Location where the built file will reside
#$output_file = "X:/Disks/Develop/NEONnoir/neonnoir.bb2"
#$data_dir = "X:/Disks/Develop/NEONnoir/data"
$output_dir = "C:/Users/mass/OneDrive/Amiga/hdf/Development/NEONnoir/"
$data_dir = "C:/Users/mass/OneDrive/Amiga/hdf/Development/NEONnoir/data"

function Copy-GameData {
    # Copy all the data files
    Copy-Item -Path $data_files -Destination $data_dir
}

function Copy-Game {
    $output_file = $output_dir + "neonnoir.bb2"

    # Create the file
    Get-Content $bb2_files | Out-File $output_file
    
    # Convert line endings
    Invoke-Expression "& external/ConvertEOL unix $output_file $output_file"
}

function Publish-Game {
    Copy-Game
    Copy-GameData
}

function Archive-Game {
    Read-Host "Press 'Enter' once the project has been built in Blitz"

    if (Test-Path -Path "./build/") {
        Write-Host "Cleaning up 'build' folder..."
        Remove-Item -Path "./build/" -Recurse
    }

    $null = New-Item -ItemType Directory -Path "./build/data" -Force


    $compiled = @(
        ($output_dir + "neonnoir")
        ($output_dir + "neonnoir.info")
    )

    Write-Host "Copying files..."
    Copy-Item -Path $compiled -Destination "./build/"
    Copy-Item -Path $data_files -Destination "./build/data/"


}

# https://stackoverflow.com/questions/4533570/in-powershell-how-do-i-split-a-large-binary-file
function Split-File($inFile, $outPrefix, [Int32] $bufSize) {
    $stream = [System.IO.File]::OpenRead($inFile)
    $chunkNum = 1
    $bytes = New-Object byte[] $bufSize

    while ($bytesRead = $stream.Read($bytes, 0, $bufSize)) {
        $outFile = "$outPrefix{0:d2}" -f $chunkNum
        $ostream = [System.IO.File]::OpenWrite($outFile)
        $ostream.Write($bytes, 0, $bytesRead)
        $ostream.Close()

        Write-Host "Writing $outFile"
        $chunkNum += 1
    }
}

