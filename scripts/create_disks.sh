ECHO Making Disk 1
lha a -rq RAM:NN1.lha data/title.iff
lha a -rq RAM:NN1.lha data/blank.iff
lha a -rq RAM:NN1.lha data/locations/loc_alleyway1.iff
lha a -rq RAM:NN1.lha data/locations/loc_alley_alleycat.iff
lha a -rq RAM:NN1.lha data/locations/loc_alleyway3.iff
lha a -rq RAM:NN1.lha data/locations/loc_alleyway_atm.iff
lha a -rq RAM:NN1.lha data/locations/loc_subway_entrance.iff
lha a -rq RAM:NN1.lha data/locations/loc_subway_interior.iff
lha a -rq RAM:NN1.lha data/locations/loc_alleycat_bar.iff
lha a -rq RAM:NN1.lha data/locations/loc_alleycat_entrance.iff
lha a -rq RAM:NN1.lha data/locations/loc_apartment_bedroom.iff
lha a -rq RAM:NN1.lha data/locations/loc_apartment_interior2.iff
lha a -rq RAM:NN1.lha data/locations/loc_blake_apartment.iff
lha a -rq RAM:NN1.lha data/locations/loc_laundry_exterior.iff
lha a -rq RAM:NN1.lha data/locations/loc_laundry_interior.iff
lha a -rq RAM:NN1.lha data/locations/loc_laundry_interior_fight.iff
lha a -rq RAM:NN1.lha data/locations/loc_b19_entrance.iff
lha a -rq RAM:NN1.lha data/locations/loc_b19_office.iff
lha a -rq RAM:NN1.lha data/locations/loc_b19_office2.iff
lha a -rq RAM:NN1.lha data/locations/loc_b19_storage.iff

ECHO Making Disk 2
lha a -rq RAM:NN2.lha data/locations/loc_b19_catwalk.iff
lha a -rq RAM:NN2.lha data/locations/loc_segs_place.iff
lha a -rq RAM:NN2.lha data/locations/loc_xantech_entrance.iff
lha a -rq RAM:NN2.lha data/locations/loc_xantech_lobby.iff
lha a -rq RAM:NN2.lha data/locations/loc_xantech_medical.iff
lha a -rq RAM:NN2.lha data/locations/loc_xantech_cafe.iff
lha a -rq RAM:NN2.lha data/locations/loc_xantech_corridor.iff
lha a -rq RAM:NN2.lha data/locations/loc_xantech_servers.iff
lha a -rq RAM:NN2.lha data/cutscenes/intro_01.iff
lha a -rq RAM:NN2.lha data/cutscenes/intro_02.iff
lha a -rq RAM:NN2.lha data/cutscenes/intro_03.iff
lha a -rq RAM:NN2.lha data/cutscenes/intro_04.iff
lha a -rq RAM:NN2.lha data/cutscenes/end_act_2.iff
lha a -rq RAM:NN2.lha data/cutscenes/end_act_1.iff
lha a -rq RAM:NN2.lha data/cutscenes/danni1.iff
lha a -rq RAM:NN2.lha data/cutscenes/danni2.iff
lha a -rq RAM:NN2.lha data/cutscenes/danni3.iff
lha a -rq RAM:NN2.lha data/cutscenes/b19.iff
lha a -rq RAM:NN2.lha data/cutscenes/xancon1.iff
lha a -rq RAM:NN2.lha data/cutscenes/xancon2.iff
lha a -rq RAM:NN2.lha data/cutscenes/xancon3.iff
lha a -rq RAM:NN2.lha data/cutscenes/end.iff
lha a -rq RAM:NN2.lha data/display.font
lha a -rq RAM:NN2.lha data/frame.shape
lha a -rq RAM:NN2.lha data/pointers.shape
lha a -rq RAM:NN2.lha data/gutter.neon
lha a -rq RAM:NN2.lha data/mpg.iff
lha a -rq RAM:NN2.lha data/lang_selection.iff

ECHO Making Disk 3
lha a -rq RAM:NN3.lha data/music/theme.mod
lha a -rq RAM:NN3.lha data/music/ambient.mod
lha a -rq RAM:NN3.lha data/music/dance.mod
lha a -rq RAM:NN3.lha data/music/intrigue.mod
lha a -rq RAM:NN3.lha data/music/boss_intro.mod
lha a -rq RAM:NN3.lha data/music/boss_theme.mod
lha a -rq RAM:NN3.lha data/music/detective.mod
lha a -rq RAM:NN3.lha data/music/crystal.mod
lha a -rq RAM:NN3.lha data/music/to_industry.mod

ECHO Making Disk 4
lha a -rq RAM:NN4.lha NEONnoir
lha a -rq RAM:NN4.lha NEONnoir.info
lha a -rq RAM:NN4.lha data/flags.shapes
lha a -rq RAM:NN4.lha data/menu.shapes
lha a -rq RAM:NN4.lha data/people.mpsh
lha a -rq RAM:NN4.lha data/ui.pal
lha a -rq RAM:NN4.lha data/lang/en.noir
lha a -rq RAM:NN4.lha data/lang/latin-1.shapes

COPY RAM:NN1.lha DF0: FORCE
COPY RAM:NN2.lha DF1: FORCE
COPY RAM:NN3.lha DF2: FORCE
COPY RAM:NN4.lha DF3: FORCE
