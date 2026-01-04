/**
 * Automaty - GUI Aplikace pro Konečné Automaty
 * 
 * Interaktivní editor deterministických konečných automatů (DFA)
 * s podporou minimalizace, doplňku a testování řetězců.
 */

#include "raylib.h"
#include "automaton.h"
#include "canvas.h"
#include "ui.h"
#include "algorithms.h"

#include <stdio.h>
#include <math.h>

// Aplikační stav
typedef struct {
    Automaton* automaton;
    CanvasState canvas;
    UIState ui;
    int screenWidth;
    int screenHeight;
} AppState;

// Inicializace aplikace
static void app_init(AppState* app) {
    app->screenWidth = 1200;
    app->screenHeight = 800;
    
    // Inicializace raylib
    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_MSAA_4X_HINT);
    InitWindow(app->screenWidth, app->screenHeight, "Automaty - Editor konecnych automatu");
    SetTargetFPS(60);
    SetExitKey(0);  // Zakázat ESC pro ukončení
    
    // Vytvoření automatu
    app->automaton = automaton_create();
    
    // Inicializace plátna
    canvas_init(&app->canvas);
    app->canvas.offsetX = app->screenWidth / 2.0f - 150;
    app->canvas.offsetY = app->screenHeight / 2.0f - 50;
    
    // Inicializace UI
    ui_init(&app->ui, app->screenWidth, app->screenHeight);
}

// Uvolnění zdrojů
static void app_cleanup(AppState* app) {
    ui_cleanup(&app->ui);
    automaton_destroy(app->automaton);
    CloseWindow();
}

// Zpracování vstupu
static void app_update(AppState* app) {
    // Kontrola změny velikosti okna
    if (IsWindowResized()) {
        app->screenWidth = GetScreenWidth();
        app->screenHeight = GetScreenHeight();
        ui_resize(&app->ui, app->screenWidth, app->screenHeight);
    }
    
    // UI má přednost
    bool uiHandled = ui_handle_input(&app->ui, app->automaton, &app->canvas);
    
    // Pokud UI nezpracovalo vstup, předej plátnu
    if (!uiHandled) {
        bool changed = canvas_handle_input(app->automaton, &app->canvas, app->ui.canvasBounds);
        
        // Kontrola, zda bylo ukončeno kreslení přechodu
        if (changed && app->canvas.currentTool == TOOL_TRANSITION && 
            !app->canvas.isDragging && 
            app->canvas.selectedState >= 0 && app->canvas.hoveredState >= 0) {
            
            // Otevři dialog pro zadání symbolu
            ui_open_symbol_dialog(&app->ui, app->canvas.selectedState, app->canvas.hoveredState);
        }
    }
    
    // Zpracování zavření dialogu se symbolem
    if (!app->ui.symbolDialogActive && 
        app->ui.pendingFromState >= 0 && app->ui.pendingToState >= 0) {
        
        // Přidej přechod pro každý zadaný symbol
        for (int i = 0; app->ui.symbolInput[i] != '\0'; i++) {
            char symbol = app->ui.symbolInput[i];
            if (automaton_is_valid_symbol(app->automaton, symbol)) {
                automaton_add_transition(app->automaton, 
                    app->ui.pendingFromState, 
                    app->ui.pendingToState, 
                    symbol);
            }
        }
        
        app->ui.pendingFromState = -1;
        app->ui.pendingToState = -1;
    }
}

// Vykreslení
static void app_draw(AppState* app) {
    BeginDrawing();
    
    // Pozadí plátna
    ClearBackground(WHITE);
    
    // Ořez na oblast plátna
    BeginScissorMode(
        (int)app->ui.canvasBounds.x, 
        (int)app->ui.canvasBounds.y,
        (int)app->ui.canvasBounds.width, 
        (int)app->ui.canvasBounds.height
    );
    
    // Mřížka na pozadí
    Color gridColor = (Color){230, 230, 230, 255};
    float gridSize = 50 * app->canvas.zoom;
    float startX = fmodf(app->canvas.offsetX, gridSize);
    float startY = fmodf(app->canvas.offsetY, gridSize);
    
    for (float x = startX; x < app->ui.canvasBounds.x + app->ui.canvasBounds.width; x += gridSize) {
        if (x >= app->ui.canvasBounds.x) {
            DrawLine((int)x, (int)app->ui.canvasBounds.y, 
                    (int)x, (int)(app->ui.canvasBounds.y + app->ui.canvasBounds.height), 
                    gridColor);
        }
    }
    for (float y = startY; y < app->ui.canvasBounds.y + app->ui.canvasBounds.height; y += gridSize) {
        if (y >= app->ui.canvasBounds.y) {
            DrawLine((int)app->ui.canvasBounds.x, (int)y, 
                    (int)(app->ui.canvasBounds.x + app->ui.canvasBounds.width), (int)y, 
                    gridColor);
        }
    }
    
    // Automat
    canvas_draw(app->automaton, &app->canvas);
    
    EndScissorMode();
    
    // UI komponenty
    ui_draw(&app->ui, app->automaton, &app->canvas);
    
    // FPS (debug)
    // DrawFPS(10, 10);
    
    EndDrawing();
}

// Hlavní smyčka
int main(void) {
    AppState app = {0};
    
    app_init(&app);
    
    // Hlavní smyčka
    while (!WindowShouldClose()) {
        app_update(&app);
        app_draw(&app);
    }
    
    app_cleanup(&app);
    
    return 0;
}
