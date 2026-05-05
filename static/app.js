const SUIT_SYMBOLS = ['♠', '♥', '♦', '♣'];
const RED_SUITS = new Set([1, 2]);

const dealBtn = document.getElementById('dealBtn');
const shuffleStage = document.getElementById('shuffleStage');
const deckPile = document.getElementById('deckPile');
const resultBox = document.getElementById('resultBox');
const winnerBanner = document.getElementById('winnerBanner');
const errBox = document.getElementById('errBox');

const sleep = (ms) => new Promise((r) => setTimeout(r, ms));

/* 建立牌堆視覺：疊 6 張背面牌 */
function buildDeckPile() {
  deckPile.innerHTML = '';
  for (let i = 0; i < 6; i++) {
    const c = document.createElement('div');
    c.className = 'deck-card';
    c.style.transform = `translate(${i * -1}px, ${i * -1}px)`;
    c.style.zIndex = i;
    deckPile.appendChild(c);
  }
}

/* 開始洗牌：每張牌隨機抖動 */
function startShuffle() {
  const cards = deckPile.querySelectorAll('.deck-card');
  cards.forEach((c, idx) => {
    const dx = (Math.random() * 60 - 30).toFixed(0) + 'px';
    const dr = (Math.random() * 30 - 15).toFixed(0) + 'deg';
    c.style.setProperty('--dx', dx);
    c.style.setProperty('--dr', dr);
    c.style.animationDelay = idx * 0.05 + 's';
    c.classList.add('shuffling');
  });
}

/* 停止洗牌並把牌飛向兩側 */
async function dealOut() {
  const cards = deckPile.querySelectorAll('.deck-card');
  cards.forEach((c) => c.classList.remove('shuffling'));
  await sleep(50);
  cards.forEach((c, idx) => {
    c.style.animationDelay = idx * 0.08 + 's';
    c.classList.add(idx % 2 === 0 ? 'deal-left' : 'deal-right');
  });
  await sleep(800);
}

/* 渲染單張牌 */
function renderCard(card, delay) {
  const isRed = RED_SUITS.has(card.suit);
  const cls = isRed ? 'card red' : 'card';
  const sym = SUIT_SYMBOLS[card.suit] || '?';
  return `<div class="${cls}" style="animation-delay:${delay}s">
        <div class="top">${card.rank_name}</div>
        <div class="suit">${sym}</div>
        <div class="bottom">${card.rank_name}</div>
    </div>`;
}

function renderHand(elId, player, baseDelay) {
  document.getElementById(elId).innerHTML = player.cards.map((c, i) => renderCard(c, baseDelay + i * 0.1)).join('');
}

function renderResult(data) {
  errBox.hidden = true;
  resultBox.hidden = false;
  renderHand('hand1', data.player1, 0);
  renderHand('hand2', data.player2, 0.5);
  document.getElementById('rank1').textContent = data.player1.rank_name;
  document.getElementById('rank2').textContent = data.player2.rank_name;

  const p1Box = document.getElementById('p1Box');
  const p2Box = document.getElementById('p2Box');
  p1Box.classList.remove('winner', 'loser');
  p2Box.classList.remove('winner', 'loser');

  const w = data.winner;
  if (w === 0) {
    winnerBanner.textContent = '🤝 平手！';
  } else {
    winnerBanner.textContent = `🏆 Player ${w} 獲勝！`;
    if (w === 1) {
      p1Box.classList.add('winner');
      p2Box.classList.add('loser');
    } else {
      p2Box.classList.add('winner');
      p1Box.classList.add('loser');
    }
  }
}

dealBtn.addEventListener('click', async () => {
  dealBtn.disabled = true;
  dealBtn.querySelector('.btn-text').textContent = '洗牌中...';
  resultBox.hidden = true;
  errBox.hidden = true;

  /* 1. 顯示牌堆並開始洗牌動畫 */
  buildDeckPile();
  startShuffle();

  /* 2. 同時 fetch 後端結果 */
  const fetchPromise = fetch('/api/deal').then((r) => r.json());

  /* 3. 至少洗牌 1.2 秒，讓動畫看得到 */
  await sleep(1200);

  /* 4. 發牌動畫 */
  dealBtn.querySelector('.btn-text').textContent = '發牌中...';
  await dealOut();

  /* 5. 等後端結果並顯示 */
  try {
    const data = await fetchPromise;
    if (data.error) throw new Error(data.error);
    renderResult(data);
  } catch (e) {
    errBox.hidden = false;
    errBox.textContent = `錯誤：${e.message}`;
  } finally {
    deckPile.innerHTML = '';
    dealBtn.disabled = false;
    dealBtn.querySelector('.btn-text').textContent = '再發一局';
  }
});
