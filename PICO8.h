//
// Created by gonengazit on 01/07/2020.
//

#ifndef CPPLESTE_PICO8_H
#define CPPLESTE_PICO8_H
#include <string>
#include <iostream>
using std::string;
template<typename cart>
class PICO8 {
    unsigned int btn_state;
    cart _game;
    int map[8192];
    int flags[256];
public:
    PICO8():_game(*this){
       btn_state=0;
       load_game();
    }
    void load_game(){
        //_game=cart(*this);
        //change: load_game doesn't reset the cart
        //TODO: fix this
        for(int i=0; i<8192; i+=2){
            map[i/2]=std::strtol((string()+_game.map_data[i]+_game.map_data[i+1]).c_str(),nullptr,16);
        }
        for(int i=8192; i<16384; i+=2){
            map[i/2]=std::strtol((string()+_game.map_data[i+1]+_game.map_data[i]).c_str(),nullptr,16);
        }
        for(int i=0; i<512; i+=2){
            flags[i/2]=std::strtol((string()+_game.flag_data[i]+_game.flag_data[i+1]).c_str(),nullptr,16);
        }
        _game._init();
    }
    void reset(){
        load_game();
    }
    bool btn(unsigned int i) const{
        return (btn_state&(1<<i))!=0;
    }
    void mset(int x, int y, int tile){
        map[x+y*128]=tile;
    }
    int mget(int x, int y) const{
        return map[x+y*128];
    }
    int fget(int n, int f=-1) const{
        int fl=flags[n];
        if(f==-1){
            return fl;
        }
        return (fl&(1<<f))!=0;

    }
    void step(){
        _game._update();
        _game._draw();
    }
    void set_inputs(bool l=false,bool r=false,bool u=false,bool d=false,bool z=false,bool x=false){
        set_btn_state(l*1+r*2+u*4+d*8+z*16+x*32);
    }
    void set_btn_state(unsigned int state){
        btn_state=state;
    }
    cart& game(){
        return _game;
    }
    string input_display() const{
       string l=btn(0)?"▓▓":"░░";
       string r=btn(1)?"▓▓":"░░";
       string u=btn(2)?"▓▓":"░░";
       string d=btn(3)?"▓▓":"░░";
       string z=btn(4)?"▓▓":"░░";
       string x=btn(5)?"▓▓":"░░";
       return "        "+u+'\n'+z+x+"  "+l+d+r;
    }
};


#endif //CPPLESTE_PICO8_H
