#include "RogueLike.h"

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