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

// O Contexto Global encapsulado
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

// Função blindada contra inputs inválidos
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
        fprintf(stderr, "Entrada inválida.\n");
    }
}

// Leitura segura de strings com suporte a espaços
void readString(char* buffer, int maxLength) {
    while (true) {
        if (fgets(buffer, maxLength, stdin) != NULL) {
            buffer[strcspn(buffer, "\n")] = 0; // Remove o \n do final
            if (strlen(buffer) > 0) return;
        }
        fprintf(stderr, "Entrada inválida.\n");
    }
}

// ================= [ INITIALIZATION (DATA-DRIVEN) ] ========================= //

void initGameData(GameState *gs) {
    // Pronoun
    gs->pronoum = 'o';

    // Inicializa o Herói
    gs->hero.coins = 15;
    gs->hero.hp = 10;
    gs->hero.maxHp = 10;
    gs->hero.atk = 1;
    gs->hero.def = 0;
    strcpy(gs->hero.atkItem, "Seus punhos");
    strcpy(gs->hero.defItem, "Nada");

    // Lógica do RNG
    srand((unsigned)time(NULL));

    strcpy(gs->monsters[0].name, "Ratazana gigante");
    srand(clock()); gs->monsters[0].maxHp = rand() % 2 + 3;
    srand(clock()); gs->monsters[0].atk = rand() % 3 + 1;
    srand(clock()); gs->monsters[0].def = rand() % 1 + 0;
    srand(clock()); gs->monsters[0].coins = rand() % 5 + 5;

    strcpy(gs->monsters[1].name, "Cobra");
    srand(clock()); gs->monsters[1].maxHp = rand() % 1 + 3;
    srand(clock()); gs->monsters[1].atk = rand() % 2 + 2;
    srand(clock()); gs->monsters[1].def = rand() % 1 + 0;
    srand(clock()); gs->monsters[1].coins = rand() % 5 + 5;

    strcpy(gs->monsters[2].name, "Gosma gigante");
    srand(clock()); gs->monsters[2].maxHp = rand() % 2 + 3;
    srand(clock()); gs->monsters[2].atk = rand() % 2 + 2;
    srand(clock()); gs->monsters[2].def = rand() % 1 + 0;
    srand(clock()); gs->monsters[2].coins = rand() % 5 + 5;

    strcpy(gs->monsters[3].name, "Ladrão louco");
    srand(clock()); gs->monsters[3].maxHp = rand() % 2 + 3;
    srand(clock()); gs->monsters[3].atk = rand() % 3 + 2;
    srand(clock()); gs->monsters[3].def = rand() % 2 + 0;
    srand(clock()); gs->monsters[3].coins = rand() % 5 + 6;

    strcpy(gs->monsters[4].name, "Esqueleto");
    srand(clock()); gs->monsters[4].maxHp = rand() % 1 + 3;
    srand(clock()); gs->monsters[4].atk = rand() % 2 + 3;
    srand(clock()); gs->monsters[4].def = rand() % 2 + 0;
    srand(clock()); gs->monsters[4].coins = rand() % 5 + 7;

    strcpy(gs->monsters[5].name, "Guarda da prisão");
    srand(clock()); gs->monsters[5].maxHp  = rand() % 2 + 3;
    srand(clock()); gs->monsters[5].atk = rand() % 2 + 3;
    srand(clock()); gs->monsters[5].def = rand() % 2 + 0;
    srand(clock()); gs->monsters[5].coins = rand() % 5 + 7;

    strcpy(gs->monsters[6].name, "Morcego vampiro");
    srand(clock()); gs->monsters[6].maxHp = rand() % 2 + 4;
    srand(clock()); gs->monsters[6].atk = rand() % 2 + 2;
    srand(clock()); gs->monsters[6].def = rand() % 2 + 1;
    srand(clock()); gs->monsters[6].coins = rand() % 5 + 10;

    strcpy(gs->monsters[7].name, "Aranha gigante");
    srand(clock()); gs->monsters[7].maxHp = rand() % 2 + 5;
    srand(clock()); gs->monsters[7].atk = rand() % 2 + 3;
    srand(clock()); gs->monsters[7].def = rand() % 2 + 1;
    srand(clock()); gs->monsters[7].coins = rand() % 5 + 10;

    strcpy(gs->monsters[8].name, "Troll selvagem");
    srand(clock()); gs->monsters[8].maxHp = rand() % 3 + 5;
    srand(clock()); gs->monsters[8].atk = rand() % 1 + 4;
    srand(clock()); gs->monsters[8].def = rand() % 1 + 2;
    srand(clock()); gs->monsters[8].coins = rand() % 5 + 10;

    strcpy(gs->monsters[9].name, "Soldado anão");
    srand(clock()); gs->monsters[9].maxHp = rand() % 2 + 6;
    srand(clock()); gs->monsters[9].atk = rand() % 3 + 4;
    srand(clock()); gs->monsters[9].def = rand() % 2 + 1;
    srand(clock()); gs->monsters[9].coins = rand() % 6 + 13;

    strcpy(gs->monsters[10].name, "Monge anão");
    srand(clock()); gs->monsters[10].maxHp = rand() % 2 + 4;
    srand(clock()); gs->monsters[10].atk = rand() % 2 + 5;
    srand(clock()); gs->monsters[10].def = rand() % 1 + 3;
    srand(clock()); gs->monsters[10].coins = rand() % 7 + 13;

    strcpy(gs->monsters[11].name, "Bruxo anão");
    srand(clock()); gs->monsters[11].maxHp = rand() % 2 + 3;
    srand(clock()); gs->monsters[11].atk = rand() % 3 + 5;
    srand(clock()); gs->monsters[11].def = rand() % 1 + 3;
    srand(clock()); gs->monsters[11].coins = rand() % 7 + 14;

    strcpy(gs->monsters[12].name, "Demônio estripador");
    srand(clock()); gs->monsters[12].maxHp = rand() % 2 + 7;
    srand(clock()); gs->monsters[12].atk = rand() % 2 + 6;
    srand(clock()); gs->monsters[12].def = rand() % 3 + 2;
    srand(clock()); gs->monsters[12].coins = rand() % 10 + 20;

    strcpy(gs->monsters[13].name, "Succubus");
    srand(clock()); gs->monsters[13].maxHp = rand() % 3 + 7;
    srand(clock()); gs->monsters[13].atk = rand() % 1 + 6;
    srand(clock()); gs->monsters[13].def = rand() % 3 + 2;
    srand(clock()); gs->monsters[13].coins = rand() % 10 + 20;

    strcpy(gs->monsters[14].name, "Cão demoníaco");
    srand(clock()); gs->monsters[14].maxHp = rand() % 2 + 8;
    srand(clock()); gs->monsters[14].atk = rand() % 2 + 6;
    srand(clock()); gs->monsters[14].def = rand() % 2 + 3;
    srand(clock()); gs->monsters[14].coins = rand() % 10 + 20;

    strcpy(gs->monsters[XAOPAI].name, "XÃOPAI");
    gs->monsters[XAOPAI].atk = 1;
    gs->monsters[XAOPAI].def = 0;

    // Inicializa lojas
    strcpy(gs->shops[0].item1Name, "Bastão de madeira");
    gs->shops[0].item1atkPoints = 2;
    gs->shops[0].item1Price = 10;
    strcpy(gs->shops[0].item2Name, "Escudo pequeno de madeira");
    gs->shops[0].item2defPoints = 1;
    gs->shops[0].item2Price = 10;

    strcpy(gs->shops[1].item1Name, "Espada curta de cobre");
    gs->shops[1].item1atkPoints = 3;
    gs->shops[1].item1Price = 15;
    strcpy(gs->shops[1].item2Name, "Escudo redondo de cobre");
    gs->shops[1].item2defPoints = 2;
    gs->shops[1].item2Price = 15;

    strcpy(gs->shops[2].item1Name, "Espada de ferro");
    gs->shops[2].item1atkPoints = 4;
    gs->shops[2].item1Price = 20;
    strcpy(gs->shops[2].item2Name, "Escudo de ferro");
    gs->shops[2].item2defPoints = 3;
    gs->shops[2].item2Price = 20;

    strcpy(gs->shops[3].item1Name, "Espada longa de aço");
    gs->shops[3].item1atkPoints = 5;
    gs->shops[3].item1Price = 30;
    strcpy(gs->shops[3].item2Name, "Escudo grande de aço");
    gs->shops[3].item2defPoints = 4;
    gs->shops[3].item2Price = 30;

    strcpy(gs->shops[4].item1Name, "Espada encantada de diamante");
    gs->shops[4].item1atkPoints = 6;
    gs->shops[4].item1Price = 40;
    strcpy(gs->shops[4].item2Name, "Escudo largo de diamante");
    gs->shops[4].item2defPoints = 5;
    gs->shops[4].item2Price = 40;

    strcpy(gs->shops[5].item1Name, "Excalibur Lendária");
    gs->shops[5].item1atkPoints = 8;
    gs->shops[5].item1Price = 55;
    strcpy(gs->shops[5].item2Name, "Égide Mítica");
    gs->shops[5].item2defPoints = 15;
    gs->shops[5].item2Price = 50;
}

// ================= [ UI & ASCII ART ] ======================================= //

void titleScreen() {
    clearScreen();
    printf("********************************************************************************\n");
    printf("                                  BEM-VINDO À                                   \n");
    printf("             __  __    _    ____  __  __  ___  ____  ____      _                \n");
    printf("            |  \\/  |  / \\  / ___||  \\/  |/ _ \\|  _ \\|  _ \\    / \\        \n");
    printf("            | |\\/| | / _ \\ \\___ \\| |\\/| | | | | |_) | |_) |  / _ \\        \n");
    printf("            | |  | |/ ___ \\ ___) | |  | | |_| |  _ <|  _ <  / ___ \\           \n");
    printf("            |_|  |_/_/   \\_\\____/|_|  |_|\\___/|_| \\_\\_| \\_\\/_/   \\_\\   \n");
    printf("                                  by: Paixão                                    \n");
    printf("********************************************************************************\n\n");
    printf("                      Pressione enter para continuar...                         \n");
    getchar();
    clearScreen();
}

void showsInfo(GameState *gs, int monsterCode) {
    printf("> %s\n", gs->hero.name);
    printf("| Saúde: %i/%i\n", gs->hero.hp, gs->hero.maxHp);
    printf("| Moedas: %i\n", gs->hero.coins);
    printf("| Ataque: %i (%s)\n", gs->hero.atk, gs->hero.atkItem);
    printf("| Defesa: %i (%s)\n", gs->hero.def, gs->hero.defItem);

    if (monsterCode > 0) {
        printf("\n");
        printf("> %s\n", gs->monsters[monsterCode].name);
        printf("| Saúde: %i/%i\n", gs->monsters[monsterCode].hp, gs->monsters[monsterCode].maxHp);
        printf("| Ataque: %i\n", gs->monsters[monsterCode].atk);
        printf("| Defesa: %i\n", gs->monsters[monsterCode].def);
    }
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

bool monsterInteraction(GameState *gs, int monsterCode) {
    printf("Você encontrou %s!\n", gs->monsters[monsterCode].name);
    gs->monsters[monsterCode].hp = gs->monsters[monsterCode].maxHp;
    
    do {
        printf("[1] Atacar\n");
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

// ================= [ EVENTS & SHOPS ] ====================================== //

void shopInteraction(GameState *gs, int shopCode) {
    bool item1Available = true;
    bool item2Available = true;
    
    printf("(Você tem %i moedas)\n", gs->hero.coins);
    printf("[1] Sair da loja\n");
    printf("[2] %s - $%i\n", gs->shops[shopCode].item1Name, gs->shops[shopCode].item1Price);
    printf("[3] %s - $%i\n", gs->shops[shopCode].item2Name, gs->shops[shopCode].item2Price);
    if (gs->currentFloor > 1)
        printf("[4] Poção de cura - $5\n");
    if (gs->currentFloor > 2)
        printf("[5] Pingente de vida - $15\n");
    
    int action;
    do {
        action = readInt();
        switch (action) {
            case 0: // INFO
                showsInfo(gs, -1);
                break;
                
            case 1: // LEAVES THE SHOP
                break;
                
            case 2: // BUYS ITEM 1
                if ((item1Available == true) && (gs->hero.coins >= gs->shops[shopCode].item1Price)) {
                    strcpy(gs->hero.atkItem, gs->shops[shopCode].item1Name);
                    printf("Você comprou %s.\n", gs->shops[shopCode].item1Name);
                    gs->hero.atk = gs->shops[shopCode].item1atkPoints;
                    item1Available = false;
                    gs->hero.coins -= gs->shops[shopCode].item1Price;
                    gs->score += gs->shops[shopCode].item1Price / 2;
                } else
                    fprintf(stderr, "Impossível realizar a ação.\n");
                break;
                
            case 3: // BUYS ITEM 2
                if ((item2Available == true) && (gs->hero.coins >= gs->shops[shopCode].item2Price)) {
                    strcpy(gs->hero.defItem, gs->shops[shopCode].item2Name);
                    printf("Você comprou %s.\n", gs->shops[shopCode].item2Name);
                    gs->hero.def = gs->shops[shopCode].item2defPoints;
                    item2Available = false;
                    gs->hero.coins -= gs->shops[shopCode].item2Price;
                    gs->score += gs->shops[shopCode].item2Price / 2;
                } else
                    fprintf(stderr, "Impossível realizar a ação.\n");
                break;
                
            case 4: // BUYS POTION
                if ((gs->hero.coins >= 5) && (gs->hero.hp < gs->hero.maxHp) && (gs->currentFloor > 1)) {
                    printf("Você comprou poção de cura.\n");
                    gs->hero.hp += 8;
                    gs->hero.coins -= 5;
                    gs->score += 5;
                    if (gs->hero.hp > gs->hero.maxHp)
                        gs->hero.hp = gs->hero.maxHp;
                } else if (gs->currentFloor > 1)
                    fprintf(stderr, "Impossível realizar a ação.\n");
                break;
                
            case 5: // BUYS PENDANT
                if (gs->hero.coins >= 15 && gs->currentFloor > 2) {
                    printf("Você comprou pingente de vida.\n");
                    gs->hero.coins -= 15;
                    gs->hero.maxHp += 5;
                    gs->hero.hp += 5;
                    gs->score += 15;
                } else if (gs->currentFloor > 2)
                    fprintf(stderr, "Impossível realizar a ação.\n");
                break;
                
            default:
                fprintf(stderr, "Comando não encontrado.\n");
                break;
        }
    } while (action != 1);
}

// ================= [ UTILITY FUNCTIONS ] ==================================== //

int compareInt(const void* a, const void* b) {
    const int* num1 = (const int*)a;
    const int* num2 = (const int*)b;
    return *num2 - *num1;
}

void sortHighscores(int* highscores) {
    for (int i = 0; i < HIGHSCORES_NUM - 1; i++) {
        for (int j = 0; j < HIGHSCORES_NUM - i - 1; j++) {
            if (highscores[j] < highscores[j + 1]) {
                int temp = highscores[j];
                highscores[j] = highscores[j + 1];
                highscores[j + 1] = temp;
            }
        }
    }
}

// ================= [ DIALOG ] =============================== //

void shopDialog(GameState *gs) {
    switch (gs->currentFloor) {
        case 0:
            printf("\nLojista: Então você quer se aventurar pela terrível masmorra, %s?\n", gs->hero.name);
            printf("Se fosse você eu não iria de mãos atadas...\n");
            break;

        case 1:
            printf("\nVocê encontra uma vendedor ambulante, carregando itens em seu carro de mão.\n");
            printf("Vendedor ambulante: O que estou fazendo aqui? Ah, é só um passeio de negócios.\n");
            printf("Você acabou de matar potenciais compradores. Você devia cobrir meu prejuízo...\n");
            break;

        case 2:
            printf("\nVocê encontra uma pequena venda de um goblin morador local.\n");
            printf("Lojista goblin: Levar o que quiser, só por favor não me matar!\n");
            printf("(Seu código moral te impede de sair sem pagar)\n");
            break;

        case 3:
            printf("\nVocê encontra uma troll ferreiro, forjando ferramentas com os minérios.\n");
            printf("Ferreiro: Um humano nesse lugar? Coisa rara de se ver...\n");
            printf("Posso te dar umas lembrancinhas em troca dessas moedas de ouro.\n");
            break;

        case 4:
            printf("\nUm misterioso ser encapuzado aparece das sombras.\n");
            printf("Ser misterioso: Então é você %c corajoso %c %s que busca derrotar o grande Xãopai?\n", 
                   gs->pronoum, gs->pronoum, gs->hero.name);
            printf("Tenha cautela, jovem guerreir%c. O caminho a seguir é perigoso e traiçoeiro.\n\n", 
                   gs->pronoum);
            break;

        case 5:
            printf("\nVocê encontra uma hedionda figura de carne e tentáculos.\n");
            printf("Criatura: (A criatura encara os mais obscuros cantos da sua alma)\n");
            printf("(Você se sente extremamente desconfortável enquanto compra)\n");
            break;
    }
}

void dungeonDialog(GameState *gs) {
    switch (gs->currentFloor) {
        case 0:
            printf("---|CIDADE|---\n");
            printf("Diversos aventureiros da cidade acima adentraram a masmorra antes de você, mas \n");
            printf("poucos foram vistos novamente. Diz-se que um mal ancestral espreita nas \n");
            printf("profundezas, o imensamente poderoso espírito Xãopai.\n\n");
            printf("Antes de começar sua aventura, você entra em uma loja.\n");
            break;

        case 1:
            printf("---|PISO %i|---\n", gs->currentFloor);
            printf("Você desce as escadarias da masmorra, entrando no sistema de esgoto da cidade.\n");
            printf("As criaturas, antes inofensivas, se tornaram ferozes com a influência maligna \n");
            printf("vinda das profundezas.\n\n");
            break;
            
        case 2:
            printf("---|PISO %i|---\n", gs->currentFloor);
            printf("Você desce as escadarias da masmorra, entrando em uma prisão antiga a muito tempo \n");
            printf("abandonada. Esta prisão costumava abrigar os mais perigosos criminosos, mas a \n");
            printf("influência corrompeu as mentes dos guardas e dos prisioneiros.\n\n");
            break;
            
        case 3:
            printf("---|PISO %i|---\n", gs->currentFloor);
            printf("Você desce as escadarias da masmorra, entrando em uma caverna escura e rica em \n");
            printf("minério. Uma vez foi um grande centro comercial habitada por anões, mas foi \n");
            printf("abandonada com a vinda da pestilência.\n\n");
            break;
            
        case 4:
            printf("---|PISO %i|---\n", gs->currentFloor);
            printf("Você desce as escadarias da masmorra, entrando na antiga metrópole do reino dos \n");
            printf("anões, hoje em ruínas. Contam as histórias que a povoada capital sucumbiu súbitamente \n");
            printf("com a chegada de um enorme mal.\n\n");
            break;
            
        case 5:
            printf("---|PISO %i|---\n", gs->currentFloor);
            printf("Você desce as escadarias da masmorra, chegando escaldantes corredores dos salões \n");
            printf("demoníacos. Você pode sentir uma poderosa aura sombria no ar...\n\n");
            break;

        case 6:
            printf("---|TEMPLO DE XÃOPAI|---\n");
            printf("Você desce as escadarias da masmorra, chegando finalmente ao trono do todo poderoso \n");
            printf("Xãopai. Você sente como se fosse passar por uns maus tempos.\n\n");
            break;
    }
}

void printsBadEnding(GameState *gs) {
    printf("Você toma o amuleto para si.\n");
    printf("Você absorve o poder do amuleto, e consegue sentir a escuridão fluir pelo seu corpo.\n");
    printf("Você tenta resistir à magia, mas ela te domina e você acaba perdendo sua noção \nda realidade.\n");
    printf("O que restou de você agora reinará no trono como %c nov%c Xãopai.\n", gs->pronoum, gs->pronoum);
    printf("|FINAL RUIM|\n");
}

void printsGoodEnding() {
    printf("Você destrói o amuleto.\n");
    printf("Você ouve um estridente ruído vindo do amuleto enquanto vê toda a energia maligna \nse dissipar no ar.\n");
    printf("Você finalmente libertou a masmorra e os habitantes da cidade das \nmãos do mau!\n");
    printf("|FINAL BOM|\n");
}

void printsTrueEnding(GameState *gs) {
    printf("Você recita a sequência mágica ao amuleto.\n");
    printf("O amuleto brilha em luz vermelha ofuscante enquanto muda sua forma para um esbelto garoto.\n");
    printf("Paixão: Uh... Onde eu estou? Eu estava programando um jogo e... acho que dormi no teclado.\n");
    printf("Paixão: Vem, %s, vamo dar um fora daqui. Ficar encarando essa tela de terminal \ntá me dando dor de cabeça...\n", gs->hero.name);
    printf("|FINAL VERDADEIRO|\n");
}

// ================= [ COMBAT & EVENT FUNCTIONS ] ============================== //

bool chest(GameState *gs, bool *chestOpened) {
    srand(clock());
    if (rand() % 5 == 0) {
        enemy_t chestEnemy;
        bool run;
        printf("Você encontrou um baú!\n");
        printf("[1] Chutar violentamente\n");
        printf("[2] Ignorar\n");

        int action;
        do {
            action = readInt();
            switch(action) {
                case 0:
                    showsInfo(gs, -1);
                    printf("|Baú|\n");
                    printf(" ???\n");
                    break;

                case 1:
                    run = false;
                    break;

                case 2:
                    run = true;
                    break;

                default:
                    fprintf(stderr, "Comando não encontrado.\n");
                    break;
            }
        } while(action != 1 && action != 2);

        if (rand() % 3 == 0 && !run) {
            strcpy(chestEnemy.name, "Mímico");
            switch(gs->currentFloor) {
                case 1:
                    chestEnemy.maxHp = rand() % 3 + 4;
                    break;
                case 2:
                    chestEnemy.maxHp = rand() % 4 + 6;
                    break;
                case 3:
                    chestEnemy.maxHp = rand() % 4 + 8;
                    break;
                case 4:
                    chestEnemy.maxHp = rand() % 4 + 9;
                    break;
                case 5:
                    chestEnemy.maxHp = rand() % 4 + 10;
                    break;
            }
            chestEnemy.atk = gs->hero.def + 1;
            chestEnemy.hp = chestEnemy.maxHp;
            chestEnemy.coins = rand() % (5 * gs->currentFloor) + 5;

            printf("O Baú era um mímico!\n");
            printf("[1] Atacar\n");

            do {
                do {
                    action = readInt();
                    switch(action) {
                        case 0:
                            showsInfo(gs, -1);
                            break;
                        case 1:
                            chestEnemy.hp -= gs->hero.atk;
                            break;
                        default:
                            fprintf(stderr, "Comando não encontrado.\n");
                            break;
                    }
                } while(action != 1);
                
                if (chestEnemy.hp > 0) {
                    if (gs->hero.def < chestEnemy.atk)
                        gs->hero.hp -= (chestEnemy.atk - gs->hero.def);
                    if (gs->hero.hp <= 0) {
                        clearScreen();
                        printf("VOCÊ MORREU PARA %s NO PISO %i\n\a", chestEnemy.name, gs->currentFloor);
                        printf("Pontuação: %i\n", gs->score);
                        return true;
                    } else {
                        printf("%s: %i/%i\n", gs->hero.name, gs->hero.hp, gs->hero.maxHp);
                        printf("%s: %i/%i\n", chestEnemy.name, chestEnemy.hp, chestEnemy.maxHp);
                    }
                }
            } while(chestEnemy.hp > 0);
            
            gs->score += chestEnemy.coins;
            gs->hero.coins += chestEnemy.coins;
            printf("Você derrotou %s.\n", chestEnemy.name);
            printf("Você encontrou %d moedas!\n", chestEnemy.coins);

        } else if (!run) {
            chestEnemy.maxHp = 10 * gs->currentFloor;
            chestEnemy.hp = chestEnemy.maxHp;
            chestEnemy.coins = rand() % (5 * gs->currentFloor) + 5;

            printf("Você chuta o baú para tentar abri-lo\n");
            printf("[1] Chutar\n");
            do {
                do {
                    action = readInt();
                    switch(action) {
                        case 0:
                            showsInfo(gs, -1);
                            break;
                        case 1:
                            chestEnemy.hp -= (gs->hero.atk + 1);
                            break;
                        default:
                            fprintf(stderr, "Comando não encontrado.\n");
                            break;
                    }
                } while(action != 1);
                
                if (chestEnemy.hp > 0)
                    printf("Baú: %i/%i\n", chestEnemy.hp, chestEnemy.maxHp);
            } while(chestEnemy.hp > 0);
            
            gs->score += chestEnemy.coins;
            gs->hero.coins += chestEnemy.coins;
            printf("Você cuidadosamente abre o baú.\n");
            printf("Você encontrou %d moedas!\n", chestEnemy.coins);
        }

        if (*chestOpened == false && !run) {
            gs->secretNumber = rand() % 900 + 100;
            printf("Você encontra uma marcação suspeita gravada no fundo do baú: %i\n", gs->secretNumber);
            *chestOpened = true;
        }
    }
    return false;
}

bool xaopaiInteraction(GameState *gs) {
    printf("Você desafia o terrível Xãopai!\n");
    printf("[1] Atacar\n");
    gs->monsters[XAOPAI].hp = gs->monsters[XAOPAI].maxHp;
    
    do {
        int action = readInt();
        switch(action) {
            case 0: // INFO
                showsInfo(gs, XAOPAI);
                break;

            case 1: // ATTACK
                gs->monsters[XAOPAI].hp -= gs->hero.atk;
                break;

            default:
                fprintf(stderr, "Comando não encontrado.\n");
                break;
        }
        
        if (action == 1 && gs->monsters[XAOPAI].hp > 0) {
            gs->hero.hp -= gs->monsters[XAOPAI].atk;
            if (gs->hero.hp <= 0) {
                clearScreen();
                printf("VOCÊ PERECEU DIANTE DE XÃOPAI.\n\a");
                printf("Pontuação: %i\n", gs->score);
                return true;
            } else {
                printf("%s: %i/%i\n", gs->hero.name, gs->hero.hp, gs->hero.maxHp);
                printf("XÃOPAI: %i/%i\n", gs->monsters[XAOPAI].hp, gs->monsters[XAOPAI].maxHp);
            }
        }
    } while(gs->monsters[XAOPAI].hp > 0);

    gs->score += 1000;
    printf("VOCÊ DERROTOU O ESPÍRITO DE XÃOPAI!\n\n");
    return false;
}

void ending(GameState *gs) {
    printf("Você atravessa a fera com a sua espada e ela é reduzida à poeira e cinzas.\n");
    printf("Você apanha um amuleto vermelho-sangue em meio aos restos da criatura\n");
    printf("O amuleto pulsa vigorosamente em suas mãos. Você pode sentir um imenso poder \nsaindo dele.\n");
    printf("O que você faz?\n");
    printf("[1] Inspecionar amuleto\n");
    printf("[2] Se apossar do amuleto de Xãopai\n");
    printf("[3] Destruir o amuleto de Xãopai\n");

    while(1) {
        int action = readInt();

        if (action == 0) {
            showsInfo(gs, -1);
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

void updatesHighscores(GameState *gs, int* highscores) {
    FILE *file;

    file = fopen("Highscores.bin", "rb+");

    if (file == NULL) {
        file = fopen("Highscores.bin", "wb+");
        if (file == NULL) {
            perror("Error opening file");
            return;
        }
    } else {
        fread(highscores, sizeof(int), HIGHSCORES_NUM, file);
        fclose(file);
    }

    highscores[HIGHSCORES_NUM - 1] = gs->score;

    sortHighscores(highscores);

    file = fopen("Highscores.bin", "wb");
    if (file == NULL) {
        perror("Error opening file");
        return;
    }

    fwrite(highscores, sizeof(int), HIGHSCORES_NUM, file);
    fclose(file);
}

int showHighscores(GameState *gs, int* highscores) {
    int input;
    clearScreen();
    printf("          - HIGH SCORES -          \n");

    for (int i = 0; i < HIGHSCORES_NUM - 1; ++i)
        if (highscores[i] != 0)
            printf("              %2i - %i\n", i + 1, highscores[i]);

    printf("\n         (1) Jogar novamente\n");
    printf("         (2) Sair do jogo\n");

    do {
        input = readInt();
        switch(input) {
            case 1:
                return 1;
                break;
            case 2:
                return 0;
                break;
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
                fprintf(stderr, "Comando não encontrado.\n");
                break;
        }
    } while(input != 1 && input != 2);
}

// ================= [ MAIN LOOP ] ============================================ //

int main() {
    setlocale(LC_ALL, "Portuguese");
    
    int iterationCounter = 0;
    int gamesWon = 0;
    int highscores[HIGHSCORES_NUM] = {0};
    
    int action;
    do {
        GameState gs = {0};
        
        int monsterCode = 0;
        gs.currentFloor = 0;
        bool chestOpened = false;
        gs.score = 0;
        gs.isDead = false;
        
        initGameData(&gs);
        
        titleScreen();
        dungeonDialog(&gs);
        
        if (iterationCounter == 0) {
            printf("Lojista: Você deve ser nov%c por aqui. Qual seu nome, viajante?\n", gs.pronoum);
            readString(gs.hero.name, STR_LENGTH);
        }
        
        char lowerName[STR_LENGTH];
        strcpy(lowerName, gs.hero.name);
        for (int i = 0; lowerName[i]; i++) {
            lowerName[i] = tolower(lowerName[i]);
        }
        if (strcmp(lowerName, "paixao") == 0 || strcmp(lowerName, "paixão") == 0) {
            printf("Lojista: Olha só! O próprio criador do jogo! Bom rever você.\n");
            gs.hero.coins += 999;
        }
        
        printf("\n+---------------------------------------------------------------------------+\n");
        printf(  "| DICA: Digite '0' a qualquer momento para mostrar status do herói/oponente |\n");
        printf(  "+---------------------------------------------------------------------------+\n");
        
        shopDialog(&gs);
        shopInteraction(&gs, gs.currentFloor);
        
        for (int floor = 0; floor < 5; ++floor) {
            gs.currentFloor++;
            clearScreen();
            dungeonDialog(&gs);
            
            for (int i = 0; i < 3; ++i) {
                gs.isDead = monsterInteraction(&gs, monsterCode);
                if (gs.isDead)
                    goto TheEnd;
                monsterCode++;
            }
            
            if (chest(&gs, &chestOpened))
                goto TheEnd;
            
            shopDialog(&gs);
            shopInteraction(&gs, gs.currentFloor);
        }
        
        gs.currentFloor++;
        clearScreen();
        dungeonDialog(&gs);
        printsXaopai();
        gs.monsters[XAOPAI].maxHp = 175 - gs.hero.def * 3;
        gs.isDead = xaopaiInteraction(&gs);
        
        if (gs.isDead)
            goto TheEnd;
        else
            ending(&gs);
        
        TheEnd:
        
        highscores[iterationCounter] = gs.score;
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
                    if (showHighscores(&gs, highscores))
                        action = 1;
                    else
                        action = 3;
                    break;
                    
                case 3:
                    break;
                    
                default:
                    fprintf(stderr, "Comando não encontrado.\n");
                    break;
            }
        } while((action != 1) && (action != 3));
        
    } while(action != 3);
    
    return PAIXAO;
}