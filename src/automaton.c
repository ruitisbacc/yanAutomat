#include "automaton.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

// Vytvoří nový prázdný automat
Automaton* automaton_create(void) {
    Automaton* a = (Automaton*)malloc(sizeof(Automaton));
    if (a == NULL) return NULL;
    
    a->stateCount = 0;
    a->transitionCount = 0;
    a->alphabetSize = 0;
    a->initialState = -1;
    
    // Výchozí abeceda: 0 a 1
    automaton_set_alphabet(a, "01");
    
    return a;
}

// Uvolní paměť automatu
void automaton_destroy(Automaton* a) {
    if (a != NULL) {
        free(a);
    }
}

// Resetuje automat do výchozího stavu
void automaton_reset(Automaton* a) {
    if (a == NULL) return;
    
    a->stateCount = 0;
    a->transitionCount = 0;
    a->initialState = -1;
}

// === Správa stavů ===

// Přidá nový stav na pozici (x, y)
int automaton_add_state(Automaton* a, float x, float y) {
    if (a == NULL || a->stateCount >= MAX_STATES) return -1;
    
    int id = a->stateCount;
    State* s = &a->states[id];
    
    s->id = id;
    s->x = x;
    s->y = y;
    s->isAccepting = false;
    s->isInitial = false;
    
    // Generuj název (q0, q1, q2, ...)
    snprintf(s->name, MAX_STATE_NAME, "q%d", id);
    
    // První stav je automaticky počáteční
    if (a->stateCount == 0) {
        s->isInitial = true;
        a->initialState = id;
    }
    
    a->stateCount++;
    return id;
}

// Odebere stav podle ID
void automaton_remove_state(Automaton* a, int stateId) {
    if (a == NULL || stateId < 0 || stateId >= a->stateCount) return;
    
    // Odstraň všechny přechody související se stavem
    for (int i = a->transitionCount - 1; i >= 0; i--) {
        if (a->transitions[i].fromState == stateId || 
            a->transitions[i].toState == stateId) {
            automaton_remove_transition(a, i);
        }
    }
    
    // Aktualizuj reference v přechodech
    for (int i = 0; i < a->transitionCount; i++) {
        if (a->transitions[i].fromState > stateId) {
            a->transitions[i].fromState--;
        }
        if (a->transitions[i].toState > stateId) {
            a->transitions[i].toState--;
        }
    }
    
    // Posuň stavy
    for (int i = stateId; i < a->stateCount - 1; i++) {
        a->states[i] = a->states[i + 1];
        a->states[i].id = i;
        snprintf(a->states[i].name, MAX_STATE_NAME, "q%d", i);
    }
    
    a->stateCount--;
    
    // Aktualizuj počáteční stav
    if (a->initialState == stateId) {
        a->initialState = (a->stateCount > 0) ? 0 : -1;
        if (a->stateCount > 0) {
            a->states[0].isInitial = true;
        }
    } else if (a->initialState > stateId) {
        a->initialState--;
    }
}

// Najde stav podle ID
State* automaton_get_state(Automaton* a, int stateId) {
    if (a == NULL || stateId < 0 || stateId >= a->stateCount) return NULL;
    return &a->states[stateId];
}

// Nastaví stav jako přijímající/nepřijímající
void automaton_set_accepting(Automaton* a, int stateId, bool accepting) {
    State* s = automaton_get_state(a, stateId);
    if (s != NULL) {
        s->isAccepting = accepting;
    }
}

// Nastaví stav jako počáteční
void automaton_set_initial(Automaton* a, int stateId) {
    if (a == NULL || stateId < 0 || stateId >= a->stateCount) return;
    
    // Odeber příznak z předchozího počátečního stavu
    if (a->initialState >= 0 && a->initialState < a->stateCount) {
        a->states[a->initialState].isInitial = false;
    }
    
    a->states[stateId].isInitial = true;
    a->initialState = stateId;
}

// === Správa přechodů ===

// Přidá přechod
int automaton_add_transition(Automaton* a, int fromId, int toId, char symbol) {
    if (a == NULL || a->transitionCount >= MAX_TRANSITIONS) return -1;
    if (fromId < 0 || fromId >= a->stateCount) return -1;
    if (toId < 0 || toId >= a->stateCount) return -1;
    
    // Zkontroluj, zda přechod už existuje
    for (int i = 0; i < a->transitionCount; i++) {
        if (a->transitions[i].fromState == fromId && 
            a->transitions[i].symbol == symbol) {
            // Přechod už existuje - aktualizuj cíl
            a->transitions[i].toState = toId;
            return i;
        }
    }
    
    int idx = a->transitionCount;
    a->transitions[idx].fromState = fromId;
    a->transitions[idx].toState = toId;
    a->transitions[idx].symbol = symbol;
    
    a->transitionCount++;
    return idx;
}

// Odebere přechod podle indexu
void automaton_remove_transition(Automaton* a, int transitionIndex) {
    if (a == NULL || transitionIndex < 0 || transitionIndex >= a->transitionCount) return;
    
    // Posuň přechody
    for (int i = transitionIndex; i < a->transitionCount - 1; i++) {
        a->transitions[i] = a->transitions[i + 1];
    }
    
    a->transitionCount--;
}

// Najde přechod z fromId se symbolem
int automaton_find_transition(Automaton* a, int fromId, char symbol) {
    if (a == NULL) return -1;
    
    for (int i = 0; i < a->transitionCount; i++) {
        if (a->transitions[i].fromState == fromId && 
            a->transitions[i].symbol == symbol) {
            return i;
        }
    }
    return -1;
}

// Získá cílový stav pro přechod
int automaton_get_next_state(Automaton* a, int fromId, char symbol) {
    int idx = automaton_find_transition(a, fromId, symbol);
    if (idx >= 0) {
        return a->transitions[idx].toState;
    }
    return -1;
}

// === Správa abecedy ===

// Nastaví abecedu
void automaton_set_alphabet(Automaton* a, const char* symbols) {
    if (a == NULL || symbols == NULL) return;
    
    a->alphabetSize = 0;
    for (int i = 0; symbols[i] != '\0' && a->alphabetSize < MAX_ALPHABET; i++) {
        // Přeskakuj mezery a čárky
        if (symbols[i] == ' ' || symbols[i] == ',') continue;
        
        // Zkontroluj duplikáty
        bool duplicate = false;
        for (int j = 0; j < a->alphabetSize; j++) {
            if (a->alphabet[j] == symbols[i]) {
                duplicate = true;
                break;
            }
        }
        
        if (!duplicate) {
            a->alphabet[a->alphabetSize++] = symbols[i];
        }
    }
}

// Zkontroluje, zda symbol patří do abecedy
bool automaton_is_valid_symbol(Automaton* a, char symbol) {
    if (a == NULL) return false;
    
    for (int i = 0; i < a->alphabetSize; i++) {
        if (a->alphabet[i] == symbol) return true;
    }
    return false;
}

// === Validace ===

// Zvaliduje automat
ValidationResult automaton_validate(Automaton* a) {
    ValidationResult result = { .isValid = true, .messageCount = 0 };
    
    if (a == NULL) {
        result.isValid = false;
        snprintf(result.messages[result.messageCount++], 256, "Automat neexistuje");
        return result;
    }
    
    // Kontrola pocatecniho stavu
    if (a->initialState < 0) {
        result.isValid = false;
        snprintf(result.messages[result.messageCount++], 256, 
                 "Neni definovan pocatecni stav");
    }
    
    // Kontrola abecedy
    if (a->alphabetSize == 0) {
        result.isValid = false;
        snprintf(result.messages[result.messageCount++], 256, 
                 "Abeceda je prazdna");
    }
    
    // Kontrola úplnosti - každý stav musí mít přechod pro každý symbol
    for (int s = 0; s < a->stateCount && result.messageCount < 10; s++) {
        for (int sym = 0; sym < a->alphabetSize && result.messageCount < 10; sym++) {
            char symbol = a->alphabet[sym];
            int next = automaton_get_next_state(a, s, symbol);
            
            if (next < 0) {
                result.isValid = false;
                snprintf(result.messages[result.messageCount++], 256,
                         "Stav %s nema prechod pro '%c'",
                         a->states[s].name, symbol);
            }
        }
    }
    
    return result;
}

// === Pomocné funkce ===

// Zjistí počet přechodů z daného stavu
int automaton_count_transitions_from(Automaton* a, int stateId) {
    if (a == NULL) return 0;
    
    int count = 0;
    for (int i = 0; i < a->transitionCount; i++) {
        if (a->transitions[i].fromState == stateId) {
            count++;
        }
    }
    return count;
}

// Najde stav na dané pozici
int automaton_find_state_at(Automaton* a, float x, float y, float radius) {
    if (a == NULL) return -1;
    
    for (int i = 0; i < a->stateCount; i++) {
        float dx = a->states[i].x - x;
        float dy = a->states[i].y - y;
        float dist = sqrtf(dx * dx + dy * dy);
        
        if (dist <= radius) {
            return i;
        }
    }
    return -1;
}
