#include "Celeste.h"
using std::abs;
using std::min;
using std::max;


template<typename T>
Celeste::Pair<T>::Pair(T x, T y) : x(x), y(y) {}

Celeste::Rect::Rect(int x, int y, int w, int h) : x(x), y(y), h(h), w(w) {}

Celeste::base_obj::base_obj(PICO8<Celeste> &p8, Celeste &g, int x, int y, int tile) :
        p8(p8),
        g(g),
        hitbox(0, 0, 8, 8),
        spd(0, 0),
        rem(0, 0),
        flip(false, false),
        x(x),
        y(y),
        spr(tile),
        solids(false),
        collideable(true),
        ascii("  "),
        type ("obj"),
        type_id(BASE_OBJ){}

void Celeste::base_obj::init() {}

void Celeste::base_obj::update() {}

void Celeste::base_obj::draw() {}

bool Celeste::base_obj::is_solid(int ox, int oy) const {
    if (oy > 0 && !collide<platform>(ox, 0) && collide<platform>(ox, oy)) {
        return true;
    }
    return g.get().tile_flag_at(x + hitbox.x + ox, y + hitbox.y + oy, hitbox.w, hitbox.h, 0) ||
           collide<fall_floor>(ox, oy) ||
           collide<fake_wall>(ox, oy);
}

bool Celeste::base_obj::is_ice(int ox, int oy) const {
    return g.get().tile_flag_at(x + hitbox.x + ox, y + hitbox.y + oy, hitbox.w, hitbox.h, 4);
}



void Celeste::base_obj::move(double ox, double oy) {
    rem.x += ox;
    int amt = floor(rem.x + 0.5);
    rem.x -= amt;
    move_x(amt, 0);
    rem.y += oy;
    amt = floor(rem.y + 0.5);
    rem.y -= amt;
    move_y(amt);
}

void Celeste::base_obj::move_x(int amt, int start) {
    if (solids) {
        int step = g.get().sign(amt);
        int end = abs(amt);
        for (int i = start; i <= end; i++) {
            if (!is_solid(step, 0)) {
                x += step;
            }
            else {
                spd.x = 0;
                rem.x = 0;
                break;
            }
        }
    }
    else {
        x += amt;
    }
}

void Celeste::base_obj::move_y(int amt) {
    if (solids) {
        int step = g.get().sign(amt);
        int end = abs(amt);
        for (int i = 0; i <= end; i++) {
            if (!is_solid(0, step)) {
                y += step;
            }
            else {
                spd.y = 0;
                rem.y = 0;
                break;
            }
        }
    }
    else {
        y += amt;
    }
}

std::ostream &operator<<(std::ostream &os, const Celeste::base_obj &b) {
    return os << "["<<b.type<<"] "<<"x: " << (int) b.x << ", y: " << (int) b.y << ", rem:{" <<
    std::fixed <<std::setprecision(4)<< b.rem.x << ", " << std::fixed << std::setprecision(4)<< b.rem.y << "}, spd:{"<<
    std::fixed << std::setprecision(4)<< b.spd.x << ", " <<std::fixed << std::setprecision(4)<< b.spd.y << "}";
}


Celeste::player_spawn::player_spawn(PICO8<Celeste> &p8, Celeste &g, int x, int y, int tile) : base_obj(p8, g, x, y, tile) {
    ascii = ":D";
    type = "player spawn";
    type_id = PLAYER_SPAWN;
}

void Celeste::player_spawn::init() {
    target = y;
    y = 128;
    spd.y = -4;
    state = 0;
    delay = 0;
}

void Celeste::player_spawn::update() {

    if (state == 0) {
        if (y < target + 16) {
            state = 1;
            delay = 3;
        }
    }
    else if (state == 1) {
        spd.y += 0.5;
        if (spd.y > 0) {
            if (delay > 0) {
                spd.y = 0;
                delay -= 1;
            }
            else if (y > target) {
                y = target;
                spd = Pair<double>(0, 0);
                state = 2;
                delay = 5;
            }
        }
    }
    else if (state == 2) {
        delay -= 1;
        if (delay < 0) {
            g.get().init_object<player>(x, y);
            g.get().destroy_object(this);
        }
    }
}

Celeste::player_spawn *Celeste::player_spawn::clone() const {
    return new player_spawn(*this);
}

Celeste::player::player(PICO8<Celeste> &p8, Celeste &g, int x, int y, int tile) : base_obj(p8, g, x, y, tile) {
    ascii = ":D";
    type = "player";
    type_id = PLAYER;
}

void Celeste::player::init() {
    p_jump = false;
    p_dash = false;
    grace = 0;
    jbuffer = 0;
    djump = 1;
    dash_time = 0;
    dash_effect_time = 0;
    dash_target = Pair<double>(0, 0);
    dash_accel = Pair<double>(0, 0);
    hitbox = Rect(1, 3, 6, 5);
    solids = true;
}

void Celeste::player::update() {
    if(g.get().pause_player){
        return;
    }
    int h_input = p8.get().btn(k_right) ? 1 : p8.get().btn(k_left) ? -1 : 0;
    bool kill=false;
    if (g.get().spikes_at(x + hitbox.x, y + hitbox.y, hitbox.w, hitbox.h, spd.x, spd.y) || y > 128) {
        kill = true;
    }
    bool on_ground = is_solid(0, 1);
    bool jump = p8.get().btn(k_jump) && !p_jump;
    bool dash = p8.get().btn(k_dash) && !p_dash;
    p_jump = p8.get().btn(k_jump);
    p_dash = p8.get().btn(k_dash);

    if (jump) {
        jbuffer = 4;
    }
    else if (jbuffer > 0) {
        jbuffer--;
    }

    if (on_ground) {
        grace = 6;
        djump = g.get().max_djump;
    }
    else if (grace > 0) {
        grace--;
    }

    dash_effect_time--;

    if (dash_time > 0) {
        dash_time--;
        spd.x = g.get().appr(spd.x, dash_target.x, dash_accel.x);
        spd.y = g.get().appr(spd.y, dash_target.y, dash_accel.y);
    }
    else {
        double maxrun = 1;
        double accel = !on_ground ? 0.4 : is_ice(0, 1) ? 0.05 : 0.6;
        double deccel = 0.15;

        spd.x = (abs(spd.x) <= 1) ? g.get().appr(spd.x, h_input * maxrun, accel) : g.get().appr(spd.x, g.get().sign(spd.x) * maxrun,
                                                                                    deccel);

        if (spd.x != 0) {
            flip.x = spd.x < 0;
        }

        double maxfall = (h_input == 0 || !is_solid(h_input, 0) || is_ice(h_input, 0)) ? 2 : 0.4;

        if (!on_ground) {
            spd.y = g.get().appr(spd.y, maxfall, abs(spd.y) > 0.15 ? 0.21 : 0.105);
        }

        if (jbuffer > 0) {
            if (grace > 0) {
                jbuffer = 0;
                grace = 0;
                spd.y = -2;
            }
            else {
                int wall_dir = is_solid(-3, 0) ? -1 : is_solid(3, 0) ? 1 : 0;
                if (wall_dir != 0) {
                    jbuffer = 0;
                    spd.y = -2;
                    spd.x = -wall_dir * (maxrun + 1);
                }
            }
        }
        //dash
        double d_full = 5;
        double d_half = 3.5355339059;
        if (djump > 0 && dash) {
            djump--;
            dash_time = 4;
            g.get().has_dashed = true;
            dash_effect_time = 10;
            int v_input = p8.get().btn(k_up) ? -1 : p8.get().btn(k_down) ? 1 : 0;
            spd.x = h_input != 0 ? h_input * (v_input == 0 ? d_full : d_half) : (v_input != 0 ? 0 : flip.x ? -1 : 1);
            spd.y = v_input != 0 ? v_input * (h_input == 0 ? d_full : d_half) : 0;

            g.get().freeze = 2;
            dash_target.x = 2 * sign(spd.x);
            dash_target.y = (spd.y > 0 ? 2 : 1.5) * sign(spd.y);
            dash_accel.x = spd.y == 0 ? 1.5 : 1.06066017177;
            dash_accel.y = spd.x == 0 ? 1.5 : 1.06066017177;
        }
    }
    if (y < -4) {
        g.get().next_room();
    }
    if(kill){
        g.get().kill_player(this);
    }
}

void Celeste::player::draw() {
    if (x < -1 || x > 121) {
        x = clamp(x, -1, 121);
        spd.x = 0;
    }
}

Celeste::player *Celeste::player::clone() const {
    return new player(*this);
}

Celeste::balloon::balloon(PICO8<Celeste> &p8, Celeste &g, int x, int y, int tile) : base_obj(p8, g, x, y, tile) {
    ascii="()";
    type="balloon";
    type_id=BALLOON;
}

void Celeste::balloon::init() {
    timer=0;
    //change: remove rng, hitbox covers full cycle
    hitbox=Rect(-1,-1-2,10,10+4);
}

void Celeste::balloon::update() {
    if (spr==22){
        auto* hit=check<player>(0,0);
        if (hit!=nullptr && hit->djump<g.get().max_djump){
            hit->djump=g.get().max_djump;
            spr=0;
            timer=60;
        }
        else if(timer>0){
            timer--;
        }
        else{
            spr=22;
        }
    }
}

Celeste::balloon *Celeste::balloon::clone() const {
    return new balloon(*this);
}

Celeste::platform::platform(PICO8<Celeste> &p8, Celeste &g, int x, int y, int tile) : base_obj(p8, g, x, y, tile) {
    ascii = "oo";
    type = "platform";
    type_id=PLATFORM;
}

void Celeste::platform::init() {
    x -= 4;
    hitbox.w = 16;
    last = x;
    dir = spr == 11 ? -1 : 1;
}

void Celeste::platform::update() {
    spd.x = dir * 0.65;
    if (x < -16) {
        x = 128;
    }
    if (x > 128) {
        x = -16;
    }
    if (!collide<player>(0, 0)) {
        player *hit = check<player>(0, -1);
        if (hit != nullptr) {
            hit->move_x(x - last, 1);
        }
    }
    last = x;
}

Celeste::platform *Celeste::platform::clone() const {
    return new platform(*this);
}

Celeste::fruit::fruit(PICO8<Celeste> &p8, Celeste &g, int x, int y, int tile) : base_obj(p8, g, x, y, tile) {
    ascii="{}";
    type = "fruit";
    type_id=FRUIT;
}

void Celeste::fruit::init() {
    start=y;
    off=0;
}

void Celeste::fruit::update() {
    auto* hit=check<player>(0,0);
    if(hit!=nullptr){
        hit->djump=g.get().max_djump;
        g.get().destroy_object(this);
        g.get().got_fruit=true;
    }
    else {
        off++;
        y = start + sin(off / 40.0) * 2.5;
    }
}

Celeste::fruit *Celeste::fruit::clone() const {
    return new fruit(*this);
}

Celeste::fly_fruit::fly_fruit(PICO8<Celeste> &p8, Celeste &g, int x, int y, int tile) : base_obj(p8, g, x, y, tile) {
    ascii="{}";
    type = "fly fruit";
    type_id=FLY_FRUIT;
}

void Celeste::fly_fruit::init() {
    fly=false;
    step=0.5;
    solids=false;
}

void Celeste::fly_fruit::update() {
    if (fly){
        spd.y=appr(spd.y,-3.5,0.25);
        if (y<-16){
            g.get().destroy_object(this);
            return;
        }
    }
    else{
        if(g.get().has_dashed){
            fly=true;
        }
        step+=0.05;
        spd.y=sin(step)*0.5;
    }
    auto* hit=check<player>(0,0);
    if(hit!= nullptr){
        hit->djump=g.get().max_djump;
        g.get().destroy_object(this);
        g.get().got_fruit=true;
    }
}

Celeste::fly_fruit *Celeste::fly_fruit::clone() const {
    return new fly_fruit(*this);
}

Celeste::fake_wall::fake_wall(PICO8<Celeste> &p8, Celeste &g, int x, int y, int tile) : base_obj(p8, g, x, y, tile) {
    ascii="▓▓";
    type= "fake wall";
    type_id=FAKE_WALL;
}

void Celeste::fake_wall::update() {
    hitbox.w=18;
    hitbox.h=18;
    auto* hit=check<player>(-1,-1);;
    if(hit!=nullptr && hit->dash_effect_time>0){
        hit->spd.x= -sign(hit->spd.x)*1.5;
        hit->spd.y=-1.5;
        hit->dash_time=-1;
        g.get().init_object<fruit>(x+4,y+4,26);
        g.get().destroy_object(this);
    }
    else {
        hitbox.w = 16;
        hitbox.h = 16;
    }
}

Celeste::fake_wall *Celeste::fake_wall::clone() const {
    return new fake_wall(*this);
}

Celeste::spring::spring(PICO8<Celeste> &p8, Celeste &g, int x, int y, int tile) : base_obj(p8, g, x, y, tile) {
    ascii="ΞΞ";
    type = "spring";
    type_id = SPRING;
}

void Celeste::spring::init() {
    hide_for=0;
    hide_in=0;
}

void Celeste::spring::update() {
    if(hide_for>0){
        hide_for--;
        if(hide_for<=0){
            delay=0;
        }
    }
    else if(spr==18){
        auto* hit=check<player>(0,0);
        if(hit!= nullptr && hit->spd.y>=0){
            spr=19;
            hit->y=y-4;
            hit->spd.x*=0.2;
            hit->spd.y=-3;
            hit->djump=g.get().max_djump;
            delay=10;
            auto* below=check<fall_floor>(0,1);
            if(below!=nullptr){
                break_fall_floor(*below);
            }
        }
    }
    else if(delay>0){
        delay--;
        if(delay<=0){
            spr=18;
        }
    }
    if(hide_in>0){
        hide_in--;
        if(hide_in<=0){
            hide_for=60;
            spr=0;
        }
    }
}

Celeste::spring *Celeste::spring::clone() const {
    return new spring(*this);
}

Celeste::fall_floor::fall_floor(PICO8<Celeste> &p8, Celeste &g, int x, int y, int tile) : base_obj(p8, g, x, y, tile) {
    ascii = "▒▒";
    type = "fall floor";
    type_id = FALL_FLOOR;
}

void Celeste::fall_floor::init() {
    state = 0;
}

void Celeste::fall_floor::update() {
    if (state == 0) {
        if (collide<player>(0, -1) || collide<player>(-1, 0) || collide<player>(1, 0)) {
            break_fall_floor(*this);
        }
    }
    else if (state == 1) {
        delay--;
        if (delay <= 0) {
            state = 2;
            delay = 60;
            collideable = false;
            spr = 0;
        }
    }
    else if (state == 2) {
        delay--;
        if (delay <= 0 && !collide<player>(0, 0)) {
            state = 0;
            collideable = true;
            spr = 23;
        }
    }
}

Celeste::fall_floor *Celeste::fall_floor::clone() const {
    return new fall_floor(*this);
}

void Celeste::break_spring(Celeste::spring &s) {
    s.hide_in = 15;
}

void Celeste::break_fall_floor(Celeste::fall_floor &s) {
    if (s.state == 0) {
        s.state = 1;
        s.delay = 15;
        auto *hit = s.check<spring>(0, -1);
        if (hit) {
            break_spring(*hit);
        }
    }
}

Celeste::key::key(PICO8<Celeste> &p8, Celeste &g, int x, int y, int tile) : base_obj(p8, g, x, y, tile) {
    ascii="¤¬";
    type = "key";
    type_id = KEY;
}

void Celeste::key::update() {
    if (collide<player>(0,0)){
        g.get().has_key=true;
        g.get().destroy_object(this);
    }
}

Celeste::key *Celeste::key::clone() const {
    return new key(*this);
}


Celeste::chest::chest(PICO8<Celeste> &p8, Celeste &g, int x, int y, int tile) : base_obj(p8, g, x, y, tile) {
    ascii="╔╗";
    type= "chest";
    type_id = CHEST;
}

void Celeste::chest::init() {
    x-=4;
    timer=20;
}

void Celeste::chest::update() {
    if(g.get().has_key){
        timer--;
        if(timer<=0){
            auto& f=g.get().init_object<fruit>(x,y-4,26);
            //change: remove rng by expanding the fruit's hitbox
            f.hitbox.x-=1;
            f.hitbox.w+=3;
            g.get().destroy_object(this);
        }
    }
}

Celeste::chest *Celeste::chest::clone() const {
    return new chest(*this);
}

Celeste::big_chest::big_chest(PICO8<Celeste> &p8, Celeste &g, int x, int y, int tile) : base_obj(p8, g, x, y, tile) {
    ascii="╔╤";
    type = "big chest";
    type_id =BIG_CHEST;
}
void Celeste::big_chest::init(){
    state=0;
    hitbox.w=16;
}
void Celeste::big_chest::draw(){
    if(state==0){
        auto* hit=check<player>(0,8);
        if(hit!=nullptr && hit->is_solid(0,1)){
            g.get().pause_player=true;
            hit->spd=Pair<double>(0,0);
            state=1;
            timer=60;
        }
    }
    else if(state==1){
        timer-=1;
        if(timer<0){
            state=2;
            g.get().init_object<orb>(x+4, y+4);
            g.get().pause_player=false;
        }
    }
}

Celeste::big_chest* Celeste::big_chest::clone() const{
    return new big_chest(*this);
}
Celeste::orb::orb(PICO8<Celeste> &p8, Celeste &g, int x, int y, int tile) : base_obj(p8, g, x, y, tile) {
    ascii="◖◗";
    type = "orb";
    type_id =ORB;
}

void Celeste::orb::init(){
    spd.y=-4;
    solids=false;
}

void Celeste::orb::draw(){
    spd.y=g.get().appr(spd.y,0,0.5);
    player* hit = check<player>(0,0);
    if (spd.y==0 && hit !=nullptr){
        g.get().freeze=10;
        g.get().max_djump=2;
        hit->djump=2;
        g.get().destroy_object(this);
    }
}

Celeste::orb* Celeste::orb::clone() const{
    return new orb(*this);
}


Celeste::Celeste(PICO8<Celeste> &p8) :
        p8(p8),
        room(0, 0) {
    freeze = 0;
    delay_restart = 0;
    max_djump = 1;
    next_rm = false;
    loop_mode = false;
    pause_player=false;
    got_fruit=false;
    prev_got_fruit=false;
}

void Celeste::_init() {
    frames = 0;
    load_room(0, 0);
}

void Celeste::_update() {
    frames = (frames + 1) % 30;
    if (freeze > 0) {
        freeze--;
        return;
    }
    if (delay_restart > 0) {
        delay_restart -= 1;
        if (delay_restart == 0) {
            load_room(room.x, room.y);
        }
    }
    for (auto &o:objects) {
        if(o!=nullptr){
            o->move(o->spd.x, o->spd.y);
            o->update();
        }
    }

    //next room transition
    if (next_rm) {
        next_rm = false;
        int lvl_id = level_index();
        int n_objs = objects.size();
        load_room(lvl_id % 8, lvl_id / 8);
        // loading jank
        if (lvl_id > 0) {
            auto it = objects.begin();
            std::advance(it,n_objs-1);
            for (; it != objects.end(); it++) {
                auto &o = *it;
                o->move(o->spd.x, o->spd.y);
                o->update();
            }
        }
    }


    auto it = objects.begin();
    while (it != objects.end()) {
        auto nxt = next(it);
        if (*it == nullptr) {
            objects.erase(it);
        }
        it = nxt;
    }
}

void Celeste::_draw() {
    if (freeze > 0) {
        return;
    }
    for (auto &o: objects) {
        o->draw();
    }

    //clear objects destroyed in draw (because chest exists)
    auto it = objects.begin();
    while (it != objects.end()) {
        auto nxt = next(it);
        if (*it == nullptr) {
            objects.erase(it);
        }
        it = nxt;
    }
}

int Celeste::level_index() {
    return room.x + room.y * 8;
}

void Celeste::restart_room() {
    delay_restart = 15;
}

void Celeste::next_room() {
    next_rm = true;
    if (loop_mode) {
        return;
    }
    room = Pair<int>((level_index() + 1) % 8, (level_index() + 1) / 8);
}

void Celeste::load_room(int x, int y) {
    has_dashed = false;
    has_key = false;
    got_fruit = false; // change: only single got_fruit stored, for the current level

    objects.clear();
    room.x = x;
    room.y = y;
    for (int tx = 0; tx < 16; tx++) {
        for (int ty = 0; ty < 16; ty++) {
            int tile = p8.mget(room.x * 16 + tx, room.y * 16 + ty);
            switch (tile) {
                case 1:
                    init_object<player_spawn>(tx * 8, ty * 8, tile);
                    break;
                case 8:
                    init_object<key>(tx*8,ty*8,tile);
                    break;
                case 11:
                case 12:
                    init_object<platform>(tx * 8, ty * 8, tile);
                    break;
                case 18:
                    init_object<spring>(tx*8,ty*8,tile);
                    break;
                case 20:
                    init_object<chest>(tx*8,ty*8,tile);
                    break;
                case 22:
                    init_object<balloon>(tx*8,ty*8,tile);
                    break;
                case 23:
                    init_object<fall_floor>(tx * 8, ty * 8, tile);
                    break;
                case 26:
                    init_object<fruit>(tx*8,ty*8,tile);
                    break;
                case 28:
                    init_object<fly_fruit>(tx*8,ty*8,tile);
                    break;
                case 64:
                    init_object<fake_wall>(tx*8,ty*8,tile);
                    break;
                case 96:
                    init_object<big_chest>(tx*8,ty*8,tile);
                    break;

            }
        }
    }
}

template<typename obj>
obj &Celeste::init_object(int x, int y, int tile) {
    objects.push_back(std::make_unique<obj>(p8, *this, x, y, tile));
    auto &o = objects.back();
    o->init();
    return static_cast<obj &>(*o);
}

template<typename obj>
void Celeste::destroy_object(obj *o) {
    for (auto &i: objects) {
        if (i.get() == o) {
            i = nullptr;
            return;
        }
    }
}

void Celeste::kill_player(Celeste::player *p) {
    destroy_object(p);
    restart_room();
}

Celeste::base_obj *Celeste::get_player() {
    for (auto &o: objects) {
        if (o && (o->type_id == Celeste::PLAYER  || o->type_id == Celeste::PLAYER_SPAWN)) {
            return o.get();
        }
    }
    return nullptr;
}

double Celeste::clamp(double val, double a, double b) {
    return max(a, min(b, val));
}

double Celeste::appr(double val, double target, double amt) {
    if (val > target) {
        return max(val - amt, target);
    }
    return min(val + amt, target);
}

double Celeste::sign(double x) {
    return (x > 0) - (x < 0);
}

// change: use custom mod function because of differences of mod for negatives between c++ and lua
int Celeste::mod(int a, int b) {
    int r=a%b;
    return (r>=0)?r:b+r;
}

//define sin function in order to match up with p8s sin
double Celeste::sin(double a){
    return std::sin(-std::numbers::pi*2*a);
}

bool Celeste::tile_flag_at(int x, int y, int w, int h, int flag) const{
    for (int i = max(0, x / 8); i <= min(15, (x + w - 1) / 8); i++) {
        for (int j = max(0, y / 8); j <= min(15, (y + h - 1) / 8); j++) {

            if (p8.fget(tile_at(i, j), flag)) {
                return true;
            }
        }
    }
    return false;
}

int Celeste::tile_at(int x, int y) const{
    return p8.mget(room.x * 16 + x, room.y * 16 + y);
}

bool Celeste::spikes_at(int x, int y, int w, int h, double spdx, double spdy) const{
    for (int i = max(0, x / 8); i <= min(15, (x + w - 1) / 8); i++) {
        for (int j = max(0, y / 8); j <= min(15, (y + h - 1) / 8); j++) {
            int tile = tile_at(i, j);
            if ((tile == 17 && (mod((y + h - 1), 8) >= 6 || y + h == j * 8 + 8) && spdy >= 0) ||
                (tile == 27 && mod(y, 8) <= 2 && spdy <= 0) ||
                (tile == 43 && mod(x, 8) <= 2 && spdx <= 0) ||
                (tile == 59 && (mod((x + w - 1), 8) >= 6 || x + w == i * 8 + 8) && spdx >= 0)) {
                return true;
            }
        }
    }
    return false;
}

std::ostream &operator<<(std::ostream &os, Celeste &c) {
    std::map<int, std::string> spikes = {
            {17, "^^"},
            {27, "vv"},
            {43, "> "},
            {59, " <"}
    };
    std::vector<std::string> map_str;
    for (int i = 0; i < 16; i++) {
        for (int j = 0; j < 16; j++) {
            map_str.emplace_back("  ");
        }
        map_str.emplace_back("\n");
    }
    for (int tx = 0; tx <= 15; tx++) {
        for (int ty = 0; ty <= 15; ty++) {
            int pos = tx + 17 * ty;
            int tile = c.p8.mget(c.room.x * 16 + tx, c.room.y * 16 + ty);
            if (c.p8.fget(tile, 4)) {
                map_str[pos] = "░░";
            }
            if (c.p8.fget(tile, 0)) {
                map_str[pos] = "██";
            }
            else if (spikes.find(tile) != spikes.end()) {
                map_str[pos] = spikes[tile];
            }
        }
    }
    for (auto &o:c.objects) {
        int ox = round(o->x / 8);
        int oy = round(o->y / 8);
        int pos = ox + 17 * oy;
        if(ox>=0&& ox<=15&&oy>=0&&oy<=15) {
            map_str[pos] = o->ascii;
            if(o && o->type_id==Celeste::PLATFORM&&ox+1<=15){
                map_str[pos+1]=o->ascii;
            }
            else if(o && o->type_id==Celeste::FLY_FRUIT){
                if(ox-1>=0) map_str[pos-1]=" »";
                if(ox+1<=15) map_str[pos+1]="« ";
            }
            else if(o && o->type_id==Celeste::FAKE_WALL){
                if (ox + 1 <= 15) map_str[pos + 1] = o->ascii;
                if (oy + 1 <= 15) map_str[pos + 17] = o->ascii;
                if (ox + 1 <= 15 && oy + 1 <= 15) map_str[pos + 18] = o->ascii;
            }
            else if(o&& o->type_id==Celeste::BIG_CHEST){
                if (ox + 1 <= 15) map_str[pos + 1] = "╤╗";
                if (oy + 1 <= 15) map_str[pos + 17] = "║ ";
                if (ox + 1 <= 15 && oy + 1 <= 15) map_str[pos + 18] = " ║";
            }
        }
    }
    for (auto s:map_str) {
        os << s;
    }
    return os;
}
