#include <SDL3/SDL.h>
#include <SDL3/SDL_error.h>
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_main.h>
#include <stdio.h>
#include <stdbool.h>
#include <math.h>           // sinf, fmodf — used for menu color oscillation
#include <SDL3_ttf/SDL_ttf.h>

// Game Data
// ## implement a window fetch function and use that instead? TODO ##
// Scale values at 75% of window real estate, 15% margin on left, 5% off the top and 2.5% on right and bottom

// different game state for state machine
enum {
	GAME_MENU = 0,
	GAME_CLASSIC = 1,
	GAME_CUSTOM = 2,

};

enum {
    LAYOUT_W  = 640,
    LAYOUT_H  = 360,
    GRID_SIZE = 3,
    CELL_SIZE = 80,
};

typedef enum {

    CELL_EMPTY,
    CELL_X,
    CELL_O,

} Cell;

typedef enum {

    PLAYER_X,
    PLAYER_O,

} Player;

typedef struct {

    Cell cells[GRID_SIZE][GRID_SIZE];   // sized by GRID_SIZE — single source of truth
    Player current_turn;
    bool game_over;
    Player winner;
    bool is_draw;

} Board;

// UI button: rect + cached label texture. Built once via button_init, reused every frame.
typedef struct {
    SDL_FRect rect;             // position + size in logical (640x360) coords
    bool hovered;               // mouse over right now?
    SDL_Texture *label_tex;     // pre-rendered text texture
    float label_w, label_h;     // cached label dims for centering
} Button;

// start of initialized data

static const Uint8 colors[][3] = {
    {152, 155, 187}, //lovely purple
    {225, 235, 237}, // azureish white
    {40, 77, 91}, // dark slate gray
    {119, 158, 169}, // Weldon Blue
    {96, 167, 189}, // Crystal Blue
    {80, 42, 80}, // Japanese Violet
};


// end of initialized data

// board initialization in memory
static void board_init(Board *b) {
    for (int r = 0; r < GRID_SIZE; r++) {
        for (int c = 0; c < GRID_SIZE; c++) {
            b->cells[r][c] = CELL_EMPTY; // CELL_EMPTY is enum value 0; zeroes every cell.
        }
    }
    b->current_turn = PLAYER_X; // starting player, alternate on rematch? ## ADD TO CHECKLIST TODO ##
    b->game_over = false; // default state
    b->winner = PLAYER_X; // default state
    b->is_draw = false;  // not a draw at the start
}
// BIG FUNCTION AHEAD

// win check func for game state
static void board_check_end(Board *b) {
    // rows & columns --> r & c
    // check if any row r is all matched, then return PLAYER_X or PLAYER_O to b->winner, which will be 1 or 2 after compile.
    for (int r = 0; r < GRID_SIZE; r++) {
        if (b->cells[r][0] != CELL_EMPTY &&
            b->cells[r][0] == b->cells[r][1] &&
            b->cells[r][1] == b->cells[r][2]) {
                b->game_over = true;
                b->winner = (b->cells[r][0] == CELL_X) ? PLAYER_X : PLAYER_O; // I LOVE TERNARY OPERATORS!!
                return;
            }
    }
    for (int c = 0; c < GRID_SIZE; c++) {
        if (b->cells[0][c] != CELL_EMPTY &&
            b->cells[0][c] == b->cells[1][c] &&
            b->cells[1][c] == b->cells[2][c]) {
                b->game_over = true;
                b->winner = (b->cells[0][c] == CELL_X) ? PLAYER_X : PLAYER_O; // // I LOVE TERNARY OPERATORS!!
                return;
            }
    }
    // top-left to bottom-right
    if (b->cells[0][0] != CELL_EMPTY &&
        b->cells[0][0] == b->cells[1][1] &&
        b->cells[1][1] == b->cells[2][2]) {
        b->game_over = true;
        b->winner = (b->cells[0][0] == CELL_X) ? PLAYER_X : PLAYER_O;
        return;
    }
    // top-right to bottom-left
    if (b->cells[0][2] != CELL_EMPTY &&
        b->cells[0][2] == b->cells[1][1] &&
        b->cells[1][1] == b->cells[2][0]) {
        b->game_over = true;
        b->winner = (b->cells[0][2] == CELL_X) ? PLAYER_X : PLAYER_O;
        return;
    }

    // Check for draw
    bool all_full = true;
    for (int r = 0; r < GRID_SIZE; r++) {
        for (int c = 0; c < GRID_SIZE; c++) {
            if (b->cells[r][c] == CELL_EMPTY) {
                all_full = false;
            }
        }
    }

    if (all_full) {
        b->game_over = true;
        b->is_draw = true;
    }
}

// BIG FUNCTION END

// check if array value = 0 to see if value can be placed
static bool board_try_place(Board *b, int row, int col) {

    // safety check set
    if (b->game_over) return false; // if game_over true --> cant place
    if (row < 0 || row >= GRID_SIZE || col < 0 || col >= GRID_SIZE) return false; // accessing out of bounds cant place
    if (b->cells[row][col] != CELL_EMPTY) return false; // if row not 0 return false.

    b->cells[row][col] = (b->current_turn == PLAYER_X) ? CELL_X : CELL_O;
    board_check_end(b);

    // switch turn if game not over
    if (!b->game_over) {
        b->current_turn = (b->current_turn == PLAYER_X) ? PLAYER_O : PLAYER_X;
    }
    return true;
}

static void board_render(const Board *b, SDL_Renderer *renderer) {

    int grid_pixel_size = CELL_SIZE * GRID_SIZE;
    int origin_x = (LAYOUT_W - grid_pixel_size) / 2;
    int origin_y = (LAYOUT_H - grid_pixel_size) / 2;

    for (int r = 0; r < GRID_SIZE; r++) {
        for (int c = 0; c < GRID_SIZE; c++) {

            SDL_FRect cell = {
                .x = (float)(origin_x + c * CELL_SIZE),
                .y = (float)(origin_y + r * CELL_SIZE),
                .w = (float)CELL_SIZE,
                .h = (float)CELL_SIZE
            };

            SDL_SetRenderDrawColor(renderer, colors[1][0], colors[1][1], colors[1][2], 255); // azurewhite
            SDL_RenderRect(renderer, &cell);

            if (b->cells[r][c] == CELL_X || b->cells[r][c] == CELL_O) {
                int idx = (b->cells[r][c] == CELL_X) ? 5 : 4; // X = Jap Vio / O = Crystal Blue
                SDL_SetRenderDrawColor(renderer, colors[idx][0], colors[idx][1], colors[idx][2], 255); //telling it to access array idx, columns 0 1 and 2
                SDL_FRect piece = {
                .x = (float)(origin_x + c * CELL_SIZE + 10),
                .y = (float)(origin_y + r * CELL_SIZE + 10),
                .w = (float)(CELL_SIZE - 20),
                .h = (float)(CELL_SIZE - 20)
                };
                SDL_RenderFillRect(renderer, &piece); // render our color filled box
            }
        }
    }
}

// ============================================================
// UI / Menu helpers
// ============================================================

// Pure geometry — is (px, py) inside this rect?
static bool point_in_rect(float px, float py, const SDL_FRect *r) {
    return px >= r->x && px <  r->x + r->w
        && py >= r->y && py <  r->y + r->h;
}

// Render text once at button creation. We cache the texture so we don't
// repeat the (slow) text-to-texture work every frame.
static void button_init(Button *b, SDL_Renderer *renderer, TTF_Font *font,
                        const char *label, float x, float y, float w, float h) {
    b->rect    = (SDL_FRect){ x, y, w, h };
    b->hovered = false;

    SDL_Color text_color = { 225, 235, 237, 255 };   // azureish white
    SDL_Surface *surf = TTF_RenderText_Blended(font, label, 0, text_color);
    if (!surf) {
        fprintf(stderr, "TTF_RenderText_Blended failed: %s\n", SDL_GetError());
        b->label_tex = NULL;
        b->label_w = b->label_h = 0.0f;
        return;
    }
    b->label_tex = SDL_CreateTextureFromSurface(renderer, surf);
    b->label_w   = (float)surf->w;
    b->label_h   = (float)surf->h;
    SDL_DestroySurface(surf);   // texture is on GPU now; CPU surface no longer needed
}

static void button_destroy(Button *b) {
    if (b->label_tex) {
        SDL_DestroyTexture(b->label_tex);
        b->label_tex = NULL;
    }
}

// Update hover state based on the current mouse position (in logical coords).
static void button_update_hover(Button *b, float mx, float my) {
    b->hovered = point_in_rect(mx, my, &b->rect);
}

// Returns true if THIS event is a left-click landing inside this button.
// Event coords must already be converted to logical via SDL_ConvertEventToRenderCoordinates.
static bool button_check_click(const Button *b, const SDL_Event *e) {
    if (e->type != SDL_EVENT_MOUSE_BUTTON_DOWN)  return false;
    if (e->button.button != SDL_BUTTON_LEFT)     return false;
    return point_in_rect(e->button.x, e->button.y, &b->rect);
}

static void button_render(const Button *b, SDL_Renderer *renderer) {
    // Body — brighter purple on hover for visual feedback.
    if (b->hovered) SDL_SetRenderDrawColor(renderer, 152, 155, 187, 255); // lovely purple
    else            SDL_SetRenderDrawColor(renderer,  80,  42,  80, 255); // Japanese violet
    SDL_RenderFillRect(renderer, &b->rect);

    // Border
    SDL_SetRenderDrawColor(renderer, 225, 235, 237, 255);                 // azureish white
    SDL_RenderRect(renderer, &b->rect);

    // Centered label
    if (b->label_tex) {
        SDL_FRect dst = {
            .x = b->rect.x + (b->rect.w - b->label_w) / 2.0f,
            .y = b->rect.y + (b->rect.h - b->label_h) / 2.0f,
            .w = b->label_w,
            .h = b->label_h,
        };
        SDL_RenderTexture(renderer, b->label_tex, NULL, &dst);
    }
}

// Compute an oscillating background color by lerping smoothly through the
// `colors` palette. Cycles through all entries; one full loop ~= 15 seconds.
// Output: writes RGB into the provided pointers (caller supplies storage).
static void menu_oscillate_color(Uint8 *out_r, Uint8 *out_g, Uint8 *out_b) {
    int num_colors = (int)(sizeof(colors) / sizeof(colors[0]));

    // SDL_GetTicks returns ms since SDL_Init. 0.0004 = roughly 2.5 sec per palette step.
    float t = (float)SDL_GetTicks() * 0.0004f;

    // Wrap t into [0, num_colors) so we can index the palette without overflow.
    float idx_f = fmodf(t, (float)num_colors);
    int   idx   = (int)idx_f;
    int   next  = (idx + 1) % num_colors;
    float phase = idx_f - (float)idx;     // 0.0 .. 1.0 lerp factor

    // Smoothstep makes the transition ease in/out instead of being linear —
    // gives the color shift a "breathing" feel rather than a constant slide.
    float smooth = phase * phase * (3.0f - 2.0f * phase);

    *out_r = (Uint8)((float)colors[idx][0] * (1.0f - smooth) + (float)colors[next][0] * smooth);
    *out_g = (Uint8)((float)colors[idx][1] * (1.0f - smooth) + (float)colors[next][1] * smooth);
    *out_b = (Uint8)((float)colors[idx][2] * (1.0f - smooth) + (float)colors[next][2] * smooth);
}

// ============================================================
// Main
// ============================================================

int main(int argc, char *argv[]) {

    (void)argc;
    (void)argv;

    // initialize SDL video subsystem, returns bool in SDL3
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        fprintf(stderr, "SDL_Init failed :%s\n", SDL_GetError()); // output to a file stderr
        return 1;
    }

    if (!TTF_Init()) {
    	fprintf(stderr, "TTF_Init failed :%s\n", SDL_GetError());
     	return 1;
    }

    // Smaller font for buttons / inline text
    TTF_Font *font = TTF_OpenFont("C:/Windows/Fonts/arialbd.ttf", 20.0f);
    if (!font) {
        fprintf(stderr, "TTF_OpenFont (button font) failed: %s\n", SDL_GetError());
        TTF_Quit();
        SDL_Quit();
        return 1;
    }

    // Bigger font for the menu title
    TTF_Font *title_font = TTF_OpenFont("C:/Windows/Fonts/arialbd.ttf", 36.0f);
    if (!title_font) {
        fprintf(stderr, "TTF_OpenFont (title font) failed: %s\n", SDL_GetError());
        TTF_CloseFont(font);
        TTF_Quit();
        SDL_Quit();
        return 1;
    }

    //init window and check if failed
    SDL_Window *window = SDL_CreateWindow("C Toys: Tic Tac Toe", 1920, 1080, 0);
    if (!window) {
        fprintf(stderr, "SDL_CreateWindow failed: %s\n", SDL_GetError());
        SDL_Quit(); // cleans up loose ends before erroring out
        return 1;
    }

    //init renderer and check if failed
    SDL_Renderer *renderer = SDL_CreateRenderer(window, NULL); // creates our renderer DO NOT DELETE
    if (!renderer) {
        fprintf(stderr, "SDL_CreateRenderer Failed: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }
    // revisit this function, we can probably get auto scaling done here.
    if (SDL_SetRenderLogicalPresentation(renderer, 640, 360, SDL_LOGICAL_PRESENTATION_INTEGER_SCALE) != true) {
        fprintf(stderr, "SDL_SetRenderLogicalPresentation failed: %s\n", SDL_GetError());
        // non fatal
    }

// ############################## GAME RENDER LOOP
    // board initialization
    Board board;
    board_init(&board);
    bool message_flag = false; // false = end message not spat out yet, true = spat out.
    bool running = true;       // window state
    int screen = GAME_MENU;    // start on the main menu

    // Grid layout (logical coords) — also used by the click-to-cell math in case GAME_CLASSIC.
    int grid_pixel_size = CELL_SIZE * GRID_SIZE;
    int origin_x = (LAYOUT_W - grid_pixel_size) / 2;
    int origin_y = (LAYOUT_H - grid_pixel_size) / 2;

    // ---------- Menu UI setup ----------
    // Pre-render the title once into a texture (text-to-texture is slow; do it ONCE).
    SDL_Color title_color = { 225, 235, 237, 255 };   // azureish white
    SDL_Surface *title_surf = TTF_RenderText_Blended(title_font,
                                                     "C Toys: Tic Tac Toe",
                                                     0, title_color);
    SDL_Texture *title_tex = NULL;
    float title_w = 0.0f, title_h = 0.0f;
    if (title_surf) {
        title_tex = SDL_CreateTextureFromSurface(renderer, title_surf);
        title_w   = (float)title_surf->w;
        title_h   = (float)title_surf->h;
        SDL_DestroySurface(title_surf);
    } else {
        fprintf(stderr, "Title text render failed: %s\n", SDL_GetError());
    }

    // Menu buttons — 150 wide, 40 tall, horizontally centered, stacked vertically.
    const float btn_w = 150.0f;
    const float btn_h = 40.0f;
    const float btn_x = (LAYOUT_W - btn_w) / 2.0f;

    Button play_btn, quit_btn;
    button_init(&play_btn, renderer, font, "Play", btn_x, 180.0f, btn_w, btn_h);
    button_init(&quit_btn, renderer, font, "Quit", btn_x, 240.0f, btn_w, btn_h);

    while (running) {

        SDL_Event event;

        switch(screen){

        case GAME_MENU: {
            // ---------- Event handling ----------
            while (SDL_PollEvent(&event)) {
                if (event.type == SDL_EVENT_QUIT) {
                    running = false;
                } else if (event.type == SDL_EVENT_KEY_DOWN
                           && event.key.key == SDLK_ESCAPE) {
                    running = false;
                } else if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN) {
                    // Convert window coords -> logical (640x360) so they line up with buttons.
                    SDL_ConvertEventToRenderCoordinates(renderer, &event);
                    if (button_check_click(&play_btn, &event)) {
                        screen = GAME_CLASSIC;       // jump into the game
                        board_init(&board);          // fresh board every time we enter
                        message_flag = false;
                    }
                    if (button_check_click(&quit_btn, &event)) running = false;
                }
            }

            // ---------- Hover update ----------
            // Get raw window mouse coords, then convert to logical for hit-testing.
            float win_mx, win_my;
            SDL_GetMouseState(&win_mx, &win_my);
            float log_mx, log_my;
            SDL_RenderCoordinatesFromWindow(renderer, win_mx, win_my, &log_mx, &log_my);
            button_update_hover(&play_btn, log_mx, log_my);
            button_update_hover(&quit_btn, log_mx, log_my);

            // ---------- Render ----------
            // 1. Oscillating background — smooth lerp through the palette.
            Uint8 br, bg, bb;
            menu_oscillate_color(&br, &bg, &bb);
            SDL_SetRenderDrawColor(renderer, br, bg, bb, 255);
            SDL_RenderClear(renderer);

            // 2. Title near the top, horizontally centered.
            if (title_tex) {
                SDL_FRect title_dst = {
                    .x = (LAYOUT_W - title_w) / 2.0f,
                    .y = 60.0f,
                    .w = title_w,
                    .h = title_h,
                };
                SDL_RenderTexture(renderer, title_tex, NULL, &title_dst);
            }

            // 3. Buttons on top.
            button_render(&play_btn, renderer);
            button_render(&quit_btn, renderer);

            SDL_RenderPresent(renderer);
            break;
        }

        case GAME_CLASSIC:
	        while (SDL_PollEvent(&event)) {
	            if (event.type == SDL_EVENT_QUIT) {
	                running = false;
	            } else if (event.type == SDL_EVENT_KEY_DOWN) {
	                if (event.key.key == SDLK_ESCAPE) {
	                    running = false;
	                } else if (event.key.key == SDLK_R && board.game_over == true) {
	                    board_init(&board);
	                    message_flag = false;
	                }
	            } else if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN
	                       && event.button.button == SDL_BUTTON_LEFT) {
	                // Convert window coords -> logical coords (in-place on event).
	                SDL_ConvertEventToRenderCoordinates(renderer, &event);

	                // Compute pixel offsets from the grid origin.
	                int mx = (int)event.button.x - origin_x;
	                int my = (int)event.button.y - origin_y;

	                // Bounds-check in pixel space BEFORE integer division.
	                int grid_px = GRID_SIZE * CELL_SIZE;
	                if (mx >= 0 && my >= 0 && mx < grid_px && my < grid_px) {
	                    int col = mx / CELL_SIZE;
	                    int row = my / CELL_SIZE;
	                    board_try_place(&board, row, col);
	                }
	            }

	    }
	        SDL_SetRenderDrawColor(renderer, colors[3][0], colors[3][1], colors[3][2], 255);
	        SDL_RenderClear(renderer);
	        board_render(&board, renderer); // rendering the grid to the screen
	        SDL_RenderPresent(renderer);


	        if (board.game_over && message_flag == 0) {
	            message_flag = true;
	            if(board.is_draw) {
	                printf("It's a draw!\n");
	                printf("Would you like to play again? Press R to restart, ESC to quit!\n");
	                fflush(stdout);
	                //running = false; // end game loop on draw
	            } else {
	                printf("Player %s wins!\n", (board.winner == PLAYER_X) ? "X" : "O");
	                printf("Would you like to play again? Press R to restart, ESC to quit!\n");
	                fflush(stdout);
	                //running = false; // end game loop on win
	            }
	        } // base game loop moved into here
			break;

        case GAME_CUSTOM:
        	// our new game mode is accessed in here.
         	break;
        }
    }

    // ---------- Cleanup (reverse order of creation) ----------
    button_destroy(&play_btn);
    button_destroy(&quit_btn);
    if (title_tex) SDL_DestroyTexture(title_tex);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_CloseFont(title_font);
    TTF_CloseFont(font);
    TTF_Quit();
    SDL_Quit();
    return 0;
}
