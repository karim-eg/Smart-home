#pragma once
#include <Arduino.h>

const char index_html[] PROGMEM = R"rawliteral(
<!doctype html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>Smart Home Console</title>
  <link rel="stylesheet" href="/dashboard.css?v=3">
</head>
<body>
  <main class="dashboard">
    <header class="topbar">
      <div>
        <p class="eyebrow">SMART HOME</p>
        <h1>System Overview</h1>
        <p class="page-sub">Monitor your sensors and staircase lighting.</p>
      </div>
      <div class="conn-pill" aria-live="polite">
        <span class="status-dot" id="wsDot"></span>
        <span id="wsText">System Offline</span>
      </div>
    </header>

    <section class="metrics" aria-label="Sensor readings">
      <article class="metric-card">
        <span class="metric-icon">&#127777;</span>
        <div><p>Temperature</p><strong id="tempBadge">--&deg;C</strong><small>Indoor reading</small></div>
      </article>
      <article class="metric-card">
        <span class="metric-icon">&#128167;</span>
        <div><p>Humidity</p><strong id="humLabel">--%</strong><small>Indoor reading</small></div>
      </article>
      <article class="metric-card" id="gasStatCard">
        <span class="metric-icon" id="gasIcon">&#9899;</span>
        <div><p>Gas Sensor</p><strong id="gasBadge">---</strong><small id="gasTrend">Waiting for data</small></div>
      </article>
    </section>

    <section class="content-grid">
      <article class="panel staircase-panel" id="stairCard">
        <div class="panel-heading">
          <div class="heading-icon">&#129501;</div>
          <div><h2>Staircase Light</h2><p>Lamp and motion sensor control</p></div>
          <span class="badge" id="l2Badge">OFF</span>
        </div>

        <div class="control-row">
          <div><h3>Manual control</h3><p>Keep the staircase lamp on.</p></div>
          <label class="switch"><input type="checkbox" id="t2_manual" onchange="toggleManual(event)"><span></span></label>
        </div>
        <div class="control-row">
          <div><h3>Automatic control</h3><p>Turn on when the PIR detects motion.</p></div>
          <label class="switch"><input type="checkbox" id="t2_auto" onchange="toggleAuto(event)"><span></span></label>
        </div>
        <div class="status-note" id="statusStrip"><span id="statusLabel">Both modes are off — lamp is off.</span></div>
      </article>

      <article class="panel gas-panel" id="gasCard">
        <div class="panel-heading">
          <div class="heading-icon">&#129514;</div>
          <div><h2>Gas Safety</h2><p>MQ-5 gas detector</p></div>
        </div>
        <div class="gas-message" id="gasStrip"><span id="gasLabel">Waiting for sensor data...</span></div>
      </article>
    </section>
  </main>
  <script src="/dashboard.js?v=3"></script>
</body>
</html>
)rawliteral";
