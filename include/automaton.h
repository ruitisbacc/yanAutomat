#ifndef AUTOMATON_H
#define AUTOMATON_H

#include <stdbool.h>

// Maximální limity
#define MAX_STATES 50
#define MAX_ALPHABET 10
#define MAX_TRANSITIONS 500
#define MAX_STATE_NAME 32

// Stav automatu
typedef struct {
    int id;
    float x, y;                    // Pozice na plátně
    char name[MAX_STATE_NAME];     // Název stavu (q0, q1, ...)
    bool isAccepting;              // Je přijímající?
    bool isInitial;                // Je počáteční?
} State;

// Přechod mezi stavy
typedef struct {
    int fromState;                 // ID zdrojového stavu
    int toState;                   // ID cílového stavu
    char symbol;                   // Symbol přechodu
} Transition;

// Kompletní automat
typedef struct {
    State states[MAX_STATES];
    int stateCount;
    
    Transition transitions[MAX_TRANSITIONS];
    int transitionCount;
    
    char alphabet[MAX_ALPHABET];
    int alphabetSize;
    
    int initialState;              // ID počátečního stavu (-1 = není definován)
} Automaton;

// Chybová zpráva pro validaci
typedef struct {
    bool isValid;
    char messages[10][256];
    int messageCount;
} ValidationResult;

// === Funkce pro správu automatu ===

// Vytvoří nový prázdný automat
Automaton* automaton_create(void);

// Uvolní paměť automatu
void automaton_destroy(Automaton* a);

// Resetuje automat do výchozího stavu
void automaton_reset(Automaton* a);

// === Správa stavů ===

// Přidá nový stav na pozici (x, y), vrací ID nového stavu nebo -1 při chybě
int automaton_add_state(Automaton* a, float x, float y);

// Odebere stav podle ID (odstraní i související přechody)
void automaton_remove_state(Automaton* a, int stateId);

// Najde stav podle ID, vrací pointer nebo NULL
State* automaton_get_state(Automaton* a, int stateId);

// Nastaví stav jako přijímající/nepřijímající
void automaton_set_accepting(Automaton* a, int stateId, bool accepting);

// Nastaví stav jako počáteční
void automaton_set_initial(Automaton* a, int stateId);

// === Správa přechodů ===

// Přidá přechod, vrací index přechodu nebo -1 při chybě
int automaton_add_transition(Automaton* a, int fromId, int toId, char symbol);

// Odebere přechod podle indexu
void automaton_remove_transition(Automaton* a, int transitionIndex);

// Najde přechod z fromId se symbolem, vrací index nebo -1
int automaton_find_transition(Automaton* a, int fromId, char symbol);

// Získá cílový stav pro přechod z fromId se symbolem, vrací ID stavu nebo -1
int automaton_get_next_state(Automaton* a, int fromId, char symbol);

// === Správa abecedy ===

// Nastaví abecedu (např. "01" nebo "abc")
void automaton_set_alphabet(Automaton* a, const char* symbols);

// Zkontroluje, zda symbol patří do abecedy
bool automaton_is_valid_symbol(Automaton* a, char symbol);

// === Validace ===

// Zvaliduje automat (kontrola úplnosti DFA)
ValidationResult automaton_validate(Automaton* a);

// === Pomocné funkce ===

// Zjistí počet přechodů z daného stavu
int automaton_count_transitions_from(Automaton* a, int stateId);

// Najde stav na dané pozici (pro kliknutí myší)
int automaton_find_state_at(Automaton* a, float x, float y, float radius);

#endif // AUTOMATON_H
