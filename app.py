import json
import os
import subprocess
import time
import uuid
from pathlib import Path

from flask import Flask, jsonify, render_template, request


BASE_DIR = Path(__file__).resolve().parent
SAVE_DIR = BASE_DIR / "data" / "saves"
ENGINE_PATH = BASE_DIR / "c_engine" / "minesweeper.exe"

SAVE_DIR.mkdir(parents=True, exist_ok=True)

app = Flask(__name__)


DIFFICULTIES = {
    "easy": (9, 9, 10),
    "medium": (16, 16, 40),
    "hard": (16, 30, 99),
}


def state_path(game_id: str) -> Path:
    return SAVE_DIR / f"{game_id}.bin"


def run_engine(args: list[str]) -> subprocess.CompletedProcess:
    if not ENGINE_PATH.exists():
        raise RuntimeError(
            "C engine not found. Compile c_engine/minesweeper.c to c_engine/minesweeper.exe first."
        )
    return subprocess.run(
        [str(ENGINE_PATH), *args],
        capture_output=True,
        text=True,
        check=False,
    )


def load_state(game_id: str) -> dict:
    target = state_path(game_id)
    if not target.exists():
        raise FileNotFoundError("Game does not exist.")
    result = run_engine(["dump", str(target)])
    if result.returncode != 0:
        raise RuntimeError(result.stderr.strip() or "Failed to load game state.")
    return json.loads(result.stdout)


@app.get("/")
def index():
    return render_template("index.html", difficulties=DIFFICULTIES)


@app.post("/new_game")
def new_game():
    payload = request.get_json(silent=True) or {}
    difficulty = payload.get("difficulty", "easy")
    if difficulty not in DIFFICULTIES:
        return jsonify({"error": "Invalid difficulty."}), 400

    rows, cols, mines = DIFFICULTIES[difficulty]
    game_id = uuid.uuid4().hex
    out_file = state_path(game_id)
    seed = str(int(time.time() * 1000) % (2**31 - 1))

    result = run_engine(["init", str(rows), str(cols), str(mines), seed, str(out_file)])
    if result.returncode != 0:
        return jsonify({"error": result.stderr.strip() or "Engine init failed."}), 500

    state = load_state(game_id)
    return jsonify({"game_id": game_id, "state": state})


@app.get("/state/<game_id>")
def get_state(game_id: str):
    try:
        return jsonify(load_state(game_id))
    except FileNotFoundError as exc:
        return jsonify({"error": str(exc)}), 404
    except RuntimeError as exc:
        return jsonify({"error": str(exc)}), 500


@app.post("/reveal")
def reveal():
    payload = request.get_json(silent=True) or {}
    game_id = payload.get("game_id")
    row = payload.get("row")
    col = payload.get("col")
    if not isinstance(game_id, str):
        return jsonify({"error": "game_id is required."}), 400
    if not isinstance(row, int) or not isinstance(col, int):
        return jsonify({"error": "row/col must be integers."}), 400

    target = state_path(game_id)
    if not target.exists():
        return jsonify({"error": "Game does not exist."}), 404

    result = run_engine(["reveal", str(target), str(row), str(col)])
    if result.returncode != 0:
        return jsonify({"error": result.stderr.strip() or "Reveal failed."}), 400
    return jsonify(load_state(game_id))


@app.post("/flag")
def flag():
    payload = request.get_json(silent=True) or {}
    game_id = payload.get("game_id")
    row = payload.get("row")
    col = payload.get("col")
    if not isinstance(game_id, str):
        return jsonify({"error": "game_id is required."}), 400
    if not isinstance(row, int) or not isinstance(col, int):
        return jsonify({"error": "row/col must be integers."}), 400

    target = state_path(game_id)
    if not target.exists():
        return jsonify({"error": "Game does not exist."}), 404

    result = run_engine(["flag", str(target), str(row), str(col)])
    if result.returncode != 0:
        return jsonify({"error": result.stderr.strip() or "Flag failed."}), 400
    return jsonify(load_state(game_id))


if __name__ == "__main__":
    app.run(debug=True)
