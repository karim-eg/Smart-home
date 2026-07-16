#pragma once
#include <Arduino.h>

// Smart Home dashboard (HTML + CSS + JS), served at "/".
// Edit this file to change the UI without touching any device logic.
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1">
<title>Smart Home</title>
<link rel="preconnect" href="https://fonts.googleapis.com">
<link href="https://fonts.googleapis.com/css2?family=Syne:wght@400;700;800&family=DM+Sans:wght@300;400;500&display=swap" rel="stylesheet">

<style>
:root {
  --bg:        #0b0d12;
  --sidebar:   #10131a;
  --surface:   #151824;
  --surface-2: #1b1f2e;
  --border:    rgba(255,255,255,0.06);
  --primary:   #6366f1;
  --primary-2: #818cf8;
  --success:   #22c55e;
  --warning:   #eab308;
  --danger:    #ef4444;
  --text:      #e5e7eb;
  --text-dim:  #9ca3af;
  --muted:     #565c6e;
  --radius:    16px;
  --shadow:    0 1px 2px rgba(0,0,0,.4), 0 8px 24px -8px rgba(0,0,0,.5);
}

*, *::before, *::after { box-sizing: border-box; margin: 0; padding: 0; }

body {
  background: var(--bg);
  color: var(--text);
  font-family: 'Inter', 'DM Sans', sans-serif;
  min-height: 100vh;
  display: flex;
  font-size: 14px;
}

/* ── Sidebar ─────────────────────────── */
.sidebar {
  width: 220px;
  flex-shrink: 0;
  background: var(--sidebar);
  border-right: 1px solid var(--border);
  min-height: 100vh;
  padding: 24px 16px;
  display: flex;
  flex-direction: column;
  gap: 28px;
  position: fixed;
  top: 0; left: 0; bottom: 0;
}

.brand {
  display: flex;
  align-items: center;
  gap: 10px;
  padding: 0 6px;
}

.brand-mark {
  width: 32px; height: 32px;
  border-radius: 9px;
  background: linear-gradient(135deg, var(--primary), var(--primary-2));
  display: flex; align-items: center; justify-content: center;
  font-size: 15px;
}

.brand-title {
  font-weight: 700;
  font-size: 15px;
  letter-spacing: -0.2px;
}

.nav-group { display: flex; flex-direction: column; gap: 2px; }

.nav-label {
  font-size: 10.5px;
  text-transform: uppercase;
  letter-spacing: 1.2px;
  color: var(--muted);
  padding: 8px 10px 4px;
}

.nav-item {
  display: flex;
  align-items: center;
  gap: 10px;
  padding: 9px 10px;
  border-radius: 10px;
  color: var(--text-dim);
  font-size: 13.5px;
  font-weight: 500;
}

.nav-item.active {
  background: rgba(99,102,241,0.14);
  color: var(--primary-2);
}

.sidebar-footer {
  margin-top: auto;
  padding: 12px 10px;
  border-top: 1px solid var(--border);
  font-size: 11px;
  color: var(--muted);
}

/* ── Main ────────────────────────────── */
.main {
  margin-left: 220px;
  flex: 1;
  min-width: 0;
  padding: 22px 32px 60px;
}

.topbar {
  display: flex;
  align-items: center;
  justify-content: space-between;
  margin-bottom: 24px;
}

.page-title { font-size: 20px; font-weight: 700; letter-spacing: -0.3px; }
.page-sub { font-size: 12.5px; color: var(--text-dim); margin-top: 2px; }

.conn-pill {
  display: flex;
  align-items: center;
  gap: 8px;
  padding: 7px 14px;
  border-radius: 999px;
  background: var(--surface);
  border: 1px solid var(--border);
  font-size: 12px;
  font-weight: 500;
  color: var(--text-dim);
}

.status-dot {
  width: 7px; height: 7px;
  border-radius: 50%;
  background: var(--success);
}

/* ── Grid / cards ────────────────────── */
.grid {
  display: grid;
  grid-template-columns: repeat(12, 1fr);
  gap: 16px;
}

.span-4 { grid-column: span 12; }
.span-6 { grid-column: span 12; }
.span-12 { grid-column: span 12; }

@media (min-width: 860px) {
  .span-4 { grid-column: span 4; }
  .span-6 { grid-column: span 6; }
}

.card {
  background: var(--surface);
  border: 1px solid var(--border);
  border-radius: var(--radius);
  padding: 20px;
  box-shadow: var(--shadow);
}

.stat-card {
  display: flex;
  flex-direction: column;
  gap: 10px;
}

.stat-top {
  display: flex;
  align-items: center;
  justify-content: space-between;
}

.stat-icon-box {
  width: 34px; height: 34px;
  border-radius: 9px;
  background: var(--surface-2);
  display: flex; align-items: center; justify-content: center;
  font-size: 15px;
}

.trend-pill {
  font-size: 11px;
  font-weight: 600;
  padding: 3px 8px;
  border-radius: 999px;
  background: rgba(34,197,94,0.12);
  color: var(--success);
}
.trend-pill.warn { background: rgba(234,179,8,0.12); color: var(--warning); }
.trend-pill.danger { background: rgba(239,68,68,0.12); color: var(--danger); }
.trend-pill.muted { background: rgba(255,255,255,0.05); color: var(--muted); }

.stat-value { font-size: 26px; font-weight: 700; letter-spacing: -0.5px; }
.stat-label { font-size: 12px; color: var(--text-dim); }

/* ── Control card ────────────────────── */
.control-head {
  display: flex;
  align-items: center;
  justify-content: space-between;
  margin-bottom: 16px;
}

.control-title { display: flex; align-items: center; gap: 12px; }
.control-icon {
  width: 38px; height: 38px;
  border-radius: 10px;
  background: var(--surface-2);
  display: flex; align-items: center; justify-content: center;
  font-size: 17px;
}
.control-name { font-size: 14.5px; font-weight: 600; }
.control-sub { font-size: 11.5px; color: var(--muted); margin-top: 1px; }

.badge {
  font-size: 10.5px;
  font-weight: 700;
  letter-spacing: 0.5px;
  padding: 4px 10px;
  border-radius: 6px;
  background: var(--surface-2);
  color: var(--muted);
}
.badge.on { background: rgba(34,197,94,0.14); color: var(--success); }
.badge.auto { background: rgba(99,102,241,0.16); color: var(--primary-2); }
.badge.danger { background: rgba(239,68,68,0.16); color: var(--danger); }

.row {
  display: flex;
  align-items: center;
  justify-content: space-between;
  padding: 11px 0;
}
.row + .row { border-top: 1px solid var(--border); }

.row-info { display: flex; flex-direction: column; gap: 1px; }
.row-name { font-size: 13px; font-weight: 500; }
.row-desc { font-size: 11px; color: var(--muted); }

.switch { position: relative; width: 42px; height: 24px; flex-shrink: 0; }
.switch input { display: none; }
.slider {
  position: absolute; inset: 0;
  background: var(--surface-2);
  border: 1px solid var(--border);
  border-radius: 999px;
  cursor: pointer;
  transition: all .25s ease;
}
.slider::before {
  content: '';
  position: absolute;
  width: 18px; height: 18px;
  left: 2px; top: 2px;
  background: var(--muted);
  border-radius: 50%;
  transition: all .25s ease;
}
input:checked + .slider { background: rgba(99,102,241,0.35); border-color: var(--primary); }
input:checked + .slider::before { transform: translateX(18px); background: #fff; }

.note-strip {
  margin-top: 14px;
  padding: 10px 12px;
  border-radius: 10px;
  background: var(--surface-2);
  font-size: 12px;
  color: var(--text-dim);
  display: flex;
  align-items: center;
  gap: 8px;
}
.note-strip.danger { background: rgba(239,68,68,0.1); color: #fca5a5; }
</style>
</head>
<body>

<aside class="sidebar">
  <div class="brand">
    <div class="brand-mark">🏠</div>
    <div class="brand-title">Home Panel</div>
  </div>

  <div class="nav-group">
    <div class="nav-label">Overview</div>
    <div class="nav-item active">📊 &nbsp;Dashboard</div>
  </div>

  <div class="nav-group">
    <div class="nav-label">Rooms</div>
    <div class="nav-item">🪜 &nbsp;Staircase</div>
    <div class="nav-item">💡 &nbsp;Living Room</div>
  </div>

  <div class="sidebar-footer">ESP32 · Local Network</div>
</aside>

<main class="main">

  <div class="topbar">
    <div>
      <div class="page-title">Dashboard</div>
      <div class="page-sub">Live status of your home</div>
    </div>
    <div class="conn-pill">
      <div class="status-dot" id="wsDot"></div>
      <span id="wsText">---</span>
    </div>
  </div>

  <div class="grid">

    <div class="card stat-card span-4">
      <div class="stat-top">
        <div class="stat-icon-box">🌡️</div>
        <span class="trend-pill muted" id="tempTrend">Room</span>
      </div>
      <div class="stat-value" id="tempBadge">--°C</div>
      <div class="stat-label">Temperature</div>
    </div>

    <div class="card stat-card span-4">
      <div class="stat-top">
        <div class="stat-icon-box">💧</div>
        <span class="trend-pill muted" id="humTrend">Room</span>
      </div>
      <div class="stat-value" id="humLabel">--%</div>
      <div class="stat-label">Humidity</div>
    </div>

    <div class="card stat-card span-4" id="gasStatCard">
      <div class="stat-top">
        <div class="stat-icon-box" id="gasIcon">⚫</div>
        <span class="trend-pill muted" id="gasTrend">---</span>
      </div>
      <div class="stat-value" id="gasBadge">---</div>
      <div class="stat-label">Gas Sensor</div>
    </div>

    <div class="card span-12" id="gasCard">
      <div class="control-head">
        <div class="control-title">
          <div class="control-icon">🧪</div>
          <div>
            <div class="control-name">Gas Detector</div>
            <div class="control-sub">MQ-5 sensor</div>
          </div>
        </div>
      </div>
      <div class="note-strip" id="gasStrip">
        <span id="gasLabel">Waiting for data...</span>
      </div>
    </div>

    <div class="card span-6" id="stairCard">
      <div class="control-head">
        <div class="control-title">
          <div class="control-icon">🪜</div>
          <div>
            <div class="control-name">Staircase</div>
            <div class="control-sub">Lamp + motion sensor</div>
          </div>
        </div>
        <span class="badge" id="l2Badge">OFF</span>
      </div>

      <div class="row">
        <div class="row-info">
          <span class="row-name">Manual</span>
          <span class="row-desc">Direct relay — disables auto</span>
        </div>
        <label class="switch">
          <input type="checkbox" id="t2_manual" onchange="toggleManual(event)">
          <span class="slider"></span>
        </label>
      </div>

      <div class="row">
        <div class="row-info">
          <span class="row-name">Auto</span>
          <span class="row-desc">PIR sensor — disables manual</span>
        </div>
        <label class="switch">
          <input type="checkbox" id="t2_auto" onchange="toggleAuto(event)">
          <span class="slider"></span>
        </label>
      </div>

      <div class="note-strip" id="statusStrip">
        <span id="statusLabel">Both modes off — lamp is off</span>
      </div>
    </div>

    <div class="card span-6" id="roomCard">
      <div class="control-head">
        <div class="control-title">
          <div class="control-icon">💡</div>
          <div>
            <div class="control-name">Living Room</div>
            <div class="control-sub">Main lamp</div>
          </div>
        </div>
        <span class="badge" id="l1Badge">OFF</span>
      </div>

      <div class="row">
        <div class="row-info">
          <span class="row-name">Power</span>
          <span class="row-desc">Toggle the room lamp</span>
        </div>
        <label class="switch">
          <input type="checkbox" id="t1" onchange="toggle1(event)">
          <span class="slider"></span>
        </label>
      </div>
    </div>

  </div>
</main>

<script>
let ws;

function startWS() {
  ws = new WebSocket("ws://" + location.hostname + ":81/");

  const dot  = document.getElementById("wsDot");
  const text = document.getElementById("wsText");

  ws.onopen = () => {
    text.textContent = "Connected";
    dot.style.background = "var(--accent)";
    dot.style.boxShadow  = "0 0 8px var(--accent)";
  };

  ws.onclose = () => {
    text.textContent = "Disconnected";
    dot.style.background = "#ff5f3e";
    dot.style.boxShadow  = "0 0 8px #ff5f3e";
    setTimeout(startWS, 2000);
  };

  ws.onmessage = (event) => {
    const d = JSON.parse(event.data);

    // Living room
    const roomOn = d.l1 === "ON";
    const l1Badge = document.getElementById("l1Badge");
    l1Badge.textContent = d.l1;
    l1Badge.className = "badge" + (roomOn ? " on" : "");
    document.getElementById("t1").checked = roomOn;

    document.getElementById("tempBadge").textContent = d.temp + "°C";
    document.getElementById("humLabel").textContent = d.hum + "%";

    // Stair lamp state
    const stairOn = d.l2 === "ON";
    const l2Badge = document.getElementById("l2Badge");
    l2Badge.textContent = d.l2;
    l2Badge.className = "badge" + (d.mode === "manual" && stairOn ? " on" : d.mode === "auto" ? " auto" : "");

    // Mode toggles
    document.getElementById("t2_manual").checked = d.mode === "manual";
    document.getElementById("t2_auto").checked   = d.mode === "auto";

    // Status strip
    const label = document.getElementById("statusLabel");
    if (d.mode === "manual") {
      label.textContent = "Manual ON — PIR sensor powered off";
    } else if (d.mode === "auto") {
      label.textContent = "Auto mode — PIR sensor armed & running";
    } else {
      label.textContent = "Both modes off — lamp is off";
    }

    // Gas detector
    const gasBadge = document.getElementById("gasBadge");
    const gasTrend = document.getElementById("gasTrend");
    const gasStatCard = document.getElementById("gasStatCard");
    const gasStrip = document.getElementById("gasStrip");
    const gasIcon  = document.getElementById("gasIcon");
    const gasLabel = document.getElementById("gasLabel");

    if (d.gasAlert === true) {
      gasBadge.textContent = "ALERT";
      gasTrend.textContent = "Danger";
      gasTrend.className = "trend-pill danger";
      gasStatCard.style.borderColor = "rgba(239,68,68,0.4)";
      gasIcon.textContent = "🔴";
      gasLabel.textContent = "Gas detected! Ventilate the area";
      gasStrip.className = "note-strip danger";
    } else if (d.gasStatus === "warming") {
      gasBadge.textContent = "Warming";
      gasTrend.textContent = "Starting";
      gasTrend.className = "trend-pill warn";
      gasStatCard.style.borderColor = "";
      gasIcon.textContent = "🟡";
      gasLabel.textContent = "Sensor warming up...";
      gasStrip.className = "note-strip";
    } else if (d.gasStatus === "offline") {
      gasBadge.textContent = "Offline";
      gasTrend.textContent = "No signal";
      gasTrend.className = "trend-pill muted";
      gasStatCard.style.borderColor = "";
      gasIcon.textContent = "⚫";
      gasLabel.textContent = "Sensor not detected / disconnected";
      gasStrip.className = "note-strip";
    } else {
      gasBadge.textContent = "Normal";
      gasTrend.textContent = "Active";
      gasTrend.className = "trend-pill";
      gasStatCard.style.borderColor = "";
      gasIcon.textContent = "🟢";
      gasLabel.textContent = "Monitoring — no gas detected";
      gasStrip.className = "note-strip";
    }
  };
}

function toggle1(e) {
  ws.send(document.getElementById("t1").checked ? "l1_on" : "l1_off");
}

function toggleManual(e) {
  if (document.getElementById("t2_auto").checked) {
    document.getElementById("t2_auto").checked = false;
    ws.send("stair_manual_on");
  } else {
    ws.send("stair_manual_off");
  }
}

function toggleAuto(e) {
  if (document.getElementById("t2_manual").checked) {
    document.getElementById("t2_manual").checked = false;
    ws.send("stair_auto_on");
  } else {
    ws.send("stair_auto_off");
  }
}

window.onload = startWS;
</script>
</body>
</html>
)rawliteral";
