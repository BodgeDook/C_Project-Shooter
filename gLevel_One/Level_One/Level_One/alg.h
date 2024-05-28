// alg.h
#ifndef ALG_H
#define ALG_H

#include <stdio.h>
#include <stdlib.h>

#define ROWS 3
#define COLS 4
#define MW 1080
#define MH 720

extern const int W;         // ширина рабочего поля
extern const int H;         // высота рабочего поля
extern const int WALL;      // непроходимая ячейка
extern const int BLANK;     // свободная непомеченная ячейка

extern int px[1080 * 720], py[1080 * 720];

typedef struct enemy {
    int x; // currrent x
    int y; // current y
    int isTriggered; //if 1 fire
    int isKilled; //if 1 is killed
} enemy;

typedef struct Player_Position {
    int x;
    int y;
    int isKilled;
} Player_Position;

typedef struct path {
    int x[MW * 2];
    int y[MH * 2];
    int len;
} path;

// Функции, объявленные в alg.c
int lee(int** grid, int ax, int ay, int bx, int by, int iter);
path enemyToHero(enemy e1, Player_Position h1, int grid[720][1080]);

#endif // ALG_H
