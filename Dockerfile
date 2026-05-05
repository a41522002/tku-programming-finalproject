# ---- Build & Run in one stage（簡單版） ----
FROM python:3.12-slim

# 安裝編譯 C 所需工具（build-essential 包含 gcc、make、libc-dev headers）
RUN apt-get update && \
    apt-get install -y --no-install-recommends build-essential && \
    rm -rf /var/lib/apt/lists/*

WORKDIR /app

# 先裝 Python 套件（利用 Docker 快取，requirements 沒變就不重裝）
COPY requirements.txt .
RUN pip install --no-cache-dir -r requirements.txt

# 複製整個 project
COPY . .

# 編譯 C 後端，產生 backend/poker
RUN cd backend && make clean && make

# Render / Railway / Fly 會用 PORT 環境變數
ENV PORT=8000
EXPOSE 8000

# 用 gunicorn 啟動 Flask（production WSGI server）
CMD gunicorn --bind 0.0.0.0:${PORT} --workers 2 --timeout 30 app:app
