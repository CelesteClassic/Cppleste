#include "Searcheline.h"
#include "CelesteUtils.h"
#include "Carts/Celeste.h"

#include <cmath>

using namespace std;


class Search2100: public Searcheline<>{
    //initial state to search from
    objlist& init_state() override{
        utils::load_room(p8,20); //load 2100m
        utils::supress_object<Celeste::balloon>(p8); //don't consider balloons
        utils::skip_player_spawn(p8); //skip to after the player has spawned
        return p8.game().objects;
    }
    //get list of available inputs for a state - only consider {r, r + z, u + r + x}
    vector<int> allowable_actions(const objlist& objs, Celeste::player& player, bool h_movement, bool can_jump, bool can_dash) override{
        vector<int> actions{0b000010}; //r
        if (can_jump){
            actions.push_back(0b010010); //r + z
        }
        if(can_dash){
            actions.push_back(0b100110);//u + r + x
        }
        return actions;
    }
    //from the input restrictions, we won't exit off of a dash- the max y displacement is 4 px off the spring
    int exit_heuristic(const Celeste::player& player) override {
        int exit_spd_y=4;
        return ceil((player.y + 4) / exit_spd_y);
    }
};

int main(){
    //search up to depth 40 completely (i.e., don't stop after reaching the optimal depth)
    Search2100 s;
    vector<vector<int>> solutions=s.search(40,true);

    //translate fastest solution to english and print
    cout<<"inputs: "<<s.inputs_to_english(solutions[0]);
}
