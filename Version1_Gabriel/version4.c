#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <termios.h>
#include <fcntl.h>
#include <time.h>
#include <math.h>

// taille du serpent
#define TAILLE 10
// dimensions du plateau
#define LARGEUR_PLATEAU 80
#define HAUTEUR_PLATEAU 40 //
// position initiale de la tête du serpent 1
#define X_INITIAL 40
#define Y_INITIAL 13
// position initiale de la tête du serpent 2
#define X_INITIAL2 40
#define Y_INITIAL2 27
// nombre de pommes à manger pour gagner
#define NB_POMMES 10
// nombre de pavé
#define NB_PAVES 6
// Taille des pavé
#define TAILLE_PAVES 5
// temporisation entre deux déplacements du serpent (en microsecondes)
#define ATTENTE 50000
// caractères pour représenter le serpent
#define CORPS 'X'
#define TETE '1'
#define TETE2 '2'
// touches de direction ou d'arrêt du jeu
#define HAUT 'z'
#define BAS 's'
#define GAUCHE 'q'
#define DROITE 'd'
#define STOP 'a'
// caractères pour les éléments du plateau
#define BORDURE '#'
#define VIDE ' '
#define POMME '6'

// définition d'un type pour le plateau : tPlateau
// Attention, pour que les indices du tableau 2D (qui commencent à 0) coincident
// avec les coordonées à l'écran (qui commencent à 1), on ajoute 1 aux dimensions
// et on neutralise la ligne 0 et la colonne 0 du tableau 2D (elles ne sont jamais
// utilisées)

typedef char tPlateau[LARGEUR_PLATEAU + 1][HAUTEUR_PLATEAU + 1];

int lesPommesX[NB_POMMES] = {40, 75, 78, 2, 9, 78, 74, 2, 72, 5};
int lesPommesY[NB_POMMES] = {20, 38, 2, 2, 5, 38, 32, 38, 32, 2};

int lesPavesX[NB_PAVES] = { 4, 73, 4, 73, 38, 38};
int lesPavesY[NB_PAVES] = { 4, 4, 33, 33, 14, 22};

void initPlateau(tPlateau plateau);
void dessinerPlateau(tPlateau plateau);
void ajouterPomme(tPlateau plateau, int iPomme);
void afficher(int, int, char);
void effacer(int x, int y);
void dessinerSerpent(int lesX[], int lesY[]);
void dessinerSerpent2(int lesX[], int lesY[]);
void progresser(int lesX[], int lesY[], char direction, tPlateau plateau, bool *collision, bool *pomme);
void progresser2(int lesX[], int lesY[], char direction, tPlateau plateau, bool *collision, bool *pomme);
void gotoxy(int x, int y);
int kbhit();
void disable_echo();
void enable_echo();
char fdirection(int lesX[], int lesY[], int lesPommesX[], int lesPommesY[], int nbPommes, char actudirection, tPlateau plateau, int lesautresX[], int lesautresY[]);
char fdirection2(int lesX[], int lesY[], int lesPommesX[], int lesPommesY[], int nbPommes, char actudirection, tPlateau plateau, int lesautresX[], int lesautresY[]);
bool verifcol(int lesX[], int lesY[], tPlateau plateau, char direction, int lesautresX[], int lesautresY[]);

int main()
{
    clock_t begin = clock();
    // 2 tableaux contenant les positions des éléments qui constituent le serpent 1
    int lesX[TAILLE];
    int lesY[TAILLE];

    // 2 tableaux contenant les positions des éléments qui constituent le serpent 2
    int lesX2[TAILLE];
    int lesY2[TAILLE];

    // représente la touche frappée par l'utilisateur : touche de direction ou pour l'arrêt
    char touche;
    // direction courante du serpent (HAUT, BAS, GAUCHE ou DROITE) pour le serpent 1
    char direction;
    // direction courante du serpent (HAUT, BAS, GAUCHE ou DROITE) pour le serpent 2
    char direction2;
    /*nombre de deplacement du serpent 1 et 2 */
    int nbdep1 = 0;
    int nbdep2 = 0;

    // le plateau de jeu
    tPlateau lePlateau;

    bool collision = false;
    bool gagne = false;
    bool pommeMangee = false;

    // compteur de pommes mangées
    int nbPommes = 0;

    // compteur de pommes mangées serpent 1
    int PommesMange1 = 0;
    // compteur de pommes mangées serpent 2
    int PommesMange2 = 0;

    // initialisation de la position du serpent1 : positionnement de la
    // tête en (X_INITIAL, Y_INITIAL), puis des anneaux à sa gauche
    for (int i = 0; i < TAILLE; i++)
    {
        lesX[i] = X_INITIAL - i;
        lesY[i] = Y_INITIAL;
    }

    // initialisation de la position du serpent2 : positionnement de la
    // tête en (X_INITIAL2, Y_INITIAL2), puis des anneaux à sa droite
    for (int i = 0; i < TAILLE; i++)
    {
        lesX2[i] = X_INITIAL2 + i;
        lesY2[i] = Y_INITIAL2;
    }

    // mise en place du plateau
    initPlateau(lePlateau);
    system("clear");
    dessinerPlateau(lePlateau);

    srand(time(NULL));
    ajouterPomme(lePlateau, nbPommes);

    // initialisation : le serpent1 se dirige vers la DROITE
    dessinerSerpent(lesX, lesY);
    direction = DROITE;
    touche = DROITE;
    // initialisation : le serpent2 se dirige vers la gauche
    dessinerSerpent2(lesX2, lesY2);
    direction2 = GAUCHE;
    disable_echo();

    // boucle de jeu. Arret si touche STOP, si collision avec une bordure ou
    // si toutes les pommes sont mangées
    do
    {
        // pour le serpent 1
        direction = fdirection(lesX, lesY, lesPommesX, lesPommesY, nbPommes, direction, lePlateau, lesX2, lesY2); // définition de la direction
        progresser(lesX, lesY, direction, lePlateau, &collision, &pommeMangee);
        nbdep1++;
        if (pommeMangee)
        { // vérifie si une pomme est mangée
            nbPommes++;
            PommesMange1++;
            gagne = (nbPommes == NB_POMMES);
            if (!gagne)
            {
                ajouterPomme(lePlateau, nbPommes);
                pommeMangee = false;
            }
        }
        if (!gagne)
        { // vérifie si toute les pomme on etait mangée
            if (!collision)
            {
                usleep(ATTENTE);
                if (kbhit() == 1)
                {
                    touche = getchar();
                }
            }
        }
        // pour le serpent 2
        direction = fdirection2(lesX2, lesY2, lesPommesX, lesPommesY, nbPommes, direction2, lePlateau, lesX, lesY); // définition de la direction
        progresser2(lesX2, lesY2, direction, lePlateau, &collision, &pommeMangee);
        nbdep2++;
        if (pommeMangee)
        { // vérifie si une pomme est mangée
            nbPommes++;
            PommesMange2++;
            gagne = (nbPommes == NB_POMMES);
            if (!gagne)
            {
                ajouterPomme(lePlateau, nbPommes);
                pommeMangee = false;
            }
        }
        if (!gagne)
        { // vérifie si toute les pomme on etait mangée
            if (!collision)
            {
                usleep(ATTENTE);
                if (kbhit() == 1)
                {
                    touche = getchar();
                }
            }
        }
    } while (touche != STOP && !collision && !gagne);
    enable_echo();

    gotoxy(1, HAUTEUR_PLATEAU + 1);
    /* calcul du temps CPU */
    clock_t end = clock();
    double tmpsCPU = ((end - begin) * 1.0) / CLOCKS_PER_SEC;
    printf("Temps CPU = %.3f secondes\n", tmpsCPU); // affiche le temps CPU
                                                    // affiche le nombre de déplacement et de pomme mangé par les 2 serpents
    printf("nombre de deplacement du serpent 1 = %d\n nombre de pomme serpent 1 = %d\n", nbdep1, PommesMange1);
    printf("nombre de deplacement du serpent 2 = %d\n nombre de pomme serpent 2= %d\n", nbdep2, PommesMange2);
    return EXIT_SUCCESS;
}

/************************************************/
/*      FONCTIONS ET PROCEDURES DU JEU          */
/************************************************/
void initPlateau(tPlateau plateau)
{

    for (int x = 1; x <= LARGEUR_PLATEAU; x++)
    {
        for (int y = 1; y <= HAUTEUR_PLATEAU; y++)
        {
            if (((x == 1) || (y == 1) || (x == LARGEUR_PLATEAU) || (y == HAUTEUR_PLATEAU)) && (x != LARGEUR_PLATEAU / 2) && (y != HAUTEUR_PLATEAU / 2))
            {
                plateau[x][y] = BORDURE;
            }
            else
            {
                plateau[x][y] = VIDE;
            }
        }
    }
    for (int p = 0; p < NB_PAVES; p++)
    {
        int startX, startY;
        startX = lesPavesX[p];
        startY = lesPavesY[p];
        // Place le pavé
        for (int x = 0; x < TAILLE_PAVES; x++)
        {
            for (int y = 0; y < TAILLE_PAVES; y++)
            {
                plateau[startX + x][startY + y] = BORDURE;
            }
        }
    }
}

void dessinerPlateau(tPlateau plateau)
{
    // affiche à l'écran le contenu du tableau 2D représentant le plateau
    for (int i = 1; i <= LARGEUR_PLATEAU; i++)
    {
        for (int j = 1; j <= HAUTEUR_PLATEAU; j++)
        {
            afficher(i, j, plateau[i][j]);
        }
    }
}

void ajouterPomme(tPlateau plateau, int iPomme)
{
    // génère aléatoirement la position d'une pomme,
    // vérifie que ça correspond à une case vide
    // du plateau puis l'ajoute au plateau et l'affiche
    int xPomme, yPomme;
    xPomme = lesPommesX[iPomme];
    yPomme = lesPommesY[iPomme];
    plateau[xPomme][yPomme] = POMME;
    afficher(xPomme, yPomme, POMME);
}

void afficher(int x, int y, char car)
{
    gotoxy(x, y);
    printf("%c", car);
    gotoxy(1, 1);
}

void effacer(int x, int y)
{
    gotoxy(x, y);
    printf(" ");
    gotoxy(1, 1);
}

void dessinerSerpent(int lesX[], int lesY[])
{
    // affiche les anneaux puis la tête
    for (int i = 1; i < TAILLE; i++)
    {
        afficher(lesX[i], lesY[i], CORPS);
    }
    afficher(lesX[0], lesY[0], TETE);
}

void dessinerSerpent2(int lesX[], int lesY[])
{
    // affiche les anneaux puis la tête
    for (int i = 1; i < TAILLE; i++)
    {
        afficher(lesX[i], lesY[i], CORPS);
    }
    afficher(lesX[0], lesY[0], TETE2);
}

void progresser(int lesX[], int lesY[], char direction, tPlateau plateau, bool *collision, bool *pomme)
{
    // efface le dernier élément avant d'actualiser la position de tous les
    // élémentds du serpent avant de le  redessiner et détecte une
    // collision avec une pomme ou avec une bordure
    effacer(lesX[TAILLE - 1], lesY[TAILLE - 1]);
    for (int i = TAILLE - 1; i > 0; i--)
    {
        lesX[i] = lesX[i - 1];
        lesY[i] = lesY[i - 1];
    }
    // faire progresser la tete dans la nouvelle direction
    switch (direction)
    {
    case HAUT:
        lesY[0] = lesY[0] - 1;
        break;
    case BAS:
        lesY[0] = lesY[0] + 1;
        break;
    case DROITE:
        lesX[0] = lesX[0] + 1;
        break;
    case GAUCHE:
        lesX[0] = lesX[0] - 1;
        break;
    }
    *pomme = false;
    // détection d'une "collision" avec une pomme
    if (plateau[lesX[0]][lesY[0]] == POMME)
    {
        *pomme = true;
        // la pomme disparait du plateau
        plateau[lesX[0]][lesY[0]] = VIDE;
    }
    // détection d'une collision avec la bordure
    else if (plateau[lesX[0]][lesY[0]] == BORDURE)
    {
        *collision = true;
    }
    // les portaux/trou du plateau
    else if ((lesX[0] == LARGEUR_PLATEAU / 2) && (lesY[0] == 0))
    {
        lesY[0] = HAUTEUR_PLATEAU;
    }
    else if ((lesX[0] == LARGEUR_PLATEAU / 2) && (lesY[0] == HAUTEUR_PLATEAU + 1))
    {
        lesY[0] = 1;
    }
    else if ((lesY[0] == HAUTEUR_PLATEAU / 2) && (lesX[0] == 0))
    {
        lesX[0] = LARGEUR_PLATEAU;
    }
    else if ((lesY[0] == HAUTEUR_PLATEAU / 2) && (lesX[0] == LARGEUR_PLATEAU + 1))
    {
        lesX[0] = 1;
    }
    else
    {
        for (int i = 1; i < TAILLE; i++) // collision avec le corp
        {
            if ((lesX[0] == lesX[i]) && (lesY[0] == lesY[i]))
            {
                *collision = true;
            }
        }
    }
    dessinerSerpent(lesX, lesY);
}

char fdirection(int lesX[], int lesY[], int lesPommesX[], int lesPommesY[], int nbPommes, char direction, tPlateau plateau, int lesautresX[], int lesautresY[])
{
    // choisie la direction prendre en fonction du meilleur chemin
    float distance_p, distance_trN, distance_trS, distance_trE, distance_trW;
    int lieuX, lieuY;

    // Calcule des distance
    distance_p = abs((lesX[0] - lesPommesX[nbPommes]) + (lesY[0] - lesPommesY[nbPommes]));

    distance_trN = abs(lesX[0] - (LARGEUR_PLATEAU / 2)) + abs(lesY[0] - 1) +
                   abs(lesPommesX[nbPommes] - (LARGEUR_PLATEAU / 2)) + abs(lesPommesY[nbPommes] - HAUTEUR_PLATEAU);

    distance_trS = abs((lesX[0] - (LARGEUR_PLATEAU / 2))) + abs(lesY[0] - HAUTEUR_PLATEAU) +
                   abs(lesPommesX[nbPommes] - (LARGEUR_PLATEAU / 2)) + abs(lesPommesY[nbPommes] - HAUTEUR_PLATEAU);

    distance_trE = abs(lesX[0] - LARGEUR_PLATEAU) +
                   abs(lesY[0] - (HAUTEUR_PLATEAU / 2)) +
                   abs(lesPommesX[nbPommes] - 1) + abs(lesPommesY[nbPommes] - 1);

    distance_trW = abs(lesX[0] - 1) + abs(lesY[0] - (HAUTEUR_PLATEAU / 2)) +
                   abs(lesPommesX[nbPommes] - LARGEUR_PLATEAU) + abs(lesPommesY[nbPommes] - 1);

    // choisie les coordonnées en fonction du chemin le plus cours
    if (distance_trW <= distance_p)
    {
        lieuX = 0;
        lieuY = HAUTEUR_PLATEAU / 2;
    }
    else if (distance_trE <= distance_p)
    {
        lieuX = LARGEUR_PLATEAU + 1;
        lieuY = HAUTEUR_PLATEAU / 2;
    }
    else if (distance_trN <= distance_p)
    {
        lieuX = LARGEUR_PLATEAU / 2;
        lieuY = 0;
    }
    else if (distance_trS <= distance_p)
    {
        lieuX = LARGEUR_PLATEAU / 2;
        lieuY = HAUTEUR_PLATEAU + 1;
    }
    else
    {
        lieuX = lesPommesX[nbPommes];
        lieuY = lesPommesY[nbPommes];
    }
    // direction dans laquelle le serpent doit aller
    if (lieuY < lesY[0])
    {
        if (verifcol(lesX, lesY, plateau, HAUT, lesautresX, lesautresY) == false)
        {
            direction = HAUT; // vas en haut si pas d'obstacle
        }
        else
        {
            if (lieuX > lesX[0] && (verifcol(lesX, lesY, plateau, DROITE, lesautresX, lesautresY) == false))
            {
                direction = DROITE; // vas à gauche
            }
            else if (verifcol(lesX, lesY, plateau, GAUCHE, lesautresX, lesautresY) == false)
            {
                direction = GAUCHE; // vas à droite
            }
            else if (verifcol(lesX, lesY, plateau, DROITE, lesautresX, lesautresY) == false)
            {
                direction = DROITE;
            }
            else
            {
                direction = BAS;
            }
        }
    }
    else if (lieuY > lesY[0])
    {
        if (verifcol(lesX, lesY, plateau, BAS, lesautresX, lesautresY) == false)
        {
            direction = BAS; // vas en bas si pas d'obstacle
        }
        else
        {
            if (lieuX > lesX[0] && (verifcol(lesX, lesY, plateau, DROITE, lesautresX, lesautresY) == false))
            {
                direction = DROITE; // vas à droite
            }
            else if (verifcol(lesX, lesY, plateau, GAUCHE, lesautresX, lesautresY) == false)
            {
                direction = GAUCHE; // vas à gauche
            }
            else if (verifcol(lesX, lesY, plateau, DROITE, lesautresX, lesautresY) == false)
            {
                direction = DROITE;
            }
            else
            {
                direction = HAUT;
            }
        }
    }
    else if (lieuX < lesX[0])
    {
        if (verifcol(lesX, lesY, plateau, GAUCHE, lesautresX, lesautresY) == false)
        {
            direction = GAUCHE; // vas à gauche si pas d'obstacle
        }
        else
        {
            if (lieuY > lesY[0] && (verifcol(lesX, lesY, plateau, BAS, lesautresX, lesautresY) == false))
            {
                direction = BAS; // vas en bas
            }
            else if (verifcol(lesX, lesY, plateau, HAUT, lesautresX, lesautresY) == false)
            {
                direction = HAUT; // vas en haut
            }
            else if (verifcol(lesX, lesY, plateau, BAS, lesautresX, lesautresY) == false)
            {
                direction = BAS;
            }
            else
            {
                direction = DROITE;
            }
        }
    }
    else if (lieuX > lesX[0])
    {
        if (verifcol(lesX, lesY, plateau, DROITE, lesautresX, lesautresY) == false)
        {
            direction = DROITE; // vas à droite si pas d'obstacle
        }
        else
        {
            if (lieuY > lesY[0] && (verifcol(lesX, lesY, plateau, BAS, lesautresX, lesautresY) == false))
            {
                direction = BAS; // vas en bas
            }
            else if (verifcol(lesX, lesY, plateau, HAUT, lesautresX, lesautresY) == false)
            {
                direction = HAUT; // vas en haut
            }
            else if (verifcol(lesX, lesY, plateau, BAS, lesautresX, lesautresY) == false)
            {
                direction = BAS;
            }
            else
            {
                direction = GAUCHE;
            }
        }
    }
    for (int i = 0; i < NB_PAVES; i++) // chemin optimal en prenant en compte les pavés
    {
        if (((lesX[0] == lesPavesX[i] - 1) && ((lesY[0] == lesPavesY[i] - 1) || (lesY[0] == lesPavesY[i] + TAILLE_PAVES))) || // si c'est un des coins des pavés
            ((lesX[0] == lesPavesX[i] + TAILLE_PAVES) && ((lesY[0] == lesPavesY[i] - 1) || (lesY[0] == lesPavesY[i] + TAILLE_PAVES))))
        {
            if ((lesX[0] > lieuX) && (lesX[0] == lesPavesX[i] + TAILLE_PAVES)) // si c'est un des coins droit du pavé et que l'objectif est a gauche
            {
                if ((lieuY >= lesPavesY[i]) && (lieuY < lesPavesY[i] + TAILLE_PAVES))
                {
                    if (lieuX > lesX[0] && (verifcol(lesX, lesY, plateau, DROITE, lesautresX, lesautresY) == false))
                    {
                        direction = DROITE; // vas à droite
                    }
                    else if (verifcol(lesX, lesY, plateau, GAUCHE, lesautresX, lesautresY) == false)
                    {
                        direction = GAUCHE; // vas à gauche
                    }
                    else if (verifcol(lesX, lesY, plateau, DROITE, lesautresX, lesautresY) == false)
                    {
                        direction = DROITE;
                    }
                }
            }
            else if ((lesX[0] == lesPavesX[i] - 1) && (lesX[0] < lieuX)) // si c'est un des coins gauches du pavé et que l'objectif est a droite
            {
                if ((lieuY >= lesPavesY[i]) && (lieuY < lesPavesY[i] + TAILLE_PAVES))
                {
                    if (lieuX > lesX[0] && (verifcol(lesX, lesY, plateau, DROITE, lesautresX, lesautresY) == false))
                    {
                        direction = DROITE; // vas à droite
                    }
                    else if (verifcol(lesX, lesY, plateau, GAUCHE, lesautresX, lesautresY) == false)
                    {
                        direction = GAUCHE; // vas à gauche
                    }
                    else if (verifcol(lesX, lesY, plateau, DROITE, lesautresX, lesautresY) == false)
                    {
                        direction = DROITE;
                    }
                }
            }
        }
    }
    return direction;
}

void progresser2(int lesX[], int lesY[], char direction, tPlateau plateau, bool *collision, bool *pomme)
{
    // efface le dernier élément avant d'actualiser la position de tous les
    // élémentds du serpent avant de le  redessiner et détecte une
    // collision avec une pomme ou avec une bordure
    effacer(lesX[TAILLE - 1], lesY[TAILLE - 1]);

    for (int i = TAILLE - 1; i > 0; i--)
    {
        lesX[i] = lesX[i - 1];
        lesY[i] = lesY[i - 1];
    }
    // faire progresser la tete dans la nouvelle direction
    switch (direction)
    {
    case HAUT:
        lesY[0] = lesY[0] - 1;
        break;
    case BAS:
        lesY[0] = lesY[0] + 1;
        break;
    case DROITE:
        lesX[0] = lesX[0] + 1;
        break;
    case GAUCHE:
        lesX[0] = lesX[0] - 1;
        break;
    }
    *pomme = false;
    // détection d'une "collision" avec une pomme
    if (plateau[lesX[0]][lesY[0]] == POMME)
    {
        *pomme = true;
        // la pomme disparait du plateau
        plateau[lesX[0]][lesY[0]] = VIDE;
    }
    // détection d'une collision avec la bordure
    else if (plateau[lesX[0]][lesY[0]] == BORDURE)
    {
        *collision = true;
    }
    else if ((lesX[0] == LARGEUR_PLATEAU / 2) && (lesY[0] == 0))
    {
        lesY[0] = HAUTEUR_PLATEAU;
    }
    else if ((lesX[0] == LARGEUR_PLATEAU / 2) && (lesY[0] == HAUTEUR_PLATEAU + 1))
    {
        lesY[0] = 1;
    }
    else if ((lesY[0] == HAUTEUR_PLATEAU / 2) && (lesX[0] == 0))
    {
        lesX[0] = LARGEUR_PLATEAU;
    }
    else if ((lesY[0] == HAUTEUR_PLATEAU / 2) && (lesX[0] == LARGEUR_PLATEAU + 1))
    {
        lesX[0] = 1;
    }
    else
    {
        for (int i = 1; i < TAILLE; i++) // collision avec le corp
        {
            if ((lesX[0] == lesX[i]) && (lesY[0] == lesY[i]))
            {
                *collision = true;
            }
        }
    }
    dessinerSerpent2(lesX, lesY);
}

char fdirection2(int lesX[], int lesY[], int lesPommesX[], int lesPommesY[], int nbPommes, char direction, tPlateau plateau, int lesautresX[], int lesautresY[])
{
    // choisie la direction prendre en fonction du meilleur chemin
    float distance_p, distance_trN, distance_trS, distance_trE, distance_trW;
    int lieuX, lieuY;

    // Calcule des distance
    distance_p = abs((lesX[0] - lesPommesX[nbPommes]) + (lesY[0] - lesPommesY[nbPommes]));

    distance_trN = abs(lesX[0] - (LARGEUR_PLATEAU / 2)) + abs(lesY[0] - 1) +
                   abs(lesPommesX[nbPommes] - (LARGEUR_PLATEAU / 2)) + abs(lesPommesY[nbPommes] - HAUTEUR_PLATEAU);

    distance_trS = abs((lesX[0] - (LARGEUR_PLATEAU / 2))) + abs(lesY[0] - HAUTEUR_PLATEAU) +
                   abs(lesPommesX[nbPommes] - (LARGEUR_PLATEAU / 2)) + abs(lesPommesY[nbPommes] - HAUTEUR_PLATEAU);

    distance_trE = abs(lesX[0] - LARGEUR_PLATEAU) +
                   abs(lesY[0] - (HAUTEUR_PLATEAU / 2)) +
                   abs(lesPommesX[nbPommes] - 1) + abs(lesPommesY[nbPommes] - 1);

    distance_trW = abs(lesX[0] - 1) + abs(lesY[0] - (HAUTEUR_PLATEAU / 2)) +
                   abs(lesPommesX[nbPommes] - LARGEUR_PLATEAU) + abs(lesPommesY[nbPommes] - 1);

    // choisie les coordonnées en fonction du chemin le plus cours
    if (distance_trW <= distance_p)
    {
        lieuX = 0;
        lieuY = HAUTEUR_PLATEAU / 2;
    }
    else if (distance_trE <= distance_p)
    {
        lieuX = LARGEUR_PLATEAU + 1;
        lieuY = HAUTEUR_PLATEAU / 2;
    }
    else if (distance_trN <= distance_p)
    {
        lieuX = LARGEUR_PLATEAU / 2;
        lieuY = 0;
    }
    else if (distance_trS <= distance_p)
    {
        lieuX = LARGEUR_PLATEAU / 2;
        lieuY = HAUTEUR_PLATEAU + 1;
    }
    else
    {
        lieuX = lesPommesX[nbPommes];
        lieuY = lesPommesY[nbPommes];
    }
    // direction dans laquelle le serpent doit aller
    if (lieuY < lesY[0])
    {
        if (verifcol(lesX, lesY, plateau, HAUT, lesautresX, lesautresY) == false)
        {
            direction = HAUT; // vas en haut si pas d'obstacle
        }
        else
        {
            if (lieuX > lesX[0] && (verifcol(lesX, lesY, plateau, DROITE, lesautresX, lesautresY) == false))
            {
                direction = DROITE; // vas à gauche
            }
            else if (verifcol(lesX, lesY, plateau, GAUCHE, lesautresX, lesautresY) == false)
            {
                direction = GAUCHE; // vas à droite
            }
            else if (verifcol(lesX, lesY, plateau, DROITE, lesautresX, lesautresY) == false)
            {
                direction = DROITE;
            }
            else
            {
                direction = BAS;
            }
        }
    }
    else if (lieuY > lesY[0])
    {
        if (verifcol(lesX, lesY, plateau, BAS, lesautresX, lesautresY) == false)
        {
            if ((lesX[0] == 39) && (lesY[0] == 20))
            {
                direction = HAUT;
            }
            else
            {
                direction = BAS; // vas en bas si pas d'obstacle
            }
        }
        else
        {
            if ((lesX[0] == 39) && (lesY[0] == 19))
            {
                direction = GAUCHE;
            }
            else if (lieuX > lesX[0] && (verifcol(lesX, lesY, plateau, DROITE, lesautresX, lesautresY) == false))
            {
                direction = DROITE; // vas à droite
            }
            else if (verifcol(lesX, lesY, plateau, GAUCHE, lesautresX, lesautresY) == false)
            {
                direction = GAUCHE; // vas à gauche
            }
            else if (verifcol(lesX, lesY, plateau, DROITE, lesautresX, lesautresY) == false)
            {
                direction = DROITE;
            }
            else 
            {
                direction = HAUT;
            }
        }
    }
    else if (lieuX < lesX[0])
    {
        if (verifcol(lesX, lesY, plateau, GAUCHE, lesautresX, lesautresY) == false)
        {
            direction = GAUCHE; // vas à gauche si pas d'obstacle
        }
        else
        {
            if (lieuY > lesY[0] && (verifcol(lesX, lesY, plateau, BAS, lesautresX, lesautresY) == false))
            {
                direction = BAS; // vas en bas
            }
            else if (verifcol(lesX, lesY, plateau, HAUT, lesautresX, lesautresY) == false)
            {
                direction = HAUT; // vas en haut
            }
            else if (verifcol(lesX, lesY, plateau, BAS, lesautresX, lesautresY) == false)
            {
                direction = BAS;
            }
            else
            {
                direction = DROITE;
            }
        }
    }
    else if (lieuX > lesX[0])
    {
        if (verifcol(lesX, lesY, plateau, DROITE, lesautresX, lesautresY) == false)
        {
            direction = DROITE; // vas à droite si pas d'obstacle
        }
        else
        {
            if (lieuY > lesY[0] && (verifcol(lesX, lesY, plateau, BAS, lesautresX, lesautresY) == false))
            {
                direction = BAS; // vas en bas
            }
            else if (verifcol(lesX, lesY, plateau, HAUT, lesautresX, lesautresY) == false)
            {
                direction = HAUT; // vas en haut
            }
            else if (verifcol(lesX, lesY, plateau, BAS, lesautresX, lesautresY) == false)
            {
                direction = BAS;
            }
            else
            {
                direction = GAUCHE;
            }
        }
    }
    for (int i = 0; i < NB_PAVES; i++) // chemin optimal en prenant en compte les pavés
    {
        if (((lesX[0] == lesPavesX[i] - 1) && ((lesY[0] == lesPavesY[i] - 1) || (lesY[0] == lesPavesY[i] + TAILLE_PAVES))) || // si c'est un des coins des pavés
            ((lesX[0] == lesPavesX[i] + TAILLE_PAVES) && ((lesY[0] == lesPavesY[i] - 1) || (lesY[0] == lesPavesY[i] + TAILLE_PAVES))))
        {
            if ((lesX[0] > lieuX) && (lesX[0] == lesPavesX[i] + TAILLE_PAVES)) // si c'est un des coins droit du pavé et que l'objectif est a gauche
            {
                if ((lieuY >= lesPavesY[i]) && (lieuY < lesPavesY[i] + TAILLE_PAVES))
                {
                    if (lieuX > lesX[0] && (verifcol(lesX, lesY, plateau, DROITE, lesautresX, lesautresY) == false))
                    {
                        direction = DROITE; // vas à droite
                    }
                    else if (verifcol(lesX, lesY, plateau, GAUCHE, lesautresX, lesautresY) == false)
                    {
                        direction = GAUCHE; // vas à gauche
                    }
                    else if (verifcol(lesX, lesY, plateau, DROITE, lesautresX, lesautresY) == false)
                    {
                        direction = DROITE;
                    }
                }
            }
            else if ((lesX[0] == lesPavesX[i] - 1) && (lesX[0] < lieuX)) // si c'est un des coins gauches du pavé et que l'objectif est a droite
            {
                if ((lieuY >= lesPavesY[i]) && (lieuY < lesPavesY[i] + TAILLE_PAVES))
                {
                    if (lieuX > lesX[0] && (verifcol(lesX, lesY, plateau, DROITE, lesautresX, lesautresY) == false))
                    {
                        direction = DROITE; // vas à droite
                    }
                    else if (verifcol(lesX, lesY, plateau, GAUCHE, lesautresX, lesautresY) == false)
                    {
                        direction = GAUCHE; // vas à gauche
                    }
                    else if (verifcol(lesX, lesY, plateau, DROITE, lesautresX, lesautresY) == false)
                    {
                        direction = DROITE;
                    }
                }
            }
        }
    }
    return direction;
}
/************************************************/
/*               FONCTIONS UTILITAIRES          */
/************************************************/
void gotoxy(int x, int y)
{
    printf("\033[%d;%df", y, x);
}

int kbhit()
{
    // la fonction retourne :
    // 1 si un caractere est present
    // 0 si pas de caractere présent
    int unCaractere = 0;
    struct termios oldt, newt;
    int ch;
    int oldf;

    // mettre le terminal en mode non bloquant
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);

    ch = getchar();

    // restaurer le mode du terminal
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    fcntl(STDIN_FILENO, F_SETFL, oldf);

    if (ch != EOF)
    {
        ungetc(ch, stdin);
        unCaractere = 1;
    }
    return unCaractere;
}

bool verifcol(int lesX[], int lesY[], tPlateau plateau, char direction, int lesautresX[], int lesautresY[])
{
    // vérifie si il y a un mur dans une direction donné
    bool col = false;
    if (direction == DROITE && plateau[lesX[0] + 1][lesY[0]] == BORDURE) // à droite
    {
        col = true;
    }
    else if (direction == GAUCHE && plateau[lesX[0] - 1][lesY[0]] == BORDURE) // à gauche
    {
        col = true;
    }
    else if (direction == BAS && plateau[lesX[0]][lesY[0] + 1] == BORDURE) // en bas
    {
        col = true;
    }
    else if (direction == HAUT && plateau[lesX[0]][lesY[0] - 1] == BORDURE) // en haut
    {
        col = true;
    }
    else
    {
        for (int i = 1; i < TAILLE; i++) // collision avec lui même
        {
            if ((direction == DROITE) && ((lesX[0] + 1 == lesX[i]) && (lesY[0] == lesY[i]))) // à droite
            {
                col = true;
            }
            if ((direction == GAUCHE) && ((lesX[0] - 1 == lesX[i]) && (lesY[0] == lesY[i]))) // à droite
            {
                col = true;
            }
            if ((direction == BAS) && ((lesX[0] == lesX[i]) && (lesY[0] + 1 == lesY[i]))) // à droite
            {
                col = true;
            }
            if ((direction == HAUT) && ((lesX[0 + 1] == lesX[i]) && (lesY[0] - 1 == lesY[i]))) // à droite
            {
                col = true;
            }
        }
        for (int i = 0; i < TAILLE; i++) // collision avec le corp de l'autre serpent
        {
            if ((direction == DROITE) && ((lesX[0] + 1 == lesautresX[i]) && (lesY[0] == lesautresY[i]))) // à droite
            {
                col = true;
            }
            if ((direction == GAUCHE) && ((lesX[0] - 1 == lesautresX[i]) && (lesY[0] == lesautresY[i]))) // à droite
            {
                col = true;
            }
            if ((direction == BAS) && ((lesX[0] == lesautresX[i]) && (lesY[0] + 1 == lesautresY[i]))) // à droite
            {
                col = true;
            }
            if ((direction == HAUT) && ((lesX[0 + 1] == lesautresX[i]) && (lesY[0] - 1 == lesautresY[i]))) // à droite
            {
                col = true;
            }
        }
    }
    return col;
}

// Fonction pour désactiver l'echo
void disable_echo()
{
    struct termios tty;

    // Obtenir les attributs du terminal
    if (tcgetattr(STDIN_FILENO, &tty) == -1)
    {
        perror("tcgetattr");
        exit(EXIT_FAILURE);
    }

    // Désactiver le flag ECHOdistance_p
    tty.c_lflag &= ~ECHO;

    // Appliquer les nouvelles configurations
    if (tcsetattr(STDIN_FILENO, TCSANOW, &tty) == -1)
    {
        perror("tcsetattr");
        exit(EXIT_FAILURE);
    }
}

// Fonction pour réactiver l'echo
void enable_echo()
{
    struct termios tty;

    // Obtenir les attributs du terminal
    if (tcgetattr(STDIN_FILENO, &tty) == -1)
    {
        perror("tcgetattr");
        exit(EXIT_FAILURE);
    }

    // Réactiver le flag ECHO
    tty.c_lflag |= ECHO;

    // Appliquer les nouvelles configurations
    if (tcsetattr(STDIN_FILENO, TCSANOW, &tty) == -1)
    {
        perror("tcsetattr");
        exit(EXIT_FAILURE);
    }
}
