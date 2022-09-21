//
// Created by gonengazit on 01/07/2020.
//

#ifndef CPPLESTE_CELESTEUTILS_H
#define CPPLESTE_CELESTEUTILS_H
#include "PICO8.h"
#include <map>
#include <string>
#include <algorithm>
#include <iostream>
#include <chrono>
#include <thread>
#include <vector>

namespace utils {
    template<typename Cart>
    void enable_loop_mode(PICO8<Cart> &p8){
        p8.game().loop_mode=true;
    }

    template<typename Cart>
    void set_max_djump(PICO8<Cart> &p8, int max_djump) {
        p8.game().max_djump = max_djump;
    }

    template<typename Cart>
    void load_room(PICO8<Cart> &p8, int level_id, bool loading_jank = false) {
        p8.set_btn_state(0);
        p8.game().load_room(level_id % 8, level_id / 8);
        if (loading_jank && level_id > 0) {
            int object_counts[] = {2, 1, 4, 14, 3, 2, 12, 9, 6, 5, 7, 3, 6, 5, 11, 8, 4, 7, 3, 6, 8, 2, 2, 1, 8, 3, 3,
                                   7, 6,
                                   7, 2};
            int cnt = 0;
            int start = object_counts[level_id - 1] - 1;
            int end = object_counts[level_id];
            for (auto &o:p8.game().objects) {
                if (cnt >= end) {
                    break;
                }
                if (cnt < start) {
                    continue;
                }
                o->move(o->spd.x, o->spd.y);
                o->update();
            }
        }
    }

    template<typename obj, typename Cart>
    void supress_object(PICO8<Cart> &p8) {
        auto &objects = p8.game().objects;
        auto it = objects.begin();
        while (it != objects.end()) {
            auto nxt = next(it);
            if (it->get() && it->get()->type_id == obj::type_enum) {
                objects.erase(it);
            }
            it = nxt;
        }
    }

    template<typename Cart>
    void skip_player_spawn(PICO8<Cart> &p8) {
        typename Cart::base_obj *p;
        while (!(p=p8.game().get_player()) || p->type_id != Cart::player::type_enum) {

            p8.step();
        }
    }

    template<typename Cart>
    void replace_room(PICO8<Cart> &p8, int level_id, std::string room_data) {
        std::map<char, int> tiles = {
                {'w', 32}, // terrain
                {'^', 17}, // up spike
                {'v', 27}, // down spike
                {'<', 59}, // left spike
                {'>', 43}, // right spike
                {'b', 22}, // balloon
                {'c', 23}, // crumble block
                {'s', 18}, // spring
                {'p', 1},  // player spawn
                {'.', 0}   // empty
        };
        room_data.erase(std::remove_if(room_data.begin(),room_data.end(),isspace),room_data.end());
        int rx = level_id % 8;
        int ry = level_id / 8;
        for (int tx = 0; tx < 16; tx++) {
            for (int ty = 0; ty < 16; ty++) {
                char tile = room_data[tx + 16 * ty];
                p8.mset(rx * 16 + tx, ry * 16 + ty, tiles[tile]); // if tile doesn't exist tiles[tile] will return 0
            }
        }
    }

    template<typename Cart>
    void
    place_maddy(PICO8<Cart> &p8, int x, int y, double remx=0, double remy=0, double spdx=0, double spdy=0, int grace=6,
                int djump=1) {
        auto *p = p8.game().get_player();
        if(p!= nullptr) {
            for (auto it = p8.game().objects.begin(); it != p8.game().objects.end(); it++) {
                if (p == it->get()) {
                    p8.game().objects.erase(it);
                    break;
                }
            }
        }
        auto &p2 = dynamic_cast<typename Cart::player &>(p8.game().template init_object<typename Cart::player>(x, y));
        p2.rem.x = remx;
        p2.rem.y = remy;
        p2.spd.x = spdx;
        p2.spd.y = spdy;
        p2.grace = grace;
        p2.djump = djump;
    }

    template<typename Cart>
    void watch_inputs(PICO8<Cart> &p8, const std::vector<int>& inputs){
        std::cout<<p8.input_display()<<std::endl;
        std::cout<<p8.game()<<std::endl;
        for(auto a:inputs) {
            std::this_thread::sleep_for(std::chrono::milliseconds(1000/30));
            p8.set_btn_state(a);
            p8.step();
            std::cout<<p8.input_display()<<std::endl;
            std::cout<<p8.game()<<std::endl;
        }
    }
}
#endif //CPPLESTE_CELESTEUTILS_H
