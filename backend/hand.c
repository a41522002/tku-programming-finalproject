#include <stdlib.h>
#include <string.h>
#include "hand.h"

/* 建立一手牌：配置 Hand 結構與裡面的 cards 陣列 */
Hand* create_hand(const Card* src, int n) {
    Hand* h = (Hand*)malloc(sizeof(Hand));         /* 配置 Hand 本體 */
    h->cards = (Card*)malloc(sizeof(Card) * n);    /* 配置 cards 陣列 */
    /* 把 src 的牌複製進來 */
    for (int i = 0; i < n; i++) {
        h->cards[i] = src[i];
    }
    h->rank = HIGH_CARD;
    h->high = 0;
    return h;
}

/* 釋放一手牌：先 free cards，再 free Hand 本體 */
void free_hand(Hand* h) {
    free(h->cards);   /* 先釋放裡面的陣列 */
    free(h);          /* 再釋放結構本身 */
}

/* 評估牌型：用最直觀的方式檢查同花、順子、配對 */
void evaluate_hand(Hand* h) {
    /* 統計每個點數出現幾次 (索引 2~14) */
    int count[15] = {0};
    int max_rank = 0;
    for (int i = 0; i < HAND_SIZE; i++) {
        count[h->cards[i].rank]++;
        if (h->cards[i].rank > max_rank) {
            max_rank = h->cards[i].rank;
        }
    }
    h->high = max_rank;

    /* 檢查同花：5 張花色都一樣 */
    int is_flush = 1;
    for (int i = 1; i < HAND_SIZE; i++) {
        if (h->cards[i].suit != h->cards[0].suit) {
            is_flush = 0;
            break;
        }
    }

    /* 檢查順子：找最小點數，看 5 張是否連續 */
    int min_rank = 99;
    for (int i = 0; i < HAND_SIZE; i++) {
        if (h->cards[i].rank < min_rank) {
            min_rank = h->cards[i].rank;
        }
    }
    int is_straight = 1;
    if (max_rank - min_rank != 4) {
        is_straight = 0;
    } else {
        /* 連續 5 個點數每個都剛好出現 1 次 */
        for (int r = min_rank; r <= max_rank; r++) {
            if (count[r] != 1) {
                is_straight = 0;
                break;
            }
        }
    }

    /* 算對子、三條、四條的數量 */
    int pairs = 0, threes = 0, fours = 0;
    for (int r = 2; r <= 14; r++) {
        if (count[r] == 2) pairs++;
        if (count[r] == 3) threes++;
        if (count[r] == 4) fours++;
    }

    /* 由大到小判斷牌型 */
    if (is_straight && is_flush)      h->rank = STRAIGHT_FLUSH;
    else if (fours == 1)               h->rank = FOUR_KIND;
    else if (threes == 1 && pairs == 1) h->rank = FULL_HOUSE;
    else if (is_flush)                 h->rank = FLUSH;
    else if (is_straight)              h->rank = STRAIGHT;
    else if (threes == 1)              h->rank = THREE_KIND;
    else if (pairs == 2)               h->rank = TWO_PAIR;
    else if (pairs == 1)               h->rank = ONE_PAIR;
    else                               h->rank = HIGH_CARD;
}

/* 比較兩手牌：>0 表示 a 贏，<0 表示 b 贏，0 表示平手 */
int compare_hands(const Hand* a, const Hand* b) {
    if (a->rank != b->rank) {
        return (int)a->rank - (int)b->rank;   /* 牌型不同直接比 */
    }
    return a->high - b->high;                  /* 牌型相同比最大牌 */
}

const char* hand_rank_name(HandRank r) {
    switch (r) {
        case HIGH_CARD:      return "High Card";
        case ONE_PAIR:       return "One Pair";
        case TWO_PAIR:       return "Two Pair";
        case THREE_KIND:     return "Three of a Kind";
        case STRAIGHT:       return "Straight";
        case FLUSH:          return "Flush";
        case FULL_HOUSE:     return "Full House";
        case FOUR_KIND:      return "Four of a Kind";
        case STRAIGHT_FLUSH: return "Straight Flush";
        default:             return "Unknown";
    }
}
