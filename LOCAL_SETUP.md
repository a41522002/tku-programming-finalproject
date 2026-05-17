# 本地端啟動指南

這份文件帶你從零開始，把「撲克牌比大小」這個專案跑起來。
**不用 Docker、不用虛擬環境，照著做就好。**

---

## 這個專案在做什麼？

- **後端**：用 **C 語言** 寫的小程式，負責發牌、洗牌、判斷牌型，然後印出一段 JSON。
- **中間層**：用 **Python (Flask)** 接住 C 程式的輸出，再丟給網頁。
- **前端**：網頁 HTML/CSS/JS，做動畫和顯示結果。

所以本地要跑起來，需要兩個東西：

1. **GCC**：用來把 C 程式碼編譯成執行檔。
2. **Python**：用來啟動網頁伺服器。

---

## 步驟 1：安裝 GCC（C 編譯器）

### macOS

打開「終端機 (Terminal)」，輸入：

```bash
xcode-select --install
```

會跳出視窗，按「安裝」，等它跑完（大概幾分鐘）。

裝完後輸入這行確認：

```bash
gcc --version
```

有出現版本號就成功。

### Windows

到 [https://www.msys2.org/](https://www.msys2.org/) 下載安裝程式，一路下一步裝完。

裝完後從開始選單打開「**MSYS2 MINGW64**」這個黑色終端機，輸入：

```bash
pacman -S --needed base-devel mingw-w64-x86_64-gcc
```

問你 Y/N 就按 Enter。

裝完後關掉視窗，**之後所有指令都在這個 MSYS2 MINGW64 終端機裡執行**（不要用 PowerShell 或 CMD）。

確認：

```bash
gcc --version
```

---

## 步驟 2：安裝 Python

### macOS

到 [https://www.python.org/downloads/](https://www.python.org/downloads/) 下載 Python 3.12，雙擊 `.pkg` 安裝。

裝完在終端機輸入：

```bash
python3 --version
pip3 --version
```

### Windows

到 [https://www.python.org/downloads/](https://www.python.org/downloads/) 下載 Python 3.12 安裝程式。

> ⚠️ 安裝畫面第一頁**一定要勾**「**Add python.exe to PATH**」，不然後面會找不到指令。

裝完打開 MSYS2 MINGW64 終端機，輸入：

```bash
python --version
pip --version
```

---

## 步驟 3：進到專案資料夾

打開終端機，`cd` 到專案目錄。例如：

```bash
cd "/Volumes/Crucial X9/tku-programming-finalproject"
```

（你的路徑換成自己放的位置）

---

## 步驟 4：安裝 Python 套件

直接裝到本地：

**macOS：**

```bash
pip3 install -r requirements.txt
```

**Windows：**

```bash
pip install -r requirements.txt
```

> 如果跳出 `externally-managed-environment` 的錯誤，加 `--break-system-packages`：
>
> ```bash
> pip3 install -r requirements.txt --break-system-packages
> ```

---

## 步驟 5：編譯 C 後端

```bash
cd backend
make
cd ..
```

成功的話 `backend/` 裡會多出一個叫 `poker`（Windows 是 `poker.exe`）的執行檔。

---

## 步驟 6：啟動網頁

**macOS：**

```bash
python3 app.py
```

**Windows：**

```bash
python app.py
```

看到類似這樣的訊息就是成功：

```
* Running on http://0.0.0.0:5000
```

打開瀏覽器，輸入網址：

👉 **http://localhost:5000**

按「開始發牌」就能玩了！

---

## 要關掉伺服器

在終端機按 `Ctrl + C`。

---

## 常見錯誤

| 看到的錯誤                                     | 怎麼辦                                                                                      |
| ---------------------------------------------- | ------------------------------------------------------------------------------------------- |
| `poker binary not found`                       | 沒編譯 C，回去做步驟 5                                                                      |
| `gcc: command not found`                       | 沒裝 GCC，回去做步驟 1                                                                      |
| `ModuleNotFoundError: No module named 'flask'` | 沒裝 Python 套件，回去做步驟 4                                                              |
| `Address already in use` (port 5000 被佔用)    | macOS 去「系統設定 → 一般 → 隔空播送接收」關掉，或改用其他 port：`PORT=8000 python3 app.py` |
| 改了 C 程式但沒效果                            | 要重新編譯：`cd backend && make clean && make && cd ..`                                     |

---

## 下次要再跑只要做兩件事

1. `cd` 到專案資料夾
2. `python3 app.py`（macOS）或 `python app.py`（Windows）

C 已經編譯過、Python 套件已經裝過，都不用再做。
