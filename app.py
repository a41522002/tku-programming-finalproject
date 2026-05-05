import json
import os
import subprocess

from flask import Flask, jsonify, render_template

app = Flask(__name__)

POKER_BIN = os.path.join(os.path.dirname(os.path.abspath(__file__)), "backend", "poker")


@app.route("/")
def index():
    return render_template("index.html")


@app.route("/api/deal")
def deal():
    if not os.path.exists(POKER_BIN):
        return jsonify({"error": f"poker binary not found at {POKER_BIN}. Run 'make' in backend/."}), 500
    try:
        result = subprocess.run(
            [POKER_BIN], capture_output=True, text=True, timeout=5, check=True
        )
    except subprocess.CalledProcessError as e:
        return jsonify({"error": "poker exited non-zero", "stderr": e.stderr}), 500
    except subprocess.TimeoutExpired:
        return jsonify({"error": "poker timeout"}), 500

    try:
        return jsonify(json.loads(result.stdout))
    except json.JSONDecodeError as e:
        return jsonify({"error": "invalid JSON from poker", "raw": result.stdout, "detail": str(e)}), 500


if __name__ == "__main__":
    port = int(os.environ.get("PORT", 5000))
    debug = os.environ.get("FLASK_DEBUG", "0") == "1"
    app.run(host="0.0.0.0", port=port, debug=debug)
