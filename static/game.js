let gameId = null;
let currentState = null;

const startBtn = document.getElementById("startBtn");
const restartBtn = document.getElementById("restartBtn");
const gamePanel = document.getElementById("gamePanel");
const boardEl = document.getElementById("board");
const statusText = document.getElementById("statusText");
const mineText = document.getElementById("mineText");

function statusLabel(status) {
  if (status === "win") return "你贏了!";
  if (status === "lose") return "踩到地雷!";
  return "進行中";
}

async function postJson(url, body) {
  const res = await fetch(url, {
    method: "POST",
    headers: { "Content-Type": "application/json" },
    body: JSON.stringify(body),
  });
  const data = await res.json();
  if (!res.ok) throw new Error(data.error || "Request failed");
  return data;
}

function renderBoard(state) {
  currentState = state;
  boardEl.innerHTML = "";
  boardEl.style.gridTemplateColumns = `repeat(${state.cols}, 28px)`;
  statusText.textContent = `狀態: ${statusLabel(state.status)}`;
  mineText.textContent = `地雷數: ${state.mines}`;

  for (let r = 0; r < state.rows; r++) {
    for (let c = 0; c < state.cols; c++) {
      const cell = document.createElement("div");
      cell.className = "cell";

      const isVisible = state.visible[r][c] === 1;
      const isFlag = state.flags[r][c] === 1;
      const value = state.board[r][c];

      if (isVisible) {
        cell.classList.add("revealed");
        if (value === -1) {
          cell.classList.add("mine");
          cell.textContent = "X";
        } else if (value > 0) {
          cell.textContent = String(value);
        }
      } else if (isFlag) {
        cell.classList.add("flagged");
        cell.textContent = "F";
      }

      if (state.status === "playing") {
        cell.addEventListener("click", async () => {
          try {
            const nextState = await postJson("/reveal", { game_id: gameId, row: r, col: c });
            renderBoard(nextState);
          } catch (err) {
            alert(err.message);
          }
        });

        cell.addEventListener("contextmenu", async (e) => {
          e.preventDefault();
          try {
            const nextState = await postJson("/flag", { game_id: gameId, row: r, col: c });
            renderBoard(nextState);
          } catch (err) {
            alert(err.message);
          }
        });
      }

      boardEl.appendChild(cell);
    }
  }
}

async function newGame() {
  const difficulty = document.getElementById("difficulty").value;
  try {
    const data = await postJson("/new_game", { difficulty });
    gameId = data.game_id;
    gamePanel.classList.remove("hidden");
    renderBoard(data.state);
  } catch (err) {
    alert(err.message);
  }
}

startBtn.addEventListener("click", newGame);
restartBtn.addEventListener("click", newGame);
