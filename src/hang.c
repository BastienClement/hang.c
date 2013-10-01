#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <stdbool.h>
#include <windows.h>

#include "tinymt32.h"

#define WORD_LEN 50
#define forever while(true)

/*
 * Affiche le visuel du pendu en fonction du nombre d'erreurs
 */
void hangman(int errors) {
	char hangman[200] = "";

#define HANGMAN(err, t, f) strcat(hangman, errors > err ? t : f)
#define HANGMAN_IF(err, t) if(errors > err) strcat(hangman, t)

	HANGMAN(2, "    -------\n", "");

	HANGMAN(1 , "    |" , "");
	HANGMAN(3 , "/    " , "");
	HANGMAN(4 , "|" , "");
	HANGMAN_IF(1, "\n");

	HANGMAN(1 , "    |     " , "");
	HANGMAN(5 , "o" , "");
	HANGMAN_IF(1, "\n");

	HANGMAN(1 , "    |    " , "");
	HANGMAN(7 , "/" , " ");
	HANGMAN(6 , "#" , " ");
	HANGMAN(8 , "\\" , "");
	HANGMAN_IF(1, "\n");

	HANGMAN(1 , "    |    " , "");
	HANGMAN(9 , "/ " , "  ");
	HANGMAN(10 , "\\" , "");
	HANGMAN_IF(1, "\n");

	HANGMAN(1 , "    |" , "");
	HANGMAN_IF(1, "\n");

	HANGMAN(0 , "  __" , "");
	HANGMAN(1 , "|" , "");
	HANGMAN(0 , "__\n" , "\n");

	strcat(hangman, "\n");

	HANGMAN_IF(1,  "    \\o\\");
	HANGMAN_IF(3,  "  \\o/");
	HANGMAN_IF(5,  "  /o/");
	HANGMAN_IF(7,  "  \\o\\");
	HANGMAN_IF(9,  "  \\o/");
	HANGMAN_IF(0,  "\n");

	HANGMAN_IF(0,  " \\o/");
	HANGMAN_IF(2,  "  /o/");
	HANGMAN_IF(4,  "  \\o\\");
	HANGMAN_IF(6,  "  \\o/");
	HANGMAN_IF(8,  "  \\o\\");
	HANGMAN_IF(10, "  \\o/");
	HANGMAN_IF(0,  "\n");

	printf(hangman);
}

/*
 * Sélectionne un mot aléatoire dans une liste de mots
 */
void select_word(FILE* fp, char *word, tinymt32_t *tinymt, int len) {
	float random = tinymt32_generate_float(tinymt);
	int   offset = random * len;
	int   end = offset;

	char buf;

	// Fin du mot
	fseek(fp, offset, SEEK_SET);
	do {
		fread(&buf, 1, 1, fp);
		end++;
	} while(buf && buf != '\n');

	// Début du mot
	do {
		if(offset < 0) {
			break;
		}
		fseek(fp, --offset, SEEK_SET);
		fread(&buf, 1, 1, fp);
	} while(buf && buf != '\n');
	offset++;

	fseek(fp, offset, SEEK_SET);

	// Vérification de la longueur du mot et lecture dans le fichier
	if(end-offset > WORD_LEN) {
		word[0] = '\0';
	} else {
		fread(word, 1, end-offset, fp);
		word[end-offset-1] = '\0';
	}
}

/*
 * Retire les diacritiques d'un mot
 * Mot de WORD_LEN caractères maximum
 */
void simplify_word(char *word, char *word_simple) {
	for(int i = 0; i < WORD_LEN; i++) {
		char c = word[i];
		switch(c) {
			case 'à':
			case 'â':
			case 'ä':
				c = 'a';
				break;

			case 'ç':
				c = 'c';
				break;

			case 'è':
			case 'é':
			case 'ê':
			case 'ë':
				c = 'e';
				break;

			case 'î':
			case 'ï':
				c = 'i';
				break;

			case 'ô':
			case 'ö':
				c = 'o';
				break;

			case 'û':
			case 'ü':
				c = 'u';
				break;
		}

		if(c > 'z') {
			printf("ERROR: BAD CHAR '%c'\n", c);
		}

		word_simple[i] = c;

		// Fin du mot
		if(!c) {
			break;
		}
	}
}

/*
 * Changement de casse
 */
char toUpper(char c) {
	if(c > 96 && c < 123) {
		c -= 32;
	}
	return c;
}

/*
 * "Efface" la console
 */
void cclear() {
	// FIXME ...
	printf(
		"\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n"
		"\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n"
		"\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n"
		"\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n"
		"\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n"
	);
}

/*
 * Une lettre à deviner
 */
typedef struct {
	char chr;		// La lettre accentuée (affichage)
	char simple;	// La lettre non-accentuée (test)
	bool unknown;	// Lettre encore inconnue ou non ?
} hang_letter;

/*
 * Main
 */
int main() {
	// Fichier de mtos
	FILE *fp = fopen("words.txt", "r");

	// Calcul de la longueur du fichier
	fseek(fp, 0, SEEK_END);
	int len = ftell(fp);

	// Initialisation du générateur d'aléatoire
	tinymt32_t tinymt;
	tinymt32_init(&tinymt, time(NULL));

	forever {
		// Buffer pour le mot sélectionné
		// Requiert au moins 4 lettres
		char word[WORD_LEN];
		do {
			select_word(fp, word, &tinymt, len);
		} while(strlen(word) < 4);

		// Simplification du mot
		char word_simple[WORD_LEN];
		simplify_word(word, word_simple);

		// Nombre de lettres encore inconnues
		int unknown_left = 0;
		hang_letter hang_letters[50];

		// Construction du tableau de 'hang_letter'
		for(int i = 0; i < 50; i++) {
			hang_letter l = {toUpper(word_simple[i]), word_simple[i], true};
			hang_letters[i] = l;

			if(l.chr == '-') {
				// Le tiret est découvert d'office
				hang_letters[i].unknown = false;
			} else if(l.chr) {
				// Toutes les autres lettres sont à deviner
				unknown_left++;
			} else {
				// Fin du mot
				break;
			}
		}

		// Nombre d'erreurs et lettres demandées
		int errors = 0;
		bool guess_letters[26] = {
			false, false, false, false,
			false, false, false, false,
			false, false, false, false,
			false, false, false, false,
			false, false, false, false,
			false, false, false, false,
			false, false  // x 26
		};

		do {
			// Effacement de la console
			cclear();	// FIXME

			// Affichage du pendu
			hangman(errors);

			// Trop d'erreurs
			if(errors >= 11) {
				break;
			}

			printf("\n\n");

			// Lettres connues et inconnues du mot
			for(int i = 0; hang_letters[i].chr; i++) {
				printf(" %c", hang_letters[i].unknown ? '_' : hang_letters[i].chr);
			}

			printf("\n\n\n\n");

			// List des lettres déjà demandées
			for(int i = 0; i < 26; i++) {
				if(guess_letters[i]) {
					printf(" %c", i+97);
				}
			}

			printf("\n\n> ");

			// Lecture de l'entrée
			char guess;
			if(scanf("%c", &guess) == EOF) {
				break;
			}

			if(guess < 97 || guess > 122 || guess_letters[guess-97]) {
				if(guess == '$') {
					// Abandon
					errors = 100;
				} else {
					continue;
				}
			} else {
				guess_letters[guess-97] = true;
			}

			// Check
			bool guessed = false;
			for(int i = 0; hang_letters[i].chr; i++) {
				if(hang_letters[i].unknown && hang_letters[i].simple == guess) {
					hang_letters[i].unknown = false;
					unknown_left--;
					guessed = true;
				}
			}

			// Aucune lettre correspondante
			if(!guessed) {
				errors++;
			}
		} while(unknown_left);

		printf("\n\n");

		// Affichage final du mot
		for(int i = 0; hang_letters[i].chr; i++) {
			printf(" %c", hang_letters[i].chr);
		}

		printf("\n\n");

		Sleep(5000);
	}
}
