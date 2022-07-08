# List all the Blitz Basic files to be merged in order they should appear
# in the output file

$bb2_files = @(
    "./src/NN_startup.bb2"
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

# Location where the built file will reside
$output_file = "C:/Users/mass/OneDrive/Amiga/hdf/Development/NEONnoir/neonnoir.bb2"

# Create the file
Get-Content $bb2_files | Out-File $output_file

# Convert line endings
Invoke-Expression "& ConvertEOL unix $output_file $output_file"


