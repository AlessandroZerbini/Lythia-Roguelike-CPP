#include <iostream>
#include <fstream> //to read/write on files
#include <string>
#include <vector>
#include <regex>  //regular expressions to avoid input errors
#include <algorithm>
#include <random>
#include <iomanip> //to manipulate output formatting
#include <limits>

std::random_device rd;
std::mt19937 g(rd());

enum class WeaponType {
    Physical,
    Ice,
    Fire,
    Thunder,
    Wind,
    Water,
    Ground,
    Dark,
    Light,
    None
};

struct Armor {
    std::string name;
    int damage_reduction;
    int durability;
    int current_durability;
    int evasion;
    WeaponType weakness1;
    WeaponType weakness2;
    WeaponType resistance1;
    WeaponType resistance2;
    bool is_unbreakable;

    Armor() {
        this->name="Unknown";
        this->damage_reduction = 0;
        this->durability = 0;
        this->current_durability=this->durability;
        this->evasion = 0;
        this->weakness1 = WeaponType::None;
        this->weakness2 = WeaponType::None;
        this->resistance1 = WeaponType::None;
        this->resistance2 = WeaponType::None;
        this->is_unbreakable = true;
    }

    Armor(const std::string &name, const int damage_reduction, const int durability, const int evasion, const bool is_unbreakable) {
        this->name=name;
        this->damage_reduction = damage_reduction;
        this->durability = durability;
        this->current_durability=this->durability;
        this->evasion = evasion;
        this->weakness1 = WeaponType::None;
        this->weakness2 = WeaponType::None;
        this->resistance1 = WeaponType::None;
        this->resistance2 = WeaponType::None;
        this->is_unbreakable = is_unbreakable;
    }

    void upgrade_armor() {
        std::uniform_int_distribution<int> distrib(1, 3);
        if (!is_unbreakable) {
            durability += distrib(g);
            current_durability = durability;
        }
        damage_reduction += distrib(g);
        evasion += 2*distrib(g);
    }

    Armor(const std::string &name, const int damage_reduction, const int durability, const int evasion, const WeaponType weakness1, const WeaponType weakness2, const WeaponType resistance1, const WeaponType resistance2,  const bool is_unbreakable) {
        this->name=name;
        this->damage_reduction = damage_reduction;
        this->durability = durability;
        this->current_durability=this->durability;
        this->evasion = evasion;
        this->weakness1 = weakness1;
        this->weakness2 = weakness2;
        this->resistance1 = resistance1;
        this->resistance2 = resistance2;
        this->is_unbreakable = is_unbreakable;
    }

    bool is_broken() const {
        if (is_unbreakable) return false;
        return current_durability <= 0;
    }
};

struct Weapon {
    std::string name;
    WeaponType type;
    int durability;
    int current_durability;
    int hit_rate;
    int damage;
    int crit_rate;
    bool is_infinite;

    Weapon() {
        this->name = "Unknown";
        this->type = WeaponType::None;
        this->durability = 0;
        this->current_durability=this->durability;
        this->hit_rate = 0;
        this->damage = 0;
        this->crit_rate=0;
        this->is_infinite=true;
    }

    Weapon (const std::string &name, const WeaponType type, const int durability, const int hit_rate, const int damage, const int crit_rate, const bool is_infinite) {
        this->name = name;
        this->type = type;
        this->durability = durability;
        this->current_durability=this->durability;
        this->hit_rate = hit_rate;
        this->damage = damage;
        this->crit_rate = crit_rate;
        this->is_infinite=is_infinite;
    }

    bool is_broken() const {
        if (!is_infinite) return current_durability <= 0;
        return false;
    }

    void upgrade_weapon() {
        std::uniform_int_distribution<int> distrib(1, 3);
        if (!is_infinite) {
            durability += distrib(g);
            current_durability = durability;
        }
        damage += distrib(g);
        hit_rate += 2*distrib(g);
    }
};

struct Monster {
    std::string name;
    int base_hp;
    int hp_remaining;
    WeaponType weakness1;
    WeaponType weakness2;
    bool known_weakness1;
    bool known_weakness2;
    WeaponType resistance1;
    WeaponType resistance2;
    bool known_resistance1;
    bool known_resistance2;
    int damage;
    int hit_rate;
    int evasion_rate;
    int crit_rate;
    WeaponType type;

    Monster() {
        this->name = "Unknown";
        this->base_hp = 0;
        this->hp_remaining = base_hp;
        this->weakness1 = WeaponType::None;
        this->weakness2 = WeaponType::None;
        this->known_weakness1 = false;
        this->known_weakness2 = false;
        this->resistance1 = WeaponType::None;
        this->resistance2 = WeaponType::None;
        this->known_resistance1 = false;
        this->known_resistance2 = false;
        this->damage = 0;
        this->hit_rate = 0;
        this->evasion_rate = 0;
        this->crit_rate = 0;
        this->type = WeaponType::None;
    }

    Monster(const std::string &name, const int base_hp, const WeaponType weakness1, const WeaponType weakness2, const WeaponType resistance1, const WeaponType resistance2, const int damage, const int hit_rate, const int evasion_rate, const int crit_rate, const WeaponType type) {
        this->name = name;
        this->base_hp = base_hp;
        this->hp_remaining = base_hp;
        this->weakness1 = weakness1;
        this->weakness2 = weakness2;
        this->known_weakness1 = false;
        this->known_weakness2 = false;
        this->resistance1 = resistance1;
        this->resistance2 = resistance2;
        this->known_resistance1 = false;
        this->known_resistance2 = false;
        this->damage = damage;
        this->hit_rate = hit_rate;
        this->evasion_rate = evasion_rate;
        this->crit_rate = crit_rate;
        this->type = type;
    }

    void upgrade_monster(const int wave_num) {
        this->base_hp += 2*wave_num;
        this->hp_remaining = this->base_hp;
        this->damage += wave_num;
        this->hit_rate += wave_num;
        this->evasion_rate += 2*wave_num;
        this->crit_rate += wave_num;
    }

    bool take_damage (Weapon & weapon, const int player_strength) {
        std::uniform_int_distribution<int> distrib(1, 100);
        int effective_hit_rate = std::max(5, weapon.hit_rate-evasion_rate);
        if (distrib(g)<=effective_hit_rate) {
            if (!weapon.is_infinite && !weapon.is_broken()) weapon.current_durability--;
            int additional_damage = 0;
            if(weapon.type == WeaponType::Physical) additional_damage += player_strength;
            if (weapon.type == weakness1) {
                known_weakness1 = true;
                additional_damage += 3;
            }
            if (weapon.type == weakness2) {
                known_weakness2 = true;
                additional_damage += 2;
            }
            if (weapon.type == resistance1) {
                known_resistance1 = true;
                additional_damage -= 2;
            }
            if (weapon.type == resistance2) {
                known_resistance2 = true;
                additional_damage -= 1;
            }
            int damage_dealt = std::max(0, weapon.damage + additional_damage);
            if (distrib(g)<=weapon.crit_rate) {
                damage_dealt *= 3;
                std::cout << "Wow! You did a critical hit! ";
            }
            std::cout << this->name << " took " << damage_dealt << " damage!" << std::endl;
            hp_remaining -= damage_dealt;
            return true;
        }
        else std::cout << "Oh no, you missed!" << std::endl;
        return false;
    }

    bool is_dead() const {
        return hp_remaining <= 0;
    }
};

struct Boss : Monster {

    WeaponType resistance3;
    bool known_resistance3;

    Boss() {
        this->name = "Unknown";
        this->base_hp = 0;
        this->hp_remaining = base_hp;
        this->weakness1 = WeaponType::None;
        this->weakness2 = WeaponType::None;
        this->known_weakness1 = false;
        this->known_weakness2 = false;
        this->resistance1 = WeaponType::None;
        this->resistance2 = WeaponType::None;
        this->resistance3 = WeaponType::None;
        this->known_resistance1 = false;
        this->known_resistance2 = false;
        this->known_resistance3 = false;
        this->damage = 0;
        this->hit_rate = 0;
        this->evasion_rate = 0;
        this->crit_rate = 0;
        this->type = WeaponType::None;
    }

    Boss(const std::string &name, const int base_hp, const WeaponType weakness1, const WeaponType weakness2, const WeaponType resistance1, const WeaponType resistance2, const WeaponType resistance3, const int damage, const int hit_rate, const int evasion_rate, const int crit_rate, const WeaponType type) {
        this->name = name;
        this->base_hp = base_hp;
        this->hp_remaining = base_hp;
        this->weakness1 = weakness1;
        this->weakness2 = weakness2;
        this->known_weakness1 = false;
        this->known_weakness2 = false;
        this->resistance1 = resistance1;
        this->resistance2 = resistance2;
        this->resistance3 = resistance3;
        this->known_resistance1 = false;
        this->known_resistance2 = false;
        this->known_resistance3 = false;
        this->damage = damage;
        this->hit_rate = hit_rate;
        this->evasion_rate = evasion_rate;
        this->crit_rate = crit_rate;
        this->type = type;
    }

    void upgrade_monster(const int wave_num) {
        this->base_hp += 5*wave_num;
        this->hp_remaining = this->base_hp;
        this->damage += 3*wave_num;
        this->hit_rate += 2*wave_num;
        this->evasion_rate += 2*wave_num;
        this->crit_rate += 3*wave_num;
    }

    bool take_damage (Weapon & weapon, const int player_strength) {
        std::uniform_int_distribution<int> distrib(1, 100);
        int effective_hit_rate = std::max(5, weapon.hit_rate-evasion_rate);
        if (distrib(g)<=effective_hit_rate) {
            if (!weapon.is_infinite && !weapon.is_broken()) weapon.current_durability--;
            int additional_damage = 0;
            if(weapon.type == WeaponType::Physical) additional_damage += player_strength;
            if (weapon.type == weakness1) {
                known_weakness1 = true;
                additional_damage += 2;
            }
            if (weapon.type == weakness2) {
                known_weakness2 = true;
                additional_damage += 1;
            }
            if (weapon.type == resistance1) {
                known_resistance1 = true;
                additional_damage -= 3;
            }
            if (weapon.type == resistance2) {
                known_resistance2 = true;
                additional_damage -= 2;
            }
            if (weapon.type == resistance3) {
                known_resistance3 = true;
                additional_damage -= 1;
            }
            int damage_dealt = std::max(0, weapon.damage + additional_damage);
            if (distrib(g)<=weapon.crit_rate) {
                damage_dealt *= 3;
                std::cout << "Wow! You did a critical hit! ";
            }
            std::cout << this->name << " took " << damage_dealt << " damage!" << std::endl;
            hp_remaining -= damage_dealt;
            return true;
        }
        else std::cout << "Oh no, you missed!" << std::endl;
        return false;
    }

};

std::vector<Weapon> list_of_weapons = {
    {"Basic sword", WeaponType::Physical, 10, 80, 6, 5, false},
    {"Ice magic", WeaponType::Ice, 0, 90, 3, 5, true},
    {"Fire magic", WeaponType::Fire, 0, 70, 7, 10, true},
    {"Thunder magic", WeaponType::Thunder, 0, 75, 6, 25, true},
    {"Wind magic", WeaponType::Wind, 0, 85, 3, 15, true},
    {"Water magic", WeaponType::Water, 0, 90, 5, 0, true},
    {"Ground magic", WeaponType::Ground, 0, 70, 8, 0, true},
    {"Dark magic", WeaponType::Dark, 0, 60, 10, 25, true},
    {"Light magic", WeaponType::Light, 0, 85, 5, 25, true},
    {"Magic Sword", WeaponType::Physical, 20, 85, 5, 5, false},
    {"Dark magic Sword", WeaponType::Dark, 5, 75, 8, 25, false},
    {"Light magic Sword", WeaponType::Light, 25, 95, 5, 5, false}
};

std::vector<Monster> list_of_monsters = {
    {"monster1", 12, WeaponType::None, WeaponType::None, WeaponType::None, WeaponType::None, 6, 80, 10, 5, WeaponType::Physical},
    {"monster2", 15, WeaponType::Ice, WeaponType::None, WeaponType::Physical, WeaponType::None, 4, 85, 0, 0, WeaponType::Physical},
    {"monster3", 18, WeaponType::Fire, WeaponType::None, WeaponType::Ice, WeaponType::None, 5, 75, 0, 10, WeaponType::Ice},
    {"monster4", 20, WeaponType::Ice, WeaponType::Thunder, WeaponType::Wind, WeaponType::Physical, 5, 70, 10, 5, WeaponType::Wind},
    {"monster5", 14, WeaponType::Light, WeaponType::Dark, WeaponType::None, WeaponType::None, 4, 90, 0, 15, WeaponType::Thunder},
    {"monster6", 20, WeaponType::Wind, WeaponType::Water, WeaponType::Fire, WeaponType::Ice, 3, 85, 0, 5, WeaponType::Fire},
    {"monster7", 10, WeaponType::None, WeaponType::None, WeaponType::Physical, WeaponType::Dark, 7, 85, 15, 15, WeaponType::Dark},
    {"monster8", 12, WeaponType::Dark, WeaponType::Thunder, WeaponType::Light, WeaponType::None, 6, 75, 20, 2, WeaponType::Light},
    {"monster9", 24, WeaponType::None, WeaponType::None, WeaponType::Fire, WeaponType::None, 2, 75, 0, 10, WeaponType::Physical},
    {"monster10", 16, WeaponType::Thunder, WeaponType::Wind, WeaponType::None, WeaponType::None, 5, 80, 0, 5, WeaponType::Ice},
    {"monster11", 22, WeaponType::Thunder, WeaponType::Ice, WeaponType::Water, WeaponType::Dark, 3, 80, 15, 0, WeaponType::Water},
    {"monster12", 14, WeaponType::Ground, WeaponType::None, WeaponType::Thunder, WeaponType::Wind, 6, 75, 0, 15, WeaponType::Thunder},
    {"monster13", 18, WeaponType::Water, WeaponType::Ground, WeaponType::Ice, WeaponType::None, 4, 85, 5, 10, WeaponType::Fire},
    {"monster14", 16, WeaponType::Wind, WeaponType::Water, WeaponType::Thunder, WeaponType::Physical, 5, 80, 0, 0, WeaponType::Ground}
};

std::vector<Boss> list_of_bosses = {
    {"boss1", 24, WeaponType::None, WeaponType::None, WeaponType::None, WeaponType::None, WeaponType::None, 6, 80, 10, 5, WeaponType::Physical},
    {"boss2", 30, WeaponType::Ice, WeaponType::None, WeaponType::None, WeaponType::Physical, WeaponType::None, 4, 85, 0, 0, WeaponType::Physical},
    {"boss3", 36, WeaponType::Fire, WeaponType::None, WeaponType::Ice, WeaponType::Physical, WeaponType::None, 5, 75, 0, 10, WeaponType::Ice},
    {"boss4", 28, WeaponType::Light, WeaponType::Dark, WeaponType::None, WeaponType::None, WeaponType::None,  4, 90, 0, 15, WeaponType::Thunder},
    {"boss5", 40, WeaponType::Wind, WeaponType::Water, WeaponType::Fire, WeaponType::Ice, WeaponType::Light, 3, 85, 0, 5, WeaponType::Fire},
    {"boss6", 20, WeaponType::None, WeaponType::None, WeaponType::Physical, WeaponType::Dark, WeaponType::Fire, 7, 85, 15, 15, WeaponType::Dark},
    {"boss7", 24, WeaponType::Dark, WeaponType::Thunder, WeaponType::Light, WeaponType::None, WeaponType::None, 6, 75, 20, 2, WeaponType::Light},
    {"boss8", 48, WeaponType::None, WeaponType::None, WeaponType::Fire, WeaponType::Ground, WeaponType::None, 2, 75, 0, 10, WeaponType::Physical},
    {"boss9", 32, WeaponType::Thunder, WeaponType::Wind, WeaponType::Physical, WeaponType::None, WeaponType::None, 5, 80, 0, 5, WeaponType::Ice},
    {"boss10", 44, WeaponType::Thunder, WeaponType::Ice, WeaponType::Water, WeaponType::Dark, WeaponType::Fire, 3, 80, 15, 0, WeaponType::Water},
    {"boss11", 28, WeaponType::Ground, WeaponType::None, WeaponType::Thunder, WeaponType::Wind, WeaponType::None, 6, 75, 0, 15, WeaponType::Thunder},
    {"boss12", 36, WeaponType::Water, WeaponType::Ground, WeaponType::Ice, WeaponType::None, WeaponType::None, 4, 85, 5, 10, WeaponType::Fire},
    {"boss13", 32, WeaponType::Wind, WeaponType::Water, WeaponType::Thunder, WeaponType::Physical, WeaponType::None, 5, 80, 0, 0, WeaponType::Ground}
};

std::vector<Armor> list_of_armor = {
    {"Basic armor", 0, 0, 0, true},
    {"armor 1", 1, 35, 0, false},
    {"armor 2", 2, 35, 5, false},
    {"armor 3", 3, 30, 5, false},
    {"armor 4", 1, 40, 20, false},
    {"armor 5", 2, 55, 10, false},
    {"armor 6", 5, 35, 0, false},
    {"armor 7", 3, 30, 25, false},
    {"armor 8", 5, 30, 15, false},
    {"armor 9", 7, 25, 0, false},
    {"armor 10", 6, 30, 10, false},
};

struct Player {
    std::string username;
    int base_hp;
    int hp_remaining;
    int strength;
    int resistance;
    std::vector<Weapon> weapons_inventory;
    std::vector<Armor> armor_inventory;
    std::vector<Weapon> current_weapons;
    Armor current_armor;

    Player(const std::string &username) {
        this->username = username;
        this->base_hp = 100;
        this->hp_remaining = base_hp;
        this->strength = 0;
        this->resistance = 0;
        this->weapons_inventory = {list_of_weapons[0], list_of_weapons[1], list_of_weapons[2]};
        this->current_weapons = this->weapons_inventory;
        this->armor_inventory = {list_of_armor[0]};
        this->current_armor = armor_inventory[0];
    }

    void take_damage(const Monster & monster) {
        std::uniform_int_distribution<int> distrib(1, 100);
        int effective_hit_rate = std::min(95, monster.hit_rate-current_armor.evasion);
        if (distrib(g)<=effective_hit_rate) {
            if(!current_armor.is_unbreakable && !current_armor.is_broken()) current_armor.current_durability--;
            int additional_damage = 0;
            if(!current_armor.is_broken()) {
                additional_damage-=current_armor.damage_reduction;
                if (monster.type == current_armor.weakness1) additional_damage += 3;
                if (monster.type == current_armor.weakness2) additional_damage += 2;
                if (monster.type == current_armor.resistance1) additional_damage-=2;
                if (monster.type == current_armor.resistance2) additional_damage-=1;
            }
            additional_damage-=resistance;
            int damage_dealt = std::max(0, monster.damage + additional_damage);
            if (distrib(g)<=monster.crit_rate) {
                damage_dealt *= 3;
                std::cout << "Oh no! You received a critical hit! ";
            }
            std::cout << "You took " << damage_dealt << " damage!" << std::endl;
            hp_remaining -= damage_dealt;
        }
        else std::cout << "The monster missed! It's now your turn to counterattack!" << std::endl;
    }

    void upgrade_health() {
        std::uniform_int_distribution<int> distrib(1, 3);
        base_hp += 10*distrib(g);
        hp_remaining = base_hp;
    }

    void upgrade_strength() {
        std::uniform_int_distribution<int> distrib(1, 3);
        strength += 2*distrib(g);
    }

    void upgrade_resistance() {
        std::uniform_int_distribution<int> distrib(1, 3);
        resistance += distrib(g);
    }

    bool has_lost() const {
        return hp_remaining <= 0;
    }
};

#ifdef _WIN32
#include <windows.h>

void enable_ansi() {
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut == INVALID_HANDLE_VALUE) return;
    DWORD dwMode = 0;
    if (!GetConsoleMode(hOut, &dwMode)) return;
    dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    SetConsoleMode(hOut, dwMode);
}
#else
void enable_ansi() {}
#endif

void wait();
void clear_screen ();
std::string type_to_string (WeaponType type);
void print_weapons_inventory (const Player & player);
void print_armor_inventory (const Player & player);
void print_current_weapons (const Player & player, const Monster & monster);
void print_current_weapons (const Player & player, const Boss & boss);
void update_weaknesses (const Weapon & weapon, std::vector<Monster> & monsters_of_the_wave, int index_monster);
void update_weaknesses (const Weapon & weapon, std::vector<Boss> & bosses_of_the_wave, int index_boss);
bool start_wave (Player & player, int wave_num);
void update_current_equipment(Player & player);
void exclude_weapons (const Player & player, std::vector<Weapon> & available_weapons);
void exclude_armor (const Player & player, std::vector<Armor> & available_armor);
bool boost(Player & player, int choice);
void boost_player (Player & player);
void print_lore (int wave_num);

void wait() {
    std::cout << "Press Enter to continue" << std::endl;
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

void clear_screen () {
    // \033[2J = clears visible buffer
    // \033[1;1H = move the cursor to top-left
    // \033[3J -> deletes all scrollback buffer
    std::cout << "\033[H\033[2J\033[3J";
}

void print_lore (const int wave_num) {
    switch (wave_num) {
        case 0: std::cout << "Lythia... A kingdom full of life, where war was considered nothing more than a legend by its inhabitants. The King, known as \"The Founder,\" was beloved by everyone in the realm. \n"
                             "However, this would not be enough to change the destiny that awaited him.\n"
                 "One day, the King's right hand—the Sorcerer—had a vision: a vision that showed HELL.\n"
                 "The realm was thrown into pure chaos by unknown creatures, guided by the Devil itself.\n"
                 "These creatures had only ever been mentioned in ancient textbooks, which most considered \"foolish tales for children.\"\n"
                 "Luckily, the King was not foolish.\n"
                 "He began studying these ancient books until he stumbled upon a myth about an unnatural figure: the Knight of Doom, who would appear when the War of Destiny began.\n"
                 "So, was everything going to be fine? Would everyone be saved? These were the hopes of the Founder, but unfortunately... he was wrong.\n"
                 "On an ordinary night, something terrible happened.\n"
                 "The \"Devil\" appeared outside the realm with an almost infinite army of unknown creatures known as the \"Monsters of Eternal Suffering.\"\n"
                 "The King tried to fight the Devil alongside his army, but he died trying to save the Sorcerer, who managed to escape.\n"
                 "Thanks to his escape, the Sorcerer was able to evacuate what remained of the population.\n"
                 "That same night, he had one final vision that guided him to a knight—the only one who had survived the battle against the monsters.\n"
                 "He had no name, only a title: The Knight of Doom. And he will avenge all the fallen and destroy the Devil, even if it costs him his life." << std::endl;
        break;
        default: break;
    }
    wait();
    clear_screen();
}

std::string type_to_string (const WeaponType type){
    switch (type) {
        case WeaponType::Physical: return "Physical";
        case WeaponType::Ice: return "Ice";
        case WeaponType::Fire: return "Fire";
        case WeaponType::Thunder: return "Thunder";
        case WeaponType::Wind: return "Wind";
        case WeaponType::Water: return "Water";
        case WeaponType::Ground: return "Ground";
        case WeaponType::Dark: return "Dark";
        case WeaponType::Light: return "Light";
        case WeaponType::None: return "None";
        default: return "Unknown";
    }
}

void print_weapons_inventory (const Player & player) {
    std::cout << std::string(105, '*') << std::endl;
    std::cout << std::left
              << std::setw(10) << "Number"
              << std::setw(20) << "Name"
              << std::setw(15) << "Strength"
              << std::setw(15) << "Precision"
              << std::setw(15) << "Criticals %"
              << std::setw(15) << "Durability"
              << std::setw(15) << "Type" << std::endl;
    std::cout << std::string(105, '-') << std::endl;

    for (int i=0; i<player.weapons_inventory.size(); i++) {
        std::string durability = std::to_string(player.weapons_inventory[i].durability);
        if (player.weapons_inventory[i].is_infinite) durability = "infinite";
        std::cout << std::setw(10) << i+1
                  << std::setw(20) << player.weapons_inventory[i].name
                  << std::setw(15) << player.weapons_inventory[i].damage
                  << std::setw(15) << player.weapons_inventory[i].hit_rate
                  << std::setw(15) << player.weapons_inventory[i].crit_rate
                  << std::setw(15) << durability
                  << std::setw(15) << type_to_string(player.weapons_inventory[i].type) << std::endl;
        if (i!=player.weapons_inventory.size()-1) std::cout << std::string(105, '-') << std::endl;
        else std::cout << std::endl;
    }
    std::cout << std::string(105, '*') << std::endl;
}

void print_armor_inventory (const Player & player) {
    std::cout << std::string(135, '*') << std::endl;
    std::cout << std::left
              << std::setw(10) << "Number"
              << std::setw(20) << "Name"
              << std::setw(15) << "Defense"
              << std::setw(15) << "Durability"
              << std::setw(15) << "Evasion"
              << std::setw(15) << "Weakness 1"
              << std::setw(15) << "Weakness 2"
              << std::setw(15) << "Resistance 1"
              << std::setw(15) << "Resistance 2" << std::endl;
    std::cout << std::string(130, '-') << std::endl;

    for (int i=0; i<player.armor_inventory.size(); i++) {
        std::string durability = std::to_string(player.armor_inventory[i].durability);
        if (player.armor_inventory[i].is_unbreakable) durability = "infinite";
        std::cout << std::setw(10) << i+1
                  << std::setw(20) << player.armor_inventory[i].name
                  << std::setw(15) << player.armor_inventory[i].damage_reduction
                  << std::setw(15) << durability
                  << std::setw(15) << player.armor_inventory[i].evasion
                  << std::setw(15) << type_to_string(player.armor_inventory[i].weakness1)
                  << std::setw(15) << type_to_string(player.armor_inventory[i].weakness2)
                  << std::setw(15) << type_to_string(player.armor_inventory[i].resistance1)
                  << std::setw(15) << type_to_string(player.armor_inventory[i].resistance2) << std::endl;
        if (i!=player.armor_inventory.size()-1) std::cout << std::string(135, '-') << std::endl;
        else std::cout << std::endl;
    }
    std::cout << std::string(135, '*') << std::endl;
}

void print_current_weapons (const Player & player, const Monster & monster) {
    std::cout << std::string(105, '*') << std::endl;
    std::cout << std::left
              << std::setw(10) << "Number"
              << std::setw(20) << "Name"
              << std::setw(15) << "Damage"
              << std::setw(15) << "Precision"
              << std::setw(15) << "Criticals %"
              << std::setw(15) << "Durability"
              << std::setw(15) << "Type" << std::endl;
    std::cout << std::string(105, '-') << std::endl;

    for (int i=0; i<player.current_weapons.size(); i++) {
        std::string durability = std::to_string(player.current_weapons[i].current_durability);
        if (player.current_weapons[i].is_infinite) durability = "infinite";
        int effective_hit_rate = std::max(5, player.current_weapons[i].hit_rate-monster.evasion_rate);
        int damage = player.current_weapons[i].damage;
        if(monster.known_resistance1 && player.current_weapons[i].type == monster.resistance1) damage -= 2;
        if(monster.known_resistance2 && player.current_weapons[i].type == monster.resistance2) damage -= 1;
        if(monster.known_weakness1 && player.current_weapons[i].type == monster.weakness1) damage += 3;
        if(monster.known_weakness2 && player.current_weapons[i].type == monster.weakness2) damage += 2;
        if(player.current_weapons[i].type == WeaponType::Physical) damage += player.strength;
        damage = std::max (0, damage);
        std::cout << std::setw(10) << i+1
                  << std::setw(20) << player.current_weapons[i].name
                  << std::setw(15) << damage
                  << std::setw(15) << effective_hit_rate
                  << std::setw(15) << player.current_weapons[i].crit_rate
                  << std::setw(15) << durability
                  << std::setw(15) << type_to_string(player.current_weapons[i].type) << std::endl;
        if (i!=player.current_weapons.size()-1) std::cout << std::string(105, '-') << std::endl;
        else std::cout << std::endl;
    }
    std::cout << std::string(105, '*') << std::endl;
}

void print_current_weapons (const Player & player, const Boss & boss) {
    std::cout << std::string(105, '*') << std::endl;
    std::cout << std::left
              << std::setw(10) << "Number"
              << std::setw(20) << "Name"
              << std::setw(15) << "Damage"
              << std::setw(15) << "Precision"
              << std::setw(15) << "Criticals %"
              << std::setw(15) << "Durability"
              << std::setw(15) << "Type" << std::endl;
    std::cout << std::string(105, '-') << std::endl;

    for (int i=0; i<player.current_weapons.size(); i++) {
        std::string durability = std::to_string(player.current_weapons[i].current_durability);
        if (player.current_weapons[i].is_infinite) durability = "infinite";
        int effective_hit_rate = std::max(5, player.current_weapons[i].hit_rate-boss.evasion_rate);
        int damage = player.current_weapons[i].damage;
        if(boss.known_resistance1 && player.current_weapons[i].type == boss.resistance1) damage -= 3;
        if(boss.known_resistance2 && player.current_weapons[i].type == boss.resistance2) damage -= 2;
        if(boss.known_resistance3 && player.current_weapons[i].type == boss.resistance3) damage -= 1;
        if(boss.known_weakness1 && player.current_weapons[i].type == boss.weakness1) damage += 3;
        if(boss.known_weakness2 && player.current_weapons[i].type == boss.weakness2) damage += 2;
        if(player.current_weapons[i].type == WeaponType::Physical) damage += player.strength;
        damage = std::max (0, damage);
        std::cout << std::setw(10) << i+1
                  << std::setw(20) << player.current_weapons[i].name
                  << std::setw(15) << damage
                  << std::setw(15) << effective_hit_rate
                  << std::setw(15) << player.current_weapons[i].crit_rate
                  << std::setw(15) << durability
                  << std::setw(15) << type_to_string(player.current_weapons[i].type) << std::endl;
        if (i!=player.current_weapons.size()-1) std::cout << std::string(105, '-') << std::endl;
        else std::cout << std::endl;
    }
    std::cout << std::string(105, '*') << std::endl;
}

void update_weaknesses (const Weapon & weapon, std::vector<Monster> & monsters_of_the_wave, int index_monster) {
    if(weapon.type == monsters_of_the_wave[index_monster].weakness1) monsters_of_the_wave[index_monster].known_weakness1 = true;
    if(weapon.type == monsters_of_the_wave[index_monster].weakness2) monsters_of_the_wave[index_monster].known_weakness2 = true;
    if(weapon.type == monsters_of_the_wave[index_monster].resistance1) monsters_of_the_wave[index_monster].known_resistance1 = true;
    if(weapon.type == monsters_of_the_wave[index_monster].resistance2) monsters_of_the_wave[index_monster].known_resistance2 = true;
}

void update_weaknesses (const Weapon & weapon, std::vector<Boss> & bosses_of_the_wave, int index_boss) {
    if(weapon.type == bosses_of_the_wave[index_boss].weakness1) bosses_of_the_wave[index_boss].known_weakness1 = true;
    if(weapon.type == bosses_of_the_wave[index_boss].weakness2) bosses_of_the_wave[index_boss].known_weakness2 = true;
    if(weapon.type == bosses_of_the_wave[index_boss].resistance1) bosses_of_the_wave[index_boss].known_resistance1 = true;
    if(weapon.type == bosses_of_the_wave[index_boss].resistance2) bosses_of_the_wave[index_boss].known_resistance2 = true;
}

bool start_wave (Player & player, const int wave_num) {

    print_lore(wave_num);

    for (int i = 0; i < player.current_weapons.size(); i++) {
        player.current_weapons[i].current_durability = player.current_weapons[i].durability;
    }

    player.current_armor.current_durability = player.current_armor.durability;;

    std::vector<Monster> monsters_of_the_wave;
    std::vector<Boss> bosses_of_the_wave;

    for(int i = 0; i < 3+wave_num; i++) {
        if (i<list_of_monsters.size()) monsters_of_the_wave.push_back(list_of_monsters[i]);
    }
    for(int i = 0; i < +wave_num; i++) {
        if (i<list_of_bosses.size()) bosses_of_the_wave.push_back(list_of_bosses[i]);
    }

    for (int i=0; i < monsters_of_the_wave.size(); i++) monsters_of_the_wave[i].upgrade_monster(wave_num);
    for (int i=0; i < bosses_of_the_wave.size(); i++) bosses_of_the_wave[i].upgrade_monster(wave_num);

    for(int i = 0; i < 3 + wave_num*2; i++) {
        clear_screen();
        std::uniform_int_distribution<int> distrib(1, static_cast<int>(monsters_of_the_wave.size()));
        int index_monster = distrib(g)-1;
        Monster monster = monsters_of_the_wave[index_monster];
        std::cout << monster.name << " has appeared!" << std::endl;
        wait();
        do {
            print_current_weapons(player, monster);
            std::string weapon_chosen;
            bool valid_weapon = false;
            do {
                std::cout << "Which weapon do you want to use?" << std::endl;
                std::getline(std::cin, weapon_chosen);
                if (player.current_weapons[stoi(weapon_chosen)-1].is_broken()) std::cout << "That weapon is broken, please select another one" << std::endl;
                else valid_weapon = true;
            }while (!valid_weapon);
            bool hit = monster.take_damage(player.current_weapons[stoi(weapon_chosen)-1],player.strength);
            if(hit) update_weaknesses(player.current_weapons[stoi(weapon_chosen)-1], monsters_of_the_wave, index_monster);
            wait();

            if (!monster.is_dead()) {
                std::cout << monster.name << " is going to attack you!" << std::endl;
                player.take_damage(monster);
            }
            else std::cout << "You killed " << monster.name << "!" << std::endl;
            wait();
            clear_screen();
        }while (!player.has_lost() && !monster.is_dead());
        if (player.has_lost()) return false;
    }
    if (wave_num>=2) {
        clear_screen();
        std::uniform_int_distribution<int> distrib(1, static_cast<int>(bosses_of_the_wave.size()));
        int index_boss = distrib(g)-1;
        Boss boss = bosses_of_the_wave[index_boss];
        std::cout << std::endl << "Boss " << boss.name << " has appeared!" << std::endl;
        wait();
        do {
            print_current_weapons(player, boss);
            std::cout << "Which weapon do you want to use?" << std::endl;
            std::string weapon_chosen;
            std::getline(std::cin, weapon_chosen);
            bool hit = boss.take_damage(player.current_weapons[stoi(weapon_chosen)-1],player.strength);
            if(hit) update_weaknesses(player.current_weapons[stoi(weapon_chosen)-1], bosses_of_the_wave, index_boss);
            wait();

            if (!boss.is_dead()) {
                std::cout << boss.name << " is going to attack you!" << std::endl;
                player.take_damage(boss);
            }
            else std::cout << "You killed " << boss.name << "!" << std::endl;
            wait();
            clear_screen();
        }while (!player.has_lost() && !boss.is_dead());
        if (player.has_lost()) return false;
    }
    std::cout << std::endl << "You completed the wave number " << wave_num+1 << "!" << std::endl << std::endl;
    boost_player(player);
    return true;
}

void update_current_equipment(Player & player) {
    for (int i = 0; i < player.current_weapons.size(); i++) {
        for (int j = 0; j < player.weapons_inventory.size(); j++) {
            if (player.current_weapons[i].name == player.weapons_inventory[j].name) {
                player.current_weapons[i]=player.weapons_inventory[j];
            }
        }
    }
    for (int i = 0; i < player.armor_inventory.size(); i++) {
        if (player.current_armor.name == player.armor_inventory[i].name) {
            player.current_armor = player.armor_inventory[i];
        }
    }
}

void exclude_weapons (const Player & player, std::vector<Weapon> & available_weapons) {
    std::vector<int> index;
    for (int i = 0; i < player.weapons_inventory.size(); i++) {
        for (int j = 0; j < available_weapons.size(); j++) {
            if (player.weapons_inventory[i].name == available_weapons[j].name) index.push_back(j);
        }
    }
    int weapons_excluded = 0;
    for (int i = 0; i < index.size(); i++) {
        available_weapons.erase(available_weapons.begin()+index[i]-weapons_excluded);
        weapons_excluded++;
    }
}


void exclude_armor (const Player & player, std::vector<Armor> & available_armor) {
    std::vector<int> index;
    for (int i = 0; i < player.armor_inventory.size(); i++) {
        for (int j = 0; j < available_armor.size(); j++) {
            if (player.armor_inventory[i].name == available_armor[j].name) index.push_back(j);
        }
    }
    int armors_excluded = 0;
    for (int i = 0; i < index.size(); i++) {
        available_armor.erase(available_armor.begin()+index[i]-armors_excluded);
        armors_excluded++;
    }
}

bool boost(Player & player, int choice) {
    switch (choice) {
        case 1: {
            std::string item_choice;
            std::cout << "What do you want to upgrade? " << std::endl << "1: Weapons, 2: Armors" << std::endl;
            std::getline (std::cin, item_choice);
            if (item_choice == "1") {
                print_weapons_inventory(player);
                std::cout << "Which weapon do you want to upgrade?" << std::endl;
                std::getline (std::cin, item_choice);
                player.weapons_inventory[stoi(item_choice)-1].upgrade_weapon();
            }
            if (item_choice == "2") {
                print_armor_inventory(player);
                std::cout << "Which armor do you want to upgrade?" << std::endl;
                std::getline (std::cin, item_choice);
                player.armor_inventory[stoi(item_choice)-1].upgrade_armor();
            }
            update_current_equipment(player);
            return true;
        }
        case 2: {
            std::vector<Weapon> available_weapons = list_of_weapons;
            std::vector<Armor> available_armor = list_of_armor;
            exclude_weapons (player, available_weapons);
            exclude_armor (player, available_armor);
            if (!available_weapons.empty() && !available_armor.empty()) {
                std::uniform_int_distribution<int> distrib(1, 2);
                int random_choice = distrib(g);
                if (random_choice == 1) {
                    std::uniform_int_distribution<int> distribution(1, static_cast<int>(available_weapons.size()));
                    player.weapons_inventory.push_back(available_weapons[distribution(g)-1]);
                    std::cout << "You obtained " << player.weapons_inventory.back().name << "!" << std::endl;
                }
                if (random_choice == 2) {
                    std::uniform_int_distribution<int> distribution(1, static_cast<int>(available_armor.size()));
                    player.armor_inventory.push_back(available_armor[distribution(g)-1]);
                    std::cout << "You obtained " << player.armor_inventory.back().name << "!" << std::endl;
                }
                return true;
            }
            else if (!available_weapons.empty()) {
                std::uniform_int_distribution<int> distribution(1, static_cast<int>(available_weapons.size()));
                player.weapons_inventory.push_back(available_weapons[distribution(g)-1]);
                std::cout << "You obtained " << player.weapons_inventory.back().name << "!" << std::endl;
                return true;
            }
            else if (!available_armor.empty()) {
                std::uniform_int_distribution<int> distribution(1, static_cast<int>(available_armor.size()));
                player.armor_inventory.push_back(available_armor[distribution(g)-1]);
                std::cout << "You obtained " << player.armor_inventory.back().name << "!" << std::endl;
                return true;
            }
            std::cout << "There no more are available items, change your choice" << std::endl;
            return false;
        }
        case 3: player.upgrade_health();
                return true;
        case 4: player.upgrade_strength();
                return true;
        case 5: player.upgrade_resistance();
                return true;
        default: return false;
    }
}

void boost_player (Player & player) {
    std::cout << "Choose what to do before the next wave starts:" << std::endl <<
                "1: Upgrade an item" << std::endl <<
                "2: Look for a new item" << std::endl <<
                "3: Train to increase your health" << std::endl <<
                "4: Train to increase your strength" << std::endl <<
                "5: Train to increase your resistance" << std::endl;
    std::string choice;
    std::getline(std::cin, choice);
    if (!boost(player, stoi(choice))) boost_player(player);
}

int main () {
    enable_ansi();
    std::cout << "Choose your name!" << std::endl;
    std::string username;
    std::getline(std::cin, username);
    Player player (username);
    wait();
    clear_screen();
    int wave_num = 0;

    while (start_wave(player, wave_num)) {
        wave_num++;
    }
    std::cout << std::endl << "You lost..." << std::endl;

}