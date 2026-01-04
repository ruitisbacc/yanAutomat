#include "ui.h"
#include "algorithms.h"
#include <string.h>
#include <stdio.h>

// Konstanty UI
#define TOOLBAR_WIDTH 60
#define RIGHT_PANEL_WIDTH 280
#define BOTTOM_PANEL_HEIGHT 100
#define BUTTON_HEIGHT 40
#define BUTTON_MARGIN 5
#define INPUT_HEIGHT 30

// Barvy
#define UI_BG_COLOR (Color){240, 240, 240, 255}
#define UI_PANEL_COLOR (Color){220, 220, 220, 255}
#define UI_BUTTON_COLOR (Color){100, 100, 200, 255}
#define UI_BUTTON_HOVER_COLOR (Color){120, 120, 220, 255}
#define UI_BUTTON_ACTIVE_COLOR (Color){80, 80, 180, 255}
#define UI_INPUT_BG_COLOR WHITE
#define UI_INPUT_BORDER_COLOR GRAY
#define UI_TEXT_COLOR BLACK
#define UI_SUCCESS_COLOR (Color){50, 150, 50, 255}
#define UI_ERROR_COLOR (Color){200, 50, 50, 255}

// Globální font pro celé UI
static Font g_font;
static bool g_fontLoaded = false;

// Pomocná funkce pro kreslení textu s vlastním fontem
static void DrawTextCustom(const char* text, int x, int y, int fontSize, Color color) {
    if (g_fontLoaded) {
        DrawTextEx(g_font, text, (Vector2){(float)x, (float)y}, (float)fontSize, 1.0f, color);
    } else {
        DrawText(text, x, y, fontSize, color);
    }
}

// Pomocná funkce pro měření textu
static int MeasureTextCustom(const char* text, int fontSize) {
    if (g_fontLoaded) {
        Vector2 size = MeasureTextEx(g_font, text, (float)fontSize, 1.0f);
        return (int)size.x;
    } else {
        return MeasureText(text, fontSize);
    }
}

// === Inicializace ===

void ui_init(UIState* ui, int screenWidth, int screenHeight) {
    ui->mode = APP_MODE_EDIT;
    
    // Výpočet rozměrů
    ui_resize(ui, screenWidth, screenHeight);
    
    // Načti font Inter
    g_font = LoadFontEx("assets/Inter.ttf", 32, NULL, 0);
    if (g_font.texture.id != 0) {
        g_fontLoaded = true;
        ui->fontLoaded = true;
        SetTextureFilter(g_font.texture, TEXTURE_FILTER_BILINEAR);
    } else {
        g_fontLoaded = false;
        ui->fontLoaded = false;
        g_font = GetFontDefault();
    }
    ui->font = g_font;
    
    // Výchozí hodnoty
    strcpy(ui->alphabetInput, "01");
    ui->alphabetInputActive = false;
    
    ui->testInput[0] = '\0';
    ui->testInputActive = false;
    
    ui->testResult[0] = '\0';
    ui->testResultShown = false;
    
    ui->symbolDialogActive = false;
    ui->pendingFromState = -1;
    ui->pendingToState = -1;
    ui->symbolInput[0] = '\0';
    
    ui->tableOutput[0] = '\0';
    
    ui->validation.isValid = true;
    ui->validation.messageCount = 0;
}

void ui_cleanup(UIState* ui) {
    if (ui->fontLoaded) {
        UnloadFont(ui->font);
        ui->fontLoaded = false;
    }
}

void ui_resize(UIState* ui, int screenWidth, int screenHeight) {
    // Toolbar vlevo
    ui->toolbarBounds = (Rectangle){0, 0, TOOLBAR_WIDTH, (float)screenHeight};
    
    // Pravý panel
    ui->rightPanelBounds = (Rectangle){
        screenWidth - RIGHT_PANEL_WIDTH, 0, 
        RIGHT_PANEL_WIDTH, (float)screenHeight
    };
    
    // Spodní panel
    ui->bottomPanelBounds = (Rectangle){
        TOOLBAR_WIDTH, screenHeight - BOTTOM_PANEL_HEIGHT,
        screenWidth - TOOLBAR_WIDTH - RIGHT_PANEL_WIDTH, BOTTOM_PANEL_HEIGHT
    };
    
    // Plátno (zbytek)
    ui->canvasBounds = (Rectangle){
        TOOLBAR_WIDTH, 0,
        screenWidth - TOOLBAR_WIDTH - RIGHT_PANEL_WIDTH,
        screenHeight - BOTTOM_PANEL_HEIGHT
    };
}

// === Pomocné funkce pro kreslení ===

bool ui_button(Rectangle bounds, const char* text, Color color) {
    Vector2 mousePos = GetMousePosition();
    bool hover = CheckCollisionPointRec(mousePos, bounds);
    bool pressed = hover && IsMouseButtonDown(MOUSE_BUTTON_LEFT);
    bool clicked = hover && IsMouseButtonReleased(MOUSE_BUTTON_LEFT);
    
    Color bgColor = color;
    if (pressed) bgColor = UI_BUTTON_ACTIVE_COLOR;
    else if (hover) bgColor = UI_BUTTON_HOVER_COLOR;
    
    DrawRectangleRec(bounds, bgColor);
    DrawRectangleLinesEx(bounds, 1, DARKGRAY);
    
    int fontSize = 14;
    int textWidth = MeasureTextCustom(text, fontSize);
    DrawTextCustom(text, 
             (int)(bounds.x + bounds.width/2 - textWidth/2),
             (int)(bounds.y + bounds.height/2 - fontSize/2),
             fontSize, WHITE);
    
    return clicked;
}

bool ui_toggle_button(Rectangle bounds, const char* text, bool active) {
    Vector2 mousePos = GetMousePosition();
    bool hover = CheckCollisionPointRec(mousePos, bounds);
    bool clicked = hover && IsMouseButtonReleased(MOUSE_BUTTON_LEFT);
    
    Color bgColor = active ? UI_BUTTON_ACTIVE_COLOR : UI_PANEL_COLOR;
    if (hover && !active) bgColor = UI_BUTTON_HOVER_COLOR;
    
    DrawRectangleRec(bounds, bgColor);
    DrawRectangleLinesEx(bounds, 1, DARKGRAY);
    
    int fontSize = 12;
    int textWidth = MeasureTextCustom(text, fontSize);
    Color textColor = active ? WHITE : UI_TEXT_COLOR;
    DrawTextCustom(text, 
             (int)(bounds.x + bounds.width/2 - textWidth/2),
             (int)(bounds.y + bounds.height/2 - fontSize/2),
             fontSize, textColor);
    
    return clicked;
}

bool ui_text_input(Rectangle bounds, char* text, int maxLen, bool* active, const char* placeholder) {
    Vector2 mousePos = GetMousePosition();
    bool hover = CheckCollisionPointRec(mousePos, bounds);
    
    // Klik aktivuje/deaktivuje
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        *active = hover;
    }
    
    // Barva
    Color borderColor = *active ? BLUE : UI_INPUT_BORDER_COLOR;
    
    DrawRectangleRec(bounds, UI_INPUT_BG_COLOR);
    DrawRectangleLinesEx(bounds, 2, borderColor);
    
    // Text nebo placeholder
    int fontSize = 14;
    const char* displayText = (text[0] != '\0') ? text : placeholder;
    Color textColor = (text[0] != '\0') ? UI_TEXT_COLOR : GRAY;
    
    DrawTextCustom(displayText, 
             (int)(bounds.x + 5),
             (int)(bounds.y + bounds.height/2 - fontSize/2),
             fontSize, textColor);
    
    // Kurzor
    if (*active) {
        int textWidth = MeasureTextCustom(text, fontSize);
        if (((int)(GetTime() * 2)) % 2 == 0) {
            DrawRectangle((int)(bounds.x + 5 + textWidth), 
                         (int)(bounds.y + 5), 
                         2, (int)(bounds.height - 10), UI_TEXT_COLOR);
        }
        
        // Zpracuj vstup
        int key = GetCharPressed();
        while (key > 0) {
            int len = (int)strlen(text);
            if (len < maxLen - 1 && key >= 32 && key <= 126) {
                text[len] = (char)key;
                text[len + 1] = '\0';
            }
            key = GetCharPressed();
        }
        
        // Backspace
        if (IsKeyPressed(KEY_BACKSPACE) || IsKeyDown(KEY_BACKSPACE)) {
            static float backspaceTimer = 0.0f;
            if (IsKeyPressed(KEY_BACKSPACE)) {
                int len = (int)strlen(text);
                if (len > 0) {
                    text[len - 1] = '\0';
                }
                backspaceTimer = 0.3f;  // Initial delay
            } else if (IsKeyDown(KEY_BACKSPACE)) {
                backspaceTimer -= GetFrameTime();
                if (backspaceTimer <= 0) {
                    int len = (int)strlen(text);
                    if (len > 0) {
                        text[len - 1] = '\0';
                    }
                    backspaceTimer = 0.05f;  // Repeat rate
                }
            }
        }
        
        // Enter
        if (IsKeyPressed(KEY_ENTER)) {
            *active = false;
            return true;
        }
    }
    
    return false;
}

// === Kreslení UI ===

void ui_draw_toolbar(UIState* ui, CanvasState* cs) {
    DrawRectangleRec(ui->toolbarBounds, UI_PANEL_COLOR);
    
    float y = 10;
    float btnSize = TOOLBAR_WIDTH - 10;
    
    // Nástroje
    const char* tools[] = {"SEL", "ADD", "TRN", "DEL"};
    CanvasTool toolValues[] = {TOOL_SELECT, TOOL_ADD_STATE, TOOL_TRANSITION, TOOL_DELETE};
    
    for (int i = 0; i < 4; i++) {
        Rectangle btn = {5, y, btnSize, btnSize};
        if (ui_toggle_button(btn, tools[i], cs->currentTool == toolValues[i])) {
            cs->currentTool = toolValues[i];
        }
        y += btnSize + 5;
    }
    
    // Oddělovač
    y += 10;
    DrawLine(5, (int)y, (int)(TOOLBAR_WIDTH - 5), (int)y, DARKGRAY);
    y += 10;
    
    // Nápověda pro aktuální nástroj
    const char* helpTexts[] = {
        "Klik:\nVyber\n\nDrag:\nPresun\n\nR-klik:\nAccept",
        "Klik:\nPridat\nstav",
        "Drag:\nPrechod\nz->do",
        "Klik:\nSmazat"
    };
    
    DrawTextCustom(helpTexts[cs->currentTool], 5, (int)y, 10, DARKGRAY);
}

void ui_draw_right_panel(UIState* ui, Automaton* a) {
    DrawRectangleRec(ui->rightPanelBounds, UI_PANEL_COLOR);
    
    float x = ui->rightPanelBounds.x + 10;
    float y = 10;
    float width = RIGHT_PANEL_WIDTH - 20;
    
    // Nadpis
    DrawTextCustom("NASTAVENI", (int)x, (int)y, 16, UI_TEXT_COLOR);
    y += 25;
    
    // Abeceda
    DrawTextCustom("Abeceda:", (int)x, (int)y, 12, UI_TEXT_COLOR);
    y += 15;
    
    Rectangle alphabetRect = {x, y, width, INPUT_HEIGHT};
    if (ui_text_input(alphabetRect, ui->alphabetInput, 32, &ui->alphabetInputActive, "01")) {
        automaton_set_alphabet(a, ui->alphabetInput);
    }
    y += INPUT_HEIGHT + 15;
    
    // Statistiky
    DrawTextCustom("AUTOMAT", (int)x, (int)y, 16, UI_TEXT_COLOR);
    y += 25;
    
    char stats[128];
    snprintf(stats, sizeof(stats), "Stavy: %d", a->stateCount);
    DrawTextCustom(stats, (int)x, (int)y, 12, UI_TEXT_COLOR);
    y += 18;
    
    snprintf(stats, sizeof(stats), "Prechody: %d", a->transitionCount);
    DrawTextCustom(stats, (int)x, (int)y, 12, UI_TEXT_COLOR);
    y += 18;
    
    // Počet přijímajících
    int acceptCount = 0;
    for (int i = 0; i < a->stateCount; i++) {
        if (a->states[i].isAccepting) acceptCount++;
    }
    snprintf(stats, sizeof(stats), "Prijimajici: %d", acceptCount);
    DrawTextCustom(stats, (int)x, (int)y, 12, UI_TEXT_COLOR);
    y += 30;
    
    // Testování řetězce
    DrawTextCustom("TEST RETEZCE", (int)x, (int)y, 16, UI_TEXT_COLOR);
    y += 25;
    
    Rectangle testRect = {x, y, width - 60, INPUT_HEIGHT};
    ui_text_input(testRect, ui->testInput, 256, &ui->testInputActive, "Zadej retezec...");
    
    Rectangle testBtn = {x + width - 55, y, 55, INPUT_HEIGHT};
    if (ui_button(testBtn, "Test", UI_BUTTON_COLOR)) {
        ui_test_string(ui, a);
    }
    y += INPUT_HEIGHT + 5;
    
    // Výsledek testu
    if (ui->testResultShown) {
        bool accepted = strstr(ui->testResult, "PRIJAT") != NULL;
        DrawTextCustom(ui->testResult, (int)x, (int)y, 12, 
                accepted ? UI_SUCCESS_COLOR : UI_ERROR_COLOR);
    }
    y += 30;
    
    // Algoritmy
    DrawTextCustom("ALGORITMY", (int)x, (int)y, 16, UI_TEXT_COLOR);
    y += 25;
    
    Rectangle minBtn = {x, y, width, BUTTON_HEIGHT};
    if (ui_button(minBtn, "Minimalizovat", UI_BUTTON_COLOR)) {
        Automaton* min = algo_minimize(a);
        if (min != NULL) {
            // Nahraď automat minimalizovaným
            memcpy(a->states, min->states, sizeof(a->states));
            memcpy(a->transitions, min->transitions, sizeof(a->transitions));
            a->stateCount = min->stateCount;
            a->transitionCount = min->transitionCount;
            a->initialState = min->initialState;
            automaton_destroy(min);
        }
    }
    y += BUTTON_HEIGHT + 5;
    
    Rectangle compBtn = {x, y, width, BUTTON_HEIGHT};
    if (ui_button(compBtn, "Doplnek", UI_BUTTON_COLOR)) {
        Automaton* comp = algo_complement(a);
        if (comp != NULL) {
            memcpy(a->states, comp->states, sizeof(a->states));
            memcpy(a->transitions, comp->transitions, sizeof(a->transitions));
            a->stateCount = comp->stateCount;
            a->transitionCount = comp->transitionCount;
            a->initialState = comp->initialState;
            automaton_destroy(comp);
        }
    }
    y += BUTTON_HEIGHT + 5;
    
    Rectangle tableBtn = {x, y, width, BUTTON_HEIGHT};
    if (ui_button(tableBtn, "Zobrazit tabulku", UI_BUTTON_COLOR)) {
        ui->mode = (ui->mode == APP_MODE_TABLE) ? APP_MODE_EDIT : APP_MODE_TABLE;
        ui_update_table(ui, a);
    }
    y += BUTTON_HEIGHT + 5;
    
    Rectangle resetBtn = {x, y, width, BUTTON_HEIGHT};
    if (ui_button(resetBtn, "Reset", (Color){180, 80, 80, 255})) {
        automaton_reset(a);
    }
}

void ui_draw_bottom_panel(UIState* ui, Automaton* a) {
    DrawRectangleRec(ui->bottomPanelBounds, UI_PANEL_COLOR);
    
    float x = ui->bottomPanelBounds.x + 10;
    float y = ui->bottomPanelBounds.y + 10;
    
    // Validace
    ui->validation = automaton_validate(a);
    
    if (ui->validation.isValid) {
        DrawTextCustom("Automat je validni DFA", (int)x, (int)y, 16, UI_SUCCESS_COLOR);
    } else {
        DrawTextCustom("CHYBY:", (int)x, (int)y, 16, UI_ERROR_COLOR);
        y += 22;
        
        for (int i = 0; i < ui->validation.messageCount && i < 3; i++) {
            DrawTextCustom(ui->validation.messages[i], (int)x, (int)y, 13, UI_ERROR_COLOR);
            y += 18;
        }
        
        if (ui->validation.messageCount > 3) {
            char more[64];
            snprintf(more, sizeof(more), "... a %d dalsich", 
                    ui->validation.messageCount - 3);
            DrawTextCustom(more, (int)x, (int)y, 13, UI_ERROR_COLOR);
        }
    }
    
    // Klavesove zkratky
    float rightX = ui->bottomPanelBounds.x + ui->bottomPanelBounds.width - 220;
    y = ui->bottomPanelBounds.y + 10;
    DrawTextCustom("Klavesove zkratky:", (int)rightX, (int)y, 14, DARKGRAY);
    y += 22;
    DrawTextCustom("I - Pocatecni stav", (int)rightX, (int)y, 13, DARKGRAY);
    y += 18;
    DrawTextCustom("A - Prijimaci stav", (int)rightX, (int)y, 13, DARKGRAY);
    y += 18;
    DrawTextCustom("Del - Smazat stav", (int)rightX, (int)y, 13, DARKGRAY);
    y += 18;
    DrawTextCustom("Kolecko - Zoom", (int)rightX, (int)y, 13, DARKGRAY);
}

void ui_draw_symbol_dialog(UIState* ui) {
    if (!ui->symbolDialogActive) return;
    
    // Ztmavení pozadí
    DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), (Color){0, 0, 0, 128});
    
    // Dialog
    float dialogWidth = 300;
    float dialogHeight = 150;
    float dx = (GetScreenWidth() - dialogWidth) / 2;
    float dy = (GetScreenHeight() - dialogHeight) / 2;
    
    Rectangle dialogRect = {dx, dy, dialogWidth, dialogHeight};
    DrawRectangleRec(dialogRect, WHITE);
    DrawRectangleLinesEx(dialogRect, 2, DARKGRAY);
    
    // Nadpis
    DrawTextCustom("Zadej symbol prechodu", (int)(dx + 20), (int)(dy + 20), 16, UI_TEXT_COLOR);
    
    // Vstup
    Rectangle inputRect = {dx + 20, dy + 50, dialogWidth - 40, INPUT_HEIGHT};
    static bool inputActive = true;
    ui_text_input(inputRect, ui->symbolInput, 8, &inputActive, "0");
    
    // Tlačítka
    Rectangle okBtn = {dx + 20, dy + 100, 120, 35};
    Rectangle cancelBtn = {dx + 160, dy + 100, 120, 35};
    
    if (ui_button(okBtn, "OK", UI_BUTTON_COLOR) || IsKeyPressed(KEY_ENTER)) {
        ui_close_symbol_dialog(ui);
    }
    
    if (ui_button(cancelBtn, "Zrusit", (Color){150, 150, 150, 255}) || IsKeyPressed(KEY_ESCAPE)) {
        ui->symbolInput[0] = '\0';
        ui_close_symbol_dialog(ui);
    }
}

void ui_draw_table(UIState* ui, Automaton* a) {
    if (ui->mode != APP_MODE_TABLE) return;
    
    // Překrývající okno
    float margin = 50;
    Rectangle tableRect = {
        ui->canvasBounds.x + margin,
        ui->canvasBounds.y + margin,
        ui->canvasBounds.width - 2 * margin,
        ui->canvasBounds.height - 2 * margin
    };
    
    DrawRectangleRec(tableRect, WHITE);
    DrawRectangleLinesEx(tableRect, 2, DARKGRAY);
    
    // Nadpis
    DrawTextCustom("Tabulka automatu", (int)(tableRect.x + 20), (int)(tableRect.y + 20), 18, UI_TEXT_COLOR);
    
    // Tabulka
    algo_export_table(a, ui->tableOutput, sizeof(ui->tableOutput));
    
    // Zobraz text
    float textX = tableRect.x + 20;
    float textY = tableRect.y + 50;
    
    char* line = strtok(ui->tableOutput, "\n");
    while (line != NULL && textY < tableRect.y + tableRect.height - 30) {
        DrawTextCustom(line, (int)textX, (int)textY, 12, UI_TEXT_COLOR);
        textY += 16;
        line = strtok(NULL, "\n");
    }
    
    // Zavřít tlačítko
    Rectangle closeBtn = {tableRect.x + tableRect.width - 100, tableRect.y + 10, 80, 30};
    if (ui_button(closeBtn, "Zavrit", UI_BUTTON_COLOR)) {
        ui->mode = APP_MODE_EDIT;
    }
}

void ui_draw(UIState* ui, Automaton* a, CanvasState* cs) {
    ui_draw_toolbar(ui, cs);
    ui_draw_right_panel(ui, a);
    ui_draw_bottom_panel(ui, a);
    ui_draw_symbol_dialog(ui);
    ui_draw_table(ui, a);
}

// === Vstup ===

bool ui_handle_input(UIState* ui, Automaton* a, CanvasState* cs) {
    (void)cs;
    
    // Dialog je aktivní - blokuje ostatní vstup
    if (ui->symbolDialogActive) {
        return true;
    }
    
    // Kontrola, zda je myš nad UI panely
    Vector2 mousePos = GetMousePosition();
    if (CheckCollisionPointRec(mousePos, ui->toolbarBounds) ||
        CheckCollisionPointRec(mousePos, ui->rightPanelBounds) ||
        CheckCollisionPointRec(mousePos, ui->bottomPanelBounds)) {
        return true;  // UI zpracovalo vstup
    }
    
    // Aplikuj změnu abecedy
    if (!ui->alphabetInputActive && ui->alphabetInput[0] != '\0') {
        automaton_set_alphabet(a, ui->alphabetInput);
    }
    
    return false;
}

// === Akce ===

void ui_test_string(UIState* ui, Automaton* a) {
    bool result = algo_test_string(a, ui->testInput);
    
    if (result) {
        snprintf(ui->testResult, sizeof(ui->testResult), 
                "PRIJAT: \"%s\"", ui->testInput);
    } else {
        snprintf(ui->testResult, sizeof(ui->testResult), 
                "ODMITNUT: \"%s\"", ui->testInput);
    }
    
    ui->testResultShown = true;
}

void ui_update_table(UIState* ui, Automaton* a) {
    algo_export_table(a, ui->tableOutput, sizeof(ui->tableOutput));
}

void ui_open_symbol_dialog(UIState* ui, int fromState, int toState) {
    ui->symbolDialogActive = true;
    ui->pendingFromState = fromState;
    ui->pendingToState = toState;
    ui->symbolInput[0] = '\0';
}

void ui_close_symbol_dialog(UIState* ui) {
    ui->symbolDialogActive = false;
}
