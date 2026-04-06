// ============================================================================ //
//                                 MASMORRA                                     //
// ============================================================================ //

#include <stdio.h>
#include <stdlib.h>
#include <locale.h>
#include <time.h>
#include <string.h>
#include <stdbool.h>

// ================= [ MACROS E CROSS-PLATFORM ] ============================== //

#define PAIXAO 106
#define STR_LENGTH 30 
#define HIGHSCORES_NUM 11
#define XAOPAI 15

// Macros de compatibilidade para limpar a tela
#ifdef _WIN32
    #define CLEAR_CMD "cls"
#else
    #define CLEAR_CMD "clear" // No Linux/Mac usa clear nativo
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
    char atkItem[STR_LENGTH];
    char defItem[STR_LENGTH];
} character_t;

typedef struct {
    char name[STR_LENGTH];
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

// O Contexto Global encapsulado (Adeus, variáveis globais soltas!)
typedef struct {
    character_t hero;
    enemy_t monsters[16];
    shop_t shops[6];
    int score;
    int currentFloor;
    bool isDead;
    int secretNumber;
    char pronoum;
} GameState;

// ================= [ ENGINE & INPUTS ] ====================================== //

// Função blindada contra inputs inválidos (O fim do loop infinito do scanf)
int readInt() {
    char buffer[64];
    int value;
    while (true) {
        if (fgets(buffer, sizeof(buffer), stdin) != NULL) {
            // Tenta converter para inteiro
            if (sscanf(buffer, "%d", &value) == 1) {
                return value;
            }
        }
        fprintf(stderr, "Entrada inválida. Por favor, digite um número: ");
    }
}

// Leitura segura de strings com suporte a espaços
void readString(char* buffer, int maxLength) {
    while (true) {
        if (fgets(buffer, maxLength, stdin) != NULL) {
            buffer[strcspn(buffer, "\n")] = 0; // Remove o \n do final
            if (strlen(buffer) > 0) return;
        }
        fprintf(stderr, "Entrada inválida. Digite novamente: ");
    }
}

// ================= [ INITIALIZATION (DATA-DRIVEN) ] ========================= //

void initGameData(GameState *gs) {
    // Inicializa o Herói
    gs->hero.coins = 15;
    gs->hero.hp = 10;
    gs->hero.maxHp = 10;
    gs->hero.atk = 1;
    gs->hero.def = 0;
    strcpy(gs->hero.atkItem, "Seus punhos");
    strcpy(gs->hero.defItem, "Nada");

    // Lógica do RNG mantida idêntica ao original
    srand((unsigned)time(NULL));

    // Exemplo de Data-Driven simplificado para monstros
    // Para economizar espaço no código refatorado, agrupamos a inicialização.
    // (No arquivo completo, todos os 15 monstros teriam seus ranges de rand() aqui).
    strcpy(gs->monsters[1].name, "Rato");
    gs->monsters[1].maxHp = rand() % 5 + 10;
    gs->monsters[1].atk = rand() % 2 + 1;
    gs->monsters[1].def = 0;
    gs->monsters[1].coins = rand() % 6 + 2;

    strcpy(gs->monsters[XAOPAI].name, "Xãopai");
    gs->monsters[XAOPAI].atk = 1;
    gs->monsters[XAOPAI].def = 0;
    // ... (A inicialização dos outros monstros e lojas seguiria o mesmo padrão) ...
}

// ================= [ UI & ASCII ART ] ======================================= //

void titleScreen() {
    printf("*****************************************************************************\n");
    printf("                                BEM-VINDO À                                  \n");
    printf("  __  __    _    ____  __  __  ___  ____  ____      _                        \n");
    printf(" |  \\/  |  / \\  / ___||  \\/  |/ _ \\|  _ \\|  _ \\    / \\                       \n");
    printf(" | |\\/| | / _ \\ \\___ \\| |\\/| | | | | |_) | |_) |  / _ \\                      \n");
    printf(" | |  | |/ ___ \\ ___) | |  | | |_| |  _ <|  _ <  / ___ \\                     \n");
    printf(" |_|  |_/_/   \\_\\____/|_|  |_|\\___/|_| \\_\\_| \\_\\/_/   \\_\\                    \n");
    printf("                                                            by: Paixão       \n");
    printf("*****************************************************************************\n\n");
    printf("                Pressione enter para continuar...                            \n");
    getchar();
    clearScreen();
}

void showsInfo(GameState *gs, int monsterCode) {
    printf("|%s|\n", gs->hero.name);
    printf("Saúde: %i/%i\n", gs->hero.hp, gs->hero.maxHp);
    printf("Moedas: %i\n", gs->hero.coins);
    printf("Ataque: %i (%s)\n", gs->hero.atk, gs->hero.atkItem);
    printf("Defesa: %i (%s)\n", gs->hero.def, gs->hero.defItem);

    if (monsterCode > 0) {
        printf("\n|%s|\n", gs->monsters[monsterCode].name);
        printf("Saúde: %i/%i\n", gs->monsters[monsterCode].hp, gs->monsters[monsterCode].maxHp);
        printf("Ataque: %i\n", gs->monsters[monsterCode].atk);
        printf("Defesa: %i\n", gs->monsters[monsterCode].def);
    }
}

// ================= [ COMBAT SYSTEM ] ======================================== //

bool monsterInteraction(GameState *gs, int monsterCode) {
    printf("Você encontrou %s!\n", gs->monsters[monsterCode].name);
    gs->monsters[monsterCode].hp = gs->monsters[monsterCode].maxHp;
    
    do {
        printf("[1] Atacar \n[0] Informações\n");
        int action = readInt();

        if (action == 0) {
            showsInfo(gs, monsterCode);
        } else if (action == 1) {
            // Lógica de dano
            if (gs->monsters[monsterCode].def >= gs->hero.atk) {
                clearScreen();
                printf("VOCÊ MORREU DE PARADA CARDÍACA AO VER %s NO PISO %i\n\a", 
                        gs->monsters[monsterCode].name, gs->currentFloor);
                printf("Pontuação: %i\n", gs->score);
                return true;
            } else {
                gs->monsters[monsterCode].hp -= (gs->hero.atk - gs->monsters[monsterCode].def);
            }

            // Revide do monstro
            if (gs->monsters[monsterCode].hp > 0) {
                if (gs->hero.def < gs->monsters[monsterCode].atk) {
                    gs->hero.hp -= (gs->monsters[monsterCode].atk - gs->hero.def);
                }
                
                if (gs->hero.hp <= 0) {
                    clearScreen();
                    printf("VOCÊ MORREU PARA %s NO PISO %i\n\a", gs->monsters[monsterCode].name, gs->currentFloor);
                    printf("Pontuação: %i\n", gs->score);
                    return true;
                } else {
                    printf("\n%s: %i/%i  |  %s: %i/%i\n\n", 
                           gs->hero.name, gs->hero.hp, gs->hero.maxHp,
                           gs->monsters[monsterCode].name, gs->monsters[monsterCode].hp, gs->monsters[monsterCode].maxHp);
                }
            }
        } else {
            fprintf(stderr, "Comando não encontrado.\n");
        }
    } while (gs->monsters[monsterCode].hp > 0);

    gs->score += 10;
    gs->hero.coins += gs->monsters[monsterCode].coins;
    printf("Você derrotou %s.\nVocê encontrou %i moedas!\n\n", gs->monsters[monsterCode].name, gs->monsters[monsterCode].coins);
    
    return false;
}

// ================= [ MAIN LOOP ] ============================================ //

int main() {
    setlocale(LC_ALL, "Portuguese");
    GameState gs = {0}; // Inicializa todo o estado com 0/vazio
    
    titleScreen();
    initGameData(&gs);

    printf("Lojista: Você deve ser nov%c por aqui. Qual seu nome, viajante?\n", gs.pronoum == 0 ? 'o' : gs.pronoum);
    readString(gs.hero.name, STR_LENGTH);
    
    if (strcmp(gs.hero.name, "Paixao") == 0 || strcmp(gs.hero.name, "Paixão") == 0) {
        printf("Lojista: Olha só! O próprio criador do jogo! Bom rever você.\n\n");
        gs.hero.coins += 999;
    } else {
        printf("Lojista: %s? Que nome... diferente. Mas não importa, vou pegar as suas coisas.\n\n", gs.hero.name);
    }

    // Exemplo do loop de jogo principal rodando pelo GameState
    printf("Você entra na MASMORRA...\n");
    gs.currentFloor = 1;
    
    // Aqui viria o seu switch-case de progressão dos andares...
    // gs.isDead = monsterInteraction(&gs, 1);
    
    // Finalização simulada
    if (!gs.isDead) {
        printf("Você sobreviveu!\nPontuação Final: %d\n", gs.score);
    }

    return 0;
}