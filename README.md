# Contents

* [Cppleste](#cppleste)
* [Searcheline](#searcheline)
  * [Example - 2100m](#example---2100m)
  * [Example - 100m](#example---100m)
* [Running Cppleste](#running-cppleste)
# Cppleste
Performance focused C++ Celeste Classic emulator based on [Pyleste](https://github.com/CelesteClassic/Pyleste). Comes with useful utils (CelesteUtils.py) for setting up and simulating specific situations in both existing and custom-specified levels.

## Sample Usage
```C++
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
```

```
████████████████████        ████
██████████████████           <██
██████vvvvvvvv               <██
████>                           
██>                             
                                
                                
                                
                  ()      ()    
                                
                                
        ^^                      
        ██>                     
        ██>                     
        ██>     :D              
████████████████████████████████
[player] x: 64, y: 112, rem:{0.0000, 0.0000}, spd:{5.0000, 0.0000}
[player] x: 64, y: 112, rem:{0.0000, 0.0000}, spd:{5.0000, 0.0000}
[player] x: 64, y: 112, rem:{0.0000, 0.0000}, spd:{5.0000, 0.0000}
[player] x: 70, y: 112, rem:{0.0000, 0.0000}, spd:{3.5000, 0.0000}
[player] x: 75, y: 112, rem:{-0.5000, 0.0000}, spd:{2.0000, 0.0000}
[player] x: 78, y: 112, rem:{-0.5000, 0.0000}, spd:{2.0000, 0.0000}
[player] x: 81, y: 112, rem:{-0.5000, 0.0000}, spd:{2.0000, 0.0000}
[player] x: 84, y: 112, rem:{-0.5000, 0.0000}, spd:{1.8500, 0.0000}
[player] x: 86, y: 112, rem:{0.3500, 0.0000}, spd:{1.7000, 0.0000}
[player] x: 89, y: 112, rem:{0.0500, 0.0000}, spd:{1.5500, 0.0000}
[player] x: 92, y: 112, rem:{-0.4000, 0.0000}, spd:{1.4000, 0.0000}
[player] x: 94, y: 112, rem:{0.0000, 0.0000}, spd:{1.2500, 0.0000}
[player] x: 96, y: 112, rem:{0.2500, 0.0000}, spd:{1.1000, 0.0000}
[player] x: 98, y: 112, rem:{0.3500, 0.0000}, spd:{1.0000, 0.0000}
[player] x: 100, y: 112, rem:{0.3500, 0.0000}, spd:{1.0000, 0.0000}
[player] x: 102, y: 112, rem:{0.3500, 0.0000}, spd:{1.0000, 0.0000}
[player] x: 104, y: 112, rem:{0.3500, 0.0000}, spd:{1.0000, 0.0000}
[player] x: 106, y: 112, rem:{0.3500, 0.0000}, spd:{1.0000, 0.0000}
[player] x: 108, y: 112, rem:{0.3500, 0.0000}, spd:{1.0000, 0.0000}
[player] x: 110, y: 112, rem:{0.3500, 0.0000}, spd:{1.0000, 0.0000}
```

# Searcheline
An iterative-deepening depth-first-search solver for Celeste Classic, built on Cppleste. 
based on Pyleste's Searcheline

## Usage
To define and run a search problem:

1. Create a class which inherits from `Searcheline<Cart>` (the default cart is celeste)
2. Override the following methods as needed:
    - `objlist& init_state()` **[REQUIRED]**
      - Initial state (list of game objects) to search from
      - e.g., load the room and place maddy in Searcheline's game instance (`this->p8.game()`), return `this->p8.game.objects()`
    - `std::vector<int> allowable_actions(const objlist &objs, typename Cart::player &player, bool h_movement, bool can_jump, bool can_dash)`
      - Get list of available inputs for a state, with the following checks already computed:
        - `h_movement`: `True` if horizontal movement/jumps available (player x speed <= 1)
        - `can_jump`: `True` if jump available (in grace frames, next to wall, didn't jump previous frame)
        - `can_dash`: `True` if dash available (dashes > 0)
      - **Default**: all actions
      - Override this to restrict inputs (e.g., only up-dashes, no directional movement when player's y < 50, etc.)
    - `double h_cost(const objlist& objs)`
      - Estimated number of steps to satisfy the goal condition
      - **Default**: infinity if `is_rip`, `exit_heuristic` otherwise (See below)
      - Override to change or include additional heuristics
      - `bool is_rip(const objlist& objs)`
        - RIP conditions (situations not worth considering further)
        - **Default**: player dies
        - Override to change or include other rip conditions (e.g., don't consider cases where player's x > 64, etc.)
      - `int exit_heuristic(const Cart::player &player)`
        - Underestimated number of steps to exit off the top
        - **Default**: assumes player zips upward at a speed of 6 px/step
        - Override to specify a less conservative estimate (e.g., if exit will be off a jump, can use 3 px/step)
    - `bool is_goal(const objlist& objs)`
      - Define goal conditions
      - **Default**: exited the level
      - Override to change goal conditions (e.g., reach certain coordinates with a dash available)
3. Instantiate the class, and call `instance.search(max_depth)`
    - Use optional argument `complete=True` to search up to `max_depth`, even if a solution has already been found

## Example - 2100m

Here we'll set up a search problem to solve 2100m. Specifically, we'll work with the assumption that the player will be dashing toward the spring, like in the following GIF:

<img src="https://celesteclassic.github.io/gifs/gifs/2100/1.gif">

First include some useful stuff:

```C++
#include "Searcheline.h"
#include "CelesteUtils.h"
#include "Carts/Celeste.h"

#include <cmath>

using namespace std;
```

Create a class which inherits from `Searcheline<>`:

```C++
class Search2100: public Searcheline<>{
```

In this class, override `objlist& init_state()`. Because we're only considering the player dashing toward the spring, we can remove the balloons to speed things up. By default, Searcheline comes with its own PICO-8 instance with a Celeste instance loaded, `this->p8`. We can use CelesteUtils to load 2100m into this PICO-8 instance, suppress the balloons, and skip to after the player has spawned. The function should then return the Celeste instance's list of objects, `this->p8.game().objects`:

```C++
  //initial state to search from
  objlist& init_state() override{
    utils::load_room(p8,20); //load 2100m
    utils::supress_object<Celeste::balloon>(p8); //don't consider balloons
    utils::skip_player_spawn(p8); //skip to after the player has spawned
    return p8.game().objects;
  }
```

Again, assuming the player will dash toward the spring, we only need to consider the options of holding right, jumping while holding right, as well as dashing while holding up and right. To restrict the search to these inputs, we override `vector<int> allowable_actions(const objlist &objs, Celeste::player &player, bool h_movement, bool can_jump, bool can_dash)`, where given a situation (specified by the list of objects), it should build and return a list of inputs to consider. We can make use of the `can_jump` and `can_dash` checks to only consider inputs when they're applicable:

```C++
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
```

By default, Searcheline's exit heuristic assumes that the player zips straight up at a speed of 6 px/step, based on an upward dash moving you at most 6 px in a single step. With prior knowledge that, from how high up the spring is, the player will be exiting off of a spring bounce, we can use a tighter heuristic of the player zipping upward at a speed of 4 px/step. Having a better estimate of the number of steps to exit the level (*while strictly being an underestimate*), we can greatly reduce the search space by pruning situations that provably can't exit within the current search depth. This can be done by overriding `int exit_heuristic(const Celeste::player& player)`:

```C++
  //from the input restrictions, we won't exit off of a dash- the max y displacement is 4 px off the spring
  int exit_heuristic(const Celeste::player& player) {
    int exit_spd_y=4;
    return ceil((player.y + 4) / exit_spd_y);
  }
```

And that's it! We can instantiate this search problem, and use `search(int max_depth, bool complete)` to run the search. In this example, we'll search up to a maximum depth of 40, and set the optional `complete` argument to `True`. This optional argument makes it exhaustively search until the maximum depth, as opposed to stopping at the earliest depth a solution was found at:

```C++
int main(){
    //search up to depth 40 completely (i.e., don't stop after reaching the optimal depth)
    Search2100 s;
    vector<vector<int>> solutions=s.search(40,true);
```

```
searching...
depth 0...
  elapsed time: 0.00 [s]
depth 1...
  elapsed time: 0.00 [s]
depth 2...
  elapsed time: 0.01 [s]
depth 3...
  elapsed time: 0.01 [s]
depth 4...
  elapsed time: 0.01 [s]
depth 5...
  elapsed time: 0.02 [s]
depth 6...
  elapsed time: 0.03 [s]
depth 7...
  elapsed time: 0.04 [s]
depth 8...
  elapsed time: 0.04 [s]
depth 9...
  elapsed time: 0.04 [s]
depth 10...
  elapsed time: 0.04 [s]
depth 11...
  elapsed time: 0.05 [s]
depth 12...
  elapsed time: 0.05 [s]
depth 13...
  elapsed time: 0.06 [s]
depth 14...
  elapsed time: 0.07 [s]
depth 15...
  elapsed time: 0.07 [s]
depth 16...
  elapsed time: 0.07 [s]
depth 17...
  elapsed time: 0.07 [s]
depth 18...
  elapsed time: 0.07 [s]
depth 19...
  elapsed time: 0.08 [s]
depth 20...
  elapsed time: 0.09 [s]
depth 21...
  elapsed time: 0.09 [s]
depth 22...
  elapsed time: 0.10 [s]
depth 23...
  elapsed time: 0.10 [s]
depth 24...
  elapsed time: 0.11 [s]
depth 25...
  elapsed time: 0.11 [s]
depth 26...
  elapsed time: 0.12 [s]
depth 27...
  elapsed time: 0.13 [s]
depth 28...
  elapsed time: 0.13 [s]
depth 29...
  elapsed time: 0.14 [s]
depth 30...
  elapsed time: 0.15 [s]
depth 31...
  elapsed time: 0.17 [s]
depth 32...
  elapsed time: 0.17 [s]
depth 33...
  elapsed time: 0.18 [s]
depth 34...
  inputs: 2, 2, 2, 2, 2, 2, 18, 2, 2, 2, 2, 2, 2, 2, 38, 0, 0, 0, 0, 0, 0, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
  frames: 33
  elapsed time: 0.20 [s]
depth 35...
  inputs: 2, 2, 2, 2, 2, 2, 18, 2, 2, 2, 2, 2, 2, 2, 2, 38, 0, 0, 0, 0, 0, 0, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
  frames: 34
  inputs: 2, 2, 2, 2, 2, 18, 2, 2, 2, 2, 2, 2, 2, 2, 2, 38, 0, 0, 0, 0, 0, 0, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
  frames: 34
  elapsed time: 0.24 [s]
depth 36...
  inputs: 2, 2, 2, 2, 2, 2, 18, 2, 2, 2, 2, 2, 2, 2, 38, 0, 0, 0, 0, 0, 0, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 38, 0, 0,
  frames: 35
  inputs: 2, 2, 2, 2, 2, 2, 18, 2, 2, 2, 2, 2, 2, 2, 38, 0, 0, 0, 0, 0, 0, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 38, 0, 0, 0,
  frames: 35
  inputs: 2, 2, 2, 2, 2, 2, 18, 2, 2, 2, 2, 2, 2, 2, 38, 0, 0, 0, 0, 0, 0, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 38, 0, 0, 0, 0,
  frames: 35
  inputs: 2, 2, 2, 2, 2, 2, 18, 2, 2, 2, 2, 2, 2, 2, 38, 0, 0, 0, 0, 0, 0, 2, 2, 2, 2, 2, 2, 2, 2, 2, 38, 0, 0, 0, 0, 0,
  frames: 35
  elapsed time: 0.30 [s]
depth 37...
  inputs: 2, 2, 2, 2, 2, 2, 18, 2, 2, 2, 2, 2, 2, 2, 2, 38, 0, 0, 0, 0, 0, 0, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 38, 0, 0,
  frames: 36
  inputs: 2, 2, 2, 2, 2, 2, 18, 2, 2, 2, 2, 2, 2, 2, 2, 38, 0, 0, 0, 0, 0, 0, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 38, 0, 0, 0,
  frames: 36
  inputs: 2, 2, 2, 2, 2, 2, 18, 2, 2, 2, 2, 2, 2, 2, 2, 38, 0, 0, 0, 0, 0, 0, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 38, 0, 0, 0, 0,
  frames: 36
  inputs: 2, 2, 2, 2, 2, 2, 18, 2, 2, 2, 2, 2, 2, 2, 2, 38, 0, 0, 0, 0, 0, 0, 2, 2, 2, 2, 2, 2, 2, 2, 2, 38, 0, 0, 0, 0, 0,
  frames: 36
  inputs: 2, 2, 2, 2, 2, 18, 2, 2, 2, 2, 2, 2, 2, 2, 2, 38, 0, 0, 0, 0, 0, 0, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 38, 0, 0,
  frames: 36
  inputs: 2, 2, 2, 2, 2, 18, 2, 2, 2, 2, 2, 2, 2, 2, 2, 38, 0, 0, 0, 0, 0, 0, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 38, 0, 0, 0,
  frames: 36
  inputs: 2, 2, 2, 2, 2, 18, 2, 2, 2, 2, 2, 2, 2, 2, 2, 38, 0, 0, 0, 0, 0, 0, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 38, 0, 0, 0, 0,
  frames: 36
  inputs: 2, 2, 2, 2, 2, 18, 2, 2, 2, 2, 2, 2, 2, 2, 2, 38, 0, 0, 0, 0, 0, 0, 2, 2, 2, 2, 2, 2, 2, 2, 2, 38, 0, 0, 0, 0, 0,
  frames: 36
  elapsed time: 0.41 [s]
depth 38...
  elapsed time: 0.43 [s]
depth 39...
  inputs: 2, 2, 2, 2, 2, 2, 18, 2, 2, 2, 2, 38, 0, 0, 0, 0, 0, 0, 2, 2, 2, 2, 18, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 18, 2, 2, 2,
  frames: 38
  elapsed time: 0.45 [s]
depth 40...
  inputs: 2, 2, 2, 2, 2, 2, 18, 2, 2, 2, 2, 38, 0, 0, 0, 0, 0, 0, 2, 2, 2, 2, 18, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 18, 2, 2, 2,
  frames: 39
  elapsed time: 0.48 [s]
```

Note that the frame counts are one less than the search depth (i.e., the number of inputs)- this is due to the first input being a *buffered* input. For readability, we can use `inputs_to_english(const vector<int> &inputs)` to see the shortest solution in english:

```C++
    //translate fastest solution to english and print
    cout<<"inputs: "<<s.inputs_to_english(solutions[0]);
}
```

```
inputs: right, right, right, right, right, right, jump right, right, right, right, right, right, right, right, up-right dash, no input, no input, no input, no input, no input, no input, right, right, right, right, right, right, right, right, right, right, right, right, right,
```

Of note, despite the assumption of dashing toward and bouncing off of the spring, the search managed to find solutions which don't use it! Recreating the depth 39 (38 frame) solution with a [TAS tool](https://github.com/CelesteClassic/UniversalClassicTas/), we see that it found a frame perfect [corner jump](https://celesteclassic.github.io/glossary/#cornerjump) to get around the spring:

<img src="https://i.imgur.com/ZY8Gktp.gif">

# Example - 100m

Here we'll set up a search problem to solve 100m, a level that's twice as long as 2100m. From its length, we'll need to rely on additional assumptions to make it manageable. Again, we start by importing some useful stuff and creating a class which inherits from `Searcheline<>`:

```C++
#include "Searcheline.h"
#include "CelesteUtils.h"
#include "Carts/Celeste.h"

using namespace std;


class Search100: public Searcheline<>{
```

We can play out some of the level ourselves with inputs that we suspect the optimal solution would start with, and then search from that point onward. Specifically, we can hand-specify and execute some initial inputs when specifying the initial state:

```C++
  //initial state to search from
  objlist& init_state() override{
      utils::load_room(p8,0); //load 100m
      utils::supress_object<Celeste::fake_wall>(p8); //don't consider berry block
      utils::skip_player_spawn(p8); //skip to after the player has spawned
      //execute list of initial inputs
      for (auto a:{18, 2, 2, 2, 2, 2, 2, 2, 2, 2, 34, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}){
          p8.set_btn_state(a);
          p8.step();
      }
  }
```

This lets the search start from the 21st frame onward, specifically from this position:

<img src="https://i.imgur.com/daxfieO.gif">

As long as these inputs are in the direction of the goal, this can reduce the search depth by up to 21! Alternatively, with access to a [TAS tool](https://github.com/CelesteClassic/UniversalClassicTas/), one can play out the above inputs in the tool and output the player's resulting coordinates, subpixels, and speed. With this information, along with the player's grace frames and number of dashes, the player can be placed directly:

```C++
    objlist& init_state() override{
        utils::load_room(p8,0); //load 100m
        utils::supress_object<Celeste::fake_wall>(p8); //don't consider berry block
        utils::skip_player_spawn(p8); //skip to after the player has spawned
        /*place player at position {55, 79}, subpixels {0.2, 0.185},
        speed {1.4, 0.63}, with 0 grace frames and 0 dashes*/
        utils::place_maddy(p8, 55, 79, 0.2, 0.185, 1.4, 0.63, 0, 0);
        return p8.game().objects;
```

Based on prior knowledge about how human players generally climb the right side of the level, we can restrict the inputs to only consider holding right, jumping right, dashing right, dashing up, and dashing up-right:

```C++
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
```

We can now run our search. Here we'll specify a maximum depth of 50, but from omitting the optional `complete` argument, the search will stop at the depth of the first solution found:

```C++
int main(){
    // search up to depth 50, but stop at the depth of the first solution found
    Search100 s;
    vector<vector<int>> solutions=s.search(50);
}
```

```
searching...
depth 0...
  elapsed time: 0.00 [s]
depth 1...
  elapsed time: 0.00 [s]
depth 2...
  elapsed time: 0.00 [s]
depth 3...
  elapsed time: 0.00 [s]
depth 4...
  elapsed time: 0.00 [s]
depth 5...
  elapsed time: 0.01 [s]
depth 6...
  elapsed time: 0.01 [s]
depth 7...
  elapsed time: 0.02 [s]
depth 8...
  elapsed time: 0.02 [s]
depth 9...
  elapsed time: 0.02 [s]
depth 10...
  elapsed time: 0.02 [s]
depth 11...
  elapsed time: 0.03 [s]
depth 12...
  elapsed time: 0.03 [s]
depth 13...
  elapsed time: 0.04 [s]
depth 14...
  elapsed time: 0.04 [s]
depth 15...
  elapsed time: 0.04 [s]
depth 16...
  elapsed time: 0.04 [s]
depth 17...
  elapsed time: 0.05 [s]
depth 18...
  elapsed time: 0.05 [s]
depth 19...
  elapsed time: 0.06 [s]
depth 20...
  elapsed time: 0.06 [s]
depth 21...
  elapsed time: 0.07 [s]
depth 22...
  elapsed time: 0.07 [s]
depth 23...
  elapsed time: 0.07 [s]
depth 24...
  elapsed time: 0.08 [s]
depth 25...
  elapsed time: 0.08 [s]
depth 26...
  elapsed time: 0.09 [s]
depth 27...
  elapsed time: 0.10 [s]
depth 28...
  elapsed time: 0.11 [s]
depth 29...
  elapsed time: 0.12 [s]
depth 30...
  elapsed time: 0.13 [s]
depth 31...
  elapsed time: 0.15 [s]
depth 32...
  elapsed time: 0.17 [s]
depth 33...
  elapsed time: 0.20 [s]
depth 34...
  elapsed time: 0.24 [s]
depth 35...
  elapsed time: 0.28 [s]
depth 36...
  elapsed time: 0.34 [s]
depth 37...
  elapsed time: 0.43 [s]
depth 38...
  elapsed time: 0.55 [s]
depth 39...
  elapsed time: 0.71 [s]
depth 40...
  elapsed time: 0.92 [s]
depth 41...
  elapsed time: 1.22 [s]
depth 42...
  elapsed time: 1.62 [s]
depth 43...
  elapsed time: 2.18 [s]
depth 44...
  elapsed time: 2.95 [s]
depth 45...
  elapsed time: 4.04 [s]
depth 46...
  inputs: 2, 34, 0, 0, 0, 0, 0, 0, 2, 18, 2, 2, 2, 2, 2, 18, 2, 2, 2, 2, 2, 36, 0, 0, 0, 0, 0, 0, 2, 2, 2, 2, 2, 2, 18, 2, 2, 2, 2, 2, 36, 0, 0, 0, 0, 0,
  frames: 45
  inputs: 2, 34, 0, 0, 0, 0, 0, 0, 2, 18, 2, 2, 2, 2, 2, 18, 2, 2, 2, 2, 2, 36, 0, 0, 0, 0, 0, 0, 2, 2, 2, 2, 2, 2, 18, 2, 2, 38, 0, 0, 0, 0, 0, 0, 18, 2,
  frames: 45
  inputs: 2, 34, 0, 0, 0, 0, 0, 0, 2, 18, 2, 2, 2, 2, 2, 18, 2, 2, 2, 2, 2, 36, 0, 0, 0, 0, 0, 0, 2, 2, 2, 2, 2, 2, 18, 2, 38, 0, 0, 0, 0, 0, 0, 18, 2, 2,
  frames: 45
  inputs: 2, 34, 0, 0, 0, 0, 0, 0, 2, 18, 2, 2, 2, 2, 2, 18, 2, 2, 2, 2, 2, 36, 0, 0, 0, 0, 0, 0, 2, 2, 2, 2, 2, 18, 2, 2, 2, 2, 2, 2, 2, 36, 0, 0, 0, 0,
  frames: 45
  inputs: 2, 34, 0, 0, 0, 0, 0, 0, 2, 18, 2, 2, 2, 2, 2, 18, 2, 2, 2, 2, 2, 36, 0, 0, 0, 0, 0, 0, 2, 2, 2, 2, 2, 18, 2, 2, 2, 2, 2, 2, 36, 0, 0, 0, 0, 0,
  frames: 45
  inputs: 2, 34, 0, 0, 0, 0, 0, 0, 2, 18, 2, 2, 2, 2, 2, 18, 2, 2, 2, 2, 2, 36, 0, 0, 0, 0, 0, 0, 2, 2, 2, 2, 2, 18, 2, 2, 2, 2, 38, 0, 0, 0, 0, 0, 0, 2,
  frames: 45
  inputs: 2, 34, 0, 0, 0, 0, 0, 0, 2, 18, 2, 2, 2, 2, 2, 18, 2, 2, 2, 2, 2, 36, 0, 0, 0, 0, 0, 0, 2, 2, 2, 2, 2, 18, 2, 2, 2, 2, 38, 0, 0, 0, 0, 0, 0, 18,
  frames: 45
  inputs: 2, 34, 0, 0, 0, 0, 0, 0, 2, 18, 2, 2, 2, 2, 2, 18, 2, 2, 2, 2, 2, 36, 0, 0, 0, 0, 0, 0, 2, 2, 2, 2, 2, 18, 2, 2, 2, 38, 0, 0, 0, 0, 0, 0, 2, 2,
  frames: 45
  inputs: 2, 34, 0, 0, 0, 0, 0, 0, 2, 18, 2, 2, 2, 2, 2, 18, 2, 2, 2, 2, 2, 36, 0, 0, 0, 0, 0, 0, 2, 2, 2, 2, 2, 18, 2, 2, 2, 38, 0, 0, 0, 0, 0, 0, 2, 18,
  frames: 45
  inputs: 2, 34, 0, 0, 0, 0, 0, 0, 2, 18, 2, 2, 2, 2, 2, 18, 2, 2, 2, 2, 2, 36, 0, 0, 0, 0, 0, 0, 2, 2, 2, 2, 2, 18, 2, 2, 2, 38, 0, 0, 0, 0, 0, 0, 18, 2,
  frames: 45
  inputs: 2, 34, 0, 0, 0, 0, 0, 0, 2, 18, 2, 2, 2, 2, 2, 18, 2, 2, 2, 2, 2, 36, 0, 0, 0, 0, 0, 0, 2, 2, 2, 2, 2, 18, 2, 2, 38, 0, 0, 0, 0, 0, 0, 2, 2, 2,
  frames: 45
  inputs: 2, 34, 0, 0, 0, 0, 0, 0, 2, 18, 2, 2, 2, 2, 2, 18, 2, 2, 2, 2, 2, 36, 0, 0, 0, 0, 0, 0, 2, 2, 2, 2, 2, 18, 2, 2, 38, 0, 0, 0, 0, 0, 0, 2, 2, 18,
  frames: 45
  inputs: 2, 34, 0, 0, 0, 0, 0, 0, 2, 18, 2, 2, 2, 2, 2, 18, 2, 2, 2, 2, 2, 36, 0, 0, 0, 0, 0, 0, 2, 2, 2, 2, 2, 18, 2, 2, 38, 0, 0, 0, 0, 0, 0, 2, 18, 2,
  frames: 45
  inputs: 2, 34, 0, 0, 0, 0, 0, 0, 2, 18, 2, 2, 2, 2, 2, 18, 2, 2, 2, 2, 2, 36, 0, 0, 0, 0, 0, 0, 2, 2, 2, 2, 2, 18, 2, 2, 38, 0, 0, 0, 0, 0, 0, 18, 2, 2,
  frames: 45
  inputs: 2, 34, 0, 0, 0, 0, 0, 0, 2, 18, 2, 2, 2, 2, 2, 18, 2, 2, 2, 2, 2, 36, 0, 0, 0, 0, 0, 0, 2, 2, 2, 2, 2, 18, 2, 38, 0, 0, 0, 0, 0, 0, 2, 2, 18, 2,
  frames: 45
  inputs: 2, 34, 0, 0, 0, 0, 0, 0, 2, 18, 2, 2, 2, 2, 2, 18, 2, 2, 2, 2, 2, 36, 0, 0, 0, 0, 0, 0, 2, 2, 2, 2, 2, 18, 2, 38, 0, 0, 0, 0, 0, 0, 2, 18, 2, 2,
  frames: 45
  inputs: 2, 34, 0, 0, 0, 0, 0, 0, 2, 18, 2, 2, 2, 2, 2, 18, 2, 2, 2, 2, 2, 36, 0, 0, 0, 0, 0, 0, 2, 2, 2, 2, 2, 18, 2, 38, 0, 0, 0, 0, 0, 0, 2, 18, 2, 18,
  frames: 45
  inputs: 2, 34, 0, 0, 0, 0, 0, 0, 2, 18, 2, 2, 2, 2, 2, 18, 2, 2, 2, 2, 2, 36, 0, 0, 0, 0, 0, 0, 2, 2, 2, 2, 2, 18, 38, 0, 0, 0, 0, 0, 0, 2, 2, 18, 2, 2,
  frames: 45
  inputs: 2, 34, 0, 0, 0, 0, 0, 0, 2, 18, 2, 2, 2, 2, 2, 18, 2, 2, 2, 2, 2, 36, 0, 0, 0, 0, 0, 0, 2, 2, 2, 2, 2, 18, 38, 0, 0, 0, 0, 0, 0, 2, 18, 2, 2, 2,
  frames: 45
  inputs: 2, 34, 0, 0, 0, 0, 0, 0, 2, 18, 2, 2, 2, 2, 2, 18, 2, 2, 2, 2, 2, 36, 0, 0, 0, 0, 0, 0, 2, 2, 2, 2, 2, 18, 38, 0, 0, 0, 0, 0, 0, 2, 18, 2, 2, 18,
  frames: 45
  inputs: 2, 34, 0, 0, 0, 0, 0, 0, 2, 18, 2, 2, 2, 2, 2, 18, 2, 2, 2, 2, 2, 36, 0, 0, 0, 0, 0, 0, 2, 2, 2, 2, 2, 38, 0, 0, 0, 0, 0, 0, 2, 2, 18, 2, 2, 2,
  frames: 45
  inputs: 2, 34, 0, 0, 0, 0, 0, 0, 2, 18, 2, 2, 2, 2, 2, 18, 2, 2, 2, 2, 2, 36, 0, 0, 0, 0, 0, 0, 2, 2, 2, 2, 2, 38, 0, 0, 0, 0, 0, 0, 2, 2, 18, 2, 2, 18,
  frames: 45
  inputs: 2, 34, 0, 0, 0, 0, 0, 0, 2, 18, 2, 2, 2, 2, 2, 18, 2, 2, 2, 2, 2, 36, 0, 0, 0, 0, 0, 0, 2, 2, 2, 2, 2, 38, 0, 0, 0, 0, 0, 0, 2, 2, 18, 2, 18, 2,
  frames: 45
  inputs: 2, 34, 0, 0, 0, 0, 0, 0, 2, 18, 2, 2, 2, 2, 2, 18, 2, 2, 2, 2, 2, 36, 0, 0, 0, 0, 0, 0, 2, 2, 2, 2, 2, 38, 0, 0, 0, 0, 0, 0, 18, 2, 2, 2, 2, 2,
  frames: 45
  inputs: 2, 34, 0, 0, 0, 0, 0, 0, 2, 18, 2, 2, 2, 2, 2, 18, 2, 2, 2, 2, 2, 36, 0, 0, 0, 0, 0, 0, 2, 2, 2, 2, 2, 38, 0, 0, 0, 0, 0, 0, 18, 2, 2, 2, 2, 18,
  frames: 45
  inputs: 2, 34, 0, 0, 0, 0, 0, 0, 2, 18, 2, 2, 2, 2, 2, 18, 2, 2, 2, 2, 2, 36, 0, 0, 0, 0, 0, 0, 2, 2, 2, 2, 2, 38, 0, 0, 0, 0, 0, 0, 18, 2, 2, 2, 18, 2,
  frames: 45
  inputs: 2, 34, 0, 0, 0, 0, 0, 0, 2, 18, 2, 2, 2, 2, 2, 18, 2, 2, 2, 2, 2, 36, 0, 0, 0, 0, 0, 0, 2, 2, 2, 2, 2, 38, 0, 0, 0, 0, 0, 0, 18, 2, 2, 18, 2, 2,
  frames: 45
  inputs: 2, 34, 0, 0, 0, 0, 0, 0, 2, 18, 2, 2, 2, 2, 2, 18, 2, 2, 2, 2, 2, 36, 0, 0, 0, 0, 0, 0, 2, 2, 2, 2, 2, 38, 0, 0, 0, 0, 0, 0, 18, 2, 18, 2, 2, 2,
  frames: 45
  inputs: 2, 34, 0, 0, 0, 0, 0, 0, 2, 18, 2, 2, 2, 2, 2, 18, 2, 2, 2, 2, 2, 36, 0, 0, 0, 0, 0, 0, 2, 2, 2, 2, 2, 38, 0, 0, 0, 0, 0, 0, 18, 2, 18, 2, 2, 18,
  frames: 45
  inputs: 2, 34, 0, 0, 0, 0, 0, 0, 2, 18, 2, 2, 2, 2, 2, 18, 2, 2, 2, 2, 2, 36, 0, 0, 0, 0, 0, 0, 2, 2, 2, 2, 2, 38, 0, 0, 0, 0, 0, 0, 18, 2, 18, 2, 18, 2,
  frames: 45
  elapsed time: 5.96 [s]
  ```

It manages to find several 45 frame solutions, which are 66 frame solutions when acknowledging that we searched from the 21st frame onward. This search only took 5 seconds, however given a search problem's exponential growth, it likely wouldn't have been feasible to search up to depth 67 from the start, even with the heavy input restrictions. This emphasizes the care needed in setting up a feasible search problem, and how it might be better to instead run several, smaller searches from promising starts! For confirmation that we set things up right, we can combine the first solution found with the initial inputs, and play it back with a TAS tool:

<img src="https://i.imgur.com/eT4pHtK.gif">

Comparing the performance of Pyleste to Cppleste on this search, we get a significant improvement: this search runs in ~1000 seconds on pyleste, whereas cppleste compiled with optimizations runs it in just below 6 seconds, giving a x167 improvement. While not all searches will get a speedup as large, it's safe to say Cppleste is much faster than Pyleste

#Running Cppleste
While you can just compile every script using cppleste that you write with all of the Cppleste files, it is recommended to use Cppleste as a statically linked library, both for ease of use and better compile times.
If you want to compile the library yourself I'll explain how to do that with CMake:

in your Cppleste directory create some subdirectory (say cppleste-build)
now run the following commands in that directory:
```
cmake ../
cmake --build .
``` 
This will create a file called libCppleste.a which is the statically linked library.
now to compile a file using the cppleste library (say cpplesteTest.cpp) simply put it and libCppleste.a in the same folder, and run
```
g++ -std=c++17 libCppleste.a cpplesteTest.cpp -o outputName
```
It is also recommended you add the -O3 flag for files where performance is important.

#Thanks 
thanks to meep for originally making pyleste, helping with understanding the code and implementation details, and allowing me to shamelessly steal his examples.
