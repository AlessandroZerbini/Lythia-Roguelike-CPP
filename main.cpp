#include <iostream>
#include <fstream> //to read/write on files
#include <string>
#include <vector>
#include <regex>  //regular expressions to avoid input errors
#include <algorithm>
#include <random>
#include <iomanip> //to manipulate output formatting

std::random_device rd;
std::mt19937 g(rd());

enum class WeaponType {
    Physical,
    Ice,
    Fire,
    Thunder,
    Wind,
    Dark,
    Light,
    None
};

struct Armor {
    std::string name;
    int damage_reduction;
    int durability;
    WeaponType weakness1;
    WeaponType weakness2;

    Armor() {
        this->name="Unknown";
        this->damage_reduction = 0;
        this->durability = 0;
        this->weakness1 = WeaponType::None;
        this->weakness2 = WeaponType::None;
    }

    Armor(const std::string &name, const int damage_reduction, const int durability) {
        this->name=name;
        this->damage_reduction = damage_reduction;
        this->durability = durability;
        this->weakness1 = WeaponType::None;
        this->weakness2 = WeaponType::None;
    }

    Armor(const std::string &name, const int damage_reduction, const int durability, const WeaponType weakness1, const WeaponType weakness2) {
        this->name=name;
        this->damage_reduction = damage_reduction;
        this->durability = durability;
        this->weakness1 = weakness1;
        this->weakness2 = weakness2;
    }

    bool is_broken() const {
        return durability == 0;
    }
};

struct Weapon {
    std::string name;
    WeaponType type;
    int durability;
    int hit_rate;
    int damage;
    int crit_rate;
    bool is_infinite;

    Weapon() {
        this->name = "Unknown";
        this->type = WeaponType::None;
        this->durability = 0;
        this->hit_rate = 0;
        this->damage = 0;
        this->crit_rate=0;
        this->is_infinite=true;
    }

    Weapon (const std::string &name, const WeaponType type, const int durability, const int hit_rate, const int damage, const int crit_rate, const bool is_infinite) {
        this->name = name;
        this->type = type;
        this->durability = durability;
        this->hit_rate = hit_rate;
        this->damage = damage;
        this->crit_rate = crit_rate;
        this->is_infinite=is_infinite;
    }

    bool is_broken() const {
        if (!is_infinite) return durability <= 0;
        return false;
    }

    void upgrade_weapon() {
        std::uniform_int_distribution<int> distrib(1, 3);
        if (!is_infinite) durability += distrib(g);
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
    int damage;
    int hit_rate;
    WeaponType type;

    Monster() {
        this->name = "Unknown";
        this->base_hp = 0;
        this->hp_remaining = base_hp;
        this->weakness1 = WeaponType::None;
        this->weakness2 = WeaponType::None;
        this->known_weakness1 = false;
        this->known_weakness2 = false;
        this->damage = 0;
        this->hit_rate = 0;
        this->type = WeaponType::None;
    }

    Monster(const std::string &name, const int base_hp, const WeaponType weakness1, const WeaponType weakness2, const int damage, const int hit_rate, const WeaponType type) {
        this->name = name;
        this->base_hp = base_hp;
        this->hp_remaining = base_hp;
        this->weakness1 = weakness1;
        this->weakness2 = weakness2;
        this->known_weakness1 = false;
        this->known_weakness2 = false;
        this->damage = damage;
        this->hit_rate = hit_rate;
        this->type = type;
    }

    void upgrade_monster(const int wave_num) {
        this->base_hp += 2*wave_num;
        this->damage += wave_num;
        this->hit_rate += wave_num;
    }

    int known_weaknesses() const {
        if (known_weakness1 && known_weakness2) return 3;
        if (known_weakness2) return 2;
        if (known_weakness1) return 1;
        return 0;
    }

    void take_damage (Weapon & weapon, const int wave_num) {
        std::uniform_int_distribution<int> distrib(1, 100);
        int effective_hit_rate = std::max(5, weapon.hit_rate-wave_num);
        if (distrib(g)<=effective_hit_rate) {
            if (!weapon.is_infinite) weapon.durability--;
            int additional_damage = 0;
            if (weapon.type == weakness1) {
                known_weakness1 = true;
                additional_damage += 3;
            }
            if (weapon.type == weakness2) {
                known_weakness2 = true;
                additional_damage += 2;
            }
            int damage_dealt = weapon.damage + additional_damage;
            if (distrib(g)<=weapon.crit_rate) {
                damage_dealt *= 3;
                std::cout << "Wow! You did a critical hit! ";
            }
            std::cout << this->name << " took " << damage_dealt << " damage!" << std::endl;
            hp_remaining -= damage_dealt;
        }
        else std::cout << "Oh no, you missed!" << std::endl;
    }

    bool is_dead() const {
        return hp_remaining <= 0;
    }
};

struct Player {
    std::string username;
    int base_hp;
    int hp_remaining;
    std::vector<Weapon> weapons_inventory;
    std::vector<Armor> armor_inventory;
    std::vector<Weapon> current_weapons;
    Armor current_armor;

    Player(const std::string &username) {
        this->username = username;
        this->base_hp = 100;
        this->hp_remaining = base_hp;
    }

    bool has_lost() const {
        return hp_remaining <= 0;
    }
};

std::vector<Weapon> list_of_weapons = {
    {"Basic sword", WeaponType::Physical, 10, 80, 6, 5, false},
    {"Ice magic", WeaponType::Ice, 0, 90, 3, 2, true},
    {"Fire magic", WeaponType::Fire, 0, 70, 7, 10, true},
    {"Thunder magic", WeaponType::Thunder, 0, 75, 6, 25, true},
    {"Wind magic", WeaponType::Wind, 0, 85, 3, 15, true},
    {"Dark magic", WeaponType::Dark, 0, 60, 10, 25, true},
    {"Light magic", WeaponType::Light, 0, 85, 5, 0, true},
    {"Magic Sword", WeaponType::Physical, 20, 85, 5, 5, false},
    {"Dark magic Sword", WeaponType::Dark, 5, 75, 8, 25, false},
    {"Light magic Sword", WeaponType::Light, 25, 95, 5, 5, false}
};

std::vector<Monster> list_of_monsters = {
    {"monster1", 12, WeaponType::None, WeaponType::None, 6, 80, WeaponType::Physical},
    {"monster2", 15, WeaponType::Ice, WeaponType::None, 4, 85, WeaponType::Physical},
    {"monster3", 18, WeaponType::Fire, WeaponType::None, 5, 75, WeaponType::Ice},
    {"monster4", 20, WeaponType::Ice, WeaponType::Thunder, 5, 70, WeaponType::Wind},
    {"monster5", 14, WeaponType::Light, WeaponType::Dark, 4, 90, WeaponType::Thunder},
    {"monster6", 20, WeaponType::Wind, WeaponType::None, 3, 85, WeaponType::Fire},
    {"monster7", 10, WeaponType::None, WeaponType::None, 7, 85, WeaponType::Dark},
    {"monster8", 12, WeaponType::Dark, WeaponType::Thunder, 6, 75, WeaponType::Light},
    {"monster9", 24, WeaponType::None, WeaponType::None, 2, 75, WeaponType::Physical},
    {"monster10", 16, WeaponType::Thunder, WeaponType::Wind, 5, 80, WeaponType::Ice},
};