#include "Searcheline.h"
#include "CelesteUtils.h"
#include "Carts/Celeste.h"

using namespace std;


class Search100: public SearchelineWorker<>{
    template <typename, typename> friend class ThreadedSearcheline;

    using SearchelineWorker<>::SearchelineWorker;
    // Search100(std::mutex& var_lock, int& waiting_count, std::queue<std::tuple<const objlist, int, std::vector<int>>> &state_queue, int worker_num):
    //     SearchelineWorker(var_lock, waiting_count,  state_queue, worker_num)
    // {}


    //initial state to search from
    objlist& init_state() override {
        utils::load_room(p8, 0); //load 100m
        utils::supress_object<Celeste::fake_wall>(p8); //don't consider berry block
        utils::skip_player_spawn(p8); //skip to after the player has spawned
        //execute list of initial inputs
        for (auto a:{18, 2, 2, 2, 2, 2, 2, 2, 2, 2, 34, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}) {
            p8.set_btn_state(a);
            p8.step();
        }
        //alternatively using output from a TAS tool:
        //utils::place_maddy(p8, 55, 79, 0.2, 0.185, 1.4, 0.63, 0, 0);
        return p8.game().objects;
    }
    // get list of available inputs for a state - only consider {r, r + z, u + r + x}
    vector<int> allowable_actions(const objlist& objs, Celeste::player& player, bool h_movement, bool can_jump, bool can_dash) override{
        vector<int> actions{0b000010}; //r
        if (can_jump){
            actions.push_back(0b010010); //r + z
        }
        if(can_dash){
            actions.insert(actions.end(),{0b100010, 0b100100, 0b100110});//r + x, u + x, u + r + x
        }
        return actions;
    }
};

int main(){
    // search up to depth 50, but stop at the depth of the first solution found
    ThreadedSearcheline<Search100> s(2); //use 2 threads
    vector<vector<int>> solutions=s.search(50);
}
