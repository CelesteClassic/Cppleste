//
// Created by gonengazit on 02/07/2020.
//

#ifndef CPPLESTE_SEARCHELINE_H
#define CPPLESTE_SEARCHELINE_H

#include <vector>
#include "PICO8.h"
#include "Carts/Celeste.h"
#include "CelesteUtils.h"
#include <tuple>
#include <ctime>
#include <iostream>
#include <iomanip>
#include <chrono>
#include <thread>

int count = 0;

template<typename Cart=Celeste>
class Searcheline {
protected:
    using objlist = std::list<std::unique_ptr<typename Cart::base_obj>>;
    PICO8<Cart> p8;
    std::vector<std::vector<int>> solutions;
public:
    explicit Searcheline() {
        utils::enable_loop_mode(p8);
    };

    virtual objlist& init_state() = 0;

    virtual std::vector<int>
    allowable_actions(const objlist &objs, typename Cart::player &player, bool h_movement, bool can_jump,
                      bool can_dash) {
        std::vector<int> actions{0b000000};
        if (h_movement) {
            actions.insert(actions.end(), {0b000001, 0b000010});
        }
        if (can_jump) {
            actions.insert(actions.end(), {0b010000});
            if (h_movement) {
                actions.insert(actions.end(), {0b010001, 0b010010});
            }
        }
        if (can_dash) {
            actions.insert(actions.end(),
                           {0b100000, 0b100001, 0b100010, 0b100100, 0b100101, 0b100110, 0b101000, 0b101001, 0b101010});
        }
        return actions;
    }

    virtual double h_cost(const objlist &objs) {
        if (is_rip(objs)) {
            return HUGE_VAL;
        }
        else {
            return exit_heuristic(*find_player(objs));
        }
    }

    virtual bool is_rip(const objlist &objs) {
        return find_player(objs) == nullptr;
    }

    virtual int exit_heuristic(const typename Cart::player &player) {
        double exit_spd_y = 6;
        return ceil((player.y + 4) / exit_spd_y);
    }

    virtual bool is_goal(const objlist &objs) {
        return find_player_spawn(objs) != nullptr;
    }

    virtual std::vector<int> get_actions(const objlist &objs) {
        typename Cart::player *p = find_player(objs);
        if (p == nullptr) {
            return {};
        }
        if (p->dash_time != 0) {
            return {0b0000000};
        }
        bool h_movement, can_jump, can_dash;
        std::tie(h_movement, can_jump, can_dash) = action_restrictions(objs, *p);
        return allowable_actions(objs, *p, h_movement, can_jump, can_dash);

    }

    static objlist deepcopy(const objlist &objs) {
        objlist cp;
        for (auto &i: objs) {
            cp.emplace_back(dynamic_cast<typename Cart::base_obj *>(i->clone()));
        }
        return cp;
    }

    std::tuple<const objlist &, int> transition(const objlist &objs, int a) {
        p8.game().objects = deepcopy(objs);
        p8.set_btn_state(a);
        p8.step();
        int freeze = p8.game().freeze;
        p8.game().freeze = 0;
        p8.game().delay_restart = 0;
        return std::forward_as_tuple(p8.game().objects, freeze);
    }

    bool iddfs(const objlist &state, int depth, std::vector<int> inputs) {
        //std::cout<<"in";
        if (depth == 0 && is_goal(state)) {
            solutions.push_back(inputs);
            std::cout << "  inputs: ";
            for (auto i: inputs) {
                std::cout << i << ", ";
            }
            std::cout << std::endl;
            std::cout << "  frames: " << inputs.size() - 1 << std::endl;
            return true;
        }

        else {
//            std::cout<<"  inputs: ";
//            for(auto i: inputs){
//                std::cout<<i<<", ";
//            }
//            std::cout<<std::endl;
            bool optimal_depth = false;
            if (depth > 0 && h_cost(state) <= depth) {
                for (auto a:get_actions(state)) {
                    std::tuple<const objlist &, int> trans = transition(state, a);
                    objlist new_state = deepcopy(std::get<0>(trans));
                    int freeze = std::get<1>(trans);
                    // change: uses current array instead of allocating a new one
                    // should be better performance wise, will probably check
                    inputs.push_back(a);
                    for (int i = 0; i < freeze; i++) {
                        inputs.push_back(0);
                    }
                    bool done = iddfs(new_state, depth - 1 - freeze, inputs);
                    if (done) {
                        optimal_depth = true;
                    }


                    for (int i = 0; i < freeze; i++) {
                        inputs.pop_back();
                    }
                    inputs.pop_back();
                }
            }
            //std::cout<<count<<std::endl;
            return optimal_depth;
        }
    }

    std::vector<std::vector<int>> search(int max_depth, bool complete = false) {
        solutions = std::vector<std::vector<int>>();
        auto t1 = std::chrono::high_resolution_clock::now();
        objlist state = deepcopy(init_state());
        std::cout << "searching..." << std::endl;
        for (int depth = 0; depth <= max_depth; depth++) {
            std::cout << "depth " << depth << "..." << std::endl;
            std::vector<int> inputs;
            bool done = iddfs(state, depth, inputs) && !complete;
            auto t2 = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double> elapsed_time = t2 - t1;
            std::cout << "  elapsed time: " << std::fixed << std::setprecision(2) << (elapsed_time.count()) << " [s]"
                      << std::endl;
            if (done) {
                break;
            }
        }
        return solutions;
    }

    typename Cart::player *find_player(const objlist &objs) {
        for (auto &o: objs) {
            auto p = dynamic_cast<typename Cart::player *>(o.get());
            if (p != nullptr) {
                return p;
            }
        }
        return nullptr;
    }

    typename Cart::player_spawn *find_player_spawn(const objlist &objs) {
        for (auto &o: objs) {
            auto p = dynamic_cast<typename Cart::player_spawn *>(o.get());
            if (p != nullptr) {
                return p;
            }
        }
        return nullptr;
    }

    double sign(double x) {
        return (x > 0) - (x < 0);
    }

    std::pair<int, int> compute_displacement(typename Cart::player &player) {
        int dx = std::round(player.rem.x + player.spd.x);
        dx += sign(dx);
        int dy = std::round(player.rem.y + player.spd.y);
        dy += sign(dy);
        while (player.is_solid(dx, 0)) {
            dx -= sign(player.spd.x);
        }
        while (player.is_solid(dx, dy)) {
            dy -= sign(player.spd.y);
        }
        return std::make_pair(dx, dy);
    }

    virtual std::tuple<bool, bool, bool> action_restrictions(const objlist &objs, typename Cart::player &player) {
        int dx, dy;
        std::tie(dx, dy) = compute_displacement(player);
        bool h_movement = std::abs(player.spd.x) <= 1;
        bool can_jump = !player.p_jump &&
                        (player.grace - 1 > 0 || player.is_solid(-3 + dx, dy) || player.is_solid(3 + dx, dy) ||
                         player.is_solid(dx, 1 + dy));
        bool can_dash = player.djump > 0 || player.is_solid(dx, 1 + dy)
                        || player.template collide<typename Cart::balloon>(0, 0) ||
                        player.template collide<typename Cart::fruit>(0, 0) ||
                        player.template collide<typename Cart::fly_fruit>(0, 0);
        return std::make_tuple(h_movement, can_jump, can_dash);
    }

    std::string inputs_to_english(std::vector<int> inputs) {
        std::map<int, std::string> action_dict = {
                {0,  "no input"},
                {1,  "left"},
                {2,  "right"},
                {16, "neutral jump"},
                {17, "jump left"},
                {18, "jump right"},
                {32, "empty dash"},
                {33, "left dash"},
                {34, "right dash"},
                {36, "up dash"},
                {37, "up-left dash"},
                {38, "up-right dash"},
                {40, "down dash"},
                {41, "down-left dash"},
                {42, "down-right dash"}
        };
        std::string ret="";
        for(auto o:inputs){
            ret+=action_dict[o]+", ";
        }
        return ret;
    }
};


class SearchelineWorker;

class ThreadedSearcheline{

    int worker_count;
    queue<tuple<const objlist &, int, std::vector<int>>> state_queue;


    std::vector<std::vector<int>> search(int max_depth, bool complete = false) {
        std::vector<SearchelineWorker> workers(active_count);

        auto t1 = std::chrono::high_resolution_clock::now();
        objlist state = deepcopy(init_state());
        std::cout << "searching..." << std::endl;

        for (int depth = 0; depth <= max_depth; depth++) {
            std::cout << "depth " << depth << "..." << std::endl;
            std::vector<int> inputs;

            vector<thread> threads;

            int active_count=0;
            for(int i=0; i<worker_count; i++){
                threads.emplace_back(&SearchelineWorker::work, workers[i]);
            }

            done = false;


            auto t2 = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double> elapsed_time = t2 - t1;
            std::cout << "  elapsed time: " << std::fixed << std::setprecision(2) << (elapsed_time.count()) << " [s]"
                      << std::endl;
            if (done) {
                break;
            }
        }
        return solutions;
}
class SearchelineWorker: Searcheline{
    SearchelineWorker();

    mutex& var_lock;
    bool &done;
    bool ret=false;
    int &waiting_count;
    queue<tuple<const objlist &, int, std::vector<int>>> state_queue;
    
    void work(){
        while(true){
            tuple<const objlist &, int, std::vector<int>> next_state;
            bool not_empty=false;
            {
                lock_guard<mutex> guard(var_lock);
                if(done){
                    return ret;
                }
                else if(!state_queue.empty()){
                    not_empty=true;
                    next_state=state_queue.front();
                    state_queue.pop();
                }
            }
            if (not_empty){
                const objlist &state;
                int depth;
                std::vector<int> inputs;

                {state,depth,inputs}=tie(next_state);
                ret |= iddfs(state,depth,inputs);
            }
        }
    }
}


#endif //CPPLESTE_SEARCHELINE_H
