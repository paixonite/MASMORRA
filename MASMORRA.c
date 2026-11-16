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

// Cross-platform screen clear and terminal animation macros
#ifdef _WIN32
    #include <windows.h>
    #include <conio.h>
    #include <io.h>
    #define CLEAR_CMD "cls"
#else
    #include <unistd.h>
    #include <sys/select.h>
    #define CLEAR_CMD "clear" // Linux/macOS uses native clear
#endif

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
                printf("> Entrada inválida.\n");
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
        printf("> Entrada inválida.\n");
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
        printf("> Entrada inválida.\n");
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
    gs->monsters[XAOPAI].atk = 10;
    gs->monsters[XAOPAI].def = 0;

    // Initialize Shops
    strcpy(gs->shops[0].item1Name, "Bastão de madeira");
    gs->shops[0].item1atkPoints = 20;
    gs->shops[0].item1Price = 100;
    strcpy(gs->shops[0].item2Name, "Escudo pequeno de madeira");
    gs->shops[0].item2defPoints = 10;
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
    gs->shops[5].item1atkPoints = 80;
    gs->shops[5].item1Price = 550;
    strcpy(gs->shops[5].item2Name, "Égide Mítica");
    gs->shops[5].item2defPoints = 150;
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

        printf("********************************************************************************\n");
        printf("  %s                         BEM-VINDO À                          %s  \n", flames[frame][0], flames[frame][0]);
        printf("  %s    __  __    _    ____  __  __  ___  ____  ____      _       %s  \n", flames[frame][1], flames[frame][1]);
        printf("  %s   |  \\/  |  / \\  / ___||  \\/  |/ _ \\|  _ \\|  _ \\    / \\      %s  \n", flames[frame][2], flames[frame][2]);
        printf("  \\=====/   | |\\/| | / _ \\ \\___ \\| |\\/| | | | | |_) | |_) |  / _ \\     \\=====/  \n");
        printf("   \\   /    | |  | |/ ___ \\ ___) | |  | | |_| |  _ <|  _ <  / ___ \\     \\   /   \n");
        printf("    | |     |_|  |_/_/   \\_\\____/|_|  |_|\\___/|_| \\_\\_| \\_\\/_/   \\_\\     | |    \n");
        printf("    |_|                           by: Paixão                             |_|    \n");
        printf("********************************************************************************\n\n");
        printf("                      Pressione enter para continuar...                         \n");
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
    printf("[");
    for (int i = 0; i < filled; i++) {
        printf("█");
    }
    for (int i = filled; i < length; i++) {
        printf("░");
    }
    printf("] %i/%i\n", current, max);
}

void showsInfo(GameState *gs, const enemy_t *enemy) {
    printf("\n+---------------- [ STATUS ] ----------------+\n");
    printf("| - %s\n", gs->hero.name);
    printf("|   Saúde:  ");
    printHpBar(gs->hero.hp, gs->hero.maxHp);
    printf("|   Poções: %i\n", gs->hero.potions);
    printf("|   Moedas: %i\n", gs->hero.coins);
    printf("|   Ataque: %i (%s)\n", gs->hero.atk, gs->hero.atkItem);
    printf("|   Defesa: %i (%s)\n", gs->hero.def, gs->hero.defItem);

    if (enemy != NULL) {
        printf("+--------------------------------------------+\n");
        printf("| - %s\n", enemy->name);
        printf("|   Saúde:  ");
        printHpBar(enemy->hp, enemy->maxHp);
        printf("|   Ataque: %i\n", enemy->atk);
        printf("|   Defesa: %i\n", enemy->def);
    }
    printf("+--------------------------------------------+\n\n");
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
    printf("                            .-.\n");
    printf("       ___,---.__          / | \\          __,---,___\n");
    printf("    ,-'    \\\\    `-.____,-'  |  `-.____,-'    //    `-.\n");
    printf("  ,'        ||          ~'\\     /`~          ||        `.\n");
    printf(" /      ___//              `. .'              \\\\___      \\\n");
    printf("|    ,-'   `-.__             |             __,-'   `-.    |\n");
    printf("\\   |          \\ \\`-.___ \\   |   / ___,-'/ /          |   /\n");
    printf(" \\  \\           | `._  %c`\\\\  |  //'%c  _,' |           /  /\n", eye, eye);
    printf("  `-.\\         /'  _ `---'' . . ``---' _  `\\         /,-'\n");
    printf("     ``       /     \\    .='/ \\`=.    /     \\       ''\n");
    printf("             |__   /|\\_.--.--.--.--._/|\\   __|\n");
    printf("             /  `./  \\\\'\\ |  |  | /'//  \\.'  \\\n");
    printf("            /   /     ||\\-+--|--+-/||     \\   \\\n");
    printf("           |   |     /'\\_\\_\\ | /_/_/'\\     |   |\n");
    printf("            \\   \\__, \\_     '~'     _/ .__/   /\n");
    printf("             `-._,-'   `-._______.-'   `-._,-'\n\n");
}

void printsAmulet() {
    printf("       .....           .....\n");
    printf("   ,ad8PPPP88b,     ,d88PPPP8ba,\n");
    printf("  d8P'      'Y8b, ,d8P'      'Y8b\n");
    printf(" dP'           '8a8'           `Yd\n");
    printf(" 8(              '              )8\n");
    printf(" I8                             8I\n");
    printf("  Yb,           o_o           ,dP\n");
    printf("   '8a,  ~me deixa entrar~  ,a8'\n");
    printf("     '8a,                 ,a8'\n");
    printf("       'Yba             adP'\n");
    printf("         `Y8a         a8P'\n");
    printf("           `88,     ,88'\n");
    printf("             '8b   d8'\n");
    printf("              '8b d8'\n");
    printf("               `888'\n");
    printf("                 '\n");
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
        printf("> Você encontrou um monstro ELITE: %s!\n", gs->monsters[monsterCode].name);
        printf("> Uma poderosa aura intimidadora emana da criatura...\n\n");
    } else {
        printf("> Você encontrou %s!\n\n", gs->monsters[monsterCode].name);
    }
    
    do {
        printf("[1] Atacar\n");
        printf("[2] Fugir\n");
        if (gs->hero.potions > 0) {
            printf("[3] Usar poção de cura (x%i)\n", gs->hero.potions);
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
                printf("VOCÊ MORREU DE PARADA CARDÍACA AO VER %s NO PISO %i\n\a", 
                        gs->monsters[monsterCode].name, gs->currentFloor);
                printf("Pontuação: %i\n", gs->score);
                return true;
            }

            int rolledHeroAtk = rollStat(gs->hero.atk);
            int rolledMonsterDef = rollStat(gs->monsters[monsterCode].def);
            int heroDamage = rolledHeroAtk - rolledMonsterDef;

            if (heroDamage <= 0) {
                printf("\n> Você atacou com %s, mas %s bloqueou o golpe!\n", 
                       gs->hero.atkItem, gs->monsters[monsterCode].name);
            } else {
                bool isCrit = (rand() % 10 == 0);
                if (isCrit) {
                    heroDamage = (heroDamage * 3 + 1) / 2;
                    printf("\n> GOLPE CRÍTICO! Você atacou com %s causando %i de dano!\n", 
                           gs->hero.atkItem, heroDamage);
                } else {
                    printf("\n> Você atacou com %s causando %i de dano!\n", 
                           gs->hero.atkItem, heroDamage);
                }
                gs->monsters[monsterCode].hp -= heroDamage;
            }

            // Monster counter-attack
            if (gs->monsters[monsterCode].hp > 0) {
                int rolledMonsterAtk = rollStat(gs->monsters[monsterCode].atk);
                int rolledHeroDef = rollStat(gs->hero.def);
                int monsterDamage = rolledMonsterAtk - rolledHeroDef;

                if (monsterDamage <= 0) {
                    printf("> %s te atacou, mas seu escudo bloqueou o golpe!\n\n", 
                           gs->monsters[monsterCode].name);
                } else {
                    gs->hero.hp -= monsterDamage;
                    printf("> %s te atacou causando %i de dano!\n\n", 
                           gs->monsters[monsterCode].name, monsterDamage);
                }
                
                if (gs->hero.hp <= 0) {
                    clearScreen();
                    printf("VOCÊ MORREU PARA %s NO PISO %i\n\a", gs->monsters[monsterCode].name, gs->currentFloor);
                    printf("Pontuação: %i\n", gs->score);
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
                printf("> %s tentou te atacar pelas costas, mas seu escudo bloqueou o golpe!\n", 
                       gs->monsters[monsterCode].name);
            } else {
                gs->hero.hp -= monsterDamage;
                printf("> %s te atingiu pelas costas causando %i de dano!\n", 
                       gs->monsters[monsterCode].name, monsterDamage);
            }
            
            if (gs->hero.hp <= 0) {
                clearScreen();
                printf("VOCÊ MORREU PARA %s TENTANDO FUGIR NO PISO %i\n\a", 
                       gs->monsters[monsterCode].name, gs->currentFloor);
                printf("Pontuação: %i\n", gs->score);
                return true;
            }

            printf("> Você conseguiu escapar com vida pelos corredores!\n\n");
            return false;
        } else if (action == 3) {
            if (gs->hero.potions <= 0) {
                printf("\n> Você não tem poções de cura guardadas!\n\n");
            } else if (gs->hero.hp >= gs->hero.maxHp) {
                printf("\n> Sua saúde já está no máximo!\n\n");
            } else {
                gs->hero.potions--;
                int heal = gs->hero.maxHp / 2;
                if (gs->hero.hp + heal > gs->hero.maxHp) {
                    heal = gs->hero.maxHp - gs->hero.hp;
                }
                gs->hero.hp += heal;
                printf("\n> Você bebeu uma poção de cura e recuperou %i de vida! (Restam: %i)\n\n", 
                       heal, gs->hero.potions);
            }
        } else {
            printf("> Comando não encontrado.\n\n");
        }
    } while (gs->monsters[monsterCode].hp > 0);

    if (isElite) {
        int eliteCoins = (gs->monsters[monsterCode].coins * 15 + 5) / 10;
        gs->score += 250;
        gs->hero.coins += eliteCoins;
        printf("> Você derrotou %s!\n> Você encontrou %i moedas!\n", 
               gs->monsters[monsterCode].name, eliteCoins);
        if (rand() % 10 < 4) {
            gs->hero.potions++;
            printf("> O monstro elite derrubou uma poção de cura! (Você tem %i)\n\n", gs->hero.potions);
        } else {
            printf("\n");
        }
    } else {
        gs->score += 100;
        gs->hero.coins += gs->monsters[monsterCode].coins;
        printf("> Você derrotou %s!\n> Você encontrou %i moedas!\n\n", 
               gs->monsters[monsterCode].name, gs->monsters[monsterCode].coins);
    }
    
    return false;
}

// ================= [ EVENTS & SHOPS ] ====================================== //

void printShopBox(GameState *gs, int shopCode, bool item1Available, bool item2Available) {
    printf("+----------------- [ LOJA ] -----------------+\n");
    printf("|   Moedas: %i\n", gs->hero.coins);
    printf("+--------------------------------------------+\n");
    printf("|   [1] Sair da loja\n");
    if (item1Available) {
        printf("|   [2] %s - $%i\n", gs->shops[shopCode].item1Name, gs->shops[shopCode].item1Price);
    } else {
        printf("|   [2] %s - (Comprado)\n", gs->shops[shopCode].item1Name);
    }
    if (item2Available) {
        printf("|   [3] %s - $%i\n", gs->shops[shopCode].item2Name, gs->shops[shopCode].item2Price);
    } else {
        printf("|   [3] %s - (Comprado)\n", gs->shops[shopCode].item2Name);
    }
    if (gs->currentFloor >= 1)
        printf("|   [4] Poção de cura - $50\n");
    if (gs->currentFloor >= 2)
        printf("|   [5] Pingente de vida - $150\n");
    if (gs->hero.potions > 0)
        printf("|   [6] Beber poção de cura (x%i)\n", gs->hero.potions);
    printf("+--------------------------------------------+\n\n");
}

void shopInteraction(GameState *gs, int shopCode) {
    bool item1Available = true;
    bool item2Available = true;
    
    printShopBox(gs, shopCode, item1Available, item2Available);
    
    int action;
    do {
        action = readInt();
        switch (action) {
            case 0: // INFO
                showsInfo(gs, NULL);
                if (!hasPendingInput()) {
                    printShopBox(gs, shopCode, item1Available, item2Available);
                }
                break;
                
            case 1: // LEAVES THE SHOP
                break;
                
            case 2: // BUYS ITEM 1
                if ((item1Available == true) && (gs->hero.coins >= gs->shops[shopCode].item1Price)) {
                    strcpy(gs->hero.atkItem, gs->shops[shopCode].item1Name);
                    printf("\n> Você comprou %s.\n\n", gs->shops[shopCode].item1Name);
                    gs->hero.atk = gs->shops[shopCode].item1atkPoints;
                    item1Available = false;
                    gs->hero.coins -= gs->shops[shopCode].item1Price;
                    gs->score += gs->shops[shopCode].item1Price / 2;
                } else {
                    printf("\n> Impossível realizar a ação.\n\n");
                }
                if (!hasPendingInput()) {
                    printShopBox(gs, shopCode, item1Available, item2Available);
                }
                break;
                
            case 3: // BUYS ITEM 2
                if ((item2Available == true) && (gs->hero.coins >= gs->shops[shopCode].item2Price)) {
                    strcpy(gs->hero.defItem, gs->shops[shopCode].item2Name);
                    printf("\n> Você comprou %s.\n\n", gs->shops[shopCode].item2Name);
                    gs->hero.def = gs->shops[shopCode].item2defPoints;
                    item2Available = false;
                    gs->hero.coins -= gs->shops[shopCode].item2Price;
                    gs->score += gs->shops[shopCode].item2Price / 2;
                } else {
                    printf("\n> Impossível realizar a ação.\n\n");
                }
                if (!hasPendingInput()) {
                    printShopBox(gs, shopCode, item1Available, item2Available);
                }
                break;
                
            case 4: // BUYS POTION
                if ((gs->hero.coins >= 50) && (gs->currentFloor >= 1)) {
                    gs->hero.potions++;
                    gs->hero.coins -= 50;
                    gs->score += 20;
                    printf("\n> Você comprou uma poção de cura. (Você tem %i)\n\n", gs->hero.potions);
                } else if (gs->currentFloor >= 1) {
                    printf("\n> Impossível realizar a ação.\n\n");
                } else {
                    printf("\n> Comando não encontrado.\n\n");
                }
                if (!hasPendingInput()) {
                    printShopBox(gs, shopCode, item1Available, item2Available);
                }
                break;
                
            case 5: // BUYS PENDANT
                if (gs->hero.coins >= 150 && gs->currentFloor >= 2) {
                    gs->hero.coins -= 150;
                    gs->hero.maxHp += 50;
                    gs->hero.hp += 50;
                    gs->score += 150;
                    printf("\n> Você comprou o pingente de vida! Você ganhou +50 de vida máxima! Saúde: %i/%i\n\n", gs->hero.hp, gs->hero.maxHp);
                } else if (gs->currentFloor >= 2) {
                    printf("\n> Impossível realizar a ação.\n\n");
                } else {
                    printf("\n> Comando não encontrado.\n\n");
                }
                if (!hasPendingInput()) {
                    printShopBox(gs, shopCode, item1Available, item2Available);
                }
                break;
                
            case 6: // DRINKS POTION IN SHOP
                if (gs->hero.potions > 0 && gs->hero.hp < gs->hero.maxHp) {
                    gs->hero.potions--;
                    int heal = gs->hero.maxHp / 2;
                    if (gs->hero.hp + heal > gs->hero.maxHp) heal = gs->hero.maxHp - gs->hero.hp;
                    gs->hero.hp += heal;
                    printf("\n> Você bebeu uma poção de cura e recuperou %i de vida! Saúde: %i/%i\n\n", heal, gs->hero.hp, gs->hero.maxHp);
                } else if (gs->hero.potions > 0 && gs->hero.hp >= gs->hero.maxHp) {
                    printf("\n> Sua saúde já está no máximo!\n\n");
                } else {
                    printf("\n> Impossível realizar a ação.\n\n");
                }
                if (!hasPendingInput()) {
                    printShopBox(gs, shopCode, item1Available, item2Available);
                }
                break;
                
            default:
                printf("\n> Comando não encontrado.\n\n");
                if (!hasPendingInput()) {
                    printShopBox(gs, shopCode, item1Available, item2Available);
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
            printf("Lojista: Então você quer se aventurar pela terrível masmorra, %s?\n", gs->hero.name);
            printf("Se fosse você eu não iria de mãos atadas...\n");
            break;

        case 1:
            printf("> Você encontra um vendedor ambulante, carregando itens em seu carro de mão.\n");
            printf("Vendedor ambulante: O que estou fazendo aqui? Ah, é só um passeio de negócios.\n");
            printf("Você acabou de matar potenciais compradores. Você devia cobrir meu prejuízo...\n\n");
            break;

        case 2:
            printf("> Você encontra uma pequena venda de um goblin morador local.\n");
            printf("Lojista goblin: Levar o que quiser, só por favor não me matar!\n");
            printf("(Seu código moral te impede de sair sem pagar)\n\n");
            break;

        case 3:
            printf("> Você encontra um troll ferreiro, forjando ferramentas com os minérios.\n");
            printf("Ferreiro: Uai, um humano por essas banda? Trem raro de se vê...\n");
            printf("Posso arranjá umas lembrancinha procê em troca dessas sua moeda de ouro.\n\n");
            break;

        case 4:
            printf("> Um misterioso ser encapuzado aparece das sombras.\n");
            printf("Ser misterioso: Então és tu %c destemid%c %s, que ousas desafiar o temível Xãopai?\n", 
                   gs->pronoum, gs->pronoum, gs->hero.name);
            printf("Acautela-te, jovem guerreir%c, pois o percurso adiante é sombrio e traiçoeiro.\n\n", 
                   gs->pronoum);
            break;

        case 5:
            printf("> Você encontra uma hedionda figura de carne e tentáculos.\n");
            printf("Criatura: (A criatura encara os mais obscuros cantos da sua alma)\n");
            printf("(Você se sente extremamente desconfortável enquanto compra)\n\n");
            break;
    }
}

void dungeonDialog(GameState *gs) {
    printf("\n================================================================================\n");
    switch (gs->currentFloor) {
        case 0:
            printf("                                   [ CIDADE ]                                   \n");
            printf("================================================================================\n\n");
            printf("  Diversos aventureiros da cidade acima adentraram a masmorra antes de você,\n");
            printf("  mas poucos foram vistos novamente. Diz-se que um mal ancestral espreita nas\n");
            printf("  profundezas, o imensamente poderoso espírito Xãopai.\n");
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
            printf("  poderoso Xãopai. Você sente como se fosse passar por uns maus tempos.\n");
            break;
    }
    printf("\n================================================================================\n\n");
}

void printsBadEnding(GameState *gs) {
    printf("> Você toma o amuleto para si.\n");
    printf("> Você absorve o poder do amuleto, e consegue sentir a escuridão fluir pelo seu corpo.\n");
    printf("> Você tenta resistir à magia, mas ela te domina e você acaba perdendo sua noção \nda realidade.\n");
    printf("> O que restou de você agora reinará no trono como %c nov%c Xãopai.\n", gs->pronoum, gs->pronoum);
    printf("|FINAL RUIM|\n");
}

void printsGoodEnding() {
    printf("> Você destrói o amuleto.\n");
    printf("> Você ouve um estridente ruído vindo do amuleto enquanto vê toda a energia maligna \nse dissipar no ar.\n");
    printf("> Você finalmente libertou a masmorra e os habitantes da cidade das \nmãos do mal!\n");
    printf("|FINAL BOM|\n");
}

void printsTrueEnding(GameState *gs) {
    printf("> Você recita a sequência mágica ao amuleto.\n");
    printf("> O amuleto brilha em luz vermelha ofuscante enquanto muda sua forma para um esbelto garoto.\n");
    printf("> Paixão: Uh... Onde eu estou? Eu estava programando um jogo e... acho que dormi no teclado.\n");
    printf("> Paixão: Vem, %s, vamo dar um fora daqui. Ficar encarando essa tela de terminal \ntá me dando dor de cabeça...\n", gs->hero.name);
    printf("|FINAL VERDADEIRO|\n");
}

void printsCowardEnding(GameState *gs) {
    printf("\n> Por um milagre do destino, você encontra uma brecha e consegue escapar de XÃOPAI!\n");
    printf("> Você corre desesperadamente escadaria acima sem olhar para trás, abandonando a masmorra.\n");
    printf("> A cidade acima eventualmente sucumbe à escuridão, mas pelo menos você salvou a própria pele...\n");
    printf("> Você sobreviveu, mas será para sempre lembrad%c como %c maior covarde da história, %s.\n", 
           gs->pronoum, gs->pronoum, gs->hero.name);
    printf("|FINAL COVARDE|\n");
    printf("Pontuação: %i\n\n", gs->score);
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

    printf("> Você encontrou um baú!\n");
    printf("[1] Chutar violentamente\n");
    printf("[2] Ignorar\n");

    int action;
    do {
        action = readInt();
        switch(action) {
            case 0:
                showsInfo(gs, NULL);
                printf("|Baú|\n");
                printf(" ???\n\n");
                break;

            case 1:
                break;

            case 2:
                printf("\n> Você decidiu não arriscar e seguiu em frente.\n\n");
                return false;

            default:
                printf("> Comando não encontrado.\n\n");
                break;
        }
    } while(action != 1);

    printf("\n> O Baú era um mímico!\n\n");

    bool fled = false;
    do {
        printf("[1] Atacar\n");
        printf("[2] Fugir\n");
        if (gs->hero.potions > 0)
            printf("[3] Usar poção de cura (x%i)\n", gs->hero.potions);
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
                    printf("\n> Você atacou o %s, mas ele repeliu seu golpe!\n", mimic.name);
                } else {
                    bool isCrit = (rand() % 10 == 0);
                    if (isCrit) {
                        heroDamage = (heroDamage * 3 + 1) / 2;
                        printf("\n> GOLPE CRÍTICO! Você atacou o %s causando %i de dano!\n", mimic.name, heroDamage);
                    } else {
                        printf("\n> Você atacou o %s causando %i de dano!\n", mimic.name, heroDamage);
                    }
                    mimic.hp -= heroDamage;
                }

                if (mimic.hp > 0) {
                    int rolledMimicAtk = rollStat(mimic.atk);
                    int rolledHeroDef = rollStat(gs->hero.def);
                    int mimicDamage = rolledMimicAtk - rolledHeroDef;

                    if (mimicDamage <= 0) {
                        printf("> %s tentou te morder, mas sua defesa bloqueou!\n\n", mimic.name);
                    } else {
                        gs->hero.hp -= mimicDamage;
                        printf("> %s te mordeu causando %i de dano!\n\n", mimic.name, mimicDamage);
                    }
                    if (gs->hero.hp <= 0) {
                        clearScreen();
                        printf("VOCÊ MORREU PARA %s NO PISO %i\n\a", mimic.name, gs->currentFloor);
                        printf("Pontuação: %i\n", gs->score);
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
                    printf("> %s tentou te morder pelas costas, mas você conseguiu se esquivar!\n", mimic.name);
                } else {
                    gs->hero.hp -= mimicDamage;
                    printf("> %s te mordeu pelas costas causando %i de dano!\n", mimic.name, mimicDamage);
                }
                if (gs->hero.hp <= 0) {
                    clearScreen();
                    printf("VOCÊ MORREU PARA %s TENTANDO FUGIR NO PISO %i\n\a", mimic.name, gs->currentFloor);
                    printf("Pontuação: %i\n", gs->score);
                    return true;
                }
                printf("> Você conseguiu escapar com vida pelos corredores!\n\n");
                fled = true;
                mimic.hp = 0;
                break;
            }
            case 3:
                if (gs->hero.potions <= 0) {
                    printf("\n> Você não tem poções de cura guardadas!\n\n");
                } else if (gs->hero.hp >= gs->hero.maxHp) {
                    printf("\n> Sua saúde já está no máximo!\n\n");
                } else {
                    gs->hero.potions--;
                    int heal = gs->hero.maxHp / 2;
                    if (gs->hero.hp + heal > gs->hero.maxHp) heal = gs->hero.maxHp - gs->hero.hp;
                    gs->hero.hp += heal;
                    printf("\n> Você bebeu uma poção de cura e recuperou %i de vida! (Restam: %i)\n\n", heal, gs->hero.potions);
                }
                break;
            default:
                printf("> Comando não encontrado.\n\n");
                break;
        }
    } while(mimic.hp > 0);

    if (!fled) {
        gs->score += mimic.coins;
        gs->hero.coins += mimic.coins;
        printf("> Você derrotou o %s!\n", mimic.name);
        printf("> Você encontrou %d moedas!\n\n", mimic.coins);

        if (*chestOpened == false) {
            gs->secretNumber = rand() % 900 + 100;
            printf("> Você encontra uma marcação suspeita gravada no fundo do baú: %i\n\n", gs->secretNumber);
            *chestOpened = true;
        }
    }

    return false;
}

bool xaopaiInteraction(GameState *gs) {
    printf("> Você desafia o terrível Xãopai!\n\n");
    gs->monsters[XAOPAI].hp = gs->monsters[XAOPAI].maxHp;
    
    do {
        printf("[1] Atacar\n");
        printf("[2] Fugir\n");
        if (gs->hero.potions > 0)
            printf("[3] Usar poção de cura (x%i)\n", gs->hero.potions);
        int action = readInt();
        switch(action) {
            case 0: // INFO
                showsInfo(gs, &gs->monsters[XAOPAI]);
                break;

            case 1: { // ATTACK
                int heroDamage = rollStat(gs->hero.atk);
                bool isCrit = (rand() % 10 == 0);
                if (isCrit) {
                    heroDamage = (heroDamage * 3 + 1) / 2;
                    printf("\n> GOLPE CRÍTICO! Você atacou XÃOPAI com %s causando %i de dano!\n", 
                           gs->hero.atkItem, heroDamage);
                } else {
                    printf("\n> Você atacou XÃOPAI com %s causando %i de dano!\n", 
                           gs->hero.atkItem, heroDamage);
                }
                gs->monsters[XAOPAI].hp -= heroDamage;
                if (gs->monsters[XAOPAI].hp > 0) {
                    int xaopaiDamage = rollStat(gs->monsters[XAOPAI].atk);
                    gs->hero.hp -= xaopaiDamage;
                    printf("> XÃOPAI revidou com poder sombrio causando %i de dano!\n\n", 
                           xaopaiDamage);
                    if (gs->hero.hp <= 0) {
                        clearScreen();
                        printf("VOCÊ PERECEU DIANTE DE XÃOPAI.\n\a");
                        printf("Pontuação: %i\n", gs->score);
                        return true;
                    }
                }
                break;
            }

            case 2: { // FLEE
                if (rand() % 10 == 0) {
                    gs->fledXaopai = true;
                    return false;
                }
                printf("\n> Não há como fugir do poder avassalador de XÃOPAI!\n");
                int xaopaiDamage = rollStat(gs->monsters[XAOPAI].atk);
                gs->hero.hp -= xaopaiDamage;
                printf("> XÃOPAI te pune pela covardia causando %i de dano!\n\n", xaopaiDamage);
                if (gs->hero.hp <= 0) {
                    clearScreen();
                    printf("VOCÊ PERECEU TENTANDO FUGIR DE XÃOPAI.\n\a");
                    printf("Pontuação: %i\n", gs->score);
                    return true;
                }
                break;
            }

            case 3: // POTION
                if (gs->hero.potions <= 0) {
                    printf("\n> Você não tem poções de cura guardadas!\n\n");
                } else if (gs->hero.hp >= gs->hero.maxHp) {
                    printf("\n> Sua saúde já está no máximo!\n\n");
                } else {
                    gs->hero.potions--;
                    int heal = gs->hero.maxHp / 2;
                    if (gs->hero.hp + heal > gs->hero.maxHp) heal = gs->hero.maxHp - gs->hero.hp;
                    gs->hero.hp += heal;
                    printf("\n> Você bebeu uma poção de cura e recuperou %i de vida! (Restam: %i)\n\n", heal, gs->hero.potions);
                }
                break;

            default:
                printf("> Comando não encontrado.\n\n");
                break;
        }
    } while(gs->monsters[XAOPAI].hp > 0);

    gs->score += 10000;
    printf("> VOCÊ DERROTOU O ESPÍRITO DE XÃOPAI!\n\n");
    return false;
}

void ending(GameState *gs) {
    printf("> Você atravessa a fera com a sua espada e ela é reduzida à poeira e cinzas.\n");
    printf("> Você apanha um amuleto vermelho-sangue em meio aos restos da criatura\n");
    printf("> O amuleto pulsa vigorosamente em suas mãos. Você pode sentir um imenso poder \nsaindo dele.\n");
    printf("> O que você faz?\n");
    printf("[1] Inspecionar amuleto\n");
    printf("[2] Se apossar do amuleto de Xãopai\n");
    printf("[3] Destruir o amuleto de Xãopai\n");

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
    printf("Pontuação: %i\n\n", gs->score);
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
    printf("                - HIGH SCORES -                \n\n");

    for (int i = 0; i < HIGHSCORES_NUM - 1; ++i)
        if (highscores[i].score != 0)
            printf("            %2i - %-18s %5i\n", i + 1, highscores[i].name, highscores[i].score);

    printf("\n         (1) Jogar novamente\n");
    printf("         (2) Sair do jogo\n");

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
                printf("\n         (1) Jogar novamente\n");
                printf("         (2) Sair do jogo\n");
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
                printf("> Comando não encontrado.\n\n");
                break;
        }
    } while(input != 1);
    
    return 0;
}

// ================= [ DUNGEON LOOP ] ========================================= //

void playDungeon(GameState *gs) {
    int monsterCode = 0;
    bool chestOpened = false;

    for (int floor = 0; floor < 5; ++floor) {
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
    gs->monsters[XAOPAI].maxHp = 1750 - gs->hero.def * 3;
    gs->isDead = xaopaiInteraction(gs);
    
    if (gs->isDead)
        return;
    else if (gs->fledXaopai)
        printsCowardEnding(gs);
    else
        ending(gs);
}

// ================= [ MAIN LOOP ] ============================================ //

int main() {
    setlocale(LC_ALL, ".UTF8");

    #ifdef _WIN32
        SetConsoleOutputCP(CP_UTF8);
        SetConsoleCP(CP_UTF8);
    #endif

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
            printf("Lojista: Você deve ser nov%c por aqui. Qual seu nome, viajante?\n", gs.pronoum);
            readString(heroName, STR_LENGTH);
        }
        strcpy(gs.hero.name, heroName);
        
        char lowerName[STR_LENGTH];
        strcpy(lowerName, gs.hero.name);
        for (int i = 0; lowerName[i]; i++) {
            lowerName[i] = tolower(lowerName[i]);
        }
        if (strcmp(lowerName, "paixao") == 0 || strcmp(lowerName, "paixão") == 0) {
            printf("Lojista: Olha só! O próprio criador do jogo! Bom rever você.\n");
            gs.hero.coins += 9990;
        }
        
        shopDialog(&gs);
        
        printf("\n+---------------------------------------------------------------------------+\n");
        printf(  "| DICA: Digite '0' a qualquer momento para mostrar status do herói/oponente |\n");
        printf(  "+---------------------------------------------------------------------------+\n\n");
        
        shopInteraction(&gs, gs.currentFloor);
        
        playDungeon(&gs);
        
        if (!gs.isDead)
            gamesWon++;
        else
            printf("\7");
        
        updatesHighscores(&gs, highscores);
        
        printf("[1] Jogar novamente\n[2] Highscores\n[3] Sair do jogo\n");
        
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
                        printf("\n[1] Jogar novamente\n[2] Highscores\n[3] Sair do jogo\n");
                    }
                    break;
                }
                    
                default:
                    printf("> Comando não encontrado.\n\n");
                    break;
            }
        } while((action != 1) && (action != 3));
        
    } while(action != 3);
    
    return PAIXAO;
}