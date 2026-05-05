#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "card.h"
#include "hand.h"

/* 把一手牌印成 JSON 格式 */
static void print_hand_json(const char *key, const Hand *h)
{
  printf("\"%s\":{\"cards\":[", key);
  for (int i = 0; i < HAND_SIZE; i++)
  {
    printf("{\"rank\":%d,\"suit\":%d,\"rank_name\":\"%s\",\"suit_name\":\"%s\"}",
           h->cards[i].rank, h->cards[i].suit,
           rank_name(h->cards[i].rank), suit_name(h->cards[i].suit));
    if (i < HAND_SIZE - 1)
      printf(",");
  }
  printf("],\"rank\":%d,\"rank_name\":\"%s\"}",
         (int)h->rank, hand_rank_name(h->rank));
}

int main(void)
{
  /* 設定隨機種子（每次執行牌局不同） */
  srand((unsigned int)time(NULL));

  /* 1. 建立並洗牌 */
  Card *deck = create_deck();
  shuffle_deck(deck);

  /* 2. 各發 5 張：玩家1 拿 0~4，玩家2 拿 5~9 */
  Hand *p1 = create_hand(&deck[0], HAND_SIZE);
  Hand *p2 = create_hand(&deck[5], HAND_SIZE);

  /* 3. 評估牌型 */
  evaluate_hand(p1);
  evaluate_hand(p2);

  /* 4. 比大小 */
  int cmp = compare_hands(p1, p2);
  int winner;
  if (cmp > 0)
    winner = 1;
  else if (cmp < 0)
    winner = 2;
  else
    winner = 0; /* 平手 */

  /* 5. 輸出 JSON 給 Flask */
  printf("{");
  print_hand_json("player1", p1);
  printf(",");
  print_hand_json("player2", p2);
  printf(",\"winner\":%d}\n", winner);

  /* 6. 釋放所有 malloc 的記憶體 */
  free_hand(p1);
  free_hand(p2);
  free_deck(deck);

  return 0;
}
