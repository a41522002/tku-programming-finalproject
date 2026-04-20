# Flask + C Minesweeper

這是一個以 **Python Flask 為前端伺服器**、**C 為遊戲邏輯後端** 的踩地雷專案。

## 1. 環境需求

- Python 3.10+
- Flask
- GCC (建議使用 MinGW-w64)

## 2. 安裝 Python 套件

```powershell
pip install -r requirements.txt
```

## 3. 編譯 C 引擎

在專案根目錄執行：

```powershell
gcc .\c_engine\minesweeper.c -O2 -o .\c_engine\minesweeper.exe
```

若系統沒有 `gcc`，請先安裝 MinGW-w64，並把 `gcc` 加入 PATH。

## 4. 啟動 Flask

```powershell
python .\app.py
```

瀏覽器開啟 `http://127.0.0.1:5000/` 即可遊玩。

## 5. 操作方式

- 左鍵：開格
- 右鍵：插旗/取消旗
- 上方可切換難度並開新局

## 6. API 摘要

- `POST /new_game`：建立新遊戲
- `POST /reveal`：開格
- `POST /flag`：插旗
- `GET /state/<game_id>`：查詢目前狀態
