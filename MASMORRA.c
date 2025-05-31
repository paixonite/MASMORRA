// LIBRARIES
	#include <stdio.h>
	#include <stdlib.h>
	#include <locale.h>
	#include <math.h>
	#include <time.h>
	#include <string.h>
	#include <ctype.h>
	#include <windows.h>
	#include <conio.h>
	#include <stdbool.h>

// CONSTANTS
	#define PAIXAO 106
	#define STR_LENGTH 30 
	#define HIGHSCORES_NUM 10 + 1
	#define XAOPAI 15

// STRUCTS

	typedef struct sCharacter {	
		char name[STR_LENGTH];
		int coins;	
		int hp;
		int maxHp;
		int atk; 	
		int def; 
		char atkItem[STR_LENGTH]; 
		char defItem[STR_LENGTH]; 
	}character_t;

	typedef struct sEnemy {
		char name[STR_LENGTH];
		int hp;
		int maxHp;
		int atk; 
		int def; 
		int coins;
	}enemy_t;

	typedef struct sShop {
		int item1Priece;
		int item2Priece;
		int item1atkPoints; 
		int item2defPoints; 
		char item1Name[STR_LENGTH];
		char item2Name[STR_LENGTH]; 
	}shop_t;

// GLOBAL VARIABLES

	int in; // Read input
	int score = 0; // Current score
	int highscores[HIGHSCORES_NUM] = {0};
	bool isDead; // Checks if you died
	int secretNumber; // True ending code
	char pronoum = 'o'; // We are gender inclusive
	int currentFloor = 0;

	character_t hero;
	enemy_t monster [15];
	shop_t shop[6];

// PROTOTYPES
	// For shop interactions
	void shopInteraction();

	// For monster fights
	bool monsterInteraction(int monsterCode);
	
	// For the boss fight
	bool xaopaiInteraction();

	// 1/5 chance for chest appearing
	bool chest(bool *chestOpened);

	// For highscores
	int compareInt(const void* a, const void* b);

    // shows shopkepper's dialog
    void shopDialog();

 	// shows currentFloor description
 	void dungeonDialog();

 	//
 	void setsShopInfo(shop_t *shop);

 	//
 	void setsMonsterInfo(enemy_t *monster);

 	//
 	void printsXaopai();

 	//
    void ending();

    //
 	void titleScreen();

 	// monsterCode < 0 for only hero info
 	// monsterCode = XAOPAI for boss info
 	void showsInfo(int monsterCode);

 	// Returns 1 to play and 0 to exit
    int showHighscores();

 	// updatesde Highscores file and array
    void updatesHighscores();

    // Uses bubble sort 
    void sortHighscores();

// MAIN
	int main()
	{
		setlocale(LC_ALL, "Portuguese");
		int iterationCounter = 0;
		int gamesWon = 0;
		do{	

			int monsterCode = 0; // The monster array index to define its stats
			currentFloor = 0;
			bool chestOpened = false;
			score = 0;
			isDead = false;

			hero.coins = 15;
			hero.hp = 10;
			hero.maxHp = 10;
			hero.atk = 1;
			hero.def = 0;
			strcpy(hero.atkItem, "Seus punhos");
			strcpy(hero.defItem, "Nada");

			setsShopInfo(shop);
			setsMonsterInfo(monster);

			titleScreen();
			dungeonDialog();

			if (iterationCounter == 0){
					printf("Lojista: Você deve ser nov%c por aqui. Qual seu nome, viajante?\n", pronoum);
				while(1){
       				scanf("%29[^\n]", hero.name);
       				if (strcmp(hero.name, "0") == 0)
       					printf("Agora não, burr%c!\n", pronoum);
       				else if (strcmp(hero.name, "root") == 0){
       					hero.atk = 999;
       					printf("Unlimited power!");
       					break;
       				} else
       					break;
       			}
			}

			shopDialog();
			shopInteraction();

			for (int i = 0; i < 5; ++i){
				currentFloor++;
				system("cls");
				dungeonDialog();
				for (int i = 0; i < 3; ++i){
					isDead = monsterInteraction(monsterCode);
					if (isDead)
						goto TheEnd;
					monsterCode ++;
				}
				if(chest(&chestOpened))
					goto TheEnd;
				shopDialog();
				shopInteraction();
			}

			currentFloor++;
			system("cls");
			dungeonDialog();
			printsXaopai();			
			/*Aegis + 22 hp | Excalibur + 20 hp...?*/
			monster[XAOPAI].maxHp = 175 - hero.def * 3;
			isDead = xaopaiInteraction();
			if (isDead)
				goto TheEnd;
			else
				ending();

			TheEnd:

			highscores[iterationCounter] = score;
			if (!isDead)
				gamesWon ++;
			else
				printf("\7");

			updatesHighscores();

			printf("[1] Jogar novamente\n[2] Highscores\n[3] Sair do jogo\n");

			do{
				scanf("%d", &in);
				switch (in){
					case 1: // GO TO THE START
						iterationCounter++;
						system("cls");
						break;

					case 2: // SHOW HIGHSCORES
						if(showHighscores())
							in = 1;
						else
							in = 3;
						break;

					case 3: // CLOSE THE GAME
						break;

					default: // ERROR
						fprintf(stderr,"Comando não encontrado.\n");
						break;
				}
			}while((in != 1) && (in != 3));

		}while(in != 3);
		return PAIXAO;
	}

// FUNCTIONS
	void shopInteraction(){

		bool item1Avaible = true; // Disponibilidade do item 1
		bool item2Avaible = true; // Disponibilidade do item 2

		printf("(Você tem %i moedas)\n", hero.coins);
		printf("[1] Sair da loja\n");
		printf("[2] %s - $%i\n", shop[currentFloor].item1Name, shop[currentFloor].item1Priece);
		printf("[3] %s - $%i\n", shop[currentFloor].item2Name, shop[currentFloor].item2Priece);
		if(currentFloor > 1)
			printf("[4] Poção de cura - $5\n");
		if(currentFloor > 2)
			printf("[5] Pingente de vida - $15\n");

		do{
			scanf("%i",&in);
			switch (in){
				case 0: // INFO
					showsInfo(-1);
					break;

				case 1: // LEAVES THE SHOP
					break;

				case 2: // BUYS ITEM 1
					if ((item1Avaible == true) && (hero.coins >= shop[currentFloor].item1Priece))
					{
						strcpy(hero.atkItem, shop[currentFloor].item1Name);
						printf("Você comprou %s.\n", shop[currentFloor].item1Name);
						hero.atk = shop[currentFloor].item1atkPoints;
						item1Avaible = false;
						hero.coins -= shop[currentFloor].item1Priece;
						score += shop[currentFloor].item1Priece / 2;
					}else
						fprintf(stderr,"Impossível realizar a ação.\n");
					break;

				case 3: // BUYS ITEM 2
					if ((item2Avaible == true) && (hero.coins >= shop[currentFloor].item2Priece))
					{
						strcpy(hero.defItem, shop[currentFloor].item2Name);
						printf("Você comprou %s.\n", shop[currentFloor].item2Name);
						hero.def = shop[currentFloor].item2defPoints;
						item2Avaible = false;
						hero.coins -= shop[currentFloor].item2Priece;
						score += shop[currentFloor].item2Priece / 2;
					}else
						fprintf(stderr,"Impossível realizar a ação.\n");
					break;

				case 4: // BUYS POTION
					if ((hero.coins >= 5) && (hero.hp < hero.maxHp) && (currentFloor > 1)){
						printf("Você comprou poção de cura.\n");
						hero.hp += 8;
						hero.coins -= 5;
						score += 5;
						if (hero.hp > hero.maxHp)
							hero.hp = hero.maxHp;
					}else if (currentFloor > 1)
						fprintf(stderr,"Impossível realizar a ação.\n");
					break;

				case 5: // BUYS PENDANT
					if (hero.coins >= 15 && currentFloor > 2){
						printf("Você comprou pingente de vida.\n");
						hero.coins -= 15;
						hero.maxHp += 5;
						hero.hp += 5;
						score += 15;
					}else if (currentFloor > 2)
						fprintf(stderr, "Impossível realizar a ação.\n");
					break;

				default: // ERROR
					fprintf(stderr,"Comando não encontrado.\n");
					break;
			}
		}while(in != 1);
	}

	bool monsterInteraction(int monsterCode){
		printf("Você encontrou %s!\n",monster[monsterCode].name);
		printf("[1] Atacar \n");
		monster[monsterCode].hp = monster[monsterCode].maxHp;
		do{
			do{
				scanf("%i",&in);
				switch(in)
				{
					case 0: // INFO
						showsInfo(monsterCode);
						break;

					case 1: // ATTACK
						if (monster[monsterCode].def >= hero.atk){
							system("cls");
							printf("VOCÊ MORREU DE PARADA CARDÍACA AO VER %s NO PISO %i\n\7",monster[monsterCode].name,currentFloor);
							printf("Pontuação: %i\n",score);
							return true;
						}
						else
							monster[monsterCode].hp -= (hero.atk - monster[monsterCode].def);
						break;

					default: //ERROR
						fprintf(stderr,"Comando não encontrado.\n");
						break;
				}
			}while(in != 1);
			if (monster[monsterCode].hp > 0){
				if (hero.def < monster[monsterCode].atk)
					hero.hp -= (monster[monsterCode].atk- hero.def);
				if (hero.hp <= 0)
				{
				system("cls");
				printf("VOCÊ MORREU PARA %s NO PISO %i\n\7",monster[monsterCode].name,currentFloor);
				printf("Pontuação: %i\n",score);
				return true;
				}else{
				printf("%s: %i/%i\n", hero.name, hero.hp, hero.maxHp);
				printf("%s: %i/%i\n", monster[monsterCode].name, monster[monsterCode].hp, monster[monsterCode].maxHp);
				}
			}		
		}while(monster[monsterCode].hp > 0);
		score += 10;
		hero.coins += monster[monsterCode].coins;
		printf("Você derrotou %s.\n", monster[monsterCode].name);
		printf("Você encontrou %i moedas!\n\n", monster[monsterCode].coins);
		return false;
	}

	bool chest(bool *chestOpened){ 
		// 1/5 chance of a chest appearing
		srand(clock());
		if (rand() % 5 == 0)
		{
			enemy_t chest;
			bool run;
			printf("Você encontrou um baú!\n");
			printf("[1] Chutar violentamente\n");
			printf("[2] Ignorar\n");

			do{
				scanf("%i", &in);
				switch(in){
					case 0:
						showsInfo(-1);
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
			}while(in != 1 && in != 2);

			// 1/3 chance for it being a mimic
			if (rand() % 3 == 0 && !run){
				strcpy(chest.name, "Mímico");
				switch(currentFloor){
					case 1:
						chest.maxHp = rand() % 3 + 4;
						break;
					case 2:
						chest.maxHp = rand() % 4 + 6;
						break;
					case 3:
						chest.maxHp = rand() % 4 + 8;
						break;
					case 4:
						chest.maxHp = rand() % 4 + 9;
						break;
					case 5:
						chest.maxHp = rand() % 4 +  10;
						break;
				}
				chest.atk = hero.def + 1;
				chest.hp = chest.maxHp;
				chest.coins = rand() % (5 * currentFloor) + 5;

				printf("O Baú era um mímico!\n");
				printf("[1]Atacar\n");

				do{
					do{
						scanf("%i", &in);
						switch(in){
							case 0:
								showsInfo(0);
								break;

							case 1:
								chest.hp -= hero.atk;
								break;

							default:
								fprintf(stderr, "Comando não encontrado.\n");
								break;
						}
					}while(in != 1);
					if (chest.hp > 0){
						if (hero.def < chest.atk)
							hero.hp -= (chest.atk - hero.def);
						if (hero.hp <= 0){
							system("cls");
							printf("VOCÊ MORREU PARA %s NO PISO %i\n\7", chest.name, currentFloor);
							printf("Pontuação: %i\n", score);
							return true;
						}else{
							printf("%s: %i/%i\n", hero.name, hero.hp, hero.maxHp);
							printf("%s: %i/%i\n", chest.name, chest.hp, chest.maxHp);
						}
					}		
				}while(chest.hp > 0);
				score += chest.coins;
				hero.coins += chest.coins;
				printf("Você derrotou %s.\n", chest.name);
				printf("Você encontrou %d moedas!\n", chest.coins);

			// 2/3 chance for it to be a chest
			} else if (!run){
				chest.maxHp = 10 * currentFloor;
				chest.hp = chest.maxHp;
				chest.coins = rand() % (5 * currentFloor) + 5;

				printf("Você chuta o baú para tentar abri-lo\n");
				printf("[1] Chutar\n");
				do{
					do{
						scanf("%i", &in);
						switch(in){
							case 0:
								showsInfo(0);
								break;

							case 1:
								chest.hp -= hero.atk;
								break;

							default:
								fprintf(stderr, "Comando não encontrado.\n");
								break;
						}
					}while(in != 1);
					if (chest.hp > 0)
						printf("Baú: %i/%i\n", chest.hp, chest.maxHp);
				}while(chest.hp > 0);
				score += chest.coins;
				hero.coins += chest.coins;
				printf("Você cuidadosamente abre o baú.\n");
				printf("Você encontrou %d moedas!\n", chest.coins);

			}
			// displays secret number on the first chest
			if (*chestOpened == false && !run){
					secretNumber = rand() % 900 + 100;
					printf("Você encontra uma marcação suspeita gravada no fundo do baú: %i\n", secretNumber);
					*chestOpened = true;
			}
		}
		return false;
	}

	int compareInt(const void* a, const void* b){
	    const int* num1 = (const int*)a;
	    const int* num2 = (const int*)b;
		return *num2 - *num1;
	}

    void shopDialog(){
 		switch (currentFloor){
 			case 0:
 				printf("\nLojista: Então você quer se aventurar pela terrível masmorra, %s?\n",hero.name);
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
				printf("Ser misterioso: Então é você %c corajos%c %s que busca derrotar o grande Xãopai?\n", pronoum, pronoum, hero.name);
				printf("Tenha cautela, jovem guerreir%c. O caminho a seguir pode ser perigoso e traiçoeiro.\n\n", pronoum);
 				break;

 			case 5:
 				printf("\nVocê encontra uma hedionda figura de carne e tentáculos.\n");
				printf("Criatura: (A criatura encara até os mais obscuros cantos da sua alma)\n");
				printf("(Você se sente extremamente desconfortável enquanto compra)\n");
 				break;
 		}
 	}

 	void dungeonDialog(){
 		switch (currentFloor){
 			case 0:
 				printf("+-------------------------------------------------------------------------+\n");
 				printf("|DICA: Digite '0' a qualquer momento para mostrar status do herói/oponente|\n");
 				printf("+-------------------------------------------------------------------------+\n");
 				printf("Diversos aventureiros da cidade acima adentraram a masmorra antes de você, mas \npoucos foram vistos novamente.");
				printf(" Diz-se que um mal ancestral espreita nas \nprofundezas, o imensamente poderoso espírito Xãopai.\n\n");
				printf("Antes de começar sua aventura, você entra em uma loja.\n");
 				break;

 			case 1:
 				printf("|PISO %i|\n", currentFloor);
				printf("Você desce as escadarias da masmorra, entrando no sistema de esgoto da cidade.\n");
				printf("As criaturas, antes inofensivas, se tornaram ferozes com a influência maligna \nvinda das profundezas.\n\n");
 				break;
 				
 			case 2:
 				printf("|PISO %i|\n", currentFloor);
				printf("Você desce as escadarias da masmorra, entrando em uma prisão antiga a muito tempo \nabandonada.");
				printf(" Esta prisão costumava abrigar os mais perigosos criminosos, mas a \ninfluência ");
			    printf("corrompeu as mentes dos guardas e dos prisioneiros.\n\n");
 				break;
 				
 			case 3:
 				printf("|PISO %i|\n", currentFloor);
				printf("Você desce as escadarias da masmorra, entrando em uma caverna escura e rica em \nminério.");
				printf(" Uma vez foi um grande centro comercial habitada por anões, mas foi \nabandonada com a vinda da pestilência.\n\n");
 				break;
 				
 			case 4:
 				printf("|PISO %i|\n", currentFloor);
				printf("Você desce as escadarias da masmorra, entrando na antiga metrópole do reino dos \nanões, hoje em ruínas.");
				printf(" Contam as histórias que a povoada capital sucumbiu súbitamente \ncom a chegada de um enorme mal.\n\n");
 				break;
 				
 			case 5:
 				printf("|PISO %i|\n", currentFloor);
				printf("Você desce as escadarias da masmorra, chegando escaldantes corredores dos salões \ndemoniacos.");
				printf(" Você pode sentir uma poderosa aura sombria no ar...\n\n");
 				break;

 			case 6:
 				printf("|TEMPLO DE XÃOPAI|\n");
				printf("Você desce as escadarias da masmorra, chegando finalmente ao trono do todo poderoso \nXãopai. ");
				printf("Você sente como se fosse passar por uns maus tempos.\n\n");
 				break;
 		}					
 	}

 	void setsShopInfo(shop_t *shop){
		strcpy(shop[0].item1Name,"Bastão de madeira");
		shop[0].item1atkPoints = 2;
		shop[0].item1Priece = 10;
		strcpy(shop[0].item2Name,"Escudo pequeno de madeira");
		shop[0].item2defPoints = 1;
		shop[0].item2Priece = 10;

		strcpy(shop[1].item1Name,"Espada curta de cobre");
		shop[1].item1atkPoints = 3;
		shop[1].item1Priece = 15;
		strcpy(shop[1].item2Name,"Escudo redondo de cobre");
		shop[1].item2defPoints = 2;
		shop[1].item2Priece = 15;

		strcpy(shop[2].item1Name,"Espada de ferro");
		shop[2].item1atkPoints = 4;
		shop[2].item1Priece = 20;
		strcpy(shop[2].item2Name,"Escudo de ferro");
		shop[2].item2defPoints = 3;
		shop[2].item2Priece = 20;

		strcpy(shop[3].item1Name,"Espada longa de aço");
		shop[3].item1atkPoints = 5;
		shop[3].item1Priece = 30;
		strcpy(shop[3].item2Name,"Escudo grande de aço");
		shop[3].item2defPoints = 4;
		shop[3].item2Priece = 30;

		strcpy(shop[4].item1Name,"Espada encantada de diamante");
		shop[4].item1atkPoints = 6;
		shop[4].item1Priece = 40;
		strcpy(shop[4].item2Name,"Escudo largo de diamante");
		shop[4].item2defPoints = 5;
		shop[4].item2Priece = 40;

		strcpy(shop[5].item1Name,"Excalibur Lendária");
		shop[5].item1atkPoints = 8;
		shop[5].item1Priece = 55;
		strcpy(shop[5].item2Name,"Égide Mítica");
		shop[5].item2defPoints = 15;
		shop[5].item2Priece = 50;
 	}

 	void setsMonsterInfo(enemy_t *monster){
	 	strcpy(monster[0].name, "Ratazana gigante");
		srand(clock()); monster[0].maxHp = rand() % 2 + 3;
		srand(clock()); monster[0].atk = rand() % 3 + 1;
		srand(clock()); monster[0].def = rand() % 1 + 0;
		srand(clock()); monster[0].coins = rand() % 5 + 5;

	 	strcpy(monster[1].name, "Cobra");
		srand(clock()); monster[1].maxHp = rand() % 1 + 3;
		srand(clock()); monster[1].atk = rand() % 2 + 2;
		srand(clock()); monster[1].def = rand() % 1 + 0;
		srand(clock()); monster[1].coins = rand() % 5 + 5;

		strcpy(monster[2].name, "Gosma gigante");
		srand(clock()); monster[2].maxHp = rand() % 2 + 3;
		srand(clock()); monster[2].atk = rand() % 2 + 2;
		srand(clock()); monster[2].def = rand() % 1 + 0;
		srand(clock()); monster[2].coins = rand() % 5 + 5;

		//Coinage: 15 + 15 | S:15

		strcpy(monster[3].name, "Ladrão louco");
		srand(clock()); monster[3].maxHp = rand() % 2 + 3;
		srand(clock()); monster[3].atk = rand() % 3 + 2;
		srand(clock()); monster[3].def = rand() % 2 + 0;
		srand(clock()); monster[3].coins = rand() % 5 + 6;

		strcpy(monster[4].name, "Esqueleto");
		srand(clock()); monster[4].maxHp = rand() % 1 + 3;
		srand(clock()); monster[4].atk = rand() % 2 + 3;
		srand(clock()); monster[4].def = rand() % 2 + 0;
		srand(clock()); monster[4].coins = rand() % 5 + 7;

		strcpy(monster[5].name, "Guarda da prisão");
		srand(clock()); monster[5].maxHp  = rand() % 2 + 3;
		srand(clock()); monster[5].atk = rand() % 2 + 3;
		srand(clock()); monster[5].def = rand() % 2 + 0;
		srand(clock()); monster[5].coins = rand() % 5 + 7;

		//Coinage: 20 + 15 | S:20

		strcpy(monster[6].name, "Morcego vampiro");
		srand(clock()); monster[6].maxHp = rand() % 2 + 4;
		srand(clock()); monster[6].atk = rand() % 2 + 2;
		srand(clock()); monster[6].def = rand() % 2 + 1;
		srand(clock()); monster[6].coins = rand() % 5 + 10;

		strcpy(monster[7].name, "Aranha gigante");
		srand(clock()); monster[7].maxHp = rand() % 2 + 5;
		srand(clock()); monster[7].atk = rand() % 2 + 3;
		srand(clock()); monster[7].def = rand() % 2 + 1;
		srand(clock()); monster[7].coins = rand() % 5 + 10;

		strcpy(monster[8].name, "Troll selvagem");
		srand(clock()); monster[8].maxHp = rand() % 3 + 5;
		srand(clock()); monster[8].atk = rand() % 1 + 4;
		srand(clock()); monster[8].def = rand() % 1 + 2;
		srand(clock()); monster[8].coins = rand() % 5 + 10;

		//Coinage: 30 + 15 | S:30

		strcpy(monster[9].name, "Soldado anão");
		srand(clock()); monster[9].maxHp = rand() % 2 + 6;
		srand(clock()); monster[9].atk = rand() % 3 + 4;
		srand(clock()); monster[9].def = rand() % 2 + 1;
		srand(clock()); monster[9].coins = rand() % 6 + 13;

		strcpy(monster[10].name, "Monge anão");
		srand(clock()); monster[10].maxHp = rand() % 2 + 4;
		srand(clock()); monster[10].atk = rand() % 2 + 5;
		srand(clock()); monster[10].def = rand() % 1 + 3;
		srand(clock()); monster[10].coins = rand() % 7 + 13;

		strcpy(monster[11].name, "Bruxo anão");
		srand(clock()); monster[11].maxHp = rand() % 2 + 3;
		srand(clock()); monster[11].atk = rand() % 3 + 5;
		srand(clock()); monster[11].def = rand() % 1 + 3;
		srand(clock()); monster[11].coins = rand() % 7 + 14;

		//Coinage: 40 + 20 | S:40

		strcpy(monster[12].name, "Demônio estripador");
		srand(clock()); monster[12].maxHp = rand() % 2 + 7;
		srand(clock()); monster[12].atk = rand() % 2 + 6;
		srand(clock()); monster[12].def = rand() % 3 + 2;
		srand(clock()); monster[12].coins = rand() % 10 + 20;

		strcpy(monster[13].name, "Succubus");
		srand(clock()); monster[13].maxHp = rand() % 3 + 7;
		srand(clock()); monster[13].atk = rand() % 1 + 6;
		srand(clock()); monster[13].def = rand() % 3 + 2;
		srand(clock()); monster[13].coins = rand() % 10 + 20;

		strcpy(monster[14].name, "Cão demoníaco");
		srand(clock()); monster[14].maxHp = rand() % 2 + 8;
		srand(clock()); monster[14].atk = rand() % 2 + 6;
		srand(clock()); monster[14].def = rand() % 2 + 3;
		srand(clock()); monster[14].coins = rand() % 10 + 20;

		//Coinage: 60 + 30 | S:55/50

		strcpy(monster[15].name, "XÃOPAI");
		srand(clock()); monster[15].atk = 1;
		srand(clock()); monster[15].def = 0;
	}

 	void printsXaopai(){
    	char eye;
    	switch (rand() % 3){
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
    	//printf("\033[31m");
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
		//printf("\033[0m");
		printf("Xãopai: ABLUBLUBÊ ABLAUBLUBLÊ! EU COMER %s!\n", hero.name);	
    }

    bool xaopaiInteraction(){
		printf("Você desafia o terrível Xãopai!\n");
		printf("[1] Atacar \n");
		monster[XAOPAI].hp = monster[XAOPAI].maxHp;
		do{
			do{
				scanf("%i", &in);
				switch(in){
					case 0: // INFO
						showsInfo(XAOPAI);
						break;

					case 1: // ATTACK
							monster[XAOPAI].hp -= hero.atk;
						break;

					default: //ERROR
						fprintf(stderr, "Comando não encontrado.\n");
						break;
				}
			}while(in != 1);
			if (monster[XAOPAI].hp > 0){
				hero.hp -= monster[XAOPAI].atk;
				if (hero.hp <= 0){
				system("cls");
				printf("VOCÊ PERECEU DIANTE DE XÃOPAI.\n\7");
				printf("Pontuação: %i\n", score);
				return true;
				}else{
				printf("%s: %i/%i\n", hero.name, hero.hp, hero.maxHp);
				printf("XÃOPAI: %i/%i\n", monster[XAOPAI].hp, monster[XAOPAI].maxHp);
				}
			}		
		}while(monster[XAOPAI].hp > 0);
		score += 1000;
		printf("VOCÊ DERROTOU O ESPÍRITO DE XÃOPAI!\n\n");
		return false;
	}

    void ending(){
 		printf("Você atravessa a fera com a sua espada e ela é reduzida à poeira e cinzas.\n");
		printf("Você apanha um amuleto vermelho-sangue em meio aos restos da criatura\n");
		printf("O amuleto pulsa vigorosamente em suas mãos. Você pode sentir um imenso poder \nsaindo dele.\n");
		printf("O que você faz?\n");
		printf("[0] Inspeciona\n");
		printf("[1] Se apossar da amuleto de Xãopai\n");
		printf("[2] Destruir o amuleto de Xãopai\n");

		while(1){

			scanf("%d",&in);

			if (in == 0){
				// printf("\033[1;31m");
				printf("       .....           .....\n");
				printf("   ,ad8PPPP88b,     ,d88PPPP8ba,\n");
				printf("  d8P'      'Y8b, ,d8P'      'Y8b\n");
				printf(" dP'           '8a8'           `Yd\n");
				printf(" 8(              '              )8\n");
				printf(" I8                             8I\n");
				printf("  Yb,           o_o           ,dP\n");
				printf("   '8a,  ~me deixa entrar~  ,a8'\n");
				// printf("   '8a,  \033[0;37;44m~me deixa entrar~\033[1;31m  ,a8'\n");
				printf("     '8a,                 ,a8'\n");
				printf("       'Yba             adP'\n");
				printf("         `Y8a         a8P'\n");
				printf("           `88,     ,88'\n");
				printf("             '8b   d8'\n");
				printf("              '8b d8'\n");
				printf("               `888'\n");
				printf("                 '\n");
				// printf("\033[0m\n");

			}else if (in == 1){ 
				printf("Você toma o amuleto para si.\n");
				printf("Você absorve o poder do amuleto, e consegue sentir a escuridão fluir pelo seu corpo.\n");
				printf("Você tenta resistir à magia, mas ela te domina e você acaba perdendo sua noção \nda realidade.\n");
				printf("O que restou de você agora reinará no trono como %c nov%c Xãopai.\n", pronoum, pronoum);
				printf("|FINAL RUIM|\n");
				break;

			}else if (in == 2){ 
				printf("Você destrói o amuleto.\n");
				printf("Você ouve um estridente ruído vindo do amuleto enquanto vê toda a energia maligna \nse dissipar no ar.\n");
				printf("Você finalmente libertou a masmorra e os habitantes da cidade das \nmãos do mau!\n");
				printf("|FINAL BOM|\n");
				break;

			}else if (in == secretNumber){ 
				printf("Você recita a sequência mágica ao amuleto.\n");
				printf("O amuleto brilha em luz vermelha ofuscante enquanto muda sua forma para um esbelto garoto.\n");
				printf("Paixão: Uh... Onde eu estou? Eu estava programando um jogo e... acho dormi no teclado.\n");
				printf("Paixão: Vem, %s, vamo dar um fora daqui. Ficar encarando essa tela de terminal \ntá me dando dor de cabeça...\n",hero.name);
				printf("|FINAL VERDADEIRO|\n");
				break;

			}else if (in == PAIXAO){
				printf("Nada acontece\3\3\3\n");
			}else 
				printf("Nada acontece...\n");
		}
		printf("Pontuação: %i\n\n", score);
 	}

 	void titleScreen(){
    	//printf("\033[31m");
 		printf("*************************************************************************************\n");
		printf("                                     BEM-VINDO À\n");
					printf("         __  __      _      ____    __  __    ___    ____    ____       _\n");
		printf("        |  \\/  |    / \\    / ___|  |  \\/  |  / _ \\  |  _ \\  |  _ \\     / \\\n");
		printf("        | |\\/| |   / _ \\   \\___ \\  | |\\/| | | | | | | |_) | | |_) |   / _ \\\n");
		printf("        | |  | |  / ___ \\   ___) | | |  | | | |_| | |  _ <  |  _ <   / ___ \\\n");
		printf("        |_|  |_| /_/   \\_\\ |____/  |_|  |_|  \\___/  |_| \\_\\ |_| \\_\\ /_/   \\_\\\n");
		printf("                                     by: Paixão\n");
		printf("*************************************************************************************\n\n");
		printf("                            Pressione enter para cotinuar...\n");
		getchar();
		system("cls");
	}

	void showsInfo(int monsterCode){
 		printf("|%s|\n", hero.name);
		printf("Saúde: %i/%i\n", hero.hp, hero.maxHp);
		printf("Moedas: %i\n", hero.coins);
		printf("Pontos de ataque: %i(%s)\n", hero.atk, hero.atkItem);
		printf("Pontos de defesa: %i(%s)\n", hero.def, hero.defItem);

		if (monsterCode > 0){
			printf("|%s|\n", monster[monsterCode].name);
			printf("Saúde: %i/%i\n", monster[monsterCode].hp, monster[monsterCode].maxHp);
			printf("Pontos de ataque: %i\n", monster[monsterCode].atk);
			if (monsterCode == XAOPAI)
				printf("Pontos de ataque: %i (dano mágico)\n", monster[monsterCode].atk);
			else
				printf("Pontos de ataque: %i\n", monster[monsterCode].atk);
			printf("Pontos de defesa: %i\n", monster[monsterCode].def);
		} 
	}

	
 	int showHighscores(){
        int input;
        system("cls");
        //printf("\033[33m");
        printf("          - HIGH SCORES -          \n");

        for (int i = 0; i < HIGHSCORES_NUM - 1; ++i)
            if (highscores[i] != 0)
                printf("              %2i - %i\n", i + 1, highscores[i]);

        //printf("\033[0m");
        printf("\n         (1) Jogar novamente\n");
        printf("         (2) Sair do jogo\n");

        do{
            scanf("%i", &input);
            switch(input){
                case 1:
                    return 1;
                    break;
                case 2:
                    return 0;
                    break;
                case 10:
                	pronoum = 'e';
                	printf("?\n");
                	break;
                case 11:
                	pronoum = 'o';
                	printf("\11\n");
                	break;
                case 12:
                	pronoum = 'a';
                	printf("\12\n");
                	break;
                default:
                	fprintf(stderr,"Comando não encontrado.\n");
                    break;
            }
        }while(input != 1 && input != 2);
    }

    void updatesHighscores(){
        FILE *file;

        file = fopen("Highscores.bin", "rb+");

        // 1st time playing
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

        highscores[HIGHSCORES_NUM - 1] = score;

        sortHighscores(highscores);

        file = fopen("Highscores.bin", "wb");
        if (file == NULL) {
            perror("Error opening file");
            return;
        }

        fwrite(highscores, sizeof(int), HIGHSCORES_NUM, file);
        fclose(file);
    }

    void sortHighscores() {
        for (int i = 0; i < HIGHSCORES_NUM - 1; i++) {
            for (int j = 0; j < HIGHSCORES_NUM - i - 1; j++) {
                if (highscores[j] < highscores[j + 1]) {
                    // Swap the elements if they are in the wrong order
                    int temp = highscores[j];
                    highscores[j] = highscores[j + 1];
                    highscores[j + 1] = temp;
                }
            }
        }
    }