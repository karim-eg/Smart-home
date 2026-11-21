const byId = id => document.getElementById(id);

async function refresh() {
	try {
		const r = await fetch('/api/adblock/stats', { cache: 'no-store' });
		if (!r.ok) throw Error();

		const d = await r.json();
		byId('q').textContent = d.queries;
		byId('b').textContent = d.blocked;
		byId('f').textContent = d.forwarded;
		byId('last').textContent = d.last || '—';
		byId('storageState').textContent = d.storage
			? 'SD card ready — settings and DNS history are saved.'
			: 'SD card unavailable — saved changes cannot be written.';

		const ul = byId('domains');
		ul.replaceChildren();
		d.domains.forEach(domain => {
			const li = document.createElement('li');
			const name = document.createElement('span');
			const button = document.createElement('button');

			name.textContent = domain;
			button.textContent = 'Remove';
			button.className = 'remove';
			button.onclick = () => removeDomain(domain);
			li.append(name, button);
			ul.append(li);
		});
	} catch (e) {
		byId('message').textContent = 'Failed to connect to the device';
	}
}

async function refreshHistory() {
	try {
		const r = await fetch('/api/adblock/history', { cache: 'no-store' });
		if (!r.ok) throw Error();
		const d = await r.json();
		const ul = byId('history');
		ul.replaceChildren();
		if (!d.events.length) {
			const li = document.createElement('li');
			li.textContent = 'No blocked requests recorded yet.';
			ul.append(li);
			return;
		}
		d.events.slice().reverse().forEach(event => {
			const li = document.createElement('li');
			const when = event.epoch
				? new Date(event.epoch * 1000).toLocaleString()
				: `Uptime ${event.uptime}s`;
			li.textContent = `${event.domain} · ${when}`;
			ul.append(li);
		});
	} catch (e) {
		byId('history').textContent = 'Could not load DNS history.';
	}
}

async function removeDomain(domain) {
	const body = new URLSearchParams({ domain });
	const r = await fetch('/api/adblock/remove', {
		method: 'POST',
		headers: { 'Content-Type': 'application/x-www-form-urlencoded' },
		body,
	});
	byId('message').textContent = r.ok ? 'Domain removed' : await r.text();
	refresh();
}

byId('add').addEventListener('submit', async e => {
	e.preventDefault();
	const input = byId('domain');
	const body = new URLSearchParams({ domain: input.value });
	const r = await fetch('/api/adblock/domains', {
		method: 'POST',
		headers: { 'Content-Type': 'application/x-www-form-urlencoded' },
		body,
	});
	byId('message').textContent = await r.text();
	if (r.ok) input.value = '';
	refresh();
});

refresh();
refreshHistory();
setInterval(refresh, 2500);
setInterval(refreshHistory, 15000);
