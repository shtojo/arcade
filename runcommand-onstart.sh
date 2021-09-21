
# This looks for a file: /home/pi/led/config
# and uses the content to determine which leds to light for each rom
# The expected format of that file is:
# <emulator name> <rom name without extension> <leds to light>
# example:   arcade 1943 AB
# One entry per line, if rom is not found then no leds will be lit
# The <led> options are: [ABXYLR] or 0
# arcade name and leds are case in-sensitive, file name is case sensitive

file="/home/pi/led/config"

leds="0"  # default to all off if game not found
if [ -f "$file" ]; then
    sys=$1
    rom="${3##*/}"      # get filename (1943.zip)
    rom="${rom%.*}"     # remove .zip (1943)

    while IFS= read -r line || [ -n "$line" ]; do
        # example file line:  arcade:1943 AB

        line="${line%%#*}"     # remove comments
        line="$(echo "$line" | tr -s " ")"    # compress whitespace to single space
        line=${line%% }       # remove leading space
        line=${line## }       # remove trailing space

        sysx="${line%% *}"    # sysx = arcade
        romx="${line% *}"     # romx = 1943 AB
        romx="${romx#* }"     # romx = 1943
        sysx="${sysx,,}"      # sysx tolower

        if [ "$sysx" = "$sys" ]; then
            if [ "$romx" = "$rom" ] || [ "$romx" = "all" ]; then
                leds="${line##* }"   # get the led setting (ex AB)
                leds="${leds^^}"     # leds toupper
                break
            fi
        fi
    done < "$file"
fi

sudo /home/pi/led/led -l "$leds"
