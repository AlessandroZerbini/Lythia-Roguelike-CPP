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

//aggiungere ai mostri un crit rate e un evasion rate, evasion all'armor

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

    int known_weaknesses() const {
        if (known_weakness1 && known_weakness2) return 3;
        if (known_weakness2) return 2;
        if (known_weakness1) return 1;
        return 0;
    }

    void take_damage (Weapon & weapon, const int player_strength) {
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
    }

    void take_damage(const Monster & monster) {
        std::uniform_int_distribution<int> distrib(1, 100);
        int effective_hit_rate = std::min(95, monster.hit_rate-current_armor.evasion);
        if (distrib(g)<=effective_hit_rate) {
            if(!current_armor.is_unbreakable && !current_armor.is_broken()) current_armor.current_durability--;
            int additional_damage = 0;
            if(!current_armor.is_broken()) {
                additional_damage-=current_armor.damage_reduction;
                additional_damage-=resistance;
                if (monster.type == current_armor.weakness1) additional_damage += 3;
                if (monster.type == current_armor.weakness2) additional_damage += 2;
                if (monster.type == current_armor.resistance1) additional_damage-=2;
                if (monster.type == current_armor.resistance2) additional_damage-=1;
            }
            int damage_dealt = std::max(0, monster.damage + additional_damage);
            if (distrib(g)<=monster.crit_rate) {
                damage_dealt *= 3;
                std::cout << "Oh no! You received a critical hit! ";
            }
            std::cout << "You took " << damage_dealt << " damage!" << std::endl;
            hp_remaining -= damage_dealt;
        }
        else std::cout << "The monster missed! Now it's your turn to counterattack!" << std::endl;
    }

    bool has_lost() const {
        return hp_remaining <= 0;
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

int main () {
    std::cout << "Choose what to do before the next wave starts:" << std::endl <<
                "1: Upgrade a weapon" << std::endl <<
                "2: Look for a new item" << std::endl <<
                "3: Train to increase your health" << std::endl <<
                "4: Train to increase your strength" << std::endl <<
                "5: Train to increase your resistance" << std::endl;
}