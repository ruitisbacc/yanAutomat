#include "canvas.h"
#include "automaton.h"
#include <math.h>
#include <string.h>
#include <stdio.h>

#ifndef PI
#define PI 3.14159265358979323846f
#endif

// Inicializuje stav plátna
void canvas_init(CanvasState* cs) {
    cs->offsetX = 0.0f;
    cs->offsetY = 0.0f;
    cs->zoom = 1.0f;
    cs->currentTool = TOOL_SELECT;
    cs->selectedState = -1;
    cs->hoveredState = -1;
    cs->dragFromState = -1;
    cs->isDragging = false;
    cs->isPanning = false;
    cs->dragStartX = 0.0f;
    cs->dragStartY = 0.0f;
    cs->lastMousePos = (Vector2){0, 0};
}

// === Transformace souřadnic ===

Vector2 canvas_screen_to_world(CanvasState* cs, Vector2 screenPos) {
    Vector2 world;
    world.x = (screenPos.x - cs->offsetX) / cs->zoom;
    world.y = (screenPos.y - cs->offsetY) / cs->zoom;
    return world;
}

Vector2 canvas_world_to_screen(CanvasState* cs, Vector2 worldPos) {
    Vector2 screen;
    screen.x = worldPos.x * cs->zoom + cs->offsetX;
    screen.y = worldPos.y * cs->zoom + cs->offsetY;
    return screen;
}

// === Pomocné funkce pro kreslení ===

// Vypočítá bod na okraji kruhu směrem k jinému bodu
static Vector2 get_circle_edge(Vector2 center, Vector2 target, float radius) {
    float dx = target.x - center.x;
    float dy = target.y - center.y;
    float dist = sqrtf(dx * dx + dy * dy);
    
    if (dist < 0.001f) {
        return (Vector2){center.x + radius, center.y};
    }
    
    return (Vector2){
        center.x + (dx / dist) * radius,
        center.y + (dy / dist) * radius
    };
}

// Nakreslí šipku
static void draw_arrow(Vector2 from, Vector2 to, Color color) {
    DrawLineEx(from, to, 2.0f, color);
    
    // Šipka
    float angle = atan2f(to.y - from.y, to.x - from.x);
    float arrowAngle = PI / 6.0f;  // 30 stupňů
    float arrowLen = ARROW_SIZE;
    
    Vector2 arrow1 = {
        to.x - arrowLen * cosf(angle - arrowAngle),
        to.y - arrowLen * sinf(angle - arrowAngle)
    };
    Vector2 arrow2 = {
        to.x - arrowLen * cosf(angle + arrowAngle),
        to.y - arrowLen * sinf(angle + arrowAngle)
    };
    
    DrawTriangle(to, arrow2, arrow1, color);
}

// === Kreslení ===

// Vykreslí jeden stav
void canvas_draw_state(State* s, CanvasState* cs, bool isSelected, bool isHovered) {
    Vector2 worldPos = {s->x, s->y};
    Vector2 screenPos = canvas_world_to_screen(cs, worldPos);
    float radius = STATE_RADIUS * cs->zoom;
    
    // Barva podle stavu
    Color fillColor = STATE_COLOR;
    if (s->isAccepting) fillColor = STATE_ACCEPTING_COLOR;
    if (isHovered) fillColor = STATE_HOVER_COLOR;
    if (isSelected) fillColor = STATE_SELECTED_COLOR;
    
    // Hlavní kruh
    DrawCircleV(screenPos, radius, fillColor);
    DrawCircleLines((int)screenPos.x, (int)screenPos.y, radius, BLACK);
    
    // Přijímající stav - dvojitý kruh
    if (s->isAccepting) {
        DrawCircleLines((int)screenPos.x, (int)screenPos.y, radius - 5 * cs->zoom, BLACK);
    }
    
    // Počáteční stav - vstupní šipka
    if (s->isInitial) {
        Vector2 arrowStart = {screenPos.x - radius - 30 * cs->zoom, screenPos.y};
        Vector2 arrowEnd = {screenPos.x - radius, screenPos.y};
        draw_arrow(arrowStart, arrowEnd, BLACK);
    }
    
    // Název stavu
    int fontSize = (int)(FONT_SIZE * cs->zoom);
    if (fontSize < 8) fontSize = 8;
    int textWidth = MeasureText(s->name, fontSize);
    DrawText(s->name, 
             (int)(screenPos.x - textWidth / 2), 
             (int)(screenPos.y - fontSize / 2), 
             fontSize, WHITE);
}

// Vykreslí přechod (šipku)
void canvas_draw_transition(Automaton* a, Transition* t, CanvasState* cs) {
    State* fromState = automaton_get_state(a, t->fromState);
    State* toState = automaton_get_state(a, t->toState);
    
    if (fromState == NULL || toState == NULL) return;
    
    // Self-loop
    if (t->fromState == t->toState) {
        canvas_draw_self_loop(fromState, t->symbol, cs);
        return;
    }
    
    Vector2 from = canvas_world_to_screen(cs, (Vector2){fromState->x, fromState->y});
    Vector2 to = canvas_world_to_screen(cs, (Vector2){toState->x, toState->y});
    float radius = STATE_RADIUS * cs->zoom;
    
    // Spočítej body na okraji kruhů
    Vector2 fromEdge = get_circle_edge(from, to, radius);
    Vector2 toEdge = get_circle_edge(to, from, radius);
    
    // Zkontroluj, zda existuje opačný přechod - pokud ano, zakřiv
    bool hasReverse = false;
    for (int i = 0; i < a->transitionCount; i++) {
        if (a->transitions[i].fromState == t->toState && 
            a->transitions[i].toState == t->fromState) {
            hasReverse = true;
            break;
        }
    }
    
    if (hasReverse) {
        // Zakřivená čára
        float midX = (from.x + to.x) / 2.0f;
        float midY = (from.y + to.y) / 2.0f;
        float perpX = -(to.y - from.y);
        float perpY = to.x - from.x;
        float perpLen = sqrtf(perpX * perpX + perpY * perpY);
        if (perpLen > 0.001f) {
            perpX /= perpLen;
            perpY /= perpLen;
        }
        float offset = 20.0f * cs->zoom;
        Vector2 ctrl = {midX + perpX * offset, midY + perpY * offset};
        
        // Zakřivená čára - ručně vykreslíme segmenty
        int segments = 20;
        Vector2 prev = fromEdge;
        for (int seg = 1; seg <= segments; seg++) {
            float t_seg = (float)seg / segments;
            float t2 = t_seg * t_seg;
            float mt = 1.0f - t_seg;
            float mt2 = mt * mt;
            Vector2 point = {
                mt2 * fromEdge.x + 2 * mt * t_seg * ctrl.x + t2 * toEdge.x,
                mt2 * fromEdge.y + 2 * mt * t_seg * ctrl.y + t2 * toEdge.y
            };
            DrawLineEx(prev, point, 2.0f, TRANSITION_COLOR);
            prev = point;
        }
        
        // Šipka na konci
        float t_param = 0.95f;
        Vector2 nearEnd = {
            (1-t_param)*(1-t_param)*fromEdge.x + 2*(1-t_param)*t_param*ctrl.x + t_param*t_param*toEdge.x,
            (1-t_param)*(1-t_param)*fromEdge.y + 2*(1-t_param)*t_param*ctrl.y + t_param*t_param*toEdge.y
        };
        draw_arrow(nearEnd, toEdge, TRANSITION_COLOR);
        
        // Symbol
        char symbolStr[4] = {t->symbol, '\0'};
        int fontSize = (int)(FONT_SIZE * cs->zoom);
        DrawText(symbolStr, (int)(ctrl.x - 5), (int)(ctrl.y - fontSize/2), fontSize, BLACK);
    } else {
        // Přímá čára
        draw_arrow(fromEdge, toEdge, TRANSITION_COLOR);
        
        // Symbol uprostřed
        char symbolStr[4] = {t->symbol, '\0'};
        float midX = (fromEdge.x + toEdge.x) / 2.0f;
        float midY = (fromEdge.y + toEdge.y) / 2.0f - 15.0f * cs->zoom;
        int fontSize = (int)(FONT_SIZE * cs->zoom);
        DrawText(symbolStr, (int)(midX - 5), (int)(midY), fontSize, BLACK);
    }
}

// Vykreslí smyčku (self-loop)
void canvas_draw_self_loop(State* s, char symbol, CanvasState* cs) {
    Vector2 screenPos = canvas_world_to_screen(cs, (Vector2){s->x, s->y});
    float radius = STATE_RADIUS * cs->zoom;
    float loopRadius = radius * 0.7f;
    
    // Smyčka nahoře
    Vector2 loopCenter = {screenPos.x, screenPos.y - radius - loopRadius};
    
    DrawCircleLines((int)loopCenter.x, (int)loopCenter.y, loopRadius, TRANSITION_COLOR);
    
    // Šipka
    Vector2 arrowPos = {loopCenter.x + loopRadius * 0.7f, loopCenter.y + loopRadius * 0.7f};
    Vector2 arrowDir = {screenPos.x + radius * 0.3f, screenPos.y - radius};
    draw_arrow(arrowPos, arrowDir, TRANSITION_COLOR);
    
    // Symbol
    char symbolStr[4] = {symbol, '\0'};
    int fontSize = (int)(FONT_SIZE * cs->zoom);
    DrawText(symbolStr, (int)(loopCenter.x - 5), (int)(loopCenter.y - loopRadius - fontSize), fontSize, BLACK);
}

// Vykreslí dočasnou čáru při kreslení přechodu
void canvas_draw_temp_transition(CanvasState* cs, float fromX, float fromY, float toX, float toY) {
    (void)cs;  // zatím nepoužito
    DrawLineEx((Vector2){fromX, fromY}, (Vector2){toX, toY}, 2.0f, GRAY);
}

// Vykreslí celý automat
void canvas_draw(Automaton* a, CanvasState* cs) {
    // Nejprve přechody
    for (int i = 0; i < a->transitionCount; i++) {
        canvas_draw_transition(a, &a->transitions[i], cs);
    }
    
    // Pak stavy (aby byly nahoře)
    for (int i = 0; i < a->stateCount; i++) {
        bool isSelected = (i == cs->selectedState);
        bool isHovered = (i == cs->hoveredState);
        canvas_draw_state(&a->states[i], cs, isSelected, isHovered);
    }
    
    // Dočasná čára při kreslení přechodu
    if (cs->isDragging && cs->currentTool == TOOL_TRANSITION && cs->dragFromState >= 0) {
        State* fromState = automaton_get_state(a, cs->dragFromState);
        if (fromState != NULL) {
            Vector2 from = canvas_world_to_screen(cs, (Vector2){fromState->x, fromState->y});
            Vector2 mouse = GetMousePosition();
            canvas_draw_temp_transition(cs, from.x, from.y, mouse.x, mouse.y);
        }
    }
}

// === Vstup ===

// Najde stav pod kurzorem
int canvas_find_state_at_screen(Automaton* a, CanvasState* cs, Vector2 screenPos) {
    Vector2 worldPos = canvas_screen_to_world(cs, screenPos);
    return automaton_find_state_at(a, worldPos.x, worldPos.y, STATE_RADIUS);
}

// Najde přechod blízko kurzoru
int canvas_find_transition_near(Automaton* a, CanvasState* cs, Vector2 screenPos) {
    Vector2 worldPos = canvas_screen_to_world(cs, screenPos);
    
    for (int i = 0; i < a->transitionCount; i++) {
        Transition* t = &a->transitions[i];
        State* from = automaton_get_state(a, t->fromState);
        State* to = automaton_get_state(a, t->toState);
        if (from == NULL || to == NULL) continue;
        
        // Vzdálenost bodu od úsečky
        float ax = from->x, ay = from->y;
        float bx = to->x, by = to->y;
        float px = worldPos.x, py = worldPos.y;
        
        float abx = bx - ax, aby = by - ay;
        float apx = px - ax, apy = py - ay;
        float ab2 = abx * abx + aby * aby;
        
        if (ab2 < 0.001f) continue;
        
        float t_param = (apx * abx + apy * aby) / ab2;
        if (t_param < 0.0f) t_param = 0.0f;
        if (t_param > 1.0f) t_param = 1.0f;
        
        float nearX = ax + t_param * abx;
        float nearY = ay + t_param * aby;
        float dist = sqrtf((px - nearX) * (px - nearX) + (py - nearY) * (py - nearY));
        
        if (dist < 10.0f) {
            return i;
        }
    }
    
    return -1;
}

// Zpracuje vstup myši a klávesnice
bool canvas_handle_input(Automaton* a, CanvasState* cs, Rectangle canvasBounds) {
    Vector2 mousePos = GetMousePosition();
    bool changed = false;
    
    // Zkontroluj, zda je myš v plátně
    if (!CheckCollisionPointRec(mousePos, canvasBounds)) {
        cs->hoveredState = -1;
        return false;
    }
    
    // Aktualizuj hovered stav
    cs->hoveredState = canvas_find_state_at_screen(a, cs, mousePos);
    
    // Zoom kolečkem
    float wheel = GetMouseWheelMove();
    if (wheel != 0) {
        float oldZoom = cs->zoom;
        cs->zoom += wheel * 0.1f;
        if (cs->zoom < 0.3f) cs->zoom = 0.3f;
        if (cs->zoom > 3.0f) cs->zoom = 3.0f;
        
        // Zoom k pozici myši
        float zoomRatio = cs->zoom / oldZoom;
        cs->offsetX = mousePos.x - (mousePos.x - cs->offsetX) * zoomRatio;
        cs->offsetY = mousePos.y - (mousePos.y - cs->offsetY) * zoomRatio;
    }
    
    // Panning prostředním tlačítkem nebo levým (pokud je panning aktivní)
    if (IsMouseButtonDown(MOUSE_BUTTON_MIDDLE)) {
        if (!cs->isPanning) {
            cs->isPanning = true;
            cs->lastMousePos = mousePos;
        }
    }
    
    // Panning pohyb
    if (cs->isPanning) {
        if (IsMouseButtonDown(MOUSE_BUTTON_MIDDLE) || IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
            cs->offsetX += mousePos.x - cs->lastMousePos.x;
            cs->offsetY += mousePos.y - cs->lastMousePos.y;
            cs->lastMousePos = mousePos;
        }
        
        // Ukonči panning
        if (IsMouseButtonReleased(MOUSE_BUTTON_MIDDLE) || IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
            cs->isPanning = false;
        }
    }
    
    // Levé tlačítko - akce podle nástroje
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        int stateAtMouse = canvas_find_state_at_screen(a, cs, mousePos);
        
        switch (cs->currentTool) {
            case TOOL_SELECT:
                cs->selectedState = stateAtMouse;
                if (stateAtMouse >= 0) {
                    cs->isDragging = true;
                    State* s = automaton_get_state(a, stateAtMouse);
                    cs->dragStartX = s->x;
                    cs->dragStartY = s->y;
                } else {
                    // Kliknutí mimo stav - začni panning
                    cs->isPanning = true;
                    cs->lastMousePos = mousePos;
                }
                break;
                
            case TOOL_ADD_STATE:
                {
                    Vector2 worldPos = canvas_screen_to_world(cs, mousePos);
                    int newId = automaton_add_state(a, worldPos.x, worldPos.y);
                    if (newId >= 0) {
                        cs->selectedState = newId;
                        changed = true;
                    }
                }
                break;
                
            case TOOL_TRANSITION:
                if (stateAtMouse >= 0) {
                    cs->dragFromState = stateAtMouse;
                    cs->isDragging = true;
                }
                break;
                
            case TOOL_DELETE:
                if (stateAtMouse >= 0) {
                    automaton_remove_state(a, stateAtMouse);
                    cs->selectedState = -1;
                    changed = true;
                } else {
                    int transIdx = canvas_find_transition_near(a, cs, mousePos);
                    if (transIdx >= 0) {
                        automaton_remove_transition(a, transIdx);
                        changed = true;
                    }
                }
                break;
        }
    }
    
    // Držení levého tlačítka
    if (IsMouseButtonDown(MOUSE_BUTTON_LEFT) && cs->isDragging) {
        if (cs->currentTool == TOOL_SELECT && cs->selectedState >= 0) {
            // Přesuň stav
            Vector2 worldPos = canvas_screen_to_world(cs, mousePos);
            State* s = automaton_get_state(a, cs->selectedState);
            if (s != NULL) {
                s->x = worldPos.x;
                s->y = worldPos.y;
                changed = true;
            }
        }
    }
    
    // Uvolnění levého tlačítka
    if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT) && cs->isDragging) {
        if (cs->currentTool == TOOL_TRANSITION && cs->dragFromState >= 0) {
            int toState = canvas_find_state_at_screen(a, cs, mousePos);
            if (toState >= 0) {
                // Vrátíme speciální hodnotu - UI musí otevřít dialog pro symbol
                // Uložíme pending stavy do cs
                cs->selectedState = cs->dragFromState;
                cs->hoveredState = toState;
                changed = true;  // signál pro UI
            }
        }
        cs->isDragging = false;
        cs->dragFromState = -1;
    }
    
    // Pravé tlačítko - přepnutí accepting stavu
    if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) {
        int stateAtMouse = canvas_find_state_at_screen(a, cs, mousePos);
        if (stateAtMouse >= 0) {
            State* s = automaton_get_state(a, stateAtMouse);
            if (s != NULL) {
                s->isAccepting = !s->isAccepting;
                changed = true;
            }
        }
    }
    
    // Klávesy
    if (cs->selectedState >= 0) {
        if (IsKeyPressed(KEY_DELETE)) {
            automaton_remove_state(a, cs->selectedState);
            cs->selectedState = -1;
            changed = true;
        }
        if (IsKeyPressed(KEY_I)) {
            automaton_set_initial(a, cs->selectedState);
            changed = true;
        }
        if (IsKeyPressed(KEY_A)) {
            State* s = automaton_get_state(a, cs->selectedState);
            if (s != NULL) {
                s->isAccepting = !s->isAccepting;
                changed = true;
            }
        }
    }
    
    return changed;
}
