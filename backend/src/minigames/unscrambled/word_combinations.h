#ifndef PEOPLEPARTY_BACKEND_WORD_COMBINATIONS_H
#define PEOPLEPARTY_BACKEND_WORD_COMBINATIONS_H

#include <vector>
#include <string>
#include <random>
#include <algorithm>

class Word_Combinations {
private:
    static inline std::vector<std::vector<std::string>> word_combinations; // will be a vector of > group of 5 words
    
    static void load_word_combinations() {
        // hardcoded word combinations for simplicity
        // Each set shares many common letters to make scrambled words confusing
        word_combinations = {
            // Set 1: i, c, a, t, n, p, s (12-13 letters)
            {"anticipation", "participating", "presentation", "practitioner", "appreciation"},
            
            // Set 2: e, r, t, i, n, a, l (7-8 letters)
            {"oriental", "reliant", "entrail", "retinal", "latrine"},
            
            // Set 3: p, r, o, t, e, c, i, n (9-10 letters)
            {"protection", "reception", "exception", "deception", "inception"},
            
            // Set 4: l, e, t, t, s, r (7-8 letters)
            {"letters", "settlers", "trestles", "nettles", "shuttle"},
            
            // Set 5: b, r, a, k, i, n, g (6-8 letters)
            {"breaking", "banking", "barking", "braking", "baking"},
            
            // Set 6: c, r, e, a, t, u, s (8-9 letters)
            {"creatures", "treasures", "features", "fractures", "captures"},
            
            // Set 7: d, e, l, i, v, r (7-9 letters)
            {"delivered", "livered", "rivaled", "reviled", "relived"},
            
            // Set 8: p, r, e, s, n, t, v (7-8 letters)
            {"present", "serpent", "repents", "parents", "prevents"},
            
            // Set 9: s, t, r, a, i, n, e, g (7-9 letters)
            {"restrain", "stainer", "retains", "nastier", "angriest"},
            
            // Set 10: i, n, t, e, r, a, c, l (8-9 letters)
            {"interact", "certain", "canister", "creatine", "clarinet"},
            
            // Set 11: m, a, t, e, r, i, l, s (8-9 letters)
            {"materials", "mistrial", "minerals", "terminal", "tramlines"},
            
            // Set 12: c, o, n, t, a, i, r (9-10 letters)
            {"container", "historian", "carbonate", "creation", "reactions"},
            
            // Set 13: p, o, i, n, t, s (6-8 letters)
            {"points", "piston", "options", "potions", "postings"},
            
            // Set 14: s, t, r, e, a, m, h (6-8 letters)
            {"stream", "master", "tamers", "steamer", "hamster"},
            
            // Set 15: g, r, e, a, t, s (5-7 letters)
            {"greats", "grates", "stager", "gaster", "garters"},
            
            // Set 16: c, h, a, n, e, s, r (7-8 letters)
            {"changes", "chasing", "cashing", "crashes", "reaches"},
            
            // Set 17: r, a, t, i, o, n, c (6-7 letters)
            {"ration", "nation", "atonic", "cation", "action"},
            
            // Set 18: p, r, i, n, t, e, s (7-9 letters)
            {"sprinter", "reprint", "printer", "pointers", "painters"},
            
            // Set 19: f, l, o, w, e, r, s, v (7-9 letters)
            {"flowers", "followers", "overflow", "foresaw", "flavors"},
            
            // Set 20: m, o, n, t, e, r, s, d (7-8 letters)
            {"monster", "mentors", "remotes", "tremors", "moderns"},
            
            // Set 21: p, l, a, n, t, e, r, x (7-9 letters)
            {"planter", "replant", "prenatal", "paternal", "external"},
            
            // Set 22: w, i, n, t, e, r (5-7 letters)
            {"winter", "twiner", "inert", "inter", "write"},
            
            // Set 23: d, a, n, g, e, r, s (6-7 letters)
            {"dangers", "gardens", "ganders", "ranged", "grandes"},
            
            // Set 24: c, a, p, t, u, r, e, h (7-8 letters)
            {"capture", "chapter", "patcher", "teacher", "cheater"},
            
            // Set 25: h, o, u, s, e, r, c (6-7 letters)
            {"houses", "source", "course", "chorus", "chores"},
            
            // Set 26: s, t, r, o, n, g, e (7-9 letters)
            {"stronger", "storage", "strange", "stoner", "gathers"},
            
            // Set 27: p, i, c, t, u, r, e, s (7-9 letters)
            {"pictures", "scripture", "spectrum", "crisper", "rustic"},
            
            // Set 28: m, a, r, k, e, t, s (6-8 letters)
            {"markets", "mastery", "stream", "master", "tamers"},
            
            // Set 29: b, r, o, t, h, e, s (7-8 letters)
            {"brothers", "bathers", "shorter", "bothers", "breaths"},
            
            // Set 30: c, l, o, s, e, r, t (6-8 letters)
            {"closet", "sector", "closer", "escort", "corset"},
            
            // Set 31: i, n, t, e, r, a, c, o, s (12-14 letters) - Long words
            {"interactions", "conservation", "conversation", "consideration", "incorporating"},
            
            // Set 32: r, e, p, s, n, t, a, i, o (12-15 letters) - Long words
            {"representation", "transportation", "impersonation", "repatriations", "presentations"},
            
            // Set 33: u, n, d, e, r, s, t, a, i, g (12-14 letters) - Long words
            {"understanding", "unregistered", "undertakings", "understating", "disagreement"},
            
            // Set 34: c, o, m, p, l, i, t, e, d, a (11-13 letters) - Long words
            {"complicated", "complimented", "implicated", "duplicated", "implored"},
            
            // Set 35: a, d, m, i, n, s, t, r, o (13-15 letters) - Long words
            {"administration", "demonstrators", "administrators", "demonstrations", "administrate"},
            
            // Set 36: r, e, s, p, o, n, i, b, l, t (13-16 letters) - Long words
            {"responsibilities", "relationships", "personalities", "probabilities", "possibilities"},
            
            // Set 37: c, o, n, g, r, a, t, u, l, i (13-15 letters) - Long words
            {"congratulations", "triangulations", "strangulation", "granulations", "triangulation"},
            
            // Set 38: i, n, f, o, r, m, a, t (11-15 letters) - Long words
            {"information", "transformation", "reformation", "confirmations", "malformation"},
            
            // Set 39: e, x, p, r, i, m, e, n, t, a (11-13 letters) - Long words
            {"experimental", "experiments", "apartments", "temperament", "trampoline"},
            
            // Set 40: c, o, n, t, r, i, b, u (11-13 letters) - Long words
            {"contribution", "distribution", "tribulations", "incubator", "contributor"},
        };
    }
    
public:
    static std::vector<std::vector<std::string>> get_random_word_combinations(int amount) {
        if (word_combinations.empty()) {
            load_word_combinations();
        }
        std::vector<std::vector<std::string>> selected_combinations;
        std::sample(word_combinations.begin(), word_combinations.end(),
                    std::back_inserter(selected_combinations),
                    amount,
                    std::mt19937{std::random_device{}()});
        return selected_combinations;
    }
};


#endif //PEOPLEPARTY_BACKEND_WORD_COMBINATIONS_H