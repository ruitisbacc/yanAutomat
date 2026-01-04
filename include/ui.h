#ifndef UI_H
#define UI_H

#include "automaton.h"
#include "canvas.h"
#include "raylib.h"

// Režimy aplikace
typedef enum {
    APP_MODE_EDIT,          // Editace automatu
    APP_MODE_TEST,          // Testování řetězce
    APP_MODE_TABLE,         // Zobrazení tabulky
    APP_MODE_INPUT_SYMBOL   // Čeká na vstup symbolu pro přechod
} AppMode;

// Stav UI
typedef struct {
    AppMode mode;
    
    // Toolbar
    Rectangle toolbarBounds;
    
    // Panel nástrojů
    Rectangle toolPanelBounds;
    
    // Pravý panel
    Rectangle rightPanelBounds;
    
    // Plátno
    Rectangle canvasBounds;
    
    // Spodní panel (validace)
    Rectangle bottomPanelBounds;
    
    // Vstupní pole pro abecedu
    char alphabetInput[32];
    bool alphabetInputActive;
    
    // Vstupní pole pro testování řetězce
    char testInput[256];
    bool testInputActive;
    
    // Výsledek testu
    char testResult[128];
    bool testResultShown;
    
    // Dialog pro zadání symbolu přechodu
    bool symbolDialogActive;
    int pendingFromState;
    int pendingToState;
    char symbolInput[8];
    
    // Tabulka automatu
    char tableOutput[4096];
    
    // Validační zprávy
    ValidationResult validation;
    
    // Styl
    Font font;
    bool fontLoaded;
} UIState;

// === Inicializace ===

// Inicializuje UI
void ui_init(UIState* ui, int screenWidth, int screenHeight);

// Uvolní prostředky UI
void ui_cleanup(UIState* ui);

// Přepočítá rozměry při změně velikosti okna
void ui_resize(UIState* ui, int screenWidth, int screenHeight);

// === Kreslení ===

// Vykreslí celé UI
void ui_draw(UIState* ui, Automaton* a, CanvasState* cs);

// Vykreslí toolbar
void ui_draw_toolbar(UIState* ui, CanvasState* cs);

// Vykreslí pravý panel
void ui_draw_right_panel(UIState* ui, Automaton* a);

// Vykreslí spodní panel s validací
void ui_draw_bottom_panel(UIState* ui, Automaton* a);

// Vykreslí dialog pro zadání symbolu
void ui_draw_symbol_dialog(UIState* ui);

// Vykreslí tabulku automatu
void ui_draw_table(UIState* ui, Automaton* a);

// === Vstup ===

// Zpracuje vstup UI, vrací true pokud UI zpracovalo vstup
bool ui_handle_input(UIState* ui, Automaton* a, CanvasState* cs);

// === Akce ===

// Spustí test řetězce
void ui_test_string(UIState* ui, Automaton* a);

// Aktualizuje tabulku automatu
void ui_update_table(UIState* ui, Automaton* a);

// Otevře dialog pro zadání symbolu přechodu
void ui_open_symbol_dialog(UIState* ui, int fromState, int toState);

// Uzavře dialog pro symbol
void ui_close_symbol_dialog(UIState* ui);

// === Pomocné funkce ===

// Tlačítko - vrací true při kliknutí
bool ui_button(Rectangle bounds, const char* text, Color color);

// Textové vstupní pole - vrací true pokud je aktivní
bool ui_text_input(Rectangle bounds, char* text, int maxLen, bool* active, const char* placeholder);

// Toggle tlačítko
bool ui_toggle_button(Rectangle bounds, const char* text, bool active);

#endif // UI_H
