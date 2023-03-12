#ifndef __ORGANISM_H__
#define __ORGANISM_H__

#include <cstdint>
#include <concepts>
#include <optional>
#include <numeric>
#include <type_traits>
#include <tuple>

template<typename species_t, bool can_eat_meat, bool can_eat_plants> requires std::equality_comparable<species_t>
class Organism {
private:
    const species_t species;
    const uint64_t vitality;

public:
    constexpr Organism(species_t const &species, uint64_t vitality) : species(species), vitality(vitality) {};

    constexpr uint64_t get_vitality() const {
        return vitality;
    }

    constexpr const species_t &get_species() const {
        return species;
    }

    constexpr bool is_dead() const {
        return vitality == 0;
    }

    constexpr static bool is_plant() {
        return !can_eat_meat && !can_eat_plants;
    }

    template<bool o_can_eat_meat, bool o_can_eat_plants>
    constexpr Organism eat(Organism<species_t, o_can_eat_meat, o_can_eat_plants> const &o) const {
        constexpr bool this_is_plant = is_plant();
        constexpr bool o_is_plant = std::remove_cvref_t<decltype(o)>::is_plant();
        constexpr bool this_can_eat = o_is_plant ? can_eat_plants : can_eat_meat;
        constexpr bool o_can_eat = this_is_plant ? o_can_eat_plants : o_can_eat_meat;
        if constexpr (this_can_eat) {
            if constexpr (o_is_plant) {
                return {species, vitality + o.get_vitality()};
            }
            if (vitality > o.get_vitality()) {
                return {species, vitality + o.get_vitality() / 2};
            }
        }
        if constexpr (o_can_eat) {
            if (this_is_plant || o.get_vitality() > vitality || (this_can_eat && o.get_vitality() == vitality)) {
                return {species, 0};
            }
        }
        return *this;
    }

    template<bool o_can_eat_meat, bool o_can_eat_plants>
    constexpr Organism breed(Organism<species_t, o_can_eat_meat, o_can_eat_plants> const &o) const {
        return {species, std::midpoint(std::min(vitality, o.get_vitality()), std::max(vitality, o.get_vitality()))};
    }

};

namespace organism_operators {
    template<typename species_t, bool o1_can_eat_meat, bool o1_can_eat_plants, bool o2_can_eat_meat, bool o2_can_eat_plants>
    constexpr Organism<species_t, o1_can_eat_meat, o1_can_eat_plants>
    operator+(Organism<species_t, o1_can_eat_meat, o1_can_eat_plants> const &o1,
              Organism<species_t, o2_can_eat_meat, o2_can_eat_plants> const &o2) {
        return get<0>(encounter(o1, o2));
    }
}

template<typename species_t>
using Carnivore = Organism<species_t, true, false>;

template<typename species_t>
using Omnivore = Organism<species_t, true, true>;

template<typename species_t>
using Herbivore = Organism<species_t, false, true>;

template<typename species_t>
using Plant = Organism<species_t, false, false>;

template<typename species_t, bool o1_eats_m, bool o1_eats_p, bool o2_eats_m, bool o2_eats_p>
requires (o1_eats_m || o1_eats_p || o2_eats_m || o2_eats_p)
constexpr std::tuple<Organism<species_t, o1_eats_m, o1_eats_p>,
        Organism<species_t, o2_eats_m, o2_eats_p>,
        std::optional<Organism<species_t, o1_eats_m, o1_eats_p>>>
encounter(Organism<species_t, o1_eats_m, o1_eats_p> organism1, Organism<species_t, o2_eats_m, o2_eats_p> organism2) {
    if (organism1.is_dead() || organism2.is_dead()) {
        return {organism1, organism2, std::nullopt};
    }
    if constexpr (o1_eats_m == o2_eats_m && o1_eats_p == o2_eats_p) {
        if (organism1.get_species() == organism2.get_species()) {
            return {organism1, organism2, organism1.breed(organism2)};
        }
    }
    return {organism1.eat(organism2), organism2.eat(organism1), std::nullopt};
}

template<typename species_t, bool o1_eats_m, bool o1_eats_p, typename ... Args>
constexpr Organism<species_t, o1_eats_m, o1_eats_p>
encounter_series(Organism<species_t, o1_eats_m, o1_eats_p> organism1, Args ... args) {
    using namespace organism_operators;
    return (organism1 + ... + args);
}

#endif // __ORGANISM_H__
