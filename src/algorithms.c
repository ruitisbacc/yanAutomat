#include "algorithms.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// === Testování řetězce ===

bool algo_test_string(Automaton* a, const char* input) {
    if (a == NULL || a->initialState < 0) return false;
    
    int currentState = a->initialState;
    
    for (int i = 0; input[i] != '\0'; i++) {
        char symbol = input[i];
        
        // Zkontroluj, zda symbol je v abecedě
        if (!automaton_is_valid_symbol(a, symbol)) {
            return false;  // Neplatný symbol
        }
        
        int nextState = automaton_get_next_state(a, currentState, symbol);
        if (nextState < 0) {
            return false;  // Chybí přechod
        }
        
        currentState = nextState;
    }
    
    // Zkontroluj, zda jsme v přijímajícím stavu
    State* finalState = automaton_get_state(a, currentState);
    return (finalState != NULL && finalState->isAccepting);
}

int algo_test_string_trace(Automaton* a, const char* input, int* states) {
    if (a == NULL || a->initialState < 0 || states == NULL) return -1;
    
    int currentState = a->initialState;
    int step = 0;
    states[step++] = currentState;
    
    for (int i = 0; input[i] != '\0'; i++) {
        char symbol = input[i];
        
        if (!automaton_is_valid_symbol(a, symbol)) {
            return -1;
        }
        
        int nextState = automaton_get_next_state(a, currentState, symbol);
        if (nextState < 0) {
            return -1;
        }
        
        currentState = nextState;
        states[step++] = currentState;
    }
    
    return step;
}

// === Doplněk ===

Automaton* algo_complement(Automaton* a) {
    if (a == NULL) return NULL;
    
    // Validuj automat
    ValidationResult val = automaton_validate(a);
    if (!val.isValid) return NULL;
    
    // Vytvoř kopii
    Automaton* comp = automaton_create();
    if (comp == NULL) return NULL;
    
    // Kopíruj abecedu
    comp->alphabetSize = a->alphabetSize;
    memcpy(comp->alphabet, a->alphabet, MAX_ALPHABET);
    
    // Kopíruj stavy (s obrácenými accepting)
    for (int i = 0; i < a->stateCount; i++) {
        int id = automaton_add_state(comp, a->states[i].x, a->states[i].y);
        comp->states[id].isAccepting = !a->states[i].isAccepting;  // Obrátit!
        if (a->states[i].isInitial) {
            automaton_set_initial(comp, id);
        }
    }
    
    // Kopíruj přechody
    for (int i = 0; i < a->transitionCount; i++) {
        automaton_add_transition(comp, 
            a->transitions[i].fromState,
            a->transitions[i].toState,
            a->transitions[i].symbol);
    }
    
    return comp;
}

// === Minimalizace (Hopcroft algoritmus zjednodušený) ===

// Pomocná struktura pro skupiny stavů
static int partition[MAX_STATES];
static int partitionCount;

static void init_partition(Automaton* a) {
    partitionCount = 2;  // Dva oddíly: přijímající a nepřijímající
    
    for (int i = 0; i < a->stateCount; i++) {
        partition[i] = a->states[i].isAccepting ? 0 : 1;
    }
}

static bool refine_partition(Automaton* a) {
    bool changed = false;
    int newPartition[MAX_STATES];
    int newCount = partitionCount;
    
    for (int i = 0; i < a->stateCount; i++) {
        newPartition[i] = partition[i];
    }
    
    // Pro každou skupinu zkontroluj, zda je potřeba rozdělit
    for (int group = 0; group < partitionCount; group++) {
        // Najdi první stav v této skupině
        int firstInGroup = -1;
        for (int i = 0; i < a->stateCount; i++) {
            if (partition[i] == group) {
                firstInGroup = i;
                break;
            }
        }
        if (firstInGroup < 0) continue;
        
        // Porovnej ostatní stavy ve skupině s prvním
        for (int i = firstInGroup + 1; i < a->stateCount; i++) {
            if (partition[i] != group) continue;
            
            // Zkontroluj, zda stavy jdou do stejných skupin pro všechny symboly
            bool same = true;
            for (int sym = 0; sym < a->alphabetSize && same; sym++) {
                int next1 = automaton_get_next_state(a, firstInGroup, a->alphabet[sym]);
                int next2 = automaton_get_next_state(a, i, a->alphabet[sym]);
                
                if (next1 >= 0 && next2 >= 0) {
                    if (partition[next1] != partition[next2]) {
                        same = false;
                    }
                }
            }
            
            if (!same) {
                // Přesuň do nové skupiny
                newPartition[i] = newCount;
                newCount++;
                changed = true;
            }
        }
    }
    
    if (changed) {
        memcpy(partition, newPartition, sizeof(partition));
        partitionCount = newCount;
    }
    
    return changed;
}

Automaton* algo_minimize(Automaton* a) {
    if (a == NULL) return NULL;
    
    // Validuj automat
    ValidationResult val = automaton_validate(a);
    if (!val.isValid) return NULL;
    
    // Inicializuj rozdělení
    init_partition(a);
    
    // Iterativně zpřesňuj rozdělení
    while (refine_partition(a)) {
        // Pokračuj dokud se něco mění
    }
    
    // Vytvoř nový automat z rozdělení
    Automaton* min = automaton_create();
    if (min == NULL) return NULL;
    
    // Kopíruj abecedu
    min->alphabetSize = a->alphabetSize;
    memcpy(min->alphabet, a->alphabet, MAX_ALPHABET);
    
    // Vytvoř stavy pro každou skupinu
    int groupToState[MAX_STATES];
    memset(groupToState, -1, sizeof(groupToState));
    
    for (int group = 0; group < partitionCount; group++) {
        // Najdi reprezentanta skupiny
        int rep = -1;
        for (int i = 0; i < a->stateCount; i++) {
            if (partition[i] == group) {
                rep = i;
                break;
            }
        }
        if (rep < 0) continue;
        
        // Vytvoř stav
        int newId = automaton_add_state(min, a->states[rep].x, a->states[rep].y);
        groupToState[group] = newId;
        
        min->states[newId].isAccepting = a->states[rep].isAccepting;
        if (a->states[rep].isInitial || partition[a->initialState] == group) {
            automaton_set_initial(min, newId);
        }
    }
    
    // Vytvoř přechody
    for (int group = 0; group < partitionCount; group++) {
        if (groupToState[group] < 0) continue;
        
        // Najdi reprezentanta
        int rep = -1;
        for (int i = 0; i < a->stateCount; i++) {
            if (partition[i] == group) {
                rep = i;
                break;
            }
        }
        if (rep < 0) continue;
        
        // Pro každý symbol
        for (int sym = 0; sym < a->alphabetSize; sym++) {
            int next = automaton_get_next_state(a, rep, a->alphabet[sym]);
            if (next >= 0) {
                int targetGroup = partition[next];
                int targetState = groupToState[targetGroup];
                if (targetState >= 0) {
                    automaton_add_transition(min, groupToState[group], targetState, a->alphabet[sym]);
                }
            }
        }
    }
    
    return min;
}

// === Analýza jazyka ===

bool algo_is_language_empty(Automaton* a) {
    if (a == NULL || a->initialState < 0) return true;
    
    // BFS z počátečního stavu
    bool visited[MAX_STATES] = {false};
    int queue[MAX_STATES];
    int front = 0, back = 0;
    
    queue[back++] = a->initialState;
    visited[a->initialState] = true;
    
    while (front < back) {
        int current = queue[front++];
        
        // Je přijímající?
        if (a->states[current].isAccepting) {
            return false;  // Jazyk není prázdný
        }
        
        // Přidej sousedy
        for (int i = 0; i < a->transitionCount; i++) {
            if (a->transitions[i].fromState == current) {
                int next = a->transitions[i].toState;
                if (!visited[next]) {
                    visited[next] = true;
                    queue[back++] = next;
                }
            }
        }
    }
    
    return true;  // Žádný přijímající stav není dosažitelný
}

bool algo_is_language_finite(Automaton* a) {
    if (a == NULL) return true;
    if (algo_is_language_empty(a)) return true;
    
    // Jazyk je nekonečný, pokud existuje cyklus na cestě k přijímajícímu stavu
    // Zjednodušená verze - detekce cyklu pomocí DFS
    
    bool visited[MAX_STATES] = {false};
    bool inStack[MAX_STATES] = {false};
    bool canReachAccepting[MAX_STATES] = {false};
    
    // Nejprve zjisti, které stavy mohou dosáhnout přijímajícího
    // (zpětné prohledávání)
    for (int i = 0; i < a->stateCount; i++) {
        if (a->states[i].isAccepting) {
            canReachAccepting[i] = true;
        }
    }
    
    bool changed = true;
    while (changed) {
        changed = false;
        for (int i = 0; i < a->transitionCount; i++) {
            int from = a->transitions[i].fromState;
            int to = a->transitions[i].toState;
            if (canReachAccepting[to] && !canReachAccepting[from]) {
                canReachAccepting[from] = true;
                changed = true;
            }
        }
    }
    
    // DFS pro detekci cyklu na produktivních stavech
    int stack[MAX_STATES];
    int stackSize = 0;
    
    stack[stackSize++] = a->initialState;
    
    while (stackSize > 0) {
        int current = stack[--stackSize];
        
        if (visited[current]) {
            if (inStack[current] && canReachAccepting[current]) {
                return false;  // Cyklus na cestě k přijímajícímu = nekonečný jazyk
            }
            continue;
        }
        
        visited[current] = true;
        inStack[current] = true;
        
        for (int i = 0; i < a->transitionCount; i++) {
            if (a->transitions[i].fromState == current) {
                int next = a->transitions[i].toState;
                stack[stackSize++] = next;
            }
        }
    }
    
    return true;  // Žádný cyklus = konečný jazyk
}

void algo_describe_language(Automaton* a, char* output, int maxLen) {
    if (a == NULL || output == NULL) return;
    
    output[0] = '\0';
    int len = 0;
    
    // Základní informace
    if (algo_is_language_empty(a)) {
        snprintf(output, maxLen, "Prázdný jazyk (∅)");
        return;
    }
    
    if (!algo_is_language_finite(a)) {
        len += snprintf(output + len, maxLen - len, "Nekonečný jazyk.\n");
    } else {
        len += snprintf(output + len, maxLen - len, "Konečný jazyk.\n");
    }
    
    // Abeceda
    len += snprintf(output + len, maxLen - len, "Abeceda: {");
    for (int i = 0; i < a->alphabetSize; i++) {
        len += snprintf(output + len, maxLen - len, "%c%s", 
                       a->alphabet[i], 
                       (i < a->alphabetSize - 1) ? ", " : "");
    }
    len += snprintf(output + len, maxLen - len, "}\n");
    
    // Počet stavů
    len += snprintf(output + len, maxLen - len, "Počet stavů: %d\n", a->stateCount);
    
    // Přijímající stavy
    len += snprintf(output + len, maxLen - len, "Přijímající stavy: {");
    bool first = true;
    for (int i = 0; i < a->stateCount; i++) {
        if (a->states[i].isAccepting) {
            len += snprintf(output + len, maxLen - len, "%s%s", 
                           first ? "" : ", ",
                           a->states[i].name);
            first = false;
        }
    }
    len += snprintf(output + len, maxLen - len, "}\n");
}

// === Export ===

void algo_export_table(Automaton* a, char* output, int maxLen) {
    if (a == NULL || output == NULL) return;
    
    output[0] = '\0';
    int len = 0;
    
    // Záhlaví
    len += snprintf(output + len, maxLen - len, "%-10s", "Stav");
    for (int i = 0; i < a->alphabetSize; i++) {
        len += snprintf(output + len, maxLen - len, "| %-6c", a->alphabet[i]);
    }
    len += snprintf(output + len, maxLen - len, "\n");
    
    // Oddělovač
    for (int i = 0; i < 10 + a->alphabetSize * 8; i++) {
        len += snprintf(output + len, maxLen - len, "-");
    }
    len += snprintf(output + len, maxLen - len, "\n");
    
    // Řádky pro každý stav
    for (int s = 0; s < a->stateCount; s++) {
        // Označení stavu
        char prefix[4] = "  ";
        if (a->states[s].isInitial) prefix[0] = '>';
        if (a->states[s].isAccepting) prefix[1] = '*';
        
        len += snprintf(output + len, maxLen - len, "%s%-8s", prefix, a->states[s].name);
        
        // Přechody
        for (int sym = 0; sym < a->alphabetSize; sym++) {
            int next = automaton_get_next_state(a, s, a->alphabet[sym]);
            if (next >= 0) {
                len += snprintf(output + len, maxLen - len, "| %-6s", a->states[next].name);
            } else {
                len += snprintf(output + len, maxLen - len, "| %-6s", "-");
            }
        }
        len += snprintf(output + len, maxLen - len, "\n");
    }
}

void algo_export_dot(Automaton* a, char* output, int maxLen) {
    if (a == NULL || output == NULL) return;
    
    output[0] = '\0';
    int len = 0;
    
    len += snprintf(output + len, maxLen - len, "digraph DFA {\n");
    len += snprintf(output + len, maxLen - len, "  rankdir=LR;\n");
    len += snprintf(output + len, maxLen - len, "  node [shape=circle];\n");
    
    // Přijímající stavy s dvojitým kruhem
    for (int i = 0; i < a->stateCount; i++) {
        if (a->states[i].isAccepting) {
            len += snprintf(output + len, maxLen - len, 
                           "  %s [shape=doublecircle];\n", a->states[i].name);
        }
    }
    
    // Počáteční stav
    if (a->initialState >= 0) {
        len += snprintf(output + len, maxLen - len, "  start [shape=none, label=\"\"];\n");
        len += snprintf(output + len, maxLen - len, "  start -> %s;\n", 
                       a->states[a->initialState].name);
    }
    
    // Přechody
    for (int i = 0; i < a->transitionCount; i++) {
        Transition* t = &a->transitions[i];
        len += snprintf(output + len, maxLen - len, 
                       "  %s -> %s [label=\"%c\"];\n",
                       a->states[t->fromState].name,
                       a->states[t->toState].name,
                       t->symbol);
    }
    
    len += snprintf(output + len, maxLen - len, "}\n");
}

// === Konstrukce z popisu (základní) ===

Automaton* algo_from_regex(const char* regex, const char* alphabet) {
    // Toto je zjednodušená verze - plná implementace by byla složitá
    // Pro teď vrátíme NULL (neimplementováno)
    (void)regex;
    (void)alphabet;
    return NULL;
}

// === Porovnání ===

bool algo_are_equivalent(Automaton* a1, Automaton* a2) {
    if (a1 == NULL || a2 == NULL) return false;
    
    // Dva automaty jsou ekvivalentní, pokud L(A1) ⊕ L(A2) = ∅
    // To znamená: (L(A1) ∩ L(A2)') ∪ (L(A1)' ∩ L(A2)) = ∅
    
    // Zjednodušená verze: minimalizujeme oba a porovnáme strukturu
    Automaton* m1 = algo_minimize(a1);
    Automaton* m2 = algo_minimize(a2);
    
    if (m1 == NULL || m2 == NULL) {
        if (m1) automaton_destroy(m1);
        if (m2) automaton_destroy(m2);
        return false;
    }
    
    bool equiv = (m1->stateCount == m2->stateCount);
    
    // Pro úplné porovnání bychom potřebovali izomorfismus grafů
    // Toto je jen aproximace
    
    automaton_destroy(m1);
    automaton_destroy(m2);
    
    return equiv;
}
