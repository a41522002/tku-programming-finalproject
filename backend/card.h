#ifndef CARD_H
#define CARD_H

#define DECK_SIZE 52   /* 一副牌 52 張 */
#define HAND_SIZE 5    /* 每位玩家手牌 5 張 */

/* 一張牌的結構 */
typedef struct {
    int rank;   /* 點數: 2~14 (J=11, Q=12, K=13, A=14) */
    int suit;   /* 花色: 0=黑桃, 1=紅心, 2=方塊, 3=梅花 */
} Card;

/* 牌堆相關函式 */
Card* create_deck(void);          /* malloc 配置 52 張牌 */
void  shuffle_deck(Card* deck);   /* 洗牌 */
void  free_deck(Card* deck);      /* free 釋放牌堆 */

/* 顯示用 */
const char* suit_name(int suit);
const char* rank_name(int rank);

#endif
