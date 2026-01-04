#ifndef ALGORITHMS_H
#define ALGORITHMS_H

#include "automaton.h"

// === Testování řetězce ===

// Otestuje, zda automat přijímá řetězec
// Vrací true pokud řetězec je přijat
bool algo_test_string(Automaton* a, const char* input);

// Otestuje řetězec a vrátí průběh (sekvenci stavů)
// states musí mít velikost alespoň strlen(input) + 1
// Vrací počet kroků nebo -1 při chybě
int algo_test_string_trace(Automaton* a, const char* input, int* states);

// === Minimalizace ===

// Vytvoří minimální ekvivalentní automat
// Vrací nový automat (volající musí uvolnit) nebo NULL při chybě
Automaton* algo_minimize(Automaton* a);

// === Doplněk ===

// Vytvoří doplněk automatu (přijímá L^c)
// Vrací nový automat (volající musí uvolnit) nebo NULL při chybě
Automaton* algo_complement(Automaton* a);

// === Analýza jazyka ===

// Vygeneruje popis jazyka automatu
// output musí mít velikost alespoň maxLen
void algo_describe_language(Automaton* a, char* output, int maxLen);

// Zkontroluje, zda jazyk je prázdný
bool algo_is_language_empty(Automaton* a);

// Zkontroluje, zda jazyk je konečný
bool algo_is_language_finite(Automaton* a);

// === Export ===

// Exportuje automat do textové tabulky
void algo_export_table(Automaton* a, char* output, int maxLen);

// Exportuje automat do formátu DOT (pro Graphviz)
void algo_export_dot(Automaton* a, char* output, int maxLen);

// === Konstrukce z popisu ===

// Vytvoří automat z regulárního výrazu (základní podpora)
// Vrací nový automat nebo NULL při chybě
Automaton* algo_from_regex(const char* regex, const char* alphabet);

// === Porovnání ===

// Zkontroluje ekvivalenci dvou automatů
bool algo_are_equivalent(Automaton* a1, Automaton* a2);

#endif // ALGORITHMS_H
