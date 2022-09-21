#ifndef CPPLESTE_THREADEDSEARCHELINE_H
#define CPPLESTE_THREADEDSEARCHELINE_H

#include <vector>
#include "PICO8.h"
#include "Carts/Celeste.h"
#include "CelesteUtils.h"
#include "Searcheline.h"
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
#include <functional>

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

    bool work(){
        while(true){

            std::unique_lock<std::mutex> lk(var_lock);

            waiting_count++;
            cv.notify_all();

            cv.wait(lk, [this]{return !this->state_queue.empty() || waiting_count==worker_num;});

            if(!state_queue.empty()){
                waiting_count--;

                objlist state=std::move(std::get<0>(state_queue.front()));
                int depth=std::get<1>(state_queue.front());
                std::vector<int> inputs=std::get<2>(state_queue.front());
                state_queue.pop();

                lk.unlock();
                for(auto& obj: state){
                    obj->p8=std::ref(this->p8);
                    obj->g=std::ref(this->p8.game());
                }

                ret |= iddfs(state,depth,inputs);
            }
            else if(waiting_count==worker_num){
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

                    if(waiting_count==0){
                        lock.unlock();
                        cv.notify_one();
                        bool done = iddfs(new_state, depth - 1 - freeze, inputs);

                        if (done) {
                            optimal_depth = true;
                        }
                    }
                    else{
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
        std::vector<std::unique_ptr<workerType>> workers;

        std::mutex var_lock;
        std::atomic<int> waiting_count;
        std::queue<std::tuple<objlist, int, std::vector<int>>> state_queue;
        std::condition_variable cv;

        for(int i=0; i<worker_count; i++){
            workers.emplace_back(std::make_unique<workerType>(var_lock, waiting_count, state_queue, worker_count,cv,i));

            if(i!=0){
                workers[i]->init_state();
            }
        }
        objlist state = Searcheline<Cart>::deepcopy(workers[0]->init_state());

        auto t1 = std::chrono::high_resolution_clock::now();
        std::cout << "searching..." << std::endl;

        for (int depth = 0; depth <= max_depth; depth++) {
            std::cout << "depth " << depth << "..." << std::endl;
            std::vector<int> inputs;

            std::vector<std::thread> threads;

            waiting_count=0;
            state_queue.emplace(Searcheline<Cart>::deepcopy(state),depth,inputs);

            for(auto &w: workers){
                threads.emplace_back(&workerType::work, w.get());
            }



            for(auto &t: threads){
                t.join();
            }

            bool done = false;
            for (auto &w: workers){
                if(w->ret){
                    done=true;
                }
                this->solutions.insert(this->solutions.end(),w->solutions.begin(),w->solutions.end());
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
