// ============================================================================ //
//                                 MASMORRA                                     //
// ============================================================================ //

#include <stdio.h>
#include <stdlib.h>
#include <locale.h>
#include <time.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>

// ================= [ MACROS & CROSS-PLATFORM ] ============================== //

#define PAIXAO 106
#define STR_LENGTH 30 
#define HIGHSCORES_NUM 11
#define XAOPAI 15
#define MAX_POTIONS 3

// Cross-platform screen clear and terminal animation macros
#ifdef _WIN32
    #include <windows.h>
    #include <conio.h>
    #include <io.h>
    #define CLEAR_CMD "cls"
    #ifndef ENABLE_VIRTUAL_TERMINAL_PROCESSING
        #define ENABLE_VIRTUAL_TERMINAL_PROCESSING 0x0004
    #endif
#else
    #include <unistd.h>
    #include <sys/select.h>
    #define CLEAR_CMD "clear" // Linux/macOS uses native clear
#endif

// Standard 16-color ANSI SGR escape sequences for universal terminal support
#define ANSI_RESET      "\033[0m"
#define ANSI_BOLD       "\033[1m"
#define ANSI_DIM        "\033[2m"
#define ANSI_GRAY       "\033[0;90m"

#define ANSI_RED        "\033[31m"
#define ANSI_GREEN      "\033[32m"
#define ANSI_YELLOW     "\033[33m"
#define ANSI_BLUE       "\033[34m"
#define ANSI_MAGENTA    "\033[35m"
#define ANSI_CYAN       "\033[36m"
#define ANSI_WHITE      "\033[37m"

#define ANSI_BRED       "\033[1;31m"
#define ANSI_BGREEN     "\033[1;32m"
#define ANSI_BYELLOW    "\033[1;33m"
#define ANSI_BBLUE      "\033[1;34m"
#define ANSI_BMAGENTA   "\033[1;35m"
#define ANSI_BCYAN      "\033[1;36m"
#define ANSI_BWHITE     "\033[1;37m"
#define ANSI_ORANGE     "\033[38;5;208m"

static bool g_useColor = false;
static bool g_use256Color = false;
#define CLR(code)       (g_useColor ? (code) : "")
#define CLR_ORANGE()    (g_useColor ? (g_use256Color ? ANSI_ORANGE : ANSI_YELLOW) : "")

static void detect256Color(bool winVtEnabled) {
    const char *colorTerm = getenv("COLORTERM");
    const char *term = getenv("TERM");
    if (winVtEnabled ||
        getenv("WT_SESSION") != NULL ||
        (colorTerm != NULL && colorTerm[0] != '\0') ||
        (term != NULL && strstr(term, "256color") != NULL)) {
        g_use256Color = true;
    }
}

// Detects terminal ANSI support at runtime with safe monochrome fallback
void initTerminalColors(int argc, char *argv[]) {
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--no-color") == 0) {
            g_useColor = false;
            g_use256Color = false;
            return;
        }
        if (strcmp(argv[i], "--color") == 0) {
            bool winVt = false;
            #ifdef _WIN32
                HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
                if (hOut != INVALID_HANDLE_VALUE && hOut != NULL) {
                    DWORD dwMode = 0;
                    if (GetConsoleMode(hOut, &dwMode)) {
                        winVt = SetConsoleMode(hOut, dwMode | ENABLE_VIRTUAL_TERMINAL_PROCESSING) != 0;
                    }
                }
            #endif
            g_useColor = true;
            detect256Color(winVt);
            return;
        }
    }

    const char *noColor = getenv("NO_COLOR");
    if (noColor != NULL && noColor[0] != '\0') {
        g_useColor = false;
        g_use256Color = false;
        return;
    }

    const char *forceColor = getenv("FORCE_COLOR");
    if (forceColor != NULL && forceColor[0] != '\0' && strcmp(forceColor, "0") != 0) {
        bool winVt = false;
        #ifdef _WIN32
            HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
            if (hOut != INVALID_HANDLE_VALUE && hOut != NULL) {
                DWORD dwMode = 0;
                if (GetConsoleMode(hOut, &dwMode)) {
                    winVt = SetConsoleMode(hOut, dwMode | ENABLE_VIRTUAL_TERMINAL_PROCESSING) != 0;
                }
            }
        #endif
        g_useColor = true;
        detect256Color(winVt);
        return;
    }

    #ifdef _WIN32
        if (!_isatty(_fileno(stdout))) {
            g_useColor = false;
            return;
        }
        HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
        if (hOut != INVALID_HANDLE_VALUE && hOut != NULL) {
            DWORD dwMode = 0;
            if (GetConsoleMode(hOut, &dwMode)) {
                if (SetConsoleMode(hOut, dwMode | ENABLE_VIRTUAL_TERMINAL_PROCESSING)) {
                    g_useColor = true;
                    detect256Color(true);
                    return;
                }
            }
        }
        const char *wtSession = getenv("WT_SESSION");
        const char *ansicon = getenv("ANSICON");
        const char *conEmu = getenv("ConEmuANSI");
        const char *term = getenv("TERM");
        if (wtSession != NULL || ansicon != NULL ||
            (conEmu != NULL && strcmp(conEmu, "ON") == 0) ||
            (term != NULL && term[0] != '\0' && strcmp(term, "dumb") != 0)) {
            g_useColor = true;
            detect256Color(false);
            return;
        }
        g_useColor = false;
    #else
        if (!isatty(STDOUT_FILENO)) {
            g_useColor = false;
            return;
        }
        const char *term = getenv("TERM");
        if (term == NULL || term[0] == '\0' || strcmp(term, "dumb") == 0) {
            g_useColor = false;
            return;
        }
        g_useColor = true;
        detect256Color(false);
    #endif
}

void clearScreen() {
    system(CLEAR_CMD);
}

// ================= [ DATA STRUCTURES ] ====================================== //

typedef struct {
    char name[STR_LENGTH];
    int coins;
    int hp;
    int maxHp;
    int atk;
    int def;
    int potions;
    char atkItem[STR_LENGTH];
    char defItem[STR_LENGTH];
} character_t;

typedef struct {
    char name[80];
    int hp;
    int maxHp;
    int atk;
    int def;
    int coins;
} enemy_t;

typedef struct {
    int item1Price;
    int item2Price;
    int item1atkPoints;
    int item2defPoints;
    char item1Name[STR_LENGTH];
    char item2Name[STR_LENGTH];
} shop_t;

typedef struct {
    char name[STR_LENGTH];
    int score;
} highscore_entry_t;

// Encapsulated Global Game State
typedef struct {
    character_t hero;
    enemy_t monsters[16];
    shop_t shops[6];
    int score;
    int currentFloor;
    bool isDead;
    int secretNumber;
    char pronoum;
    bool eliteSpawnedThisFloor;
    bool fledXaopai;
    bool mimicSpawnedThisFloor;
} GameState;

// ================= [ ENGINE & INPUTS ] ====================================== //

static char inputLine[256];
static char *inputPtr = NULL;

bool hasPendingInput() {
    if (inputPtr == NULL) return false;
    char *p = inputPtr;
    while (*p && isspace((unsigned char)*p)) p++;
    return (*p != '\0');
}

// Robust input reader with multi-integer buffer support per line
int readInt() {
    int value;
    while (true) {
        // If buffer is empty or end-of-line reached, read a new line
        while (inputPtr == NULL || *inputPtr == '\0') {
            printf("< ");
            fflush(stdout);
            if (fgets(inputLine, sizeof(inputLine), stdin) == NULL) {
                if (feof(stdin)) exit(0);
                printf("> %sEntrada inválida.%s\n", CLR(ANSI_BRED), CLR(ANSI_RESET));
                continue;
            }
            inputPtr = inputLine;
        }

        // Skip leading whitespace and blank newlines
        while (*inputPtr && isspace((unsigned char)*inputPtr)) {
            inputPtr++;
        }

        // If line contains only whitespace, fetch next line
        if (*inputPtr == '\0') {
            continue;
        }

        // Try parsing an integer from current position
        int charsRead = 0;
        if (sscanf(inputPtr, "%d%n", &value, &charsRead) == 1) {
            inputPtr += charsRead;
            return value;
        }

        // Token is not an integer: advance to next whitespace to avoid infinite loops
        while (*inputPtr && !isspace((unsigned char)*inputPtr)) {
            inputPtr++;
        }
        printf("> %sEntrada inválida.%s\n", CLR(ANSI_BRED), CLR(ANSI_RESET));
    }
}

// Safe string reader supporting spaces
void readString(char* buffer, int maxLength) {
    inputPtr = NULL; // Discard any pending integer tokens before reading string
    while (true) {
        printf("< ");
        fflush(stdout);
        if (fgets(buffer, maxLength, stdin) != NULL) {
            buffer[strcspn(buffer, "\n")] = 0; // Strip trailing newline
            if (strlen(buffer) > 0) return;
        } else if (feof(stdin)) {
            exit(0);
        }
        printf("> %sEntrada inválida.%s\n", CLR(ANSI_BRED), CLR(ANSI_RESET));
    }
}

// ================= [ INITIALIZATION (DATA-DRIVEN) ] ========================= //

void initGameData(GameState *gs) {
    // Pronoun
    gs->pronoum = 'o';
    gs->eliteSpawnedThisFloor = false;
    gs->fledXaopai = false;
    gs->mimicSpawnedThisFloor = false;

    // Initialize Hero
    gs->hero.coins = 150;
    gs->hero.hp = 100;
    gs->hero.maxHp = 100;
    gs->hero.atk = 10;
    gs->hero.def = 0;
    gs->hero.potions = 0;
    strcpy(gs->hero.atkItem, "Seus punhos");
    strcpy(gs->hero.defItem, "Nada");

    strcpy(gs->monsters[0].name, "Ratazana gigante");
    gs->monsters[0].maxHp = rand() % 11 + 30;
    gs->monsters[0].atk = rand() % 21 + 10;
    gs->monsters[0].def = 0;
    gs->monsters[0].coins = rand() % 41 + 50;

    strcpy(gs->monsters[1].name, "Cobra");
    gs->monsters[1].maxHp = 30;
    gs->monsters[1].atk = rand() % 11 + 20;
    gs->monsters[1].def = 0;
    gs->monsters[1].coins = rand() % 41 + 50;

    strcpy(gs->monsters[2].name, "Gosma gigante");
    gs->monsters[2].maxHp = rand() % 11 + 30;
    gs->monsters[2].atk = rand() % 11 + 20;
    gs->monsters[2].def = 0;
    gs->monsters[2].coins = rand() % 41 + 50;

    strcpy(gs->monsters[3].name, "Ladrão louco");
    gs->monsters[3].maxHp = rand() % 11 + 30;
    gs->monsters[3].atk = rand() % 21 + 20;
    gs->monsters[3].def = rand() % 11 + 0;
    gs->monsters[3].coins = rand() % 41 + 60;

    strcpy(gs->monsters[4].name, "Esqueleto");
    gs->monsters[4].maxHp = 30;
    gs->monsters[4].atk = rand() % 11 + 30;
    gs->monsters[4].def = rand() % 11 + 0;
    gs->monsters[4].coins = rand() % 41 + 70;

    strcpy(gs->monsters[5].name, "Guarda da prisão");
    gs->monsters[5].maxHp = rand() % 11 + 30;
    gs->monsters[5].atk = rand() % 11 + 30;
    gs->monsters[5].def = rand() % 11 + 0;
    gs->monsters[5].coins = rand() % 41 + 70;

    strcpy(gs->monsters[6].name, "Morcego vampiro");
    gs->monsters[6].maxHp = rand() % 11 + 40;
    gs->monsters[6].atk = rand() % 11 + 20;
    gs->monsters[6].def = rand() % 11 + 10;
    gs->monsters[6].coins = rand() % 41 + 100;

    strcpy(gs->monsters[7].name, "Aranha gigante");
    gs->monsters[7].maxHp = rand() % 11 + 50;
    gs->monsters[7].atk = rand() % 11 + 30;
    gs->monsters[7].def = rand() % 11 + 10;
    gs->monsters[7].coins = rand() % 41 + 100;

    strcpy(gs->monsters[8].name, "Troll selvagem");
    gs->monsters[8].maxHp = rand() % 21 + 50;
    gs->monsters[8].atk = 40;
    gs->monsters[8].def = 20;
    gs->monsters[8].coins = rand() % 41 + 100;

    strcpy(gs->monsters[9].name, "Soldado anão");
    gs->monsters[9].maxHp = rand() % 11 + 60;
    gs->monsters[9].atk = rand() % 21 + 40;
    gs->monsters[9].def = rand() % 11 + 10;
    gs->monsters[9].coins = rand() % 51 + 130;

    strcpy(gs->monsters[10].name, "Monge anão");
    gs->monsters[10].maxHp = rand() % 11 + 40;
    gs->monsters[10].atk = rand() % 11 + 50;
    gs->monsters[10].def = 30;
    gs->monsters[10].coins = rand() % 61 + 130;

    strcpy(gs->monsters[11].name, "Bruxo anão");
    gs->monsters[11].maxHp = rand() % 11 + 30;
    gs->monsters[11].atk = rand() % 21 + 50;
    gs->monsters[11].def = 30;
    gs->monsters[11].coins = rand() % 61 + 140;

    strcpy(gs->monsters[12].name, "Demônio estripador");
    gs->monsters[12].maxHp = rand() % 11 + 70;
    gs->monsters[12].atk = rand() % 11 + 60;
    gs->monsters[12].def = rand() % 21 + 20;
    gs->monsters[12].coins = rand() % 91 + 200;

    strcpy(gs->monsters[13].name, "Succubus");
    gs->monsters[13].maxHp = rand() % 21 + 70;
    gs->monsters[13].atk = 60;
    gs->monsters[13].def = rand() % 21 + 20;
    gs->monsters[13].coins = rand() % 91 + 200;

    strcpy(gs->monsters[14].name, "Cão demoníaco");
    gs->monsters[14].maxHp = rand() % 11 + 80;
    gs->monsters[14].atk = rand() % 11 + 60;
    gs->monsters[14].def = rand() % 11 + 30;
    gs->monsters[14].coins = rand() % 91 + 200;

    strcpy(gs->monsters[XAOPAI].name, "XÃOPAI");
    gs->monsters[XAOPAI].maxHp = 720;
    gs->monsters[XAOPAI].atk = 82;
    gs->monsters[XAOPAI].def = 25;

    // Initialize Shops
    strcpy(gs->shops[0].item1Name, "Bastão de madeira");
    gs->shops[0].item1atkPoints = 20;
    gs->shops[0].item1Price = 100;
    strcpy(gs->shops[0].item2Name, "Escudo pequeno de madeira");
    gs->shops[0].item2defPoints = 15;
    gs->shops[0].item2Price = 100;

    strcpy(gs->shops[1].item1Name, "Espada curta de cobre");
    gs->shops[1].item1atkPoints = 30;
    gs->shops[1].item1Price = 150;
    strcpy(gs->shops[1].item2Name, "Escudo redondo de cobre");
    gs->shops[1].item2defPoints = 20;
    gs->shops[1].item2Price = 150;

    strcpy(gs->shops[2].item1Name, "Espada de ferro");
    gs->shops[2].item1atkPoints = 40;
    gs->shops[2].item1Price = 200;
    strcpy(gs->shops[2].item2Name, "Escudo de ferro");
    gs->shops[2].item2defPoints = 30;
    gs->shops[2].item2Price = 200;

    strcpy(gs->shops[3].item1Name, "Espada longa de aço");
    gs->shops[3].item1atkPoints = 50;
    gs->shops[3].item1Price = 300;
    strcpy(gs->shops[3].item2Name, "Escudo grande de aço");
    gs->shops[3].item2defPoints = 40;
    gs->shops[3].item2Price = 300;

    strcpy(gs->shops[4].item1Name, "Espada encantada de diamante");
    gs->shops[4].item1atkPoints = 60;
    gs->shops[4].item1Price = 400;
    strcpy(gs->shops[4].item2Name, "Escudo largo de diamante");
    gs->shops[4].item2defPoints = 50;
    gs->shops[4].item2Price = 400;

    strcpy(gs->shops[5].item1Name, "Excalibur Lendária");
    gs->shops[5].item1atkPoints = 85;
    gs->shops[5].item1Price = 500;
    strcpy(gs->shops[5].item2Name, "Égide Mítica");
    gs->shops[5].item2defPoints = 65;
    gs->shops[5].item2Price = 500;
}

// ================= [ UI & ASCII ART ] ======================================= //

/*
 * ACKNOWLEDGMENTS - ASCII ART
 * Some of the ASCII art rendered below was adapted from original works by various artists.
 * Untouched files with artist signatures can be found in /assets.
 * - Xãopai: Adaptation of work by eVil
 * - Amulet: Adaptation of work by [...]
 */

void titleScreen() {
    const char *flames[4][3] = {
        {
            "  (  . ",
            " (  )  ",
            "( ()  )"
        },
        {
            "   , ) ",
            " ( ) ) ",
            "(  )( )"
        },
        {
            "  .  ) ",
            "  (  ) ",
            "(  () )"
        },
        {
            "  ( ,  ",
            " ( ( ) ",
            "( )(  )"
        }
    };

    clearScreen();

    #ifdef _WIN32
        HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    #else
        bool isTty = (isatty(STDIN_FILENO) != 0);
    #endif

    int frame = 0;
    bool running = true;

    while (running) {
        #ifdef _WIN32
            COORD homeCoords = {0, 0};
            SetConsoleCursorPosition(hOut, homeCoords);
        #else
            if (isTty) {
                printf("\033[H");
            }
        #endif

        const char *topFlame = (frame % 2 == 0) ? CLR(ANSI_BYELLOW) : CLR(ANSI_BRED);
        const char *midFlame = (frame % 2 == 0) ? CLR(ANSI_BRED)    : CLR(ANSI_BYELLOW);
        const char *botFlame = CLR(ANSI_RED);

        printf("%s********************************************************************************%s\n", CLR_ORANGE(), CLR(ANSI_RESET));
        printf("  %s%s%s                         %sBEM-VINDO À%s                          %s%s%s  \n", topFlame, flames[frame][0], CLR(ANSI_RESET), CLR(ANSI_BWHITE), CLR(ANSI_RESET), topFlame, flames[frame][0], CLR(ANSI_RESET));
        printf("  %s%s%s    %s__  __    _    ____  __  __  ___  ____  ____      _%s       %s%s%s  \n", midFlame, flames[frame][1], CLR(ANSI_RESET), CLR(ANSI_RED), CLR(ANSI_RESET), midFlame, flames[frame][1], CLR(ANSI_RESET));
        printf("  %s%s%s   %s|  \\/  |  / \\  / ___||  \\/  |/ _ \\|  _ \\|  _ \\    / \\%s      %s%s%s  \n", botFlame, flames[frame][2], CLR(ANSI_RESET), CLR(ANSI_RED), CLR(ANSI_RESET), botFlame, flames[frame][2], CLR(ANSI_RESET));
        printf("  %s\\=====/%s   %s| |\\/| | / _ \\ \\___ \\| |\\/| | | | | |_) | |_) |  / _ \\%s     %s\\=====/%s  \n", CLR(ANSI_YELLOW), CLR(ANSI_RESET), CLR(ANSI_RED), CLR(ANSI_RESET), CLR(ANSI_YELLOW), CLR(ANSI_RESET));
        printf("   %s\\   /%s    %s| |  | |/ ___ \\ ___) | |  | | |_| |  _ <|  _ <  / ___ \\%s     %s\\   /%s   \n", CLR(ANSI_YELLOW), CLR(ANSI_RESET), CLR(ANSI_RED), CLR(ANSI_RESET), CLR(ANSI_YELLOW), CLR(ANSI_RESET));
        printf("    %s| |%s     %s|_|  |_/_/   \\_\\____/|_|  |_|\\___/|_| \\_\\_| \\_\\/_/   \\_\\%s     %s| |%s    \n", CLR(ANSI_YELLOW), CLR(ANSI_RESET), CLR(ANSI_RED), CLR(ANSI_RESET), CLR(ANSI_YELLOW), CLR(ANSI_RESET));
        printf("    %s|_|%s                           by: Paixão                             %s|_|%s    \n", CLR(ANSI_YELLOW), CLR(ANSI_RESET), CLR(ANSI_YELLOW), CLR(ANSI_RESET));
        printf("%s********************************************************************************%s\n\n", CLR_ORANGE(), CLR(ANSI_RESET));
        printf("                      %sPressione enter para continuar...%s                         \n", CLR(ANSI_DIM), CLR(ANSI_RESET));
        fflush(stdout);

        #ifdef _WIN32
            if (!_isatty(_fileno(stdin))) {
                int c;
                while ((c = getchar()) != '\n' && c != EOF);
                break;
            }
            for (int i = 0; i < 9; i++) {
                if (_kbhit()) {
                    int ch = _getch();
                    if (ch == '\r' || ch == '\n') {
                        running = false;
                        break;
                    }
                }
                Sleep(20);
            }
        #else
            if (!isTty) {
                int c;
                while ((c = getchar()) != '\n' && c != EOF);
                break;
            }
            fd_set readfds;
            FD_ZERO(&readfds);
            FD_SET(STDIN_FILENO, &readfds);
            struct timeval tv;
            tv.tv_sec = 0;
            tv.tv_usec = 180000;

            int ret = select(STDIN_FILENO + 1, &readfds, NULL, NULL, &tv);
            if (ret > 0 && FD_ISSET(STDIN_FILENO, &readfds)) {
                int c;
                while ((c = getchar()) != '\n' && c != EOF);
                running = false;
            }
        #endif

        frame = (frame + 1) % 4;
    }

    clearScreen();
}

void printHpBar(int current, int max) {
    int length = 10;
    if (current < 0) current = 0;
    int filled = (max > 0) ? (current * length) / max : 0;
    if (filled > length) filled = length;
    if (filled < 0) filled = 0;

    const char *hpColor = CLR(ANSI_BRED);
    if (current * 2 > max) {
        hpColor = CLR(ANSI_BGREEN);
    } else if (current * 4 > max) {
        hpColor = CLR(ANSI_BYELLOW);
    }

    printf("[");
    printf("%s", hpColor);
    for (int i = 0; i < filled; i++) {
        printf("█");
    }
    printf("%s%s", CLR(ANSI_RESET), CLR(ANSI_DIM));
    for (int i = filled; i < length; i++) {
        printf("░");
    }
    printf("%s] %s%i/%i%s\n", CLR(ANSI_RESET), hpColor, current, max, CLR(ANSI_RESET));
}

void showsInfo(GameState *gs, const enemy_t *enemy) {
    printf("\n%s+---------------- [ %sSTATUS%s%s ] ----------------+%s\n", CLR(ANSI_CYAN), CLR(ANSI_BCYAN), CLR(ANSI_RESET), CLR(ANSI_CYAN), CLR(ANSI_RESET));
    printf("%s|%s - %s%s%s\n", CLR(ANSI_CYAN), CLR(ANSI_RESET), CLR(ANSI_BWHITE), gs->hero.name, CLR(ANSI_RESET));
    printf("%s|%s   Saúde:  ", CLR(ANSI_CYAN), CLR(ANSI_RESET));
    printHpBar(gs->hero.hp, gs->hero.maxHp);
    printf("%s|%s   Poções: %i\n", CLR(ANSI_CYAN), CLR(ANSI_RESET), gs->hero.potions);
    printf("%s|%s   Moedas: %s%i%s\n", CLR(ANSI_CYAN), CLR(ANSI_RESET), CLR(ANSI_BYELLOW), gs->hero.coins, CLR(ANSI_RESET));
    printf("%s|%s   Ataque: %i (%s)\n", CLR(ANSI_CYAN), CLR(ANSI_RESET), gs->hero.atk, gs->hero.atkItem);
    printf("%s|%s   Defesa: %i (%s)\n", CLR(ANSI_CYAN), CLR(ANSI_RESET), gs->hero.def, gs->hero.defItem);

    if (enemy != NULL) {
        const char *enemyColor = (strcmp(enemy->name, "XÃOPAI") == 0) ? CLR(ANSI_BMAGENTA) : CLR(ANSI_BRED);
        printf("%s+--------------------------------------------+%s\n", CLR(ANSI_CYAN), CLR(ANSI_RESET));
        printf("%s|%s - %s%s%s\n", CLR(ANSI_CYAN), CLR(ANSI_RESET), enemyColor, enemy->name, CLR(ANSI_RESET));
        printf("%s|%s   Saúde:  ", CLR(ANSI_CYAN), CLR(ANSI_RESET));
        printHpBar(enemy->hp, enemy->maxHp);
        printf("%s|%s   Ataque: %i\n", CLR(ANSI_CYAN), CLR(ANSI_RESET), enemy->atk);
        printf("%s|%s   Defesa: %i\n", CLR(ANSI_CYAN), CLR(ANSI_RESET), enemy->def);
    }
    printf("%s+--------------------------------------------+%s\n\n", CLR(ANSI_CYAN), CLR(ANSI_RESET));
}

void printsXaopai() {
    char eye;
    switch (rand() % 3) {
        case 0:
            eye = '@';
            break;
        case 1:
            eye = '#';
            break;
        case 2:
            eye = '0';
            break;
    }
    printf("%s", CLR(ANSI_BMAGENTA));
    printf("                            .-.\n");
    printf("       ___,---.__          / | \\          __,---,___\n");
    printf("    ,-'    \\\\    `-.____,-'  |  `-.____,-'    //    `-.\n");
    printf("  ,'        ||          ~'\\     /`~          ||        `.\n");
    printf(" /      ___//              `. .'              \\\\___      \\\n");
    printf("|    ,-'   `-.__             |             __,-'   `-.    |\n");
    printf("\\   |          \\ \\`-.___ \\   |   / ___,-'/ /          |   /\n");
    printf(" \\  \\           | `._  %s%c%s`\\\\  |  //'%s%c%s  _,' |           /  /\n", CLR(ANSI_BRED), eye, CLR(ANSI_BMAGENTA), CLR(ANSI_BRED), eye, CLR(ANSI_BMAGENTA));
    printf("  `-.\\         /'  _ `---'' . . ``---' _  `\\         /,-'\n");
    printf("     ``       /     \\    .='/ \\`=.    /     \\       ''\n");
    printf("             |__   /|\\_.--.--.--.--._/|\\   __|\n");
    printf("             /  `./  \\\\'\\ |  |  | /'//  \\.'  \\\n");
    printf("            /   /     ||\\-+--|--+-/||     \\   \\\n");
    printf("           |   |     /'\\_\\_\\ | /_/_/'\\     |   |\n");
    printf("            \\   \\__, \\_     '~'     _/ .__/   /\n");
    printf("             `-._,-'   `-._______.-'   `-._,-'%s\n\n", CLR(ANSI_RESET));
}

void printsAmulet() {
    printf("%s", CLR(ANSI_BRED));
    printf("       .....           .....\n");
    printf("   ,ad8PPPP88b,     ,d88PPPP8ba,\n");
    printf("  d8P'      'Y8b, ,d8P'      'Y8b\n");
    printf(" dP'           '8a8'           `Yd\n");
    printf(" 8(              '              )8\n");
    printf(" I8                             8I\n");
    printf("  Yb,           o_o           ,dP\n");
    printf("   '8a,  %s~me deixa entrar~%s  ,a8'\n", CLR(ANSI_GRAY), CLR(ANSI_BRED));
    printf("     '8a,                 ,a8'\n");
    printf("       'Yba             adP'\n");
    printf("         `Y8a         a8P'\n");
    printf("           `88,     ,88'\n");
    printf("             '8b   d8'\n");
    printf("              '8b d8'\n");
    printf("               `888'\n");
    printf("                 '%s\n", CLR(ANSI_RESET));
}

// ================= [ COMBAT SYSTEM ] ======================================== //

// Calculates attribute stat with ±15% variance (85% to 115%)
int rollStat(int baseStat) {
    if (baseStat <= 0) return 0;
    int percent = 85 + (rand() % 31);
    int rolled = (baseStat * percent + 50) / 100;
    return (rolled < 1) ? 1 : rolled;
}

bool monsterInteraction(GameState *gs, int monsterCode) {
    bool isElite = false;
    // Elite Monsters: only appear after floor 1 and at most 1 per floor
    if (gs->currentFloor > 1 && !gs->eliteSpawnedThisFloor && (rand() % 5 == 0)) {
        isElite = true;
        gs->eliteSpawnedThisFloor = true;
        char originalName[40];
        strncpy(originalName, gs->monsters[monsterCode].name, sizeof(originalName) - 1);
        originalName[sizeof(originalName) - 1] = '\0';

        int eliteType = rand() % 3;
        switch (eliteType) {
            case 0: // Fierce (+40% ATK)
                snprintf(gs->monsters[monsterCode].name, sizeof(gs->monsters[monsterCode].name), "★ %s Feroz ★", originalName);
                gs->monsters[monsterCode].atk = (gs->monsters[monsterCode].atk * 14 + 5) / 10;
                break;
            case 1: // Armored (+50% DEF, +20% HP)
                snprintf(gs->monsters[monsterCode].name, sizeof(gs->monsters[monsterCode].name), "★ %s Couraçado ★", originalName);
                gs->monsters[monsterCode].def = (gs->monsters[monsterCode].def * 15 + 5) / 10;
                if (gs->monsters[monsterCode].def == 0) {
                    gs->monsters[monsterCode].def = 5;
                }
                gs->monsters[monsterCode].maxHp = (gs->monsters[monsterCode].maxHp * 12 + 5) / 10;
                break;
            case 2: // Colossal (+80% HP, +10% ATK)
                snprintf(gs->monsters[monsterCode].name, sizeof(gs->monsters[monsterCode].name), "★ %s Colossal ★", originalName);
                gs->monsters[monsterCode].maxHp = (gs->monsters[monsterCode].maxHp * 18 + 5) / 10;
                gs->monsters[monsterCode].atk = (gs->monsters[monsterCode].atk * 11 + 5) / 10;
                break;
        }
    }

    gs->monsters[monsterCode].hp = gs->monsters[monsterCode].maxHp;

    if (isElite) {
        printf("> Você encontrou um monstro %sELITE%s: %s%s%s!\n", 
               CLR(ANSI_BMAGENTA), CLR(ANSI_RESET), CLR(ANSI_BRED), gs->monsters[monsterCode].name, CLR(ANSI_RESET));
        printf("> %sUma poderosa aura intimidadora emana da criatura...%s\n\n", CLR(ANSI_MAGENTA), CLR(ANSI_RESET));
    } else {
        printf("> Você encontrou %s%s%s!\n\n", CLR(ANSI_BRED), gs->monsters[monsterCode].name, CLR(ANSI_RESET));
    }
    
    do {
        printf("%s[1]%s Atacar\n", CLR(ANSI_BCYAN), CLR(ANSI_RESET));
        printf("%s[2]%s Fugir\n", CLR(ANSI_BCYAN), CLR(ANSI_RESET));
        if (gs->hero.potions > 0) {
            printf("%s[3]%s Usar poção de cura %s(x%i)%s\n", CLR(ANSI_BCYAN), CLR(ANSI_RESET), CLR(ANSI_DIM), gs->hero.potions, CLR(ANSI_RESET));
        }
        int action = readInt();

        if (action == 0) {
            showsInfo(gs, &gs->monsters[monsterCode]);
        } else if (action == 1) {
            // Hero damage logic
            int heroMaxAtk = (gs->hero.atk * 115 + 50) / 100;
            int monsterMinDef = (gs->monsters[monsterCode].def * 85 + 50) / 100;
            if (gs->monsters[monsterCode].def > 0 && heroMaxAtk <= monsterMinDef) {
                clearScreen();
                printf("%sVOCÊ MORREU DE PARADA CARDÍACA AO VER %s NO PISO %i%s\n\a", 
                        CLR(ANSI_BRED), gs->monsters[monsterCode].name, gs->currentFloor, CLR(ANSI_RESET));
                printf("Pontuação: %s%i%s\n", CLR(ANSI_BYELLOW), gs->score, CLR(ANSI_RESET));
                return true;
            }

            int rolledHeroAtk = rollStat(gs->hero.atk);
            int rolledMonsterDef = rollStat(gs->monsters[monsterCode].def);
            int heroDamage = rolledHeroAtk - rolledMonsterDef;

            if (heroDamage <= 0) {
                printf("\n> Você atacou com %s, mas %s %sbloqueou o golpe%s!\n", 
                       gs->hero.atkItem, gs->monsters[monsterCode].name, CLR(ANSI_BRED), CLR(ANSI_RESET));
            } else {
                bool isCrit = (rand() % 10 == 0);
                if (isCrit) {
                    heroDamage = (heroDamage * 3 + 1) / 2;
                    printf("\n> %sGOLPE CRÍTICO%s! Você atacou com %s causando %s%i de dano%s!\n", 
                           CLR(ANSI_BGREEN), CLR(ANSI_RESET), gs->hero.atkItem, CLR(ANSI_BGREEN), heroDamage, CLR(ANSI_RESET));
                } else {
                    printf("\n> Você atacou com %s causando %s%i de dano%s!\n", 
                           gs->hero.atkItem, CLR(ANSI_BGREEN), heroDamage, CLR(ANSI_RESET));
                }
                gs->monsters[monsterCode].hp -= heroDamage;
            }

            // Monster counter-attack
            if (gs->monsters[monsterCode].hp > 0) {
                int rolledMonsterAtk = rollStat(gs->monsters[monsterCode].atk);
                int rolledHeroDef = rollStat(gs->hero.def);
                int monsterDamage = rolledMonsterAtk - rolledHeroDef;

                if (monsterDamage <= 0) {
                    printf("> %s te atacou, mas %sseu escudo bloqueou o golpe%s!\n\n", 
                           gs->monsters[monsterCode].name, CLR(ANSI_BGREEN), CLR(ANSI_RESET));
                } else {
                    gs->hero.hp -= monsterDamage;
                    printf("> %s te atacou causando %s%i de dano%s!\n\n", 
                           gs->monsters[monsterCode].name, CLR(ANSI_BRED), monsterDamage, CLR(ANSI_RESET));
                }
                
                if (gs->hero.hp <= 0) {
                    clearScreen();
                    printf("%sVOCÊ MORREU PARA %s NO PISO %i%s\n\a", CLR(ANSI_BRED), gs->monsters[monsterCode].name, gs->currentFloor, CLR(ANSI_RESET));
                    printf("Pontuação: %s%i%s\n", CLR(ANSI_BYELLOW), gs->score, CLR(ANSI_RESET));
                    return true;
                }
            }
        } else if (action == 2) {
            // Flee: take a hit from behind and forfeit coins
            printf("\n> Você deu meia-volta e tentou fugir!\n");
            int rolledMonsterAtk = rollStat(gs->monsters[monsterCode].atk);
            int rolledHeroDef = rollStat(gs->hero.def);
            int monsterDamage = rolledMonsterAtk - rolledHeroDef;

            if (monsterDamage <= 0) {
                printf("> %s tentou te atacar pelas costas, mas %sseu escudo bloqueou o golpe%s!\n", 
                       gs->monsters[monsterCode].name, CLR(ANSI_BGREEN), CLR(ANSI_RESET));
            } else {
                gs->hero.hp -= monsterDamage;
                printf("> %s te atingiu pelas costas causando %s%i de dano%s!\n", 
                       gs->monsters[monsterCode].name, CLR(ANSI_BRED), monsterDamage, CLR(ANSI_RESET));
            }
            
            if (gs->hero.hp <= 0) {
                clearScreen();
                printf("%sVOCÊ MORREU PARA %s TENTANDO FUGIR NO PISO %i%s\n\a", 
                       CLR(ANSI_BRED), gs->monsters[monsterCode].name, gs->currentFloor, CLR(ANSI_RESET));
                printf("Pontuação: %s%i%s\n", CLR(ANSI_BYELLOW), gs->score, CLR(ANSI_RESET));
                return true;
            }

            printf("> Você conseguiu escapar com vida pelos corredores!\n\n");
            return false;
        } else if (action == 3) {
            if (gs->hero.potions <= 0) {
                printf("\n> %sVocê não tem poções de cura guardadas!%s\n\n", CLR(ANSI_BRED), CLR(ANSI_RESET));
            } else if (gs->hero.hp >= gs->hero.maxHp) {
                printf("\n> %sSua saúde já está no máximo!%s\n\n", CLR(ANSI_BYELLOW), CLR(ANSI_RESET));
            } else {
                gs->hero.potions--;
                int heal = gs->hero.maxHp / 2;
                if (gs->hero.hp + heal > gs->hero.maxHp) {
                    heal = gs->hero.maxHp - gs->hero.hp;
                }
                gs->hero.hp += heal;
                printf("\n> Você bebeu uma poção de cura e recuperou %s%i de vida%s! %s(Restam: %i)%s\n\n", 
                       CLR(ANSI_BGREEN), heal, CLR(ANSI_RESET), CLR(ANSI_DIM), gs->hero.potions, CLR(ANSI_RESET));
            }
        } else {
            printf("> %sComando não encontrado.%s\n\n", CLR(ANSI_BRED), CLR(ANSI_RESET));
        }
    } while (gs->monsters[monsterCode].hp > 0);

    if (isElite) {
        int eliteCoins = (gs->monsters[monsterCode].coins * 15 + 5) / 10;
        gs->score += 250;
        gs->hero.coins += eliteCoins;
        printf("> Você derrotou %s%s%s!\n> Você encontrou %s%i moedas%s!\n", 
               CLR(ANSI_BRED), gs->monsters[monsterCode].name, CLR(ANSI_RESET), CLR(ANSI_BYELLOW), eliteCoins, CLR(ANSI_RESET));
        if (rand() % 10 < 4) {
            if (gs->hero.potions < MAX_POTIONS) {
                gs->hero.potions++;
                printf("> O monstro elite derrubou uma %spoção de cura%s! %s(Você tem %i)%s\n\n", 
                       CLR(ANSI_BGREEN), CLR(ANSI_RESET), CLR(ANSI_DIM), gs->hero.potions, CLR(ANSI_RESET));
            } else if (gs->hero.hp < gs->hero.maxHp) {
                int heal = gs->hero.maxHp / 2;
                if (gs->hero.hp + heal > gs->hero.maxHp) heal = gs->hero.maxHp - gs->hero.hp;
                gs->hero.hp += heal;
                printf("> O monstro elite derrubou uma %spoção de cura%s! Como sua bolsa está cheia, você a bebeu na hora e recuperou %s%i de vida%s!\n\n",
                       CLR(ANSI_BGREEN), CLR(ANSI_RESET), CLR(ANSI_BGREEN), heal, CLR(ANSI_RESET));
            } else {
                printf("> O monstro elite derrubou uma poção de cura, mas sua bolsa já está cheia %s(x%i)(max)%s!\n\n",
                       CLR(ANSI_DIM), gs->hero.potions, CLR(ANSI_RESET));
            }
        } else {
            printf("\n");
        }
    } else {
        gs->score += 100;
        gs->hero.coins += gs->monsters[monsterCode].coins;
        printf("> Você derrotou %s%s%s!\n> Você encontrou %s%i moedas%s!\n\n", 
               CLR(ANSI_BRED), gs->monsters[monsterCode].name, CLR(ANSI_RESET), CLR(ANSI_BYELLOW), gs->monsters[monsterCode].coins, CLR(ANSI_RESET));
    }
    
    return false;
}

// ================= [ EVENTS & SHOPS ] ====================================== //

void printShopBox(GameState *gs, int shopCode, bool item1Available, bool item2Available, bool pendantAvailable) {
    printf("%s+----------------- [ %sLOJA%s%s ] -----------------+%s\n", CLR(ANSI_YELLOW), CLR(ANSI_BYELLOW), CLR(ANSI_RESET), CLR(ANSI_YELLOW), CLR(ANSI_RESET));
    printf("%s|%s   Moedas: %s%i%s\n", CLR(ANSI_YELLOW), CLR(ANSI_RESET), CLR(ANSI_BYELLOW), gs->hero.coins, CLR(ANSI_RESET));
    printf("%s+--------------------------------------------+%s\n", CLR(ANSI_YELLOW), CLR(ANSI_RESET));
    printf("%s|%s   %s[1]%s Sair da loja\n", CLR(ANSI_YELLOW), CLR(ANSI_RESET), CLR(ANSI_BCYAN), CLR(ANSI_RESET));
    if (item1Available) {
        printf("%s|%s   %s[2]%s %s - %s$%i%s\n", CLR(ANSI_YELLOW), CLR(ANSI_RESET), CLR(ANSI_BCYAN), CLR(ANSI_RESET), gs->shops[shopCode].item1Name, CLR(ANSI_BYELLOW), gs->shops[shopCode].item1Price, CLR(ANSI_RESET));
    } else if (shopCode == 5 && strcmp(gs->hero.atkItem, gs->shops[shopCode].item1Name) != 0) {
        printf("%s|%s   %s[2]%s %s%s - (Recolhido)%s\n", CLR(ANSI_YELLOW), CLR(ANSI_RESET), CLR(ANSI_BCYAN), CLR(ANSI_RESET), CLR(ANSI_DIM), gs->shops[shopCode].item1Name, CLR(ANSI_RESET));
    } else {
        printf("%s|%s   %s[2]%s %s%s - (Comprado)%s\n", CLR(ANSI_YELLOW), CLR(ANSI_RESET), CLR(ANSI_BCYAN), CLR(ANSI_RESET), CLR(ANSI_DIM), gs->shops[shopCode].item1Name, CLR(ANSI_RESET));
    }
    if (item2Available) {
        printf("%s|%s   %s[3]%s %s - %s$%i%s\n", CLR(ANSI_YELLOW), CLR(ANSI_RESET), CLR(ANSI_BCYAN), CLR(ANSI_RESET), gs->shops[shopCode].item2Name, CLR(ANSI_BYELLOW), gs->shops[shopCode].item2Price, CLR(ANSI_RESET));
    } else if (shopCode == 5 && strcmp(gs->hero.defItem, gs->shops[shopCode].item2Name) != 0) {
        printf("%s|%s   %s[3]%s %s%s - (Recolhido)%s\n", CLR(ANSI_YELLOW), CLR(ANSI_RESET), CLR(ANSI_BCYAN), CLR(ANSI_RESET), CLR(ANSI_DIM), gs->shops[shopCode].item2Name, CLR(ANSI_RESET));
    } else {
        printf("%s|%s   %s[3]%s %s%s - (Comprado)%s\n", CLR(ANSI_YELLOW), CLR(ANSI_RESET), CLR(ANSI_BCYAN), CLR(ANSI_RESET), CLR(ANSI_DIM), gs->shops[shopCode].item2Name, CLR(ANSI_RESET));
    }
    if (gs->currentFloor >= 1) {
        if (gs->hero.potions == 0) {
            printf("%s|%s   %s[4]%s Poção de cura - %s$50%s\n", CLR(ANSI_YELLOW), CLR(ANSI_RESET), CLR(ANSI_BCYAN), CLR(ANSI_RESET), CLR(ANSI_BYELLOW), CLR(ANSI_RESET));
        } else if (gs->hero.potions < MAX_POTIONS) {
            printf("%s|%s   %s[4]%s Poção de cura - %s$50%s %s(x%i)%s\n", CLR(ANSI_YELLOW), CLR(ANSI_RESET), CLR(ANSI_BCYAN), CLR(ANSI_RESET), CLR(ANSI_BYELLOW), CLR(ANSI_RESET), CLR(ANSI_DIM), gs->hero.potions, CLR(ANSI_RESET));
        } else {
            printf("%s|%s   %s[4]%s %sPoção de cura - (x%i)(max)%s\n", CLR(ANSI_YELLOW), CLR(ANSI_RESET), CLR(ANSI_BCYAN), CLR(ANSI_RESET), CLR(ANSI_DIM), gs->hero.potions, CLR(ANSI_RESET));
        }
    }
    if (gs->currentFloor >= 2) {
        if (pendantAvailable) {
            printf("%s|%s   %s[5]%s Pingente de vida - %s$100%s\n", CLR(ANSI_YELLOW), CLR(ANSI_RESET), CLR(ANSI_BCYAN), CLR(ANSI_RESET), CLR(ANSI_BYELLOW), CLR(ANSI_RESET));
        } else {
            printf("%s|%s   %s[5]%s %sPingente de vida - (Comprado)%s\n", CLR(ANSI_YELLOW), CLR(ANSI_RESET), CLR(ANSI_BCYAN), CLR(ANSI_RESET), CLR(ANSI_DIM), CLR(ANSI_RESET));
        }
    }
    if (gs->hero.potions > 0)
        printf("%s|%s   %s[6]%s Beber poção de cura %s(x%i)%s\n", CLR(ANSI_YELLOW), CLR(ANSI_RESET), CLR(ANSI_BCYAN), CLR(ANSI_RESET), CLR(ANSI_DIM), gs->hero.potions, CLR(ANSI_RESET));
    printf("%s+--------------------------------------------+%s\n\n", CLR(ANSI_YELLOW), CLR(ANSI_RESET));
}

void shopInteraction(GameState *gs, int shopCode) {
    bool item1Available = true;
    bool item2Available = true;
    bool pendantAvailable = true;
    
    printShopBox(gs, shopCode, item1Available, item2Available, pendantAvailable);
    
    int action;
    do {
        action = readInt();
        switch (action) {
            case 0: // INFO
                showsInfo(gs, NULL);
                if (!hasPendingInput()) {
                    printShopBox(gs, shopCode, item1Available, item2Available, pendantAvailable);
                }
                break;
                
            case 1: // LEAVES THE SHOP
                break;
                
            case 2: // BUYS ITEM 1
                if ((item1Available == true) && (gs->hero.coins >= gs->shops[shopCode].item1Price)) {
                    strcpy(gs->hero.atkItem, gs->shops[shopCode].item1Name);
                    printf("\n> Você comprou %s%s%s.\n", CLR(ANSI_BGREEN), gs->shops[shopCode].item1Name, CLR(ANSI_RESET));
                    gs->hero.atk = gs->shops[shopCode].item1atkPoints;
                    item1Available = false;
                    gs->hero.coins -= gs->shops[shopCode].item1Price;
                    gs->score += gs->shops[shopCode].item1Price / 2;
                    if (shopCode == 5) {
                        item2Available = false;
                        printf("> A lâmina sagrada ressoa em suas mãos! A criatura recolhe a Égide Mítica de volta para as sombras.\n\n");
                    } else {
                        printf("\n");
                    }
                } else {
                    printf("\n> %sImpossível realizar a ação.%s\n\n", CLR(ANSI_BRED), CLR(ANSI_RESET));
                }
                if (!hasPendingInput()) {
                    printShopBox(gs, shopCode, item1Available, item2Available, pendantAvailable);
                }
                break;
                
            case 3: // BUYS ITEM 2
                if ((item2Available == true) && (gs->hero.coins >= gs->shops[shopCode].item2Price)) {
                    strcpy(gs->hero.defItem, gs->shops[shopCode].item2Name);
                    printf("\n> Você comprou %s%s%s.\n", CLR(ANSI_BGREEN), gs->shops[shopCode].item2Name, CLR(ANSI_RESET));
                    gs->hero.def = gs->shops[shopCode].item2defPoints;
                    item2Available = false;
                    gs->hero.coins -= gs->shops[shopCode].item2Price;
                    gs->score += gs->shops[shopCode].item2Price / 2;
                    if (shopCode == 5) {
                        item1Available = false;
                        printf("> O escudo divino reluz diante da escuridão! A criatura recolhe a Excalibur Lendária de volta para as sombras.\n\n");
                    } else {
                        printf("\n");
                    }
                } else {
                    printf("\n> %sImpossível realizar a ação.%s\n\n", CLR(ANSI_BRED), CLR(ANSI_RESET));
                }
                if (!hasPendingInput()) {
                    printShopBox(gs, shopCode, item1Available, item2Available, pendantAvailable);
                }
                break;
                
            case 4: // BUYS POTION
                if (gs->currentFloor >= 1 && gs->hero.potions >= MAX_POTIONS) {
                    printf("\n> %sVocê só consegue carregar no máximo %i poções na bolsa!%s\n\n", CLR(ANSI_BYELLOW), MAX_POTIONS, CLR(ANSI_RESET));
                } else if ((gs->hero.coins >= 50) && (gs->currentFloor >= 1)) {
                    gs->hero.potions++;
                    gs->hero.coins -= 50;
                    gs->score += 20;
                    printf("\n> Você comprou uma %spoção de cura%s. %s(Você tem %i)%s\n\n", CLR(ANSI_BGREEN), CLR(ANSI_RESET), CLR(ANSI_DIM), gs->hero.potions, CLR(ANSI_RESET));
                } else if (gs->currentFloor >= 1) {
                    printf("\n> %sImpossível realizar a ação.%s\n\n", CLR(ANSI_BRED), CLR(ANSI_RESET));
                } else {
                    printf("\n> %sComando não encontrado.%s\n\n", CLR(ANSI_BRED), CLR(ANSI_RESET));
                }
                if (!hasPendingInput()) {
                    printShopBox(gs, shopCode, item1Available, item2Available, pendantAvailable);
                }
                break;
                
            case 5: // BUYS PENDANT
                if (pendantAvailable && gs->hero.coins >= 100 && gs->currentFloor >= 2) {
                    pendantAvailable = false;
                    gs->hero.coins -= 100;
                    gs->hero.maxHp += 50;
                    gs->hero.hp += 50;
                    gs->score += 150;
                    printf("\n> Você comprou o %spingente de vida%s! Você ganhou %s+50 de vida máxima%s! Saúde: %s%i/%i%s\n\n", 
                           CLR(ANSI_BGREEN), CLR(ANSI_RESET), CLR(ANSI_BGREEN), CLR(ANSI_RESET), CLR(ANSI_BGREEN), gs->hero.hp, gs->hero.maxHp, CLR(ANSI_RESET));
                } else if (gs->currentFloor >= 2) {
                    printf("\n> %sImpossível realizar a ação.%s\n\n", CLR(ANSI_BRED), CLR(ANSI_RESET));
                } else {
                    printf("\n> %sComando não encontrado.%s\n\n", CLR(ANSI_BRED), CLR(ANSI_RESET));
                }
                if (!hasPendingInput()) {
                    printShopBox(gs, shopCode, item1Available, item2Available, pendantAvailable);
                }
                break;
                
            case 6: // DRINKS POTION IN SHOP
                if (gs->hero.potions > 0 && gs->hero.hp < gs->hero.maxHp) {
                    gs->hero.potions--;
                    int heal = gs->hero.maxHp / 2;
                    if (gs->hero.hp + heal > gs->hero.maxHp) heal = gs->hero.maxHp - gs->hero.hp;
                    gs->hero.hp += heal;
                    printf("\n> Você bebeu uma poção de cura e recuperou %s%i de vida%s! Saúde: %s%i/%i%s\n\n", 
                           CLR(ANSI_BGREEN), heal, CLR(ANSI_RESET), CLR(ANSI_BGREEN), gs->hero.hp, gs->hero.maxHp, CLR(ANSI_RESET));
                } else if (gs->hero.potions > 0 && gs->hero.hp >= gs->hero.maxHp) {
                    printf("\n> %sSua saúde já está no máximo!%s\n\n", CLR(ANSI_BYELLOW), CLR(ANSI_RESET));
                } else {
                    printf("\n> %sImpossível realizar a ação.%s\n\n", CLR(ANSI_BRED), CLR(ANSI_RESET));
                }
                if (!hasPendingInput()) {
                    printShopBox(gs, shopCode, item1Available, item2Available, pendantAvailable);
                }
                break;
                
            default:
                printf("\n> %sComando não encontrado.%s\n\n", CLR(ANSI_BRED), CLR(ANSI_RESET));
                if (!hasPendingInput()) {
                    printShopBox(gs, shopCode, item1Available, item2Available, pendantAvailable);
                }
                break;
        }
    } while (action != 1);
}

// ================= [ UTILITY FUNCTIONS ] ==================================== //

int compareHighscores(const void* a, const void* b) {
    const highscore_entry_t* h1 = (const highscore_entry_t*)a;
    const highscore_entry_t* h2 = (const highscore_entry_t*)b;
    return h2->score - h1->score;
}

// ================= [ DIALOG ] =============================== //

void shopDialog(GameState *gs) {
    switch (gs->currentFloor) {
        case 0:
            printf("%sLojista:%s Então você quer se aventurar pela terrível masmorra, %s%s%s?\n", 
                   CLR(ANSI_BYELLOW), CLR(ANSI_RESET), CLR(ANSI_BWHITE), gs->hero.name, CLR(ANSI_RESET));
            printf("Se fosse você eu não iria de mãos atadas...\n");
            break;

        case 1:
            printf("> Você encontra um vendedor ambulante, carregando itens em seu carro de mão.\n");
            printf("%sVendedor ambulante:%s O que estou fazendo aqui? Ah, é só um passeio de negócios.\n", CLR(ANSI_BYELLOW), CLR(ANSI_RESET));
            printf("Você acabou de matar potenciais compradores. Você devia cobrir meu prejuízo...\n\n");
            break;

        case 2:
            printf("> Você encontra uma pequena venda de um goblin morador local.\n");
            printf("%sLojista goblin:%s Levar o que quiser, só por favor não me matar!\n", CLR(ANSI_BYELLOW), CLR(ANSI_RESET));
            printf("%s(Seu código moral te impede de sair sem pagar)%s\n\n", CLR(ANSI_DIM), CLR(ANSI_RESET));
            break;

        case 3:
            printf("> Você encontra um troll ferreiro, forjando ferramentas com os minérios.\n");
            printf("%sFerreiro:%s Uai, um humano por essas banda? Trem raro de se vê...\n", CLR(ANSI_BYELLOW), CLR(ANSI_RESET));
            printf("Posso arranjá umas lembrancinha procê em troca dessas sua moeda de ouro.\n\n");
            break;

        case 4:
            printf("> Um misterioso ser encapuzado aparece das sombras.\n");
            printf("%sSer misterioso:%s Então és tu %c destemid%c %s%s%s, que ousas desafiar o temível %sXãopai%s?\n", 
                   CLR(ANSI_BYELLOW), CLR(ANSI_RESET), gs->pronoum, gs->pronoum, CLR(ANSI_BWHITE), gs->hero.name, CLR(ANSI_RESET), CLR(ANSI_BMAGENTA), CLR(ANSI_RESET));
            printf("Acautela-te, jovem guerreir%c, pois o percurso adiante é sombrio e traiçoeiro.\n\n", 
                   gs->pronoum);
            break;

        case 5:
            printf("> Você encontra uma hedionda figura de carne e tentáculos.\n");
            printf("> Em seus tentáculos repousam a %sExcalibur Lendária%s e a %sÉgide Mítica%s — apenas uma poderá ser escolhida.\n",
                   CLR(ANSI_BCYAN), CLR(ANSI_RESET), CLR(ANSI_BCYAN), CLR(ANSI_RESET));
            printf("%sCriatura:%s %s(A criatura encara os mais obscuros cantos da sua alma)%s\n", CLR(ANSI_BYELLOW), CLR(ANSI_RESET), CLR(ANSI_DIM), CLR(ANSI_RESET));
            printf("%s(Você se sente extremamente desconfortável enquanto negocia)%s\n\n", CLR(ANSI_DIM), CLR(ANSI_RESET));
            break;
    }
}

void dungeonDialog(GameState *gs) {
    printf("\n%s================================================================================\n", CLR_ORANGE());
    switch (gs->currentFloor) {
        case 0:
            printf("                                   [ CIDADE ]                                   \n");
            printf("================================================================================\n\n");
            printf("  Diversos aventureiros da cidade acima adentraram a masmorra antes de você,\n");
            printf("  mas poucos foram vistos novamente. Diz-se que um mal ancestral espreita nas\n");
            printf("  profundezas, o imensamente poderoso espírito %sXãopai%s%s.\n", CLR(ANSI_BMAGENTA), CLR(ANSI_RESET), CLR_ORANGE());
            break;

        case 1:
            printf("                                    [ PISO 1 ]                                  \n");
            printf("================================================================================\n\n");
            printf("  Você desce as escadarias da masmorra, entrando no sistema de esgoto da cidade.\n");
            printf("  As criaturas, antes inofensivas, se tornaram ferozes com a influência maligna\n");
            printf("  vinda das profundezas.\n");
            break;
            
        case 2:
            printf("                                    [ PISO 2 ]                                  \n");
            printf("================================================================================\n\n");
            printf("  Você desce as escadarias da masmorra, entrando em uma prisão antiga há muito\n");
            printf("  tempo abandonada. Esta prisão costumava abrigar os mais perigosos criminosos,\n");
            printf("  mas a influência corrompeu as mentes dos guardas e dos prisioneiros.\n");
            break;
            
        case 3:
            printf("                                    [ PISO 3 ]                                  \n");
            printf("================================================================================\n\n");
            printf("  Você desce as escadarias da masmorra, entrando em uma caverna escura e rica em\n");
            printf("  minério. Uma vez foi um grande centro comercial habitada por anões, mas foi\n");
            printf("  abandonada com a vinda da pestilência.\n");
            break;
            
        case 4:
            printf("                                    [ PISO 4 ]                                  \n");
            printf("================================================================================\n\n");
            printf("  Você desce as escadarias da masmorra, entrando na antiga metrópole do reino dos\n");
            printf("  anões, hoje em ruínas. Contam as histórias que a povoada capital sucumbiu\n");
            printf("  subitamente com a chegada de um enorme mal.\n");
            break;
            
        case 5:
            printf("                                    [ PISO 5 ]                                  \n");
            printf("================================================================================\n\n");
            printf("  Você desce as escadarias da masmorra, chegando aos escaldantes corredores dos\n");
            printf("  salões demoníacos. Você pode sentir uma poderosa aura sombria no ar...\n");
            break;

        case 6:
            printf("                               [ TEMPLO DE XÃOPAI ]                             \n");
            printf("================================================================================\n\n");
            printf("  Você desce as escadarias da masmorra, chegando finalmente ao trono do todo\n");
            printf("  poderoso %sXãopai%s%s. Você sente como se fosse passar por uns maus tempos.\n", CLR(ANSI_BMAGENTA), CLR(ANSI_RESET), CLR_ORANGE());
            break;
    }
    printf("\n================================================================================%s\n\n", CLR(ANSI_RESET));
}

void printsBadEnding(GameState *gs) {
    printf("> Você toma o amuleto para si.\n");
    printf("> Você absorve o poder do amuleto, e consegue sentir a escuridão fluir pelo seu corpo.\n");
    printf("> Você tenta resistir à magia, mas ela te domina e você acaba perdendo sua noção \nda realidade.\n");
    printf("> O que restou de você agora reinará no trono como %c nov%c %sXãopai%s.\n", gs->pronoum, gs->pronoum, CLR(ANSI_BMAGENTA), CLR(ANSI_RESET));
    printf("%s|FINAL RUIM|%s\n", CLR(ANSI_BRED), CLR(ANSI_RESET));
}

void printsGoodEnding() {
    printf("> Você destrói o amuleto.\n");
    printf("> Você ouve um estridente ruído vindo do amuleto enquanto vê toda a energia maligna \nse dissipar no ar.\n");
    printf("> Você finalmente libertou a masmorra e os habitantes da cidade das \nmãos do mal!\n");
    printf("%s|FINAL BOM|%s\n", CLR(ANSI_BGREEN), CLR(ANSI_RESET));
}

void printsTrueEnding(GameState *gs) {
    printf("> Você recita a sequência mágica ao amuleto.\n");
    printf("> O amuleto brilha em luz vermelha ofuscante enquanto muda sua forma para um esbelto garoto.\n");
    printf("> %sPaixão:%s Uh... Onde eu estou? Eu estava programando um jogo e... acho que dormi no teclado.\n", CLR(ANSI_BCYAN), CLR(ANSI_RESET));
    printf("> %sPaixão:%s Vem, %s%s%s, vamo dar um fora daqui. Ficar encarando essa tela de terminal \ntá me dando dor de cabeça...\n", CLR(ANSI_BCYAN), CLR(ANSI_RESET), CLR(ANSI_BWHITE), gs->hero.name, CLR(ANSI_RESET));
    printf("%s|FINAL VERDADEIRO|%s\n", CLR(ANSI_BCYAN), CLR(ANSI_RESET));
}

void printsCowardEnding(GameState *gs) {
    printf("\n> Por um milagre do destino, você encontra uma brecha e consegue escapar de %sXÃOPAI%s!\n", CLR(ANSI_BMAGENTA), CLR(ANSI_RESET));
    printf("> Você corre desesperadamente escadaria acima sem olhar para trás, abandonando a masmorra.\n");
    printf("> A cidade acima eventualmente sucumbe à escuridão, mas pelo menos você salvou a própria pele...\n");
    printf("> Você sobreviveu, mas será para sempre lembrad%c como %c maior covarde da história, %s%s%s.\n", 
           gs->pronoum, gs->pronoum, CLR(ANSI_BWHITE), gs->hero.name, CLR(ANSI_RESET));
    printf("%s|FINAL COVARDE|%s\n", CLR(ANSI_BYELLOW), CLR(ANSI_RESET));
    printf("Pontuação: %s%i%s\n\n", CLR(ANSI_BYELLOW), gs->score, CLR(ANSI_RESET));
}

// ================= [ COMBAT & EVENT FUNCTIONS ] ============================== //

bool mimicInteraction(GameState *gs, bool *chestOpened) {
    enemy_t mimic = {0};
    strcpy(mimic.name, "Mímico");
    switch(gs->currentFloor) {
        case 1:
            mimic.maxHp = rand() % 21 + 40;
            break;
        case 2:
            mimic.maxHp = rand() % 31 + 60;
            break;
        case 3:
            mimic.maxHp = rand() % 31 + 80;
            break;
        case 4:
            mimic.maxHp = rand() % 31 + 90;
            break;
        case 5:
            mimic.maxHp = rand() % 31 + 100;
            break;
    }
    mimic.atk = gs->hero.def + 10;
    mimic.def = 0;
    mimic.hp = mimic.maxHp;
    mimic.coins = rand() % (50 * gs->currentFloor) + 50;

    printf("> Você encontrou um %sbaú%s!\n", CLR(ANSI_BYELLOW), CLR(ANSI_RESET));
    printf("%s[1]%s Chutar violentamente\n", CLR(ANSI_BCYAN), CLR(ANSI_RESET));
    printf("%s[2]%s Ignorar\n", CLR(ANSI_BCYAN), CLR(ANSI_RESET));

    int action;
    do {
        action = readInt();
        switch(action) {
            case 0:
                showsInfo(gs, NULL);
                printf("%s|Baú|%s\n", CLR(ANSI_BYELLOW), CLR(ANSI_RESET));
                printf(" %s???%s\n\n", CLR(ANSI_DIM), CLR(ANSI_RESET));
                break;

            case 1:
                break;

            case 2:
                printf("\n> Você decidiu não arriscar e seguiu em frente.\n\n");
                return false;

            default:
                printf("> %sComando não encontrado.%s\n\n", CLR(ANSI_BRED), CLR(ANSI_RESET));
                break;
        }
    } while(action != 1);

    printf("\n> O Baú era um %smímico%s!\n\n", CLR(ANSI_BRED), CLR(ANSI_RESET));

    bool fled = false;
    do {
        printf("%s[1]%s Atacar\n", CLR(ANSI_BCYAN), CLR(ANSI_RESET));
        printf("%s[2]%s Fugir\n", CLR(ANSI_BCYAN), CLR(ANSI_RESET));
        if (gs->hero.potions > 0)
            printf("%s[3]%s Usar poção de cura %s(x%i)%s\n", CLR(ANSI_BCYAN), CLR(ANSI_RESET), CLR(ANSI_DIM), gs->hero.potions, CLR(ANSI_RESET));
        action = readInt();
        switch(action) {
            case 0:
                showsInfo(gs, &mimic);
                break;
            case 1: {
                int rolledHeroAtk = rollStat(gs->hero.atk);
                int rolledMimicDef = rollStat(mimic.def);
                int heroDamage = rolledHeroAtk - rolledMimicDef;

                if (heroDamage <= 0) {
                    printf("\n> Você atacou o %s, mas ele %srepeliu seu golpe%s!\n", mimic.name, CLR(ANSI_BRED), CLR(ANSI_RESET));
                } else {
                    bool isCrit = (rand() % 10 == 0);
                    if (isCrit) {
                        heroDamage = (heroDamage * 3 + 1) / 2;
                        printf("\n> %sGOLPE CRÍTICO%s! Você atacou o %s causando %s%i de dano%s!\n", 
                               CLR(ANSI_BGREEN), CLR(ANSI_RESET), mimic.name, CLR(ANSI_BGREEN), heroDamage, CLR(ANSI_RESET));
                    } else {
                        printf("\n> Você atacou o %s causando %s%i de dano%s!\n", mimic.name, CLR(ANSI_BGREEN), heroDamage, CLR(ANSI_RESET));
                    }
                    mimic.hp -= heroDamage;
                }

                if (mimic.hp > 0) {
                    int rolledMimicAtk = rollStat(mimic.atk);
                    int rolledHeroDef = rollStat(gs->hero.def);
                    int mimicDamage = rolledMimicAtk - rolledHeroDef;

                    if (mimicDamage <= 0) {
                        printf("> %s tentou te morder, mas %ssua defesa bloqueou%s!\n\n", mimic.name, CLR(ANSI_BGREEN), CLR(ANSI_RESET));
                    } else {
                        gs->hero.hp -= mimicDamage;
                        printf("> %s te mordeu causando %s%i de dano%s!\n\n", mimic.name, CLR(ANSI_BRED), mimicDamage, CLR(ANSI_RESET));
                    }
                    if (gs->hero.hp <= 0) {
                        clearScreen();
                        printf("%sVOCÊ MORREU PARA %s NO PISO %i%s\n\a", CLR(ANSI_BRED), mimic.name, gs->currentFloor, CLR(ANSI_RESET));
                        printf("Pontuação: %s%i%s\n", CLR(ANSI_BYELLOW), gs->score, CLR(ANSI_RESET));
                        return true;
                    }
                }
                break;
            }
            case 2: {
                printf("\n> Você deu meia-volta e tentou fugir do mímico!\n");
                int rolledMimicAtk = rollStat(mimic.atk);
                int rolledHeroDef = rollStat(gs->hero.def);
                int mimicDamage = rolledMimicAtk - rolledHeroDef;

                if (mimicDamage <= 0) {
                    printf("> %s tentou te morder pelas costas, mas %svocê conseguiu se esquivar%s!\n", mimic.name, CLR(ANSI_BGREEN), CLR(ANSI_RESET));
                } else {
                    gs->hero.hp -= mimicDamage;
                    printf("> %s te mordeu pelas costas causando %s%i de dano%s!\n", mimic.name, CLR(ANSI_BRED), mimicDamage, CLR(ANSI_RESET));
                }
                if (gs->hero.hp <= 0) {
                    clearScreen();
                    printf("%sVOCÊ MORREU PARA %s TENTANDO FUGIR NO PISO %i%s\n\a", CLR(ANSI_BRED), mimic.name, gs->currentFloor, CLR(ANSI_RESET));
                    printf("Pontuação: %s%i%s\n", CLR(ANSI_BYELLOW), gs->score, CLR(ANSI_RESET));
                    return true;
                }
                printf("> Você conseguiu escapar com vida pelos corredores!\n\n");
                fled = true;
                mimic.hp = 0;
                break;
            }
            case 3:
                if (gs->hero.potions <= 0) {
                    printf("\n> %sVocê não tem poções de cura guardadas!%s\n\n", CLR(ANSI_BRED), CLR(ANSI_RESET));
                } else if (gs->hero.hp >= gs->hero.maxHp) {
                    printf("\n> %sSua saúde já está no máximo!%s\n\n", CLR(ANSI_BYELLOW), CLR(ANSI_RESET));
                } else {
                    gs->hero.potions--;
                    int heal = gs->hero.maxHp / 2;
                    if (gs->hero.hp + heal > gs->hero.maxHp) heal = gs->hero.maxHp - gs->hero.hp;
                    gs->hero.hp += heal;
                    printf("\n> Você bebeu uma poção de cura e recuperou %s%i de vida%s! %s(Restam: %i)%s\n\n", 
                           CLR(ANSI_BGREEN), heal, CLR(ANSI_RESET), CLR(ANSI_DIM), gs->hero.potions, CLR(ANSI_RESET));
                }
                break;
            default:
                printf("> %sComando não encontrado.%s\n\n", CLR(ANSI_BRED), CLR(ANSI_RESET));
                break;
        }
    } while(mimic.hp > 0);

    if (!fled) {
        gs->score += mimic.coins;
        gs->hero.coins += mimic.coins;
        printf("> Você derrotou o %s%s%s!\n", CLR(ANSI_BRED), mimic.name, CLR(ANSI_RESET));
        printf("> Você encontrou %s%d moedas%s!\n\n", CLR(ANSI_BYELLOW), mimic.coins, CLR(ANSI_RESET));

        if (*chestOpened == false) {
            gs->secretNumber = rand() % 900 + 100;
            printf("> Você encontra uma marcação suspeita gravada no fundo do baú: %s%i%s\n\n", CLR(ANSI_BMAGENTA), gs->secretNumber, CLR(ANSI_RESET));
            *chestOpened = true;
        }
    }

    return false;
}

bool xaopaiInteraction(GameState *gs) {
    printf("> Você desafia o terrível %sXãopai%s!\n", CLR(ANSI_BMAGENTA), CLR(ANSI_RESET));
    bool hasExcalibur = (strcmp(gs->hero.atkItem, "Excalibur Lendária") == 0);
    bool hasEgide = (strcmp(gs->hero.defItem, "Égide Mítica") == 0);

    if (hasExcalibur) {
        printf("> %sA Excalibur Lendária brilha intensamente em suas mãos, pronta para cortar as trevas!%s\n\n",
               CLR(ANSI_BCYAN), CLR(ANSI_RESET));
    } else if (hasEgide) {
        printf("> %sA Égide Mítica irradia uma barreira divina ao seu redor, refletindo a aura maligna!%s\n\n",
               CLR(ANSI_BCYAN), CLR(ANSI_RESET));
    } else {
        printf("> %sSuas armas comuns tremem diante da pressão esmagadora do espírito ancestral...%s\n\n",
               CLR(ANSI_BRED), CLR(ANSI_RESET));
    }

    gs->monsters[XAOPAI].hp = gs->monsters[XAOPAI].maxHp;
    bool isCharged = false;
    bool phase2 = false;
    int turnsSinceCharge = 0;

    do {
        printf("%s[1]%s Atacar\n", CLR(ANSI_BCYAN), CLR(ANSI_RESET));
        printf("%s[2]%s Defender\n", CLR(ANSI_BCYAN), CLR(ANSI_RESET));
        if (gs->hero.potions > 0)
            printf("%s[3]%s Usar poção de cura %s(x%i)%s\n", CLR(ANSI_BCYAN), CLR(ANSI_RESET), CLR(ANSI_DIM), gs->hero.potions, CLR(ANSI_RESET));
        printf("%s[4]%s Fugir\n", CLR(ANSI_BCYAN), CLR(ANSI_RESET));

        int action = readInt();
        bool bossTurn = false;
        bool defending = false;

        switch (action) {
            case 0: // INFO
                showsInfo(gs, &gs->monsters[XAOPAI]);
                break;

            case 1: { // ATTACK
                bossTurn = true;
                int effectiveBossDef = hasExcalibur ? (gs->monsters[XAOPAI].def * 2) / 5 : gs->monsters[XAOPAI].def;
                int heroDamage = rollStat(gs->hero.atk) - rollStat(effectiveBossDef);

                if (heroDamage <= 0) {
                    printf("\n> Você atacou %sXÃOPAI%s com %s, mas a aura sombria %sabsorveu o golpe%s!\n",
                           CLR(ANSI_BMAGENTA), CLR(ANSI_RESET), gs->hero.atkItem, CLR(ANSI_BRED), CLR(ANSI_RESET));
                } else {
                    bool isCrit = hasExcalibur ? (rand() % 4 == 0) : (rand() % 10 == 0);
                    if (isCrit) {
                        heroDamage = (heroDamage * 3 + 1) / 2;
                        printf("\n> %sGOLPE CRÍTICO%s! Você atacou %sXÃOPAI%s com %s causando %s%i de dano%s!\n",
                               CLR(ANSI_BGREEN), CLR(ANSI_RESET), CLR(ANSI_BMAGENTA), CLR(ANSI_RESET), gs->hero.atkItem, CLR(ANSI_BGREEN), heroDamage, CLR(ANSI_RESET));
                    } else {
                        printf("\n> Você atacou %sXÃOPAI%s com %s causando %s%i de dano%s!\n",
                               CLR(ANSI_BMAGENTA), CLR(ANSI_RESET), gs->hero.atkItem, CLR(ANSI_BGREEN), heroDamage, CLR(ANSI_RESET));
                    }
                    gs->monsters[XAOPAI].hp -= heroDamage;
                }
                break;
            }

            case 2: // DEFEND
                bossTurn = true;
                defending = true;
                printf("\n> Você firma os pés e ergue sua guarda com %s%s%s!\n",
                       CLR(ANSI_BCYAN), gs->hero.defItem, CLR(ANSI_RESET));
                break;

            case 3: // POTION
                if (gs->hero.potions <= 0) {
                    printf("\n> %sVocê não tem poções de cura guardadas!%s\n\n", CLR(ANSI_BRED), CLR(ANSI_RESET));
                } else if (gs->hero.hp >= gs->hero.maxHp) {
                    printf("\n> %sSua saúde já está no máximo!%s\n\n", CLR(ANSI_BYELLOW), CLR(ANSI_RESET));
                } else {
                    gs->hero.potions--;
                    int heal = gs->hero.maxHp / 2;
                    if (gs->hero.hp + heal > gs->hero.maxHp) heal = gs->hero.maxHp - gs->hero.hp;
                    gs->hero.hp += heal;
                    printf("\n> Você bebeu uma poção de cura e recuperou %s%i de vida%s! %s(Restam: %i)%s\n\n",
                           CLR(ANSI_BGREEN), heal, CLR(ANSI_RESET), CLR(ANSI_DIM), gs->hero.potions, CLR(ANSI_RESET));
                }
                break;

            case 4: { // FLEE
                if (rand() % 10 == 0) {
                    gs->fledXaopai = true;
                    return false;
                }
                bossTurn = true;
                printf("\n> %sNão há como fugir do poder avassalador de XÃOPAI!%s\n", CLR(ANSI_BMAGENTA), CLR(ANSI_RESET));
                break;
            }

            default:
                printf("> %sComando não encontrado.%s\n\n", CLR(ANSI_BRED), CLR(ANSI_RESET));
                break;
        }

        // Boss turn execution
        if (bossTurn && gs->monsters[XAOPAI].hp > 0) {
            bool wasCharged = isCharged;
            if (isCharged) {
                int rawBlast = rollStat((gs->monsters[XAOPAI].atk * 165) / 100);
                isCharged = false;
                turnsSinceCharge = 0;

                if (defending) {
                    if (hasEgide) {
                        int xaopaiDamage = (rawBlast - rollStat(gs->hero.def)) / 3;
                        if (xaopaiDamage < 12) xaopaiDamage = 12;
                        gs->hero.hp -= xaopaiDamage;
                        printf("> %sXÃOPAI%s dispara a %sEXPLOSÃO ABISSAL%s, mas a %sÉgide Mítica%s absorve o impacto (você sofreu %s%i de dano%s)!\n",
                               CLR(ANSI_BMAGENTA), CLR(ANSI_RESET), CLR(ANSI_BRED), CLR(ANSI_RESET), CLR(ANSI_BCYAN), CLR(ANSI_RESET), CLR(ANSI_BRED), xaopaiDamage, CLR(ANSI_RESET));

                        if (gs->hero.hp > 0) {
                            int reflected = rollStat(65);
                            gs->monsters[XAOPAI].hp -= reflected;
                            printf("> %sREFLEXO MÍTICO%s! A Égide devolve a onda de choque contra %sXÃOPAI%s causando %s%i de dano%s!\n\n",
                                   CLR(ANSI_BCYAN), CLR(ANSI_RESET), CLR(ANSI_BMAGENTA), CLR(ANSI_RESET), CLR(ANSI_BGREEN), reflected, CLR(ANSI_RESET));
                        } else {
                            printf("\n");
                        }
                    } else if (hasExcalibur) {
                        int xaopaiDamage = (rawBlast - rollStat(gs->hero.def)) / 3;
                        if (xaopaiDamage < 16) xaopaiDamage = 16;
                        gs->hero.hp -= xaopaiDamage;
                        printf("> %sXÃOPAI%s dispara a %sEXPLOSÃO ABISSAL%s! Você apara o feixe com a %sExcalibur Lendária%s (sofrendo %s%i de dano%s)!\n",
                               CLR(ANSI_BMAGENTA), CLR(ANSI_RESET), CLR(ANSI_BRED), CLR(ANSI_RESET), CLR(ANSI_BCYAN), CLR(ANSI_RESET), CLR(ANSI_BRED), xaopaiDamage, CLR(ANSI_RESET));

                        if (gs->hero.hp > 0) {
                            int counter = rollStat(gs->hero.atk) - rollStat(gs->monsters[XAOPAI].def / 2);
                            if (counter < 1) counter = 1;
                            gs->monsters[XAOPAI].hp -= counter;
                            printf("> %sCONTRA-ATAQUE SAGRADO%s! Você avança pelo feixe partido e golpeia %sXÃOPAI%s causando %s%i de dano%s!\n\n",
                                   CLR(ANSI_BGREEN), CLR(ANSI_RESET), CLR(ANSI_BMAGENTA), CLR(ANSI_RESET), CLR(ANSI_BGREEN), counter, CLR(ANSI_RESET));
                        } else {
                            printf("\n");
                        }
                    } else {
                        int xaopaiDamage = ((rawBlast - rollStat(gs->hero.def)) * 3) / 5;
                        if (xaopaiDamage < 35) xaopaiDamage = 35;
                        gs->hero.hp -= xaopaiDamage;
                        printf("> %sXÃOPAI%s dispara a %sEXPLOSÃO ABISSAL%s! Seu escudo comum mal suporta o impacto, causando %s%i de dano%s!\n",
                               CLR(ANSI_BMAGENTA), CLR(ANSI_RESET), CLR(ANSI_BRED), CLR(ANSI_RESET), CLR(ANSI_BRED), xaopaiDamage, CLR(ANSI_RESET));
                        printf("> %sO choque colossal adormece seus braços, impedindo qualquer contra-ataque!%s\n\n",
                               CLR(ANSI_DIM), CLR(ANSI_RESET));
                    }
                } else {
                    int xaopaiDamage = rawBlast - (rollStat(gs->hero.def) / 2);
                    if (xaopaiDamage < 45) xaopaiDamage = 45;
                    gs->hero.hp -= xaopaiDamage;
                    printf("> %sEXPLOSÃO ABISSAL%s! %sXÃOPAI%s descarrega toda a energia acumulada em cheio, causando %s%i de dano devastador%s!\n\n",
                           CLR(ANSI_BRED), CLR(ANSI_RESET), CLR(ANSI_BMAGENTA), CLR(ANSI_RESET), CLR(ANSI_BRED), xaopaiDamage, CLR(ANSI_RESET));
                }
            } else {
                int xaopaiDamage = rollStat(gs->monsters[XAOPAI].atk) - rollStat(gs->hero.def);
                if (xaopaiDamage < 5) xaopaiDamage = 5;

                if (defending) {
                    xaopaiDamage = (xaopaiDamage + 1) / 2;
                    if (xaopaiDamage < 5) xaopaiDamage = 5;
                    gs->hero.hp -= xaopaiDamage;
                    printf("> %sXÃOPAI%s atacou com garras de sombra, mas sua postura defensiva reduziu o impacto para %s%i de dano%s!\n\n",
                           CLR(ANSI_BMAGENTA), CLR(ANSI_RESET), CLR(ANSI_BRED), xaopaiDamage, CLR(ANSI_RESET));
                } else {
                    gs->hero.hp -= xaopaiDamage;
                    printf("> %sXÃOPAI%s revidou com poder sombrio causando %s%i de dano%s!\n",
                           CLR(ANSI_BMAGENTA), CLR(ANSI_RESET), CLR(ANSI_BRED), xaopaiDamage, CLR(ANSI_RESET));

                    if (gs->hero.hp > 0 && hasEgide && action == 1) {
                        int passiveReflect = rollStat(16);
                        gs->monsters[XAOPAI].hp -= passiveReflect;
                        printf("> A luz da %sÉgide Mítica%s queima %sXÃOPAI%s no choque, refletindo %s%i de dano%s!\n",
                               CLR(ANSI_BCYAN), CLR(ANSI_RESET), CLR(ANSI_BMAGENTA), CLR(ANSI_RESET), CLR(ANSI_BGREEN), passiveReflect, CLR(ANSI_RESET));
                    }

                    if (gs->hero.hp > 0 && gs->monsters[XAOPAI].hp > 0 && phase2 && (rand() % 10 < 3)) {
                        int drain = xaopaiDamage / 2;
                        if (drain < 1) drain = 1;
                        gs->monsters[XAOPAI].hp += drain;
                        if (gs->monsters[XAOPAI].hp > gs->monsters[XAOPAI].maxHp) {
                            gs->monsters[XAOPAI].hp = gs->monsters[XAOPAI].maxHp;
                        }
                        printf("> %sDRENO DE ALMA%s! %sXÃOPAI%s absorve sua força vital e regenera %s%i de vida%s!\n",
                               CLR(ANSI_BMAGENTA), CLR(ANSI_RESET), CLR(ANSI_BMAGENTA), CLR(ANSI_RESET), CLR(ANSI_BRED), drain, CLR(ANSI_RESET));
                    }
                    printf("\n");
                }
            }

            if (gs->hero.hp <= 0) {
                clearScreen();
                if (action == 4) {
                    printf("%sVOCÊ PERECEU TENTANDO FUGIR DE %sXÃOPAI%s.%s\n\a", CLR(ANSI_BRED), CLR(ANSI_BMAGENTA), CLR(ANSI_BRED), CLR(ANSI_RESET));
                } else {
                    printf("%sVOCÊ PERECEU DIANTE DE %sXÃOPAI%s.%s\n\a", CLR(ANSI_BRED), CLR(ANSI_BMAGENTA), CLR(ANSI_BRED), CLR(ANSI_RESET));
                }
                printf("Pontuação: %s%i%s\n", CLR(ANSI_BYELLOW), gs->score, CLR(ANSI_RESET));
                return true;
            }

            // Phase 2 transition or 3-turn charge telegraph
            if (gs->monsters[XAOPAI].hp > 0) {
                if (!phase2 && gs->monsters[XAOPAI].hp <= gs->monsters[XAOPAI].maxHp / 2) {
                    phase2 = true;
                    gs->monsters[XAOPAI].atk = 94;
                    isCharged = true;
                    turnsSinceCharge = 0;
                    printf("> %sO corpo de XÃOPAI se desfaz em chamas negras! SEUS OLHOS BRILHAM EM FÚRIA ABISSAL!%s\n",
                           CLR(ANSI_BRED), CLR(ANSI_RESET));
                    printf("%sXÃOPAI: VOCÊ JAMAIS SAIRÁ VIVO DESTE TEMPLO! (O ataque de XÃOPAI aumentou!)%s\n",
                           CLR(ANSI_BMAGENTA), CLR(ANSI_RESET));
                    printf("> %s[!] XÃOPAI ergue os braços e concentra uma esfera de energia abissal!%s\n\n",
                           CLR(ANSI_BYELLOW), CLR(ANSI_RESET));
                } else if (!wasCharged) {
                    turnsSinceCharge++;
                    if (turnsSinceCharge >= 3) {
                        isCharged = true;
                        turnsSinceCharge = 0;
                        printf("> %s[!] XÃOPAI ergue os braços e concentra uma esfera de energia abissal!%s\n\n",
                               CLR(ANSI_BYELLOW), CLR(ANSI_RESET));
                    }
                }
            }
        }
    } while (gs->monsters[XAOPAI].hp > 0);

    gs->score += 10000;
    printf("> %sVOCÊ DERROTOU O ESPÍRITO DE %sXÃOPAI%s!%s\n\n", CLR(ANSI_BGREEN), CLR(ANSI_BMAGENTA), CLR(ANSI_BGREEN), CLR(ANSI_RESET));
    return false;
}

void ending(GameState *gs) {
    printf("> Você atravessa a fera com a sua espada e ela é reduzida à poeira e cinzas.\n");
    printf("> Você apanha um %samuleto vermelho-sangue%s em meio aos restos da criatura\n", CLR(ANSI_BRED), CLR(ANSI_RESET));
    printf("> O amuleto pulsa vigorosamente em suas mãos. Você pode sentir um imenso poder \nsaindo dele.\n");
    printf("> O que você faz?\n");
    printf("%s[1]%s Inspecionar amuleto\n", CLR(ANSI_BCYAN), CLR(ANSI_RESET));
    printf("%s[2]%s Se apossar do amuleto de %sXãopai%s\n", CLR(ANSI_BCYAN), CLR(ANSI_RESET), CLR(ANSI_BMAGENTA), CLR(ANSI_RESET));
    printf("%s[3]%s Destruir o amuleto de %sXãopai%s\n", CLR(ANSI_BCYAN), CLR(ANSI_RESET), CLR(ANSI_BMAGENTA), CLR(ANSI_RESET));

    while(1) {
        int action = readInt();

        if (action == 0) {
            showsInfo(gs, NULL);
        }
        else if (action == 1) {
            printsAmulet();
        } else if (action == 2) {
            printsBadEnding(gs);
            break;
        } else if (action == 3) {
            printsGoodEnding();
            break;
        } else if (action == gs->secretNumber) {
            printsTrueEnding(gs);
            break;
        } else if (action == PAIXAO) {
            printf("Nada acontece\a\a\a\n");
        } else {
            printf("Nada acontece...\n");
        }
    }
    printf("Pontuação: %s%i%s\n\n", CLR(ANSI_BYELLOW), gs->score, CLR(ANSI_RESET));
}

void updatesHighscores(GameState *gs, highscore_entry_t* highscores) {
    FILE *file = fopen("Highscores.bin", "rb");

    if (file != NULL) {
        size_t readCount = fread(highscores, sizeof(highscore_entry_t), HIGHSCORES_NUM, file);
        for (size_t i = readCount; i < HIGHSCORES_NUM; ++i) {
            highscores[i].score = 0;
            highscores[i].name[0] = '\0';
        }
        fclose(file);
    } else {
        memset(highscores, 0, sizeof(highscore_entry_t) * HIGHSCORES_NUM);
    }

    highscores[HIGHSCORES_NUM - 1].score = gs->score;
    if (strlen(gs->hero.name) > 0) {
        strncpy(highscores[HIGHSCORES_NUM - 1].name, gs->hero.name, STR_LENGTH - 1);
        highscores[HIGHSCORES_NUM - 1].name[STR_LENGTH - 1] = '\0';
    } else {
        strcpy(highscores[HIGHSCORES_NUM - 1].name, "Aventureiro");
    }

    qsort(highscores, HIGHSCORES_NUM, sizeof(highscore_entry_t), compareHighscores);

    file = fopen("Highscores.bin", "wb");
    if (file == NULL) {
        perror("Error opening file");
        return;
    }

    fwrite(highscores, sizeof(highscore_entry_t), HIGHSCORES_NUM, file);
    fclose(file);
}

int highscoresMenu(GameState *gs, highscore_entry_t* highscores) {
    int input;
    clearScreen();
    printf("                %s- HIGH SCORES -%s                \n\n", CLR(ANSI_BYELLOW), CLR(ANSI_RESET));

    for (int i = 0; i < HIGHSCORES_NUM - 1; ++i) {
        if (highscores[i].score != 0) {
            const char *rankColor = CLR(ANSI_DIM);
            if (i == 0) rankColor = CLR(ANSI_BYELLOW);
            else if (i == 1) rankColor = CLR(ANSI_BWHITE);
            else if (i == 2) rankColor = CLR(ANSI_YELLOW);
            printf("            %s%2i - %-18s %5i%s\n", rankColor, i + 1, highscores[i].name, highscores[i].score, CLR(ANSI_RESET));
        }
    }

    printf("\n         %s(1)%s Jogar novamente\n", CLR(ANSI_BCYAN), CLR(ANSI_RESET));
    printf("         %s(2)%s Sair do jogo\n", CLR(ANSI_BCYAN), CLR(ANSI_RESET));

    do {
        input = readInt();
        switch(input) {
            case 1:
                return 1;
                break;
            case 2: {
                printf("> Deseja realmente sair? (s/n)\n");
                char confirm[STR_LENGTH];
                readString(confirm, STR_LENGTH);
                if (tolower((unsigned char)confirm[0]) == 's' || tolower((unsigned char)confirm[0]) == 'y') {
                    return 0;
                }
                printf("\n         %s(1)%s Jogar novamente\n", CLR(ANSI_BCYAN), CLR(ANSI_RESET));
                printf("         %s(2)%s Sair do jogo\n", CLR(ANSI_BCYAN), CLR(ANSI_RESET));
                break;
            }
            case 10:
                gs->pronoum = 'e';
                printf("?\n");
                break;
            case 11:
                gs->pronoum = 'o';
                printf("\11\n");
                break;
            case 12:
                gs->pronoum = 'a';
                printf("\12\n");
                break;
            default:
                printf("> %sComando não encontrado.%s\n\n", CLR(ANSI_BRED), CLR(ANSI_RESET));
                break;
        }
    } while(input != 1);
    
    return 0;
}

// ================= [ DUNGEON LOOP ] ========================================= //

void playDungeon(GameState *gs) {
    int monsterCode = gs->currentFloor * 3;
    bool chestOpened = false;

    for (int floor = gs->currentFloor; floor < 5; ++floor) {
        gs->currentFloor++;
        gs->eliteSpawnedThisFloor = false;
        gs->mimicSpawnedThisFloor = false;
        clearScreen();
        dungeonDialog(gs);
        
        for (int i = 0; i < 3; ++i) {
            gs->isDead = monsterInteraction(gs, monsterCode);
            if (gs->isDead)
                return;
            monsterCode++;

            // Mimic: 10% chance to appear after any defeated enemy, at most 1 per floor
            if (!gs->mimicSpawnedThisFloor && (rand() % 10 == 0)) {
                gs->mimicSpawnedThisFloor = true;
                gs->isDead = mimicInteraction(gs, &chestOpened);
                if (gs->isDead)
                    return;
            }
        }
        
        shopDialog(gs);
        shopInteraction(gs, gs->currentFloor);
    }
    
    gs->currentFloor++;
    clearScreen();
    dungeonDialog(gs);
    printsXaopai();
    gs->isDead = xaopaiInteraction(gs);
    
    if (gs->isDead)
        return;
    else if (gs->fledXaopai)
        printsCowardEnding(gs);
    else
        ending(gs);
}

// ================= [ MAIN LOOP ] ============================================ //

int main(int argc, char *argv[]) {
    setlocale(LC_ALL, ".UTF8");

    #ifdef _WIN32
        SetConsoleOutputCP(CP_UTF8);
        SetConsoleCP(CP_UTF8);
    #endif

    initTerminalColors(argc, argv);

    srand((unsigned)time(NULL));

    int iterationCounter = 0;
    int gamesWon = 0;
    highscore_entry_t highscores[HIGHSCORES_NUM] = {0};
    char heroName[STR_LENGTH] = "";
    char savedPronoun = 'o';

    FILE *initFile = fopen("Highscores.bin", "rb");
    if (initFile != NULL) {
        fread(highscores, sizeof(highscore_entry_t), HIGHSCORES_NUM, initFile);
        fclose(initFile);
    }
    
    int action;
    do {
        GameState gs = {0};
        
        gs.currentFloor = 0;
        gs.score = 0;
        gs.isDead = false;
        
        initGameData(&gs);
        gs.pronoum = savedPronoun;
        
        titleScreen();
        dungeonDialog(&gs);
        
        printf("\n> Antes de começar sua aventura, você entra em uma loja.\n");
        if (iterationCounter == 0) {
            printf("%sLojista:%s Você deve ser nov%c por aqui. Qual seu nome, viajante?\n", CLR(ANSI_BYELLOW), CLR(ANSI_RESET), gs.pronoum);
            readString(heroName, STR_LENGTH);
        }
        strcpy(gs.hero.name, heroName);
        
        char lowerName[STR_LENGTH];
        strcpy(lowerName, gs.hero.name);
        for (int i = 0; lowerName[i]; i++) {
            lowerName[i] = tolower(lowerName[i]);
        }
        if (strcmp(lowerName, "paixao") == 0 || strcmp(lowerName, "paixão") == 0 || strcmp(lowerName, "root") == 0) {
            printf("%sLojista:%s Olha só! O próprio criador do jogo! Bom rever você. %s(Pular para loja 0-5?)%s\n", CLR(ANSI_BYELLOW), CLR(ANSI_RESET), CLR(ANSI_DIM), CLR(ANSI_RESET));
            gs.hero.coins += 9990;
            int skip = readInt();
            if (skip >= 1 && skip <= 5) {
                gs.currentFloor = skip;
                gs.hero.maxHp = gs.hero.hp = 100 + (skip / 2) * 50;
                strcpy(gs.hero.atkItem, gs.shops[skip - 1].item1Name);
                gs.hero.atk = gs.shops[skip - 1].item1atkPoints;
                strcpy(gs.hero.defItem, gs.shops[skip - 1].item2Name);
                gs.hero.def = gs.shops[skip - 1].item2defPoints;
            }
        }
        
        shopDialog(&gs);
        
        printf("\n%s+---------------------------------------------------------------------------+\n", CLR(ANSI_BWHITE));
        printf(  "| DICA: Digite '0' a qualquer momento para mostrar status do herói/oponente |\n");
        printf(  "+---------------------------------------------------------------------------+%s\n\n", CLR(ANSI_RESET));
        
        shopInteraction(&gs, gs.currentFloor);
        
        playDungeon(&gs);
        
        if (!gs.isDead)
            gamesWon++;
        else
            printf("\7");
        
        updatesHighscores(&gs, highscores);
        
        printf("%s[1]%s Jogar novamente\n%s[2]%s Highscores\n%s[3]%s Sair do jogo\n", 
               CLR(ANSI_BCYAN), CLR(ANSI_RESET), CLR(ANSI_BCYAN), CLR(ANSI_RESET), CLR(ANSI_BCYAN), CLR(ANSI_RESET));
        
        do {
            action = readInt();
            switch(action) {
                case 1:
                    iterationCounter++;
                    clearScreen();
                    break;
                    
                case 2:
                    if (highscoresMenu(&gs, highscores)) {
                        iterationCounter++;
                        action = 1;
                    } else {
                        action = 3;
                    }
                    savedPronoun = gs.pronoum;
                    break;
                    
                case 3: {
                    printf("> Deseja realmente sair? (s/n)\n");
                    char confirm[STR_LENGTH];
                    readString(confirm, STR_LENGTH);
                    if (tolower((unsigned char)confirm[0]) != 's' && tolower((unsigned char)confirm[0]) != 'y') {
                        action = 0;
                        printf("\n%s[1]%s Jogar novamente\n%s[2]%s Highscores\n%s[3]%s Sair do jogo\n", 
                               CLR(ANSI_BCYAN), CLR(ANSI_RESET), CLR(ANSI_BCYAN), CLR(ANSI_RESET), CLR(ANSI_BCYAN), CLR(ANSI_RESET));
                    }
                    break;
                }
                    
                default:
                    printf("> %sComando não encontrado.%s\n\n", CLR(ANSI_BRED), CLR(ANSI_RESET));
                    break;
            }
        } while((action != 1) && (action != 3));
        
    } while(action != 3);
    
    return PAIXAO;
}