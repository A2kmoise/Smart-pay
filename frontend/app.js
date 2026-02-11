const socket = io('http://localhost:3000');

const statusDisplay = document.getElementById('status-display');
const uidInput = document.getElementById('uid');
const amountInput = document.getElementById('amount');
const topupBtn = document.getElementById('topup-btn');
const logList = document.getElementById('log-list');
const cardVisual = document.getElementById('card-visual');
const cardUidDisplay = document.getElementById('card-uid-display');
const cardBalanceDisplay = document.getElementById('card-balance-display');

let lastScannedUid = null;

socket.on('connect', () => {
  addLog('Connected to backend server');
});

socket.on('card-status', async (data) => {
  addLog(`Card detected: ${data.uid} | Balance: $${data.balance}`);
  lastScannedUid = data.uid;
  uidInput.value = data.uid;
  topupBtn.disabled = false;

  // Update Visual Card
  cardVisual.classList.add('active');
  cardUidDisplay.textContent = data.uid;
  cardBalanceDisplay.textContent = `$${data.balance.toFixed(2)}`;

  statusDisplay.innerHTML = `
        <div class="data-row">
            <span class="data-label">UID:</span>
            <span class="data-value">${data.uid}</span>
        </div>
        <div class="data-row">
            <span class="data-label">Balance:</span>
            <span class="data-value" style="color: #6366f1;">$${data.balance.toFixed(2)}</span>
        </div>
        <div class="data-row">
            <span class="data-label">Status:</span>
            <span class="data-value" style="color: #4ade80;">Active</span>
        </div>
    `;
});

socket.on('card-balance', (data) => {
  addLog(`Balance updated for ${data.uid}: $${data.new_balance}`);

  // Update Visual Card if this card is still active
  if (data.uid === lastScannedUid) {
    cardBalanceDisplay.textContent = `$${data.new_balance.toFixed(2)}`;

    // Brief pulse effect
    cardVisual.style.transform = 'scale(1.1)';
    setTimeout(() => {
      cardVisual.style.transform = '';
    }, 200);
  }

  statusDisplay.innerHTML += `
        <div class="data-row">
            <span class="data-label">New Balance:</span>
            <span class="data-value" style="color: #6366f1;">$${data.new_balance}</span>
        </div>
    `;
});

topupBtn.addEventListener('click', async () => {
  const amount = parseFloat(amountInput.value);
  if (isNaN(amount) || amount <= 0) {
    alert('Please enter a valid amount');
    return;
  }

  try {
    const response = await fetch('http://localhost:3000/topup', {
      method: 'POST',
      headers: {
        'Content-Type': 'application/json'
      },
      body: JSON.stringify({
        uid: lastScannedUid,
        amount: amount
      })
    });

    const result = await response.json();
    if (result.success) {
      addLog(`Top-up request sent for ${lastScannedUid}`);
      amountInput.value = '';
    } else {
      addLog(`Error: ${result.error}`);
    }
  } catch (err) {
    addLog('Failed to connect to backend for top-up');
    console.error(err);
  }
});

function addLog(message) {
  const li = document.createElement('li');
  const now = new Date();
  const time = now.toLocaleTimeString();
  li.textContent = `[${time}] ${message}`;
  logList.prepend(li);

  // Keep only last 20 logs
  if (logList.children.length > 20) {
    logList.removeChild(logList.lastChild);
  }
}
