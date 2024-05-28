#include <stdio.h>
#include <stdlib.h>
#include "alg.h"

#define MW 1080
#define MH 720

const int W = 1080;         // ширина рабочего поля
const int H = 720;         // высота рабочего поля
const int WALL = -1;         // непроходимая ячейка
const int BLANK = -2;         // свободная непомеченная ячейка

int px[1080 * 720], py[1080 * 720];

path p1[22];

int lee(int grid[720][1080], int ax, int ay, int bx, int by, int iter)   // поиск пути из ячейки (ax, ay) в ячейку (bx, by)
{
    int len = 0;
    //1078, 360

    int dx[4] = { 1, 0, -1, 0 };
    int dy[4] = { 0, 1, 0, -1 };   // справа, снизу, слева и сверху
    int d = 0, x = 0, y = 0, k = 0;
    int stop = 0;

    if (grid[ay][ax] == WALL || grid[by][bx] == WALL) return -1;  // ячейка (ax, ay) или (bx, by) - стена


    // распространение волны
    d = 0;
    grid[ay][ax] = 0;

    do {
        stop = 1;
        for (y = 0; y < H; y++)
            for (x = 0; x < W; x++)
                if (grid[y][x] == d)
                {
                    for (k = 0; k < 4; ++k)
                    {
                        int iy = y + dy[k], ix = x + dx[k];
                        if (iy >= 0 && iy < H && ix >= 0 && ix < W && grid[iy][ix] == BLANK)
                        {
                            stop = 0;
                            grid[iy][ix] = d + 1;
                        }
                    }
                }

        d++;

    } while (!stop && grid[by][bx] == BLANK);

    if (grid[by][bx] == BLANK) return -2;  // путь не найден

    // восстановление пути
    len = grid[by][bx];
    x = bx;
    y = by;
    d = len;
    while (d > 0)
    {
        p1[iter].x[d] = x;
        p1[iter].y[d] = y;
        d--;
        for (k = 0; k < 4; ++k)
        {
            int iy = y + dy[k], ix = x + dx[k];
            if (iy >= 0 && iy < H && ix >= 0 && ix < W && grid[iy][ix] == d)
            {
                x = x + dx[k];
                y = y + dy[k];
                break;
            }
        }
    }

    p1[iter].x[0] = ax;
    p1[iter].y[0] = ay;

    return len;
}

path enemyToHero(enemy e1, Player_Position h1, int grid[720][1080]) {//В функцию нужно передать врага и героя

    //int** grid = (int**)malloc((MH + 10) * sizeof(int*) + 11);
    //for (int i = 0; i < MH + 10; i++) grid[i] = (int*)malloc(MW * sizeof(int) + 10);

    //for (int i = 0; i < H; i++) {
      //  for (int j = 0; j < W; j++) grid[i][j] = -2;
    //}//инициализация пустой карты

    //Далее идёт рассчёт предполагаемого пути персонажа к выходу
    int heroSpeed = 10;
    int ggExit = lee(grid, h1.x, h1.y, 1015, 480, 0);  // внимание, для корректного рассчёта пути необходимо заранее инициализировать карту
    p1[0].len = ggExit;
    if (ggExit == -1) {
        printf("Path blocked\n");
        return p1[0];
    }

    if (ggExit == -2) {
        printf("Path not found\n");
        return p1[0];
    }

    printf("Player to exit dist = %d; hero cord x:%d y:%d\n", ggExit, h1.x, h1.y);
    //printf("PATH hero to exit:\n");
    //for (int i = 0; i <= ggExit; i++) printf("%d %d\n", p1[0].x[i], p1[0].y[i]);//Вывод предполагемог пути персонажа к выходу
    // 
    //Очистка карты
    for (int i = 0; i < H; i++) {
        for (int j = 0; j < W; j++) if (grid[i][j] != WALL) grid[i][j] = -2;
    }

    //Далее идёт рассчёт пути на перехват персонажа врагом
    int heroInterceptionDist[21] = { 0 };
    int contact[21];
    for (int i = 1; i < 21; i++) {
        int heroInterceptionX = p1[0].x[heroSpeed * i];
        int heroInterceptionY = p1[0].y[heroSpeed * i];
        heroInterceptionDist[i] = lee(grid, e1.x, e1.y, heroInterceptionX, heroInterceptionY, i);
        //if (heroInterceptionDist[i] == -2) printf("%d - Path not found - 2. From x:%d y:%d to x:%d y:%d\n", i, e1.x, e1.y, heroInterceptionX, heroInterceptionY);
        p1[i].len = heroInterceptionDist[i];
        contact[i] = i - heroInterceptionDist[i] / 10;
        //printf("heroInterceptionDist:%d i:%d contact[%d]:%d\n", heroInterceptionDist[i], i, i, contact[i]);//Отладка нахождения пути враг - игрок
        for (int i = 0; i < H; i++) {
            for (int j = 0; j < W; j++) if (grid[i][j] != WALL) grid[i][j] = -2;
        }
    }
    int min = 20;
    for (int i = 1; i < 21; i++) {
        if (contact[min] > contact[i] && contact[i] >= 0) {
            //printf("YES: %d %d > %d\n", i, min, contact[i]);
            min = i;
        }
    }

    if (min < 0) {
        printf("Min error %d\n", min);
        min = 1;
    }
    //printf("Min = %d; Enemy to hero dist = %d\n", min, heroInterceptionDist[min]);


    return p1[min];
}