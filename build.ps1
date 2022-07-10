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
    "./src/DG_dialogue.bb2"
    "./src/NS_opcodes.bb2"
    "./src/NS_script.bb2"
    "./src/location_tester.bb2"
)

$data_files = @(
    "./data/alleyway1.iff",
    "./data/alleyway2.iff",
    "./data/alleyway3.iff",
    "./data/display.font",
    "./data/frame.shape",
    "./data/pointers.shape",
    "./data/gutter.neon"
)

# Location where the built file will reside
$output_file = "X:/Disks/Develop/NEONnoir/neonnoir.bb2"
$data_dir = "X:/Disks/Develop/NEONnoir/data"

function Copy-GameData {
# Copy all the data files
    Copy-Item -Path $data_files -Destination $data_dir
}

function Copy-Game {
    # Create the file
    Get-Content $bb2_files | Out-File $output_file
    
    # Convert line endings
    Invoke-Expression "& external/ConvertEOL unix $output_file $output_file"
}

function Publish-Game {
    Copy-Game
    Copy-GameData
}

