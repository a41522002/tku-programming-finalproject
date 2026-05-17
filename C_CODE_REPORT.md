# C 語言後端 — 程式邏輯與報告講稿

---

## 0. 報告前必懂的 4 個 C 概念

你只要先記住這四個東西，後面看程式碼就會順。

### (1) `struct` — 把多個變數綁成一包

像 Python 的 dict 或物件。例如：

```c
typedef struct {
    int rank;   // 點數
    int suit;   // 花色
} Card;
```

這定義了一個叫 `Card` 的「型態」，裡面裝兩個 `int`。之後可以這樣用：

```c
Card c;
c.rank = 13;   // K
c.suit = 1;    // 紅心
```

### (2) 指標 (`*`) — 存「記憶體位址」的變數

`Card* deck` 的意思：`deck` 不是一張牌本身，而是「指向某張牌所在位置」的箭頭。
配上 `malloc` 之後，`deck` 可以當成一個陣列來用：`deck[0]`、`deck[1]`…

### (3) `malloc` / `free` — 手動跟系統借/還記憶體

C 沒有自動的垃圾回收，借了就要還。

- `malloc(N)`：跟系統借 N bytes 的記憶體，回傳一個指標。
- `free(指標)`：把借的記憶體還回去。
- **沒 free 就會記憶體洩漏 (memory leak)**。這份作業很在意這件事。

### (4) `.h` 標頭檔 vs `.c` 實作檔

- `.h` (header)：寫**宣告**（我有哪些函式、哪些結構），給別人 include 用。
- `.c`：寫**實作**（函式內部到底做什麼）。
- `#include "card.h"` 就是把 `card.h` 的內容貼進來。

> **口頭講稿**：
>
> > 「在進入程式碼之前，先說明四個 C 的核心觀念：struct 是把資料打包、指標是記住記憶體位置的箭頭、malloc / free 是手動管理記憶體、header 檔負責對外宣告。這四個觀念貫穿整個專案。」

---

## 1. 整體架構：4 個檔案的分工

```
backend/
├── card.h / card.c     ← 一張牌、整副牌堆
├── hand.h / hand.c     ← 一手 5 張牌、牌型判斷
├── main.c              ← 主流程：發牌 → 評估 → 比大小 → 輸出 JSON
└── Makefile            ← 編譯指令
```

> **口頭講稿**：
>
> > 「我把後端拆成三個模組：`card` 負責牌與牌堆、`hand` 負責一手牌與牌型判斷、`main` 負責整個遊戲流程。每個模組都有 `.h` 對外宣告、`.c` 寫實作，這是 C 語言典型的模組化方式。」

---

## 2. `card.h` / `card.c` — 牌與牌堆

### 2.1 `Card` 結構（card.h）

```c
typedef struct {
    int rank;   // 2~14 (J=11, Q=12, K=13, A=14)
    int suit;   // 0=黑桃, 1=紅心, 2=方塊, 3=梅花
} Card;
```

**重點**：用兩個整數表示一張牌，後面比大小、判牌型都用數字比就好，不用處理字串。
A 用 14 是因為 A 通常最大，比大小直接比數字。

### 2.2 `create_deck()` — 用 malloc 建立 52 張牌

```c
Card* create_deck(void) {
    Card* deck = (Card*)malloc(sizeof(Card) * DECK_SIZE);
    if (deck == NULL) return NULL;

    int i = 0;
    for (int s = 0; s < 4; s++) {        // 4 種花色
        for (int r = 2; r <= 14; r++) {  // 13 種點數
            deck[i].suit = s;
            deck[i].rank = r;
            i++;
        }
    }
    return deck;
}
```

逐行解釋：

1. `malloc(sizeof(Card) * 52)`：跟系統借「52 張 Card 大小」的記憶體。
2. `if (deck == NULL)`：如果借不到（記憶體不夠），回傳 NULL。
3. 雙層迴圈：外層 4 種花色、內層 13 種點數，**剛好填滿 52 張**。

### 2.3 `shuffle_deck()` — Fisher-Yates 洗牌

```c
void shuffle_deck(Card* deck) {
    for (int i = DECK_SIZE - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        Card tmp = deck[i];
        deck[i] = deck[j];
        deck[j] = tmp;
    }
}
```

**白話流程**：

- 從最後一張 (`i=51`) 開始往前。
- 每次從 `0~i` 隨機挑一個位置 `j`，把第 `i` 張和第 `j` 張交換。
- 這樣每張牌出現在每個位置的機率都相等 → 公平洗牌。

### 2.4 `free_deck()` — 還記憶體

```c
void free_deck(Card* deck) { free(deck); }
```

借了 52 張的空間，用完就還。

> **口頭講稿**：
>
> > 「`card.c` 做三件事：第一，用 `malloc` 動態配置 52 張牌的記憶體；第二，用 Fisher-Yates 演算法洗牌，這個演算法的好處是每張牌出現在每個位置的機率都一樣；第三，用 `free` 釋放記憶體。這展示了 C 的動態記憶體管理。」

---

## 3. `hand.h` / `hand.c` — 一手牌與牌型判斷

### 3.1 `HandRank` 列舉（hand.h）

```c
typedef enum {
    HIGH_CARD = 1, ONE_PAIR = 2, TWO_PAIR = 3,
    THREE_KIND = 4, STRAIGHT = 5, FLUSH = 6,
    FULL_HOUSE = 7, FOUR_KIND = 8, STRAIGHT_FLUSH = 9
} HandRank;
```

**重點**：用 1~9 代表 9 種牌型，**數字越大牌型越強**。比大小就變成「比兩個整數」這麼簡單。

### 3.2 `Hand` 結構

```c
typedef struct {
    Card*    cards;   // malloc 配置 5 張
    HandRank rank;    // 牌型 1~9
    int      high;    // 最大點數（牌型相同時用來比）
} Hand;
```

注意 `cards` 是**指標**，代表這 5 張牌也是 malloc 借來的 → 後面 free 要記得連 `cards` 一起放。

### 3.3 `create_hand()` — 配兩層記憶體

```c
Hand* create_hand(const Card* src, int n) {
    Hand* h = (Hand*)malloc(sizeof(Hand));         // 第一次 malloc
    h->cards = (Card*)malloc(sizeof(Card) * n);    // 第二次 malloc
    for (int i = 0; i < n; i++) h->cards[i] = src[i];
    h->rank = HIGH_CARD;
    h->high = 0;
    return h;
}
```

**這裡很重要**：malloc 了 **兩次**

1. 一次給 `Hand` 結構本身。
2. 一次給裡面的 5 張 `cards`。
3. `h->cards` 是「箭頭運算子」，因為 `h` 是指標，要這樣存取裡面的欄位。

### 3.4 `free_hand()` — 順序要反過來

```c
void free_hand(Hand* h) {
    free(h->cards);   // 先還裡面的
    free(h);          // 再還外面的
}
```

**為什麼順序很重要**：如果先 `free(h)`，那 `h->cards` 這個指標就失效了，再去 `free(h->cards)` 會 crash。**先內後外**。

### 3.5 `evaluate_hand()` — 牌型判斷的核心邏輯

這是整個專案最關鍵的函式。它分成 **四個步驟**：

#### 步驟 A：點數計次 + 找最大牌

```c
int count[15] = {0};
int max_rank = 0;
for (int i = 0; i < HAND_SIZE; i++) {
    count[h->cards[i].rank]++;
    if (h->cards[i].rank > max_rank) max_rank = h->cards[i].rank;
}
```

- `count[r]` = 點數 `r` 出現幾次。
  例：手牌是 K K 5 5 2，那 `count[13]=2`、`count[5]=2`、`count[2]=1`。
- 同時記下最大點數 `max_rank`。

#### 步驟 B：判斷同花

```c
int is_flush = 1;
for (int i = 1; i < HAND_SIZE; i++) {
    if (h->cards[i].suit != h->cards[0].suit) { is_flush = 0; break; }
}
```

**邏輯**：先假設是同花 (1)，只要有一張花色不一樣就改成 0 跳出。

#### 步驟 C：判斷順子

```c
int min_rank = 99;
for (int i = 0; i < HAND_SIZE; i++)
    if (h->cards[i].rank < min_rank) min_rank = h->cards[i].rank;

int is_straight = 1;
if (max_rank - min_rank != 4) {
    is_straight = 0;
} else {
    for (int r = min_rank; r <= max_rank; r++)
        if (count[r] != 1) { is_straight = 0; break; }
}
```

**邏輯（兩個條件都要成立）**：

1. 最大牌 − 最小牌 = 4（連續 5 個數的差是 4，例如 5~9）。
2. 中間每個點數都剛好出現 1 次。

#### 步驟 D：算對子、三條、四條的數量

```c
int pairs = 0, threes = 0, fours = 0;
for (int r = 2; r <= 14; r++) {
    if (count[r] == 2) pairs++;
    if (count[r] == 3) threes++;
    if (count[r] == 4) fours++;
}
```

這裡用上面 `count` 的結果直接統計。

#### 最後：組合條件決定牌型（**順序很重要，由大到小判**）

```c
if (is_straight && is_flush)        h->rank = STRAIGHT_FLUSH;  // 同花順
else if (fours == 1)                h->rank = FOUR_KIND;       // 四條
else if (threes == 1 && pairs == 1) h->rank = FULL_HOUSE;      // 葫蘆 (3+2)
else if (is_flush)                  h->rank = FLUSH;           // 同花
else if (is_straight)               h->rank = STRAIGHT;        // 順子
else if (threes == 1)               h->rank = THREE_KIND;      // 三條
else if (pairs == 2)                h->rank = TWO_PAIR;        // 兩對
else if (pairs == 1)                h->rank = ONE_PAIR;        // 一對
else                                h->rank = HIGH_CARD;       // 散牌
```

**為什麼要由大到小判**：例如「葫蘆」其實同時滿足「三條」和「一對」，如果先判三條就會誤判。**先判嚴格的，再判寬鬆的**。

### 3.6 `compare_hands()` — 比大小

```c
int compare_hands(const Hand* a, const Hand* b) {
    if (a->rank != b->rank) return (int)a->rank - (int)b->rank;
    return a->high - b->high;
}
```

- 牌型不同：直接比 `rank` 編號。
- 牌型相同：比最大牌。
- 回傳值：正 = a 贏、負 = b 贏、0 = 平手。

> **口頭講稿**：
>
> > 「`evaluate_hand` 是整個程式的核心。我用一個長度 15 的陣列 `count` 統計每個點數出現的次數，這個技巧叫做『計數陣列』。透過 `count` 加上同花、順子的判斷，就能組合出 9 種牌型。判斷順序由大到小，先檢查最嚴格的同花順，再依序往下，避免誤判。比大小則是先比牌型編號，相同再比最大牌。」

---

## 4. `main.c` — 整個流程串起來

```c
int main(void) {
    srand((unsigned int)time(NULL));   // ① 隨機種子

    Card *deck = create_deck();         // ② 建牌堆
    shuffle_deck(deck);                 // ③ 洗牌

    Hand *p1 = create_hand(&deck[0], 5);// ④ 玩家 1 拿前 5 張
    Hand *p2 = create_hand(&deck[5], 5);//    玩家 2 拿接下來 5 張

    evaluate_hand(p1);                  // ⑤ 評估牌型
    evaluate_hand(p2);

    int cmp = compare_hands(p1, p2);    // ⑥ 比大小
    int winner = (cmp > 0) ? 1 : (cmp < 0) ? 2 : 0;

    printf("{");                        // ⑦ 輸出 JSON 給 Flask
    print_hand_json("player1", p1);
    printf(",");
    print_hand_json("player2", p2);
    printf(",\"winner\":%d}\n", winner);

    free_hand(p1);                      // ⑧ 釋放記憶體
    free_hand(p2);
    free_deck(deck);
    return 0;
}
```

8 步驟：
| 步驟 | 動作 | 為什麼 |
|---|---|---|
| ① | `srand(time(NULL))` | 用「目前時間」當亂數種子，每次跑結果不同 |
| ② | `create_deck()` | malloc 52 張 |
| ③ | `shuffle_deck()` | 洗牌 |
| ④ | `&deck[0]`、`&deck[5]` | 從牌堆切前 5 張、再切 5 張給兩位玩家 |
| ⑤ | `evaluate_hand()` | 判牌型 |
| ⑥ | `compare_hands()` | 比大小 |
| ⑦ | `printf` 輸出 JSON | Flask 用 `subprocess` 接 stdout |
| ⑧ | `free_*` | 還記憶體（重點：**有借有還**） |

`&deck[0]` 的意思：「deck 第 0 張的位址」，等於把指標指到第 0 張，`create_hand` 從那邊開始讀 5 張。`&deck[5]` 同理，從第 5 張開始。

> **口頭講稿**：
>
> > 「`main.c` 把所有模組串起來，分成八個步驟：設亂數種子、建牌堆、洗牌、發牌給兩位玩家、評估牌型、比大小、輸出 JSON、釋放記憶體。輸出用 `printf` 直接印 JSON 字串，因為 Python 那邊用 `subprocess` 抓 stdout，這是一種最簡單的跨語言整合方式。」

---

## 5. 記憶體配置與釋放的完整對照表

這是報告時的**亮點題**，老師喜歡看這個。

| 何時 malloc            | 配置內容             | 何時 free                             |
| ---------------------- | -------------------- | ------------------------------------- |
| `create_deck()`        | 52 張 Card 陣列      | `free_deck()`                         |
| `create_hand()` 第一次 | Hand 結構本體        | `free_hand()` 第二行 `free(h)`        |
| `create_hand()` 第二次 | Hand 裡的 5 張 cards | `free_hand()` 第一行 `free(h->cards)` |

`main` 跑一次總共：**3 次 malloc × 2 個 Hand + 1 次 deck = 5 次 malloc，5 次 free**。

可以用 macOS 的 `leaks` 工具驗證：

```bash
leaks --atExit -- ./backend/poker | grep "leaks for"
```

預期 0 leaks。

> **口頭講稿**：
>
> > 「整支程式跑一次共有 5 次 malloc，對應 5 次 free。我特別注意 `Hand` 結構是兩層 malloc — 結構本身一次、裡面的 cards 陣列一次，所以 `free_hand` 必須先 free 內層 cards、再 free 外層結構，順序顛倒會 crash。我用 macOS 的 leaks 工具驗證過，記憶體完全沒有洩漏。」

---

## 6. 完整呼叫流程圖（投影片可直接畫）

```
main()
 │
 ├─ srand(time(NULL))
 │
 ├─ create_deck() ──→ malloc 52 張
 │
 ├─ shuffle_deck(deck) ──→ Fisher-Yates 洗牌
 │
 ├─ create_hand(&deck[0], 5) ──→ malloc Hand + malloc 5 張
 ├─ create_hand(&deck[5], 5) ──→ malloc Hand + malloc 5 張
 │
 ├─ evaluate_hand(p1) ─┐
 │                     ├─ 統計 count[]、判同花、判順子、算對子 → 設定 rank
 ├─ evaluate_hand(p2) ─┘
 │
 ├─ compare_hands(p1, p2) ──→ 先比 rank、再比 high
 │
 ├─ printf(...)  ──→ 輸出 JSON 到 stdout
 │
 ├─ free_hand(p1) ──→ free(cards); free(h);
 ├─ free_hand(p2)
 └─ free_deck(deck) ──→ free(deck);
```

---

## 7. 報告開頭與結尾建議（可直接照唸）

### 開頭

> 「大家好，這次我們做的是一個撲克牌比大小的遊戲。後端核心邏輯用 **C 語言**實作，重點展示了 `struct` 結構體、指標、`malloc` 動態記憶體配置與 `free` 釋放這四個 C 語言的核心特性。前端我們用 Python Flask 包一層 Web 介面，但今天我主要報告 C 後端的部分。」

### 中段（依序帶過 2 → 3 → 4 章）

按上面每節的「口頭講稿」唸即可。

### 結尾

> 「總結來說，我們用三個模組：`card` 處理牌與牌堆、`hand` 處理一手牌與牌型判斷、`main` 串起整個流程。整支程式跑一次有 5 次 malloc 對應 5 次 free，沒有記憶體洩漏。牌型判斷使用計數陣列加上由大到小的條件判斷，邏輯清楚也容易擴充。以上是我的報告，謝謝大家。」

---

## 8. 老師可能會問的問題（先準備好）

| 問題                                   | 怎麼答                                                        |
| -------------------------------------- | ------------------------------------------------------------- |
| 為什麼 A 設成 14 不是 1？              | 比大小最直觀，數字越大牌越強                                  |
| 為什麼牌型用 enum 而不是字串？         | 比大小只要比整數，效能好也好寫                                |
| Fisher-Yates 為什麼比較好？            | 每張牌出現在每個位置機率相等，分佈均勻                        |
| 為什麼 `free_hand` 要先 free `cards`？ | 先 free `h` 的話 `h->cards` 就失效了，再 free 會 crash        |
| 為什麼牌型由大到小判？                 | 葫蘆同時滿足三條和一對，順序錯會誤判                          |
| C 怎麼跟 Python 溝通？                 | C 用 `printf` 印 JSON 到 stdout，Python 用 `subprocess` 接    |
| Ace-low 順子 (A-2-3-4-5) 有處理嗎？    | **沒有**（誠實答），目前 A=14 只能組 10-J-Q-K-A，未來可加判斷 |
