#ifndef CANVAS_H
#define CANVAS_H

#include "automaton.h"
#include "raylib.h"

// Nástroje pro editaci
typedef enum {
    TOOL_SELECT,       // Vybírání a přesun stavů
    TOOL_ADD_STATE,    // Přidávání nových stavů
    TOOL_TRANSITION,   // Kreslení přechodů
    TOOL_DELETE        // Mazání stavů a přechodů
} CanvasTool;

// Stav plátna
typedef struct {
    float offsetX, offsetY;    // Posun plátna (panning)
    float zoom;                // Přiblížení
    CanvasTool currentTool;    // Aktuální nástroj
    int selectedState;         // ID vybraného stavu (-1 = nic)
    int hoveredState;          // ID stavu pod kurzorem (-1 = nic)
    int dragFromState;         // Pro kreslení přechodů - výchozí stav
    bool isDragging;           // Probíhá přetahování?
    bool isPanning;            // Probíhá posun plátna?
    float dragStartX, dragStartY;  // Počáteční pozice při drag
    Vector2 lastMousePos;      // Předchozí pozice myši
} CanvasState;

// Konstanty pro kreslení
#define STATE_RADIUS 35.0f
#define STATE_COLOR DARKBLUE
#define STATE_ACCEPTING_COLOR DARKGREEN
#define STATE_INITIAL_COLOR ORANGE
#define STATE_SELECTED_COLOR RED
#define STATE_HOVER_COLOR SKYBLUE
#define TRANSITION_COLOR DARKGRAY
#define ARROW_SIZE 12.0f
#define FONT_SIZE 16

// === Inicializace ===

// Inicializuje stav plátna
void canvas_init(CanvasState* cs);

// === Kreslení ===

// Vykreslí celý automat na plátno
void canvas_draw(Automaton* a, CanvasState* cs);

// Vykreslí jeden stav
void canvas_draw_state(State* s, CanvasState* cs, bool isSelected, bool isHovered);

// Vykreslí šipku přechodu
void canvas_draw_transition(Automaton* a, Transition* t, CanvasState* cs);

// Vykreslí smyčku (self-loop) na stavu
void canvas_draw_self_loop(State* s, char symbol, CanvasState* cs);

// Vykreslí dočasnou čáru při kreslení přechodu
void canvas_draw_temp_transition(CanvasState* cs, float fromX, float fromY, float toX, float toY);

// === Transformace souřadnic ===

// Převede souřadnice obrazovky na souřadnice plátna
Vector2 canvas_screen_to_world(CanvasState* cs, Vector2 screenPos);

// Převede souřadnice plátna na souřadnice obrazovky
Vector2 canvas_world_to_screen(CanvasState* cs, Vector2 worldPos);

// === Vstup ===

// Zpracuje vstup myši a klávesnice, vrací true pokud došlo ke změně
bool canvas_handle_input(Automaton* a, CanvasState* cs, Rectangle canvasBounds);

// === Pomocné ===

// Najde stav pod kurzorem
int canvas_find_state_at_screen(Automaton* a, CanvasState* cs, Vector2 screenPos);

// Najde přechod blízko kurzoru
int canvas_find_transition_near(Automaton* a, CanvasState* cs, Vector2 screenPos);

#endif // CANVAS_H
