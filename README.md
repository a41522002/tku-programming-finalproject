# 撲克牌比大小

一個用 **C 語言** 實作後端核心邏輯、**Python Flask** 提供前端介面的撲克牌比大小遊戲。
重點展示 C 語言的 `struct`、指標、`malloc` 動態記憶體配置與 `free` 釋放。

---

## 功能特色

### 遊戲玩法
- 兩位玩家各發 5 張牌
- 自動評估牌型（9 種牌型）並比大小
- 牌型相同時比最大點數決定勝負

### 支援的牌型（由小到大）
| 等級 | 英文 | 中文 |
|---|---|---|
| 1 | High Card | 散牌 |
| 2 | One Pair | 一對 |
| 3 | Two Pair | 兩對 |
| 4 | Three of a Kind | 三條 |
| 5 | Straight | 順子 |
| 6 | Flush | 同花 |
| 7 | Full House | 葫蘆 |
| 8 | Four of a Kind | 四條（鐵支） |
| 9 | Straight Flush | 同花順 |

### C 後端核心
- `struct Card` 封裝點數與花色
- `struct Hand` 封裝一手牌與牌型
- 用 `malloc` 動態配置 52 張牌堆與每位玩家的手牌陣列
- 用 `free` 完整釋放所有動態記憶體（無記憶體洩漏）
- Fisher-Yates 演算法洗牌
- 透過 stdout 輸出 JSON 給前端解析

### Flask 前端
- 單頁互動 UI，按鈕點擊即可發牌
- **洗牌動畫**：6 張紅底背面牌隨機抖動模擬洗牌
- **發牌動畫**：牌飛向左右兩側
- **翻牌動畫**：手牌依序翻開（3D rotateY 效果）
- 勝者區塊金色發光、敗者半透明
- 賭桌綠色漸層背景 + 金色光暈

---

## 專案結構

```
c-homework/
├── backend/                # C 後端
│   ├── card.h              # Card 結構與牌堆函式宣告
│   ├── card.c              # 建立/洗牌/釋放牌堆（malloc/free）
│   ├── hand.h              # Hand 結構與牌型評估宣告
│   ├── hand.c              # 牌型評估、比較邏輯
│   ├── main.c              # 主程式：發牌、評估、輸出 JSON
│   └── Makefile            # 編譯腳本，產生 ./poker 執行檔
│
├── app.py                  # Flask 主程式（subprocess 呼叫 C 執行檔）
├── templates/
│   └── index.html          # 單頁 UI 模板
├── static/
│   ├── style.css           # 樣式 + 動畫 keyframes
│   └── app.js              # 洗牌/發牌/翻牌動畫邏輯與 fetch
│
├── requirements.txt        # Python 依賴（flask）
└── README.md
```

---

## 系統流程

```
使用者點「發牌」
      ↓
[前端] JS 顯示牌堆 → 洗牌動畫
      ↓
[前端] fetch('/api/deal')
      ↓
[Flask] subprocess 執行 ./poker
      ↓
[C] create_deck() → shuffle → create_hand × 2
    → evaluate_hand → compare_hands
    → printf JSON → free 全部記憶體
      ↓
[Flask] 解析 stdout JSON → 回傳前端
      ↓
[前端] 發牌動畫 → 翻牌 → 顯示勝負
```

---

## C 後端 API 設計

### `card.h` / `card.c` — 牌堆
```c
typedef struct {
    int rank;   // 2~14 (J=11, Q=12, K=13, A=14)
    int suit;   // 0=Spade, 1=Heart, 2=Diamond, 3=Club
} Card;

Card* create_deck(void);        // malloc 52 張
void  shuffle_deck(Card* deck); // Fisher-Yates 洗牌
void  free_deck(Card* deck);    // free 釋放
```

### `hand.h` / `hand.c` — 手牌與牌型
```c
typedef struct {
    Card*    cards;   // malloc 配置 5 張
    HandRank rank;    // 牌型編號 1~9
    int      high;    // 最大點數（比大小用）
} Hand;

Hand* create_hand(const Card* src, int n);
void  evaluate_hand(Hand* h);
int   compare_hands(const Hand* a, const Hand* b);
void  free_hand(Hand* h);
```

### 記憶體管理對照
| 配置 | 釋放 |
|---|---|
| `create_deck()` → `malloc(52)` | `free_deck()` |
| `create_hand()` → `malloc(Hand)` + `malloc(cards)` | `free_hand()` 先 free cards 再 free hand |

---

## 執行方式

### 1. 編譯 C 後端
```bash
cd backend
make
./poker          # 測試輸出 JSON
cd ..
```

### 2. 安裝 Python 套件
```bash
pip install -r requirements.txt
```

### 3. 啟動 Flask
```bash
python app.py
```

開啟瀏覽器到 [http://localhost:5000](http://localhost:5000)，按「開始發牌」即可遊玩。

---

## 後端輸出範例

```json
{
  "player1": {
    "cards": [
      {"rank": 13, "suit": 1, "rank_name": "K", "suit_name": "Heart"},
      {"rank": 13, "suit": 3, "rank_name": "K", "suit_name": "Club"},
      ...
    ],
    "rank": 2,
    "rank_name": "One Pair"
  },
  "player2": { ... },
  "winner": 1
}
```

`winner` 值：`1` = 玩家1勝、`2` = 玩家2勝、`0` = 平手。

---

## 記憶體洩漏檢查（選用）

macOS：
```bash
leaks --atExit -- ./backend/poker | grep "leaks for"
```

Linux：
```bash
valgrind --leak-check=full ./backend/poker
```

預期結果：0 leaks。

---

## 技術棧

- **後端**：C (C11)、gcc、Make
- **前端**：Python 3、Flask、HTML/CSS/JS（原生，無框架）
- **整合**：Python `subprocess` 呼叫 C 執行檔，stdin/stdout 傳 JSON
