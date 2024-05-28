// alg.h
#ifndef ALG_H
#define ALG_H

#include <stdio.h>
#include <stdlib.h>

#define ROWS 3
#define COLS 4
#define MW 1080
#define MH 720

extern const int W;         // ширина рабочего пол€
extern const int H;         // высота рабочего пол€
extern const int WALL;      // непроходима€ €чейка
extern const int BLANK;     // свободна€ непомеченна€ €чейка

extern int px[1080 * 720], py[1080 * 720];

typedef struct enemy {
    int x; // current x
    int y; // current y
    int isTriggered; //if found the player
    int isKilled; //if is killed (3 times to shoot)
} enemy;

typedef struct Player_Position {
    int x;
    int y;
    // int h;
} Player_Position;

typedef struct path {
    int x[MW*2];
    int y[MH*2];
    int len;
} path;

// ‘ункции, объ€вленные в alg.c
int lee(int** grid, int ax, int ay, int bx, int by, int iter);
path enemyToHero(enemy e1, Player_Position h1);

#endif // ALG_H
