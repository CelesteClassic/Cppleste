//include PICO-8 emulator and Celeste
#include "PICO8.h"
#include "Carts/Celeste.h"

//useful celeste utils
#include "CelesteUtils.h"

#include <iostream>
#include <string>

int main(){
    //create a PICO8 instance with Celeste loaded
    PICO8<Celeste> p8;

    //swap 100m with this level and reload it
    std::string room_data=
    "w w w w w w w w w w . . . . w w"
    "w w w w w w w w w . . . . . < w"
    "w w w v v v v . . . . . . . < w"
    "w w > . . . . . . . . . . . . ."
    "w > . . . . . . . . . . . . . ."
    ". . . . . . . . . . . . . . . ."
    ". . . . . . . . . . . . . . . ."
    ". . . . . . . . . . . . . . . ."
    ". . . . . . . . . b . . . b . ."
    ". . . . . . . . . . . . . . . ."
    ". . . . . . . . . . . . . . . ."
    ". . . . ^ . . . . . . . . . . ."
    ". . . . w > . . . . . . . . . ."
    ". . . . w > . . . . . . . . . ."
    ". . . . w > . . p . . . . . . ."
    "w w w w w w w w w w w w w w w w";

    utils::replace_room<Celeste>(p8,0,room_data);
    utils::load_room(p8,0);

    //skip the player spawn
    utils::skip_player_spawn(p8);

    // view the room
    std::cout<<p8.game();

    //hold right+x
    p8.set_inputs(false,true,false,false,false,true);

    //run for 20f while outputting player info
    for(int i=0; i<20; i++){
        p8.step();
        std::cout<<*p8.game().get_player()<<std::endl;
    }
}

