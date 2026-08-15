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

struct Resistance {
    WeaponType type;
    int modifier;
    bool known = false;

    Resistance () {
        this->type = WeaponType::None;
        this->modifier = 0;
    }

    Resistance (WeaponType type, int modifier) {
        this->type = type;
        this->modifier = modifier;
    }
};

using Weakness = Resistance;

struct Armor {
    std::string name;
    int damage_reduction;
    int durability;
    int current_durability;
    int evasion;
    std::vector<Resistance> resistances;
    std::vector<Weakness> weaknesses;
    bool is_unbreakable;

    Armor() {
        this->name="Unknown";
        this->damage_reduction = 0;
        this->durability = 0;
        this->current_durability=this->durability;
        this->evasion = 0;
        this->is_unbreakable = true;
    }

    Armor(const std::string &name, const int damage_reduction, const int durability, const int evasion, const bool is_unbreakable) {
        this->name=name;
        this->damage_reduction = damage_reduction;
        this->durability = durability;
        this->current_durability=this->durability;
        this->evasion = evasion;
        this->is_unbreakable = is_unbreakable;
    }

    Armor(const std::string &name, const int damage_reduction, const int durability, const int evasion, const std::vector<Resistance> & resistances, const std::vector<Weakness> & weaknesses,  const bool is_unbreakable) {
        this->name=name;
        this->damage_reduction = damage_reduction;
        this->durability = durability;
        this->current_durability=this->durability;
        this->evasion = evasion;
        this->resistances = resistances;
        this->weaknesses = weaknesses;
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

    [[nodiscard]] bool is_broken() const {
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

    [[nodiscard]] bool is_broken() const {
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
    std::vector<Resistance> resistances;
    std::vector<Weakness> weaknesses;
    int damage;
    int hit_rate;
    int evasion_rate;
    int crit_rate;
    WeaponType type;

    Monster() {
        this->name = "Unknown";
        this->base_hp = 0;
        this->hp_remaining = base_hp;
        this->damage = 0;
        this->hit_rate = 0;
        this->evasion_rate = 0;
        this->crit_rate = 0;
        this->type = WeaponType::None;
    }

    Monster(const std::string &name, const int base_hp, const std::vector<Weakness> & weaknesses, const std::vector<Resistance> & resistances, const int damage, const int hit_rate, const int evasion_rate, const int crit_rate, const WeaponType type) {
        this->name = name;
        this->base_hp = base_hp;
        this->hp_remaining = base_hp;
        this->resistances = resistances;
        this->weaknesses = weaknesses;
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

    [[nodiscard]] int incoming_damage (const Weapon & weapon, int strength) const {
        int incoming_damage = weapon.damage;
        if (weapon.type == WeaponType::Physical) incoming_damage += strength;
        for (const auto & r : resistances) {
            if (weapon.type == r.type && r.known) incoming_damage += r.modifier;
        }
        for (const auto & w : weaknesses) {
            if (weapon.type == w.type && w.known) incoming_damage += w.modifier;
        }
        return std::max(0,incoming_damage);
    }

    void reveal_affinities (const Weapon & weapon) {
        for (auto & r : resistances) {
            if (weapon.type == r.type) r.known = true;
        }
        for (auto & w : weaknesses) {
            if (weapon.type == w.type) w.known = true;
        }
    }

    [[nodiscard]] bool take_damage (Weapon & weapon, const int player_strength) {
        std::uniform_int_distribution<int> distrib(1, 100);
        int effective_hit_rate = std::max(5, weapon.hit_rate-evasion_rate);
        if (distrib(g)<=effective_hit_rate) {
            if (!weapon.is_infinite && !weapon.is_broken()) weapon.current_durability--;
            int additional_damage = 0;
            if(weapon.type == WeaponType::Physical) additional_damage += player_strength;
            for (auto & r : resistances) {
                if (weapon.type == r.type) {
                    additional_damage += r.modifier;
                    r.known = true;
                }
            }
            for (auto & w : weaknesses) {
                if (weapon.type == w.type) {
                    additional_damage += w.modifier;
                    w.known = true;
                }
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

    [[nodiscard]] bool is_dead() const {
        return hp_remaining <= 0;
    }
};

std::vector<Weapon> list_of_weapons = {
    {"Basic sword", WeaponType::Physical, 10, 85, 6, 5, false},
    {"Ice magic", WeaponType::Ice, 0, 95, 3, 5, true},
    {"Fire magic", WeaponType::Fire, 0, 75, 7, 10, true},
    {"Thunder magic", WeaponType::Thunder, 0, 80, 6, 25, true},
    {"Wind magic", WeaponType::Wind, 0, 90, 3, 15, true},
    {"Water magic", WeaponType::Water, 0, 95, 5, 0, true},
    {"Ground magic", WeaponType::Ground, 0, 75, 8, 0, true},
    {"Dark magic", WeaponType::Dark, 0, 65, 10, 25, true},
    {"Light magic", WeaponType::Light, 0, 90, 5, 25, true},
    {"Magic Sword", WeaponType::Physical, 20, 90, 5, 5, false},
    {"Dark magic Sword", WeaponType::Dark, 5, 80, 8, 25, false},
    {"Light magic Sword", WeaponType::Light, 25, 100, 5, 5, false}
};

std::vector<Monster> list_of_monsters = {
    {"monster1", 12, {}, {}, 4, 80, 10, 5, WeaponType::Physical},
    {"monster2", 15, {Weakness(WeaponType::Ice, 2)}, {Resistance(WeaponType::Physical, -2)}, 2, 85, 0, 0, WeaponType::Physical},
    {"monster3", 18, {Weakness(WeaponType::Fire, 2)}, {Resistance(WeaponType::Ice, -2)}, 3, 75, 0, 10, WeaponType::Ice},
    {"monster4", 20, {Weakness(WeaponType::Ice, 2), Weakness(WeaponType::Thunder, 1)}, {Resistance(WeaponType::Wind, -2), Resistance(WeaponType::Physical, -1)}, 3, 70, 10, 5, WeaponType::Wind},
    {"monster5", 14, {Weakness(WeaponType::Light, 2), Weakness(WeaponType::Dark, 1)}, {}, 2, 90, 0, 15, WeaponType::Thunder},
    {"monster6", 20, {Weakness(WeaponType::Wind, 2), Weakness(WeaponType::Water, 1)}, {Resistance(WeaponType::Fire, -2), Resistance(WeaponType::Ice, -1)}, 1, 85, 0, 5, WeaponType::Fire},
    {"monster7", 10, {}, {Resistance(WeaponType::Physical, -2), Resistance(WeaponType::Dark, -1)}, 5, 85, 15, 15, WeaponType::Dark},
    {"monster8", 12, {Weakness(WeaponType::Dark, 2), Weakness(WeaponType::Thunder, 1)}, {Resistance(WeaponType::Light, -2)}, 4, 75, 20, 2, WeaponType::Light},
    {"monster9", 24, {}, {Resistance(WeaponType::Fire, -2)}, 1, 75, 0, 10, WeaponType::Physical},
    {"monster10", 16, {Weakness(WeaponType::Thunder, 2), Weakness(WeaponType::Wind, 1)}, {}, 3, 80, 0, 5, WeaponType::Ice},
    {"monster11", 22, {Weakness(WeaponType::Thunder, 2), Weakness(WeaponType::Ice, 1)}, {Resistance(WeaponType::Water, -2), Resistance(WeaponType::Dark, -1)}, 1, 80, 15, 0, WeaponType::Water},
    {"monster12", 14, {Weakness(WeaponType::Ground, 2)}, {Resistance(WeaponType::Thunder, -2), Resistance(WeaponType::Wind, -1)}, 4, 75, 0, 15, WeaponType::Thunder},
    {"monster13", 18, {Weakness(WeaponType::Water, 2), Weakness(WeaponType::Ground, 1)}, {Resistance(WeaponType::Ice, -2)}, 2, 85, 5, 10, WeaponType::Fire},
    {"monster14", 16, {Weakness(WeaponType::Wind, 2), Weakness(WeaponType::Water, 1)}, {Resistance(WeaponType::Thunder, -2), Resistance(WeaponType::Physical, -1)}, 3, 80, 0, 0, WeaponType::Ground}
};

std::vector<Monster> list_of_bosses = {
    {"boss1", 24, {}, {}, 6, 80, 10, 5, WeaponType::Physical},
    {"boss2", 30, {Weakness(WeaponType::Ice, 2)}, {Resistance(WeaponType::Physical, -2)}, 4, 85, 0, 0, WeaponType::Physical},
    {"boss3", 36, {Weakness(WeaponType::Fire, 2)}, {Resistance(WeaponType::Ice, -3), Resistance(WeaponType::Physical, -2)}, 5, 75, 0, 10, WeaponType::Ice},
    {"boss4", 28, {Weakness(WeaponType::Light, 2), Weakness(WeaponType::Dark, 1)}, {}, 4, 90, 0, 15, WeaponType::Thunder},
    {"boss5", 40, {Weakness(WeaponType::Wind, 2), Weakness(WeaponType::Water, 1)}, {Resistance(WeaponType::Fire, -3), Resistance(WeaponType::Ice, -2), Resistance(WeaponType::Light, -1)}, 3, 85, 0, 5, WeaponType::Fire},
    {"boss6", 20, {}, {Resistance(WeaponType::Physical, -3), Resistance(WeaponType::Dark, -2), Resistance(WeaponType::Fire, -1)}, 7, 85, 15, 15, WeaponType::Dark},
    {"boss7", 24, {Weakness(WeaponType::Dark, 2), Weakness(WeaponType::Thunder, 1)}, {Resistance(WeaponType::Light, -1)}, 6, 75, 20, 2, WeaponType::Light},
    {"boss8", 48, {}, {Resistance(WeaponType::Fire, -3), Resistance(WeaponType::Ground, -2)}, 2, 75, 0, 10, WeaponType::Physical},
    {"boss9", 32, {Weakness(WeaponType::Thunder, 2), Weakness(WeaponType::Wind, 1)}, {Resistance(WeaponType::Physical, -1)}, 5, 80, 0, 5, WeaponType::Ice},
    {"boss10", 44, {Weakness(WeaponType::Thunder, 2), Weakness(WeaponType::Ice, 1)}, {Resistance(WeaponType::Water, -3), Resistance(WeaponType::Dark, -2), Resistance(WeaponType::Fire, -1)}, 3, 80, 15, 0, WeaponType::Water},
    {"boss11", 28, {Weakness(WeaponType::Ground, 2)}, {Resistance(WeaponType::Thunder, -3), Resistance(WeaponType::Wind, -2)}, 6, 75, 0, 15, WeaponType::Thunder},
    {"boss12", 36, {Weakness(WeaponType::Water, 2), Weakness(WeaponType::Ground, 1)}, {Resistance(WeaponType::Ice, -1)}, 4, 85, 5, 10, WeaponType::Fire},
    {"boss13", 32, {Weakness(WeaponType::Wind, 2), Weakness(WeaponType::Water, 1)}, {Resistance(WeaponType::Thunder, -3), Resistance(WeaponType::Physical, -2)}, 5, 80, 0, 0, WeaponType::Ground}
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
            int additional_damage = 0;
            if(!current_armor.is_broken()) {
                additional_damage-=current_armor.damage_reduction;
                for (const auto & r : current_armor.resistances) {
                    if (monster.type == r.type) additional_damage += r.modifier;
                }
                for (const auto & w : current_armor.weaknesses) {
                    if (monster.type == w.type) additional_damage += w.modifier;
                }
            }
            if(!current_armor.is_unbreakable && !current_armor.is_broken()) current_armor.current_durability--;
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

    [[nodiscard]] bool has_lost() const {
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
void print_current_armor (const Player & player);
void print_current_weapons (const Player & player, const Monster & monster);
void print_current_weapons (const Player & player);
bool start_wave (Player & player, int wave_num);
void update_current_equipment(Player & player);
void exclude_weapons (const Player & player, std::vector<Weapon> & available_weapons);
void exclude_armor (const Player & player, std::vector<Armor> & available_armor);
bool boost(Player & player, int choice);
void boost_player (Player & player);
void print_lore (int wave_num);
void print_monster_affinities (const Monster & monster);
void choose_armor (Player & player);
void choose_weapons (Player & player);

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

void print_monster_affinities (const Monster & monster) {
    std::cout << "Weaknesses: ";
    if (monster.weaknesses.empty()) std::cout << "None";
    else {
        if (monster.weaknesses[0].known) std::cout << type_to_string(monster.weaknesses[0].type);
        else std::cout << "???";
        if (monster.weaknesses.size()>1) {
            std::cout << ", ";
            if (monster.weaknesses[1].known) std::cout << type_to_string(monster.weaknesses[1].type);
            else std::cout << "???";
            if (monster.weaknesses.size()>2) {
                std::cout << ", ";
                if (monster.weaknesses[2].known) std::cout << type_to_string(monster.weaknesses[2].type);
                else std::cout << "???";
            }
        }
    }
    std::cout << std::endl << "Resistances: ";
    if (monster.resistances.empty()) std::cout << "None";
    else {
        if (monster.resistances[0].known) std::cout << type_to_string(monster.resistances[0].type);
        else std::cout << "???";
        if (monster.resistances.size()>1) {
            std::cout << ", ";
            if (monster.resistances[1].known) std::cout << type_to_string(monster.resistances[1].type);
            else std::cout << "???";
            if (monster.resistances.size()>2) {
                std::cout << ", ";
                if (monster.resistances[2].known) std::cout << type_to_string(monster.resistances[2].type);
                else std::cout << "???";
            }
        }
    }
    std::cout << std::endl;
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

    int index = 1;
    for (const auto &weapon : player.weapons_inventory) {
        std::string durability = weapon.is_infinite ? "infinite" : std::to_string(weapon.durability);

        std::cout << std::setw(10) << index++
                  << std::setw(20) << weapon.name
                  << std::setw(15) << weapon.damage
                  << std::setw(15) << weapon.hit_rate
                  << std::setw(15) << weapon.crit_rate
                  << std::setw(15) << durability
                  << std::setw(15) << type_to_string(weapon.type) << std::endl;

        if (index <= player.weapons_inventory.size()) std::cout << std::string(105, '-') << std::endl;
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

    int index = 1;
    for (const auto &armor : player.armor_inventory) {
        std::string durability = armor.is_unbreakable ? "infinite" : std::to_string(armor.durability);

        std::string res1 = (!armor.resistances.empty()) ? type_to_string(armor.resistances[0].type) : "None";
        std::string res2 = (armor.resistances.size() > 1) ? type_to_string(armor.resistances[1].type) : "None";
        std::string weak1 = (!armor.weaknesses.empty()) ? type_to_string(armor.weaknesses[0].type) : "None";
        std::string weak2 = (armor.weaknesses.size() > 1) ? type_to_string(armor.weaknesses[1].type) : "None";

        std::cout << std::setw(10) << index++
                  << std::setw(20) << armor.name
                  << std::setw(15) << armor.damage_reduction
                  << std::setw(15) << durability
                  << std::setw(15) << armor.evasion
                  << std::setw(15) << weak1
                  << std::setw(15) << weak2
                  << std::setw(15) << res1
                  << std::setw(15) << res2 << std::endl;

        if (index <= player.armor_inventory.size()) std::cout << std::string(135, '-') << std::endl;
    }
    std::cout << std::string(135, '*') << std::endl;
}

void print_current_armor (const Player & player) {
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
    std::cout << std::string(135, '-') << std::endl;

    const std::string durability = player.current_armor.is_unbreakable ? "infinite" : std::to_string(player.current_armor.durability);

    const std::string res1 = (!player.current_armor.resistances.empty()) ? type_to_string(player.current_armor.resistances[0].type) : "None";
    const  std::string res2 = (player.current_armor.resistances.size() > 1) ? type_to_string(player.current_armor.resistances[1].type) : "None";
    const std::string weak1 = (!player.current_armor.weaknesses.empty()) ? type_to_string(player.current_armor.weaknesses[0].type) : "None";
    const std::string weak2 = (player.current_armor.weaknesses.size() > 1) ? type_to_string(player.current_armor.weaknesses[1].type) : "None";

    std::cout << std::setw(10) << 1
              << std::setw(20) << player.current_armor.name
              << std::setw(15) << player.current_armor.damage_reduction
              << std::setw(15) << durability
              << std::setw(15) << player.current_armor.evasion
              << std::setw(15) << weak1
              << std::setw(15) << weak2
              << std::setw(15) << res1
              << std::setw(15) << res2 << std::endl;
    std::cout << std::string(135, '*') << std::endl;
}
void print_weapons_header () {
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
}

void print_current_weapons (const Player & player, const Monster & monster) {
    print_weapons_header();

    int index = 1;
    for (const auto &weapon : player.current_weapons) {
        std::string durability = weapon.is_infinite ? "infinite" : std::to_string(weapon.current_durability);
        int effective_hit_rate = std::max(5, weapon.hit_rate - monster.evasion_rate);
        int damage = std::max(0, monster.incoming_damage(weapon, player.strength));

        std::cout << std::setw(10) << index++
                  << std::setw(20) << weapon.name
                  << std::setw(15) << damage
                  << std::setw(15) << effective_hit_rate
                  << std::setw(15) << weapon.crit_rate
                  << std::setw(15) << durability
                  << std::setw(15) << type_to_string(weapon.type) << std::endl;

        if (index <= player.current_weapons.size()) std::cout << std::string(105, '-') << std::endl;
    }
    std::cout << std::string(105, '*') << std::endl;
}

void print_current_weapons (const Player & player) {
    print_weapons_header();

    int index = 1;
    for (const auto &weapon : player.current_weapons) {
        std::string durability = weapon.is_infinite ? "infinite" : std::to_string(weapon.durability);

        std::cout << std::setw(10) << index++
                  << std::setw(20) << weapon.name
                  << std::setw(15) << weapon.damage
                  << std::setw(15) << weapon.hit_rate
                  << std::setw(15) << weapon.crit_rate
                  << std::setw(15) << durability
                  << std::setw(15) << type_to_string(weapon.type) << std::endl;

        if (index <= player.current_weapons.size()) std::cout << std::string(105, '-') << std::endl;
    }
    std::cout << std::string(105, '*') << std::endl;
}

void combat (Player & player, Monster & monster, std::vector<Monster> & monsters_of_the_wave, int index_monster) {

            do {
            print_current_weapons(player, monster);
            print_monster_affinities(monster);
            std::cout << "Current HP: " << player.hp_remaining << "/" << player.base_hp << std::endl;
            std::cout << "Current monster's HP: " << monster.hp_remaining << "/" << monster.base_hp << std::endl << std::endl;
            std::string weapon_chosen;
            bool valid_weapon = false;
            do {
                std::cout << "Which weapon do you want to use?" << std::endl;
                std::getline(std::cin, weapon_chosen);
                if (player.current_weapons[stoi(weapon_chosen)-1].is_broken()) std::cout << "That weapon is broken, please select another one" << std::endl;
                else valid_weapon = true;
            }while (!valid_weapon);
            if(monster.take_damage(player.current_weapons[stoi(weapon_chosen)-1],player.strength)) {
                monsters_of_the_wave[index_monster].reveal_affinities(player.current_weapons[stoi(weapon_chosen)-1]);
                list_of_monsters[index_monster].reveal_affinities(player.current_weapons[stoi(weapon_chosen)-1]);
            }
            wait();

            if (!monster.is_dead()) {
                std::cout << monster.name << " is going to attack you!" << std::endl;
                player.take_damage(monster);
            }
            else std::cout << "You killed " << monster.name << "!" << std::endl;
            wait();
            clear_screen();
        }while (!player.has_lost() && !monster.is_dead());
}

void start_setup (Player & player, std::vector<Monster> & monsters_of_the_wave, std::vector<Monster> & bosses_of_the_wave, int wave_num) {
    for (auto & w : player.current_weapons) w.current_durability = w.durability;
    player.current_armor.current_durability = player.current_armor.durability;
    player.hp_remaining = player.base_hp;

    for(int i = 0; i < 3+wave_num; i++) {
        if (i<list_of_monsters.size()) monsters_of_the_wave.push_back(list_of_monsters[i]);
    }
    for(int i = 0; i < wave_num; i++) {
        if (i<list_of_bosses.size()) bosses_of_the_wave.push_back(list_of_bosses[i]);
    }
    for (auto & m : monsters_of_the_wave) m.upgrade_monster(wave_num);
    for (auto & b : bosses_of_the_wave) b.upgrade_monster(wave_num);
}

int select_monster (const std::vector<Monster> & monsters_of_the_wave) {
    std::uniform_int_distribution<int> distrib(1, static_cast<int>(monsters_of_the_wave.size()));
    return distrib(g)-1;
}

bool start_wave (Player & player, const int wave_num) {

    print_lore(wave_num);
    std::vector<Monster> monsters_of_the_wave;
    std::vector<Monster> bosses_of_the_wave;
    start_setup(player, monsters_of_the_wave, bosses_of_the_wave, wave_num);

    for(int i = 0; i < 3 + wave_num*2; i++) {
        clear_screen();
        int index_monster = select_monster(monsters_of_the_wave);
        Monster monster = monsters_of_the_wave[index_monster];
        std::cout << monster.name << " has appeared!" << std::endl;
        wait();
        combat (player, monster, monsters_of_the_wave, index_monster);
        if (player.has_lost()) return false;
    }
    if (wave_num>=2) {
        clear_screen();
        int index_boss = select_monster(bosses_of_the_wave);
        Monster boss = bosses_of_the_wave[index_boss];
        std::cout << std::endl << "Boss " << boss.name << " has appeared!" << std::endl;
        wait();
        combat (player, boss, bosses_of_the_wave, index_boss);
        if (player.has_lost()) return false;
    }
    std::cout << std::endl << "You completed the wave number " << wave_num+1 << "!" << std::endl << std::endl;
    boost_player(player);
    std::string choice;
    std::cout << "Do you want to change your current weapons? (Y/N)" << std::endl;
    std::getline(std::cin, choice);
    if (choice=="Y" || choice=="y") choose_weapons(player);
    clear_screen();
    std::cout << "Do you want to change your current armor? (Y/N)" << std::endl;
    std::getline(std::cin, choice);
    if (choice=="Y" || choice=="y") choose_armor(player);
    clear_screen();
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
    for (const auto &player_weapon : player.weapons_inventory) {
        available_weapons.erase(
            std::remove_if(available_weapons.begin(), available_weapons.end(),
                [&player_weapon](const Weapon &w) { return w.name == player_weapon.name; }),
            available_weapons.end()
        );
    }
}

void exclude_armor (const Player & player, std::vector<Armor> & available_armor) {
    for (const auto &player_armor : player.armor_inventory) {
        available_armor.erase(
            std::remove_if(available_armor.begin(), available_armor.end(),
                [&player_armor](const Armor &a) { return a.name == player_armor.name; }),
            available_armor.end()
        );
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
                std::cout << "You upgraded " << player.weapons_inventory[stoi(item_choice)-1].name << "!" << std::endl;
            }
            if (item_choice == "2") {
                print_armor_inventory(player);
                std::cout << "Which armor do you want to upgrade?" << std::endl;
                std::getline (std::cin, item_choice);
                player.armor_inventory[stoi(item_choice)-1].upgrade_armor();
                std::cout << "You upgraded " << player.armor_inventory[stoi(item_choice)-1].name << "!" << std::endl;
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
        case 3: {
            int start_health = player.base_hp;
            player.upgrade_health();
            std::cout << "You increased your HP by " << player.base_hp-start_health << " points!" << std::endl;
            return true;
        }
        case 4: {
            int start_strength = player.strength;
            player.upgrade_strength();
            std::cout << "You increased your strength by " << player.strength-start_strength << " points!" << std::endl;
            return true;
        }
        case 5: {
            int start_resistance = player.resistance;
            player.upgrade_resistance();
            std::cout << "You increased your resistance by " << player.resistance-start_resistance << " points!" << std::endl;
            return true;
        }
        default: return false;
    }
}

void choose_weapons (Player & player) {
    std::string choice;
    do {
        clear_screen();
        std::cout << "These are your current equipped weapons:" << std::endl;
        print_current_weapons(player);
        std::cout << "This is your current weapons' inventory:" << std::endl;
        print_weapons_inventory(player);
        std::cout << "Which weapon do you want to replace? (current)" << std::endl;
        std::string choice1, choice2;
        std::getline(std::cin, choice1);
        std::cout << "Which weapon do you want to replace it with? (inventory)" << std::endl;
        std::getline(std::cin, choice2);
        bool valid_option = true;
        for (int i = 0; i < 3; i++) {
            if (player.weapons_inventory[stoi(choice2)-1].name==player.current_weapons[i].name) {
                std::cout << "That weapon is already equipped, please select another one" << std::endl;
                valid_option = false;
            }
        }
        if (valid_option) player.current_weapons[stoi(choice1)-1] = player.weapons_inventory[stoi(choice2)-1];
        std::cout << "Do you want to change your current weapons? (Y/N)" << std::endl;
        std::getline(std::cin, choice);
        if (choice=="N" || choice=="n") return;
    }while (choice=="Y" || choice=="y");
}

void choose_armor (Player & player) {
    std::string choice;
    do {
        clear_screen();
        std::cout << "This is your current equipped armor:" << std::endl;
        print_current_armor(player);
        std::cout << "This is your current armors' inventory:" << std::endl;
        print_armor_inventory(player);
        std::string choice1;
        std::cout << "Which armor do you want to equip? (inventory)" << std::endl;
        std::getline(std::cin, choice1);
        player.current_armor = player.armor_inventory[stoi(choice1)-1];
        std::cout << "Do you want to change your current armor? (Y/N)" << std::endl;
        std::getline(std::cin, choice);
        if (choice=="N" || choice=="n") return;
    }while (choice=="Y" || choice=="y");
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
/*
Da aggiungere: 
problema col crash di exclude_weapons(?)
regex,
findif per update_current_equipment,
mettere a posto i costruttori,
aggiungere nomi e lore,
sistema di salvataggio e di record,
dividere in file diversi
*/