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
#include <mutex>
#include <condition_variable>
#include <queue>
#include <atomic>

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


template<typename Cart=Celeste>
class SearchelineWorker: public Searcheline<Cart>{

    template <typename, typename> friend class ThreadedSearcheline;

public:
    using objlist=typename Searcheline<Cart>::objlist;
    std::mutex& var_lock;
    bool ret;
    std::atomic<int> &waiting_count;
    std::queue<std::tuple<objlist , int, std::vector<int>>> &state_queue;
    const int worker_num;
    std::condition_variable &cv;
    int id;

    SearchelineWorker(std::mutex& var_lock, std::atomic<int>& waiting_count, std::queue<std::tuple<objlist, int, std::vector<int>>> &state_queue, int worker_num, std::condition_variable& cv, int id):
        var_lock(var_lock),
        waiting_count(waiting_count),
        state_queue(state_queue),
        worker_num(worker_num),
        ret(false),
        cv(cv),
        id(id){}

    SearchelineWorker(const SearchelineWorker&)=delete;
    SearchelineWorker(SearchelineWorker&& other):
        var_lock(other.var_lock),
        waiting_count(other.waiting_count),
        state_queue(other.state_queue),
        worker_num(other.worker_num),
        ret(other.ret),
        cv(other.cv),
        id(other.id){}

    bool work(){
        while(true){

            std::unique_lock<std::mutex> lk(var_lock);

            std::cout<<id<<" waiting "<<std::endl;
            waiting_count++;
            cv.notify_all();
            cv.wait(lk, [this]{std::cout<<id<<" checking condition"<<std::endl;return !this->state_queue.empty() || waiting_count==worker_num;});
            std::cout<<id<<" releasing "<<std::endl;

            if(!state_queue.empty()){
                waiting_count--;

                objlist state=std::move(std::get<0>(state_queue.front())); // i'd prefer to use move() here, but cpp is being a bitch
                int depth=std::get<1>(state_queue.front());
                std::vector<int> inputs=std::get<2>(state_queue.front());
                state_queue.pop();

                lk.unlock();

                ret |= iddfs(state,depth,inputs);
            }
            else if(waiting_count==worker_num){
                std::cout<<id<<" exiting"<<std::endl;
                return ret;
            }
        }
    }

    bool iddfs(const objlist &state, int depth, std::vector<int> inputs) {
        if (depth == 0 && this->is_goal(state)) {
            std::lock_guard<std::mutex> lock(var_lock);
            this->solutions.push_back(inputs);
            std::cout << "  inputs: ";
            for (auto i: inputs) {
                std::cout << i << ", ";
            }
            std::cout << std::endl;
            std::cout << "  frames: " << inputs.size() - 1 << std::endl;
            return true;
        }

        else {

            bool optimal_depth = false;
            if (depth > 0 && this->h_cost(state) <= depth) {
                for (auto a:this->get_actions(state)) {
                    std::tuple<const objlist &, int> trans = this->transition(state, a);
                    objlist new_state = this->deepcopy(std::get<0>(trans));
                    int freeze = std::get<1>(trans);
                    // change: uses current array instead of allocating a new one
                    // should be better performance wise, will probably check
                    inputs.push_back(a);
                    for (int i = 0; i < freeze; i++) {
                        inputs.push_back(0);
                    }

                    std::unique_lock<std::mutex> lock(var_lock);
                    lock.lock();

                    if(waiting_count==0){
                        std::cout<<id<<" recursing"<<std::endl;
                        lock.unlock();
                        cv.notify_one();
                        bool done = iddfs(new_state, depth - 1 - freeze, inputs);

                        if (done) {
                            optimal_depth = true;
                        }
                    }
                    else{
                        std::cout<<id<<" pushing"<<std::endl;
                        state_queue.emplace(move(new_state),depth-1-freeze,inputs);
                        lock.unlock();
                        cv.notify_one();
                    }

                    for (int i = 0; i < freeze; i++) {
                        inputs.pop_back();
                    }
                    inputs.pop_back();
                }
            }
            //std::cout<<count<<std::endl;

            std::cout<<id<<" exiting iddfs"<<std::endl;
            return optimal_depth;
        }
    }
};

template<typename workerType, typename Cart=Celeste>
class ThreadedSearcheline {

protected:
    int worker_count;

public:
    ThreadedSearcheline(int worker_count): worker_count(worker_count){}

    using objlist=typename workerType::objlist;
    std::vector<std::vector<int>> solutions;
    std::vector<std::vector<int>> search(int max_depth, bool complete = false) {
        std::vector<workerType> workers;

        std::mutex var_lock;
        std::atomic<int> waiting_count;
        std::queue<std::tuple<objlist, int, std::vector<int>>> state_queue;
        std::condition_variable cv;

        for(int i=0; i<worker_count; i++){
            workers.emplace_back(var_lock, waiting_count, state_queue, worker_count,cv,i);

            if(i!=0){
                workers[i].init_state();
            }
        }
        objlist state = Searcheline<Cart>::deepcopy(workers[0].init_state());

        auto t1 = std::chrono::high_resolution_clock::now();
        std::cout << "searching..." << std::endl;

        for (int depth = 0; depth <= max_depth; depth++) {
            std::cout << "depth " << depth << "..." << std::endl;
            std::vector<int> inputs;

            std::vector<std::thread> threads;

            waiting_count=0;
            state_queue.emplace(Searcheline<Cart>::deepcopy(state),depth,inputs);

            for(auto &w: workers){
                threads.emplace_back(&workerType::work, &w);
            }



            for(auto &t: threads){
                t.join();
            }

            bool done = false;
            for (auto &w: workers){
                if(w.ret){
                    done=true;
                }
                this->solutions.insert(this->solutions.end(),w.solutions.begin(),w.solutions.end());
            }
            done = done && !complete;

            auto t2 = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double> elapsed_time = t2 - t1;
            std::cout << "  elapsed time: " << std::fixed << std::setprecision(2) << (elapsed_time.count()) << " [s]"
                      << std::endl;
            if (done) {
                break;
            }
        }
        return this->solutions;
    }
};


#endif //CPPLESTE_SEARCHELINE_H
