#ifndef ROGUELIKE_ROGUELIKE_H
#define ROGUELIKE_ROGUELIKE_H

#include <iostream>
#include <fstream> //to read/write on files
#include <string>
#include <vector>
#include <regex>  //regular expressions to avoid input errors
#include <algorithm>
#include <random>
#include <iomanip> //to manipulate output formatting
#include <limits>

extern std::random_device rd;
extern std::mt19937 g;

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
    WeaponType type = WeaponType::None;
    int modifier = 0;
    bool known = false;

    Resistance () = default;

    Resistance (WeaponType type, int modifier);
};

using Weakness = Resistance;

struct Armor {
    std::string name = "Unknown";
    int damage_reduction = 0;
    int durability = 0;
    int current_durability = 0;
    int evasion = 0;
    std::vector<Resistance> resistances;
    std::vector<Weakness> weaknesses;
    bool is_unbreakable = true;

    Armor() = default;
    Armor(std::string name, int damage_reduction, int durability, int evasion, bool is_unbreakable);
    Armor(std::string name, int damage_reduction, int durability, int evasion, std::vector<Resistance> resistances, std::vector<Weakness> weaknesses,  bool is_unbreakable);
    void upgrade_armor();
    [[nodiscard]] bool is_broken() const;
};

struct Weapon {
    std::string name = "Unknown";
    WeaponType type = WeaponType::None;
    int durability = 0;
    int current_durability = 0;
    int hit_rate = 0;
    int damage = 0;
    int crit_rate = 0;
    bool is_infinite = true;

    Weapon() = default;

    Weapon (std::string name, WeaponType type, int durability, int hit_rate, int damage, int crit_rate, bool is_infinite);

    [[nodiscard]] bool is_broken() const;
    void upgrade_weapon();
};

struct Monster {
    std::string name = "Unknown";
    int base_hp = 0;
    int hp_remaining = 0;
    std::vector<Resistance> resistances;
    std::vector<Weakness> weaknesses;
    int damage = 0;
    int hit_rate = 0;
    int evasion_rate = 0;
    int crit_rate = 0;
    WeaponType type = WeaponType::None;

    Monster() = default;
    Monster(std::string name, int base_hp, std::vector<Weakness> weaknesses, std::vector<Resistance> resistances, int damage, int hit_rate, int evasion_rate, int crit_rate, WeaponType type);
    void upgrade_monster(int wave_num);
    [[nodiscard]] int incoming_damage (const Weapon & weapon, int strength) const;
    void reveal_affinities (const Weapon & weapon);
    [[nodiscard]] bool take_damage (Weapon & weapon, int player_strength);
    [[nodiscard]] bool is_dead() const;
};

extern std::vector<Weapon> list_of_weapons;

extern std::vector<Monster> list_of_monsters;

extern std::vector<Monster> list_of_bosses;

extern std::vector<Armor> list_of_armor;

struct Player {
    std::string username = "???";
    int base_hp = 100;
    int hp_remaining = 100;
    int strength = 0;
    int resistance = 0;
    std::vector<Weapon> weapons_inventory;
    std::vector<Armor> armor_inventory;
    std::vector<Weapon> current_weapons;
    Armor current_armor;

    Player() = default;
    explicit Player(std::string name);
    Player(std::string name, std::vector<Weapon> weapons, std::vector<Armor> armors);
    void take_damage(const Monster & monster);
    void upgrade_health();
    void upgrade_strength();
    void upgrade_resistance();
    [[nodiscard]] bool has_lost() const;
};

void enable_ansi();
void wait();
void clear_screen ();
std::string type_to_string (WeaponType type);
void print_weapons_inventory (const Player & player);
void print_armor_inventory (const Player & player);
void print_current_armor (const Player & player);
void print_weapons_header ();
void print_current_weapons (const Player & player, const Monster & monster);
void print_current_weapons (const Player & player);
bool start_wave (Player & player, int wave_num);
int select_monster (const std::vector<Monster> & monsters_of_the_wave);
void start_setup (Player & player, std::vector<Monster> & monsters_of_the_wave, std::vector<Monster> & bosses_of_the_wave, int wave_num);
void combat (Player & player, Monster & monster, std::vector<Monster> & monsters_of_the_wave, int index_monster);
void update_current_equipment(Player & player);
void exclude_weapons (const Player & player, std::vector<Weapon> & available_weapons);
void exclude_armor (const Player & player, std::vector<Armor> & available_armor);
bool boost(Player & player, int choice);
void boost_player (Player & player);
void print_lore (int wave_num);
void print_monster_affinities (const Monster & monster);
void choose_armor (Player & player);
void choose_weapons (Player & player);
#endif //ROGUELIKE_ROGUELIKE_H