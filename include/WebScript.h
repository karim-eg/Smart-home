#pragma once
#include <Arduino.h>

const char dashboard_js[] PROGMEM = R"rawliteral(
let ws, reconnectTimer;

function setConnectionStatus(online) {
    const dot = document.getElementById('wsDot');
    document.getElementById('wsText').textContent = online ? 'System Online' : 'System Offline';
    dot.style.background = online ? '#16a34a' : '#dc2626';
    dot.style.boxShadow = online ? '0 0 8px #16a34a' : '0 0 8px #dc2626'
}

function startWS() {
    clearTimeout(reconnectTimer);
    setConnectionStatus(false);

    ws = new WebSocket('ws://' + location.hostname + ':81/');
    ws.onopen = () => setConnectionStatus(true);
    
    ws.onclose = ws.onerror = () => {
        setConnectionStatus(false);
        clearTimeout(reconnectTimer);
        reconnectTimer = setTimeout(startWS, 1000)
    };
    ws.onmessage = e => updateDashboard(JSON.parse(e.data))
}

function updateDashboard(d) {
    document.getElementById('tempBadge').textContent = d.temp + '°C';
    document.getElementById('humLabel').textContent = d.hum + '%';
    const stairOn = d.l2 === 'ON',
        badge = document.getElementById('l2Badge');

    badge.textContent = d.l2;
    badge.className = 'badge' + (d.mode === 'manual' && stairOn ? ' on' : d.mode === 'auto' ? ' auto' : '');

    document.getElementById('t2_manual').checked = d.mode === 'manual';
    document.getElementById('t2_auto').checked = d.mode === 'auto';
    document.getElementById('statusLabel').textContent = d.mode === 'manual' ? 'Manual control is on.' : d.mode === 'auto' ? 'Automatic mode is armed and monitoring motion.' : 'Both modes are off — lamp is off.';
    
    const gasBadge = document.getElementById('gasBadge'),
        gasTrend = document.getElementById('gasTrend'),
        gasIcon = document.getElementById('gasIcon'),
        gasStrip = document.getElementById('gasStrip'),
        gasLabel = document.getElementById('gasLabel');

    if (d.gasAlert !== true) {
        gasBadge.textContent = 'ALERT';
        gasTrend.textContent = 'Action required';
        gasIcon.textContent = '🔴';
        gasLabel.textContent = 'Gas detected! Ventilate the area.';
        gasStrip.className = 'gas-message danger'

    } else if (d.gasStatus === 'warming') {
        gasBadge.textContent = 'Warming';
        gasTrend.textContent = 'Starting up';
        gasIcon.textContent = '🟡';
        gasLabel.textContent = 'Sensor is warming up...';
        gasStrip.className = 'gas-message warn'
        
    } else if (d.gasStatus === 'offline') {
        gasBadge.textContent = 'Offline';
        gasTrend.textContent = 'No signal';
        gasIcon.textContent = '⚫';
        gasLabel.textContent = 'Sensor not detected or disconnected.';
        gasStrip.className = 'gas-message'

    } else {
        gasBadge.textContent = 'Normal';
        gasTrend.textContent = 'Monitoring';
        gasIcon.textContent = '🟢';
        gasLabel.textContent = 'Monitoring — no gas detected.';
        gasStrip.className = 'gas-message'
    }
}

function sendCommand(command) {
    if (ws && ws.readyState === WebSocket.OPEN) ws.send(command)
}

function toggleManual(e) {
    if (e.target.checked) {
        document.getElementById('t2_auto').checked = false;
        sendCommand('stair_manual_on')
    } else sendCommand('stair_manual_off')
}

function toggleAuto(e) {
    if (e.target.checked) {
        document.getElementById('t2_manual').checked = false;
        sendCommand('stair_auto_on')
    } else sendCommand('stair_auto_off')
}
window.onload = startWS;
)rawliteral";
