//
// Created by User on 14/03/2025.
//

#include "entities/GameObject.h"

#include "Corrade/Utility/String.h"


void GameObject::giveDefaultName() {

    if (!_app->getObjects().contains(_name)) {
        return;
    }
    size_t i = _name.size();

    // Trouver le début du nombre à la fin (si existant)
    while (i > 0 && std::isdigit(_name[i - 1])) {
        --i;
    }

    std::string base = _name.substr(0, i); // Partie non numérique
    int number = (i < _name.size()) ? std::stoi(_name.substr(i)) + 1 : 1; // Incrément ou 1 si absent

    _name = base + std::to_string(number);
}
