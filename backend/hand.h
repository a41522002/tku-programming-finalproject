#ifndef HAND_H
#define HAND_H

#include "card.h"

/* 牌型由小到大編號，數字越大越強 */
typedef enum {
    HIGH_CARD   = 1,   /* 散牌 */
    ONE_PAIR    = 2,   /* 一對 */
    TWO_PAIR    = 3,   /* 兩對 */
    THREE_KIND  = 4,   /* 三條 */
    STRAIGHT    = 5,   /* 順子 */
    FLUSH       = 6,   /* 同花 */
    FULL_HOUSE  = 7,   /* 葫蘆 */
    FOUR_KIND   = 8,   /* 四條 */
    STRAIGHT_FLUSH = 9 /* 同花順 */
} HandRank;

/* 一手牌的結構 */
typedef struct {
    Card*    cards;       /* 用 malloc 配置 5 張牌 */
    HandRank rank;        /* 牌型 */
    int      high;        /* 比大小用：最大牌的點數 */
} Hand;

Hand* create_hand(const Card* src, int n);
void  evaluate_hand(Hand* h);
int   compare_hands(const Hand* a, const Hand* b);
void  free_hand(Hand* h);
const char* hand_rank_name(HandRank r);

#endif
