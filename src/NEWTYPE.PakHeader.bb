NEWTYPE.PakHeader

End NEWTYPE

NEWTYPE.Location
    name_id.w           ; String Id fo the location name "Your Office"
    bg_image_id.w       ; Id of the background image

    ; Contrain the mouse pointer to this view
    view_x.w
    view_y.w
    view_width.w
    view_height.w

    ; Normally at 0,0, this offset the baground image
    offset_x.w
    offset_y.w

    num_regions.w       ; Number of interactive regions in this location
    regions.w[8]        ; 8 aught to be enough regions, right?
end NEWTYPE

NEWTYPE.Region
    ; Rectangle for the region
    x.w
    y.w
    width.w
    height.w

    has_shape.w         ; If true there is a shape defined for this area
    shape_id.w          ; Shape that should be in this space if has_shape is true

    ; Interaction behavior
    description_id.w    ; Id of the string that describes this region
    script_id.w         ; Id of the script to run when the region is clicked
end NEWTYPE

NEWTYPE.Item
    shape_id.w          ; Id of the shape that should show up in inventory
    name_id.w           ; String id for the item's name
    description_id.w    ; String id for the item's description
end NEWTYPE

NEWTYPE.Script
    start.w             ; Starting word of first script
    length.w            ; Size of script in words
end NEWTYPE

Dim NN_strings.s(10)
NN_strings(0) = "First Room"
NN_strings(1) = "Second Room"
NN_strings(2) = "A light switch"

Dim NN_images.s(3)
NN_images(0) = "data/first_lightsoff.iff"
NN_images(1) = "data/first_lightson.iff"
NN_images(2) = "data/second.iff"

NN_shapes.s = "data/shapes.iff"

Dim NN_scripts.Script(4)
; Light switch
NN_scripts(0)\start = 0
NN_scripts(0)\length = 0
; Exit to scene 2
NN_scripts(1)\start = 0
NN_scripts(1)\length = 0
; Breaker
NN_scripts(2)\start = 0
NN_scripts(3)\length = 0
; Exit to scene 1
NN_scripts(3)\start = 0
NN_scripts(3)\length = 0

Dim NN_locations.Location(2)
NN_locations(0)\name_id = 0
NN_locations(0)\bg_image_id = 0
NN_locations(0)\view_x = 0
NN_locations(0)\view_y = 0
NN_locations(0)\view_width = 320
NN_locations(0)\view_x = 200
NN_locations(0)\offset_x = 0
NN_locations(0)\offset_y = 0
NN_locations(0)\num_regions = 2
NN_locations(0)\regions[0] = 0      ; Light switch
NN_locations(0)\regions[1] = 1      ; Exit to screen 2

NN_locations(1)\name_id = 1
NN_locations(1)\bg_images = 2
NN_locations(1)\view_x = 0
NN_locations(1)\view_y = 0
NN_locations(1)\view_width = 320
NN_locations(1)\view_x = 200
NN_locations(1)\offset_x = 0
NN_locations(1)\offset_y = 0
NN_locations(1)\num_regions = 2
NN_locations(1)\regions[0] = 2      ; Breaker
NN_locations(1)\regions[1] = 3      ; Exit to screen 1

Dim NN_regions.Region(4)
; Light Switch
NN_regions(0)\x = 100
NN_regions(0)\y = 100
NN_regions(0)\width = 16
NN_regions(0)\height = 16
NN_regions(0)\has_shape = True
NN_regions(0)\shape_id = 0
NN_regions(0)\script_id = 0

; Exit to screen 2
NN_regions(1)\x = 300
NN_regions(1)\y = 0
NN_regions(1)\width = 20
NN_regions(1)\height = 200
NN_regions(1)\has_shape = False
NN_regions(1)\shape_id = 0
NN_regions(1)\script_id = 1

; Breaker
NN_regions(0)\x = (320 + 32) / 2
NN_regions(0)\y = (200 + 40) / 2
NN_regions(0)\width = 32
NN_regions(0)\height = 40
NN_regions(0)\has_shape = True
NN_regions(0)\shape_id = 1
NN_regions(0)\script_id = 2

; Exit to screen 2
NN_regions(1)\x = 0
NN_regions(1)\y = 0
NN_regions(1)\width = 20
NN_regions(1)\height = 200
NN_regions(1)\has_shape = False
NN_regions(1)\shape_id = 0
NN_regions(1)\script_id = 3






