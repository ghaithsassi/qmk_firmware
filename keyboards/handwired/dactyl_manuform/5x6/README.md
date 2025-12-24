# SETUP

qmk setup

# COMPILE

qmk compile -kb handwired/dactyl_manuform/5x6 -km default


# Flash

qmk flash handwired_dactyl_manuform_5x6_default.hex -bl avrdude-split-left 
