#include <stdlib.h>
#include <time.h>
#include "card.h"

/* 動態配置一副 52 張的牌堆 */
Card* create_deck(void) {
    /* 用 malloc 在 heap 配置記憶體，回傳 Card 指標 */
    Card* deck = (Card*)malloc(sizeof(Card) * DECK_SIZE);
    if (deck == NULL) return NULL;   /* 配置失敗檢查 */

    /* 初始化 4 種花色 × 13 種點數 = 52 張 */
    int i = 0;
    for (int s = 0; s < 4; s++) {
        for (int r = 2; r <= 14; r++) {
            deck[i].suit = s;
            deck[i].rank = r;
            i++;
        }
    }
    return deck;
}

/* 洗牌：Fisher-Yates 從後面隨機交換 */
void shuffle_deck(Card* deck) {
    for (int i = DECK_SIZE - 1; i > 0; i--) {
        int j = rand() % (i + 1);   /* 隨機選一張 */
        Card tmp = deck[i];          /* 交換兩張牌 */
        deck[i] = deck[j];
        deck[j] = tmp;
    }
}

/* 釋放牌堆記憶體 */
void free_deck(Card* deck) {
    free(deck);
}

const char* suit_name(int suit) {
    const char* names[] = {"Spade", "Heart", "Diamond", "Club"};
    if (suit < 0 || suit > 3) return "?";
    return names[suit];
}

const char* rank_name(int rank) {
    static const char* names[] = {
        "?", "?", "2", "3", "4", "5", "6", "7", "8", "9",
        "10", "J", "Q", "K", "A"
    };
    if (rank < 2 || rank > 14) return "?";
    return names[rank];
}
