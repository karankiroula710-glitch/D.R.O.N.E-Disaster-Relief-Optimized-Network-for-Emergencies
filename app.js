(() => {
  'use strict';

  // PART 3 — Zone network data and shortest-route calculations.
  const STORAGE_KEY = 'drone-pbl-demo-v1';
  // PART 2 — Urgency ranks used by the priority queue.
  const PRIORITY = { Critical: 4, High: 3, Medium: 2, Low: 1 };
  const ZONES = [
    { name: 'North Ridge', code: 'NR', x: 72, y: 92, condition: 'Landslide watch' },
    { name: 'Old Town', code: 'OT', x: 225, y: 126, condition: 'Flood watch' },
    { name: 'Riverbend', code: 'RB', x: 383, y: 72, condition: 'River level elevated' },
    { name: 'Cedar Crossing', code: 'CC', x: 536, y: 139, condition: 'Bridge access restricted' },
    { name: 'Lakeview', code: 'LV', x: 140, y: 292, condition: 'Shelter open' },
    { name: 'Southbank', code: 'SB', x: 364, y: 287, condition: 'Flood watch' },
    { name: 'Central Depot', code: 'CD', x: 548, y: 306, condition: 'Relief hub' }
  ];
  const EDGES = [
    ['North Ridge', 'Old Town', 6.4], ['North Ridge', 'Lakeview', 7.8],
    ['Old Town', 'Riverbend', 5.2], ['Old Town', 'Lakeview', 4.6],
    ['Riverbend', 'Cedar Crossing', 8.1], ['Riverbend', 'Southbank', 6.3],
    ['Cedar Crossing', 'Central Depot', 5.6], ['Cedar Crossing', 'Southbank', 5.9],
    ['Lakeview', 'Southbank', 6.8], ['Southbank', 'Central Depot', 4.4],
    ['Old Town', 'Southbank', 9.2]
  ];

  // PART 1 — Request/team records and browser-persisted project state.
  const freshDemo = () => {
    const now = Date.now();
    return {
      nextId: 1049,
      teams: [
        { id: 'M-01', name: 'North Star Medical', type: 'Medical', position: 'Old Town', status: 'deployed', requestId: 'D-1048' },
        { id: 'M-02', name: 'Cedar Aid Unit', type: 'Medical', position: 'Lakeview', status: 'deployed', requestId: 'D-1047' },
        { id: 'R-01', name: 'Rapid Rescue One', type: 'Rescue', position: 'Central Depot', status: 'deployed', requestId: 'D-1046' },
        { id: 'R-02', name: 'River Response', type: 'Rescue', position: 'Riverbend', status: 'deployed', requestId: 'D-1045' },
        { id: 'S-01', name: 'Relief Supply One', type: 'Supply', position: 'Southbank', status: 'deployed', requestId: 'D-1044' },
        { id: 'S-02', name: 'Central Relief Store', type: 'Supply', position: 'Central Depot', status: 'available', requestId: null }
      ],
      requests: [
        { id: 'D-1048', type: 'Medical', severity: 'Critical', zone: 'North Ridge', description: 'Two residents need urgent medical attention after a slope collapse.', reporter: 'North Ridge ward desk', status: 'dispatched', teamId: 'M-01', createdAt: now - 44 * 60000 },
        { id: 'D-1047', type: 'Medical', severity: 'High', zone: 'Old Town', description: 'First aid and medication requested at the community shelter.', reporter: 'Old Town shelter', status: 'dispatched', teamId: 'M-02', createdAt: now - 37 * 60000 },
        { id: 'D-1046', type: 'Rescue', severity: 'High', zone: 'Riverbend', description: 'Residents are stranded on the far side of a flooded lane.', reporter: 'River watch volunteer', status: 'dispatched', teamId: 'R-01', createdAt: now - 28 * 60000 },
        { id: 'D-1045', type: 'Rescue', severity: 'Medium', zone: 'Southbank', description: 'A household needs assistance moving to the marked safe area.', reporter: 'Southbank block lead', status: 'dispatched', teamId: 'R-02', createdAt: now - 19 * 60000 },
        { id: 'D-1044', type: 'Supply', severity: 'High', zone: 'Southbank', description: 'Drinking water and dry food needed for the temporary shelter.', reporter: 'Southbank shelter', status: 'dispatched', teamId: 'S-01', createdAt: now - 13 * 60000 },
        { id: 'D-1043', type: 'Rescue', severity: 'Critical', zone: 'Cedar Crossing', description: 'People are waiting on an isolated road section after the bridge closure.', reporter: 'Cedar crossing point', status: 'queued', teamId: null, createdAt: now - 8 * 60000 },
        { id: 'D-1042', type: 'Rescue', severity: 'High', zone: 'Lakeview', description: 'A family needs an assisted evacuation from a rising-water area.', reporter: 'Lakeview resident', status: 'queued', teamId: null, createdAt: now - 4 * 60000 }
      ],
      activity: [
        { text: 'D-1043 marked critical and added to the rescue queue', time: now - 7 * 60000, icon: '!' },
        { text: 'D-1044 assigned to Relief Supply One', time: now - 12 * 60000, icon: '↗' },
        { text: 'D-1045 routed to Southbank response zone', time: now - 18 * 60000, icon: '⌖' },
        { text: 'D-1046 assigned to Rapid Rescue One', time: now - 27 * 60000, icon: '↗' }
      ]
    };
  };

  function loadState() {
    try {
      const saved = JSON.parse(localStorage.getItem(STORAGE_KEY));
      if (saved && Array.isArray(saved.requests) && Array.isArray(saved.teams) && Array.isArray(saved.activity)) return saved;
    } catch (_) { /* Use the demo state when storage is unavailable or incomplete. */ }
    return freshDemo();
  }

  let state = loadState();
  let currentView = 'overview';
  let currentFilter = 'all';
  let requestSearchTerm = '';
  let globalSearchTerm = '';
  let selectedZone = 'Cedar Crossing';
  let toastTimer;
  let chatStarted = false;

  const $ = (selector, root = document) => root.querySelector(selector);
  const $$ = (selector, root = document) => [...root.querySelectorAll(selector)];
  const esc = (value = '') => String(value).replace(/[&<>"']/g, (char) => ({ '&': '&amp;', '<': '&lt;', '>': '&gt;', '"': '&quot;', "'": '&#39;' })[char]);
  const zoneByName = (name) => ZONES.find((zone) => zone.name === name);
  const teamById = (id) => state.teams.find((team) => team.id === id);
  const fmtAge = (timestamp) => {
    const minutes = Math.max(0, Math.floor((Date.now() - Number(timestamp || Date.now())) / 60000));
    if (minutes < 1) return 'Just now';
    if (minutes < 60) return `${minutes}m ago`;
    const hours = Math.floor(minutes / 60);
    return `${hours}h ${minutes % 60}m ago`;
  };
  const statusLabel = (status) => ({ queued: 'Waiting for team', dispatched: 'In the field', resolved: 'Resolved' })[status] || status;
  const typeIcon = (type) => ({ Medical: '✚', Rescue: '⌁', Supply: '▣' })[type] || '•';
  const prettyDistance = (distance) => Number.isFinite(distance) ? `${distance.toFixed(1)} km` : 'No route';

  function saveState() {
    try { localStorage.setItem(STORAGE_KEY, JSON.stringify(state)); } catch (_) { /* App remains usable for this browser session. */ }
  }

  // PART 2 — Binary min-heap dispatch engine: higher urgency first; older requests break ties.
  class PriorityQueue {
    constructor(compare) { this.items = []; this.compare = compare; }
    get size() { return this.items.length; }
    push(value) {
      this.items.push(value);
      let index = this.items.length - 1;
      while (index > 0) {
        const parent = Math.floor((index - 1) / 2);
        if (this.compare(this.items[parent], value) <= 0) break;
        this.items[index] = this.items[parent];
        index = parent;
      }
      this.items[index] = value;
    }
    pop() {
      if (!this.items.length) return undefined;
      const first = this.items[0];
      const last = this.items.pop();
      if (this.items.length) {
        let index = 0;
        while (true) {
          const left = index * 2 + 1;
          const right = left + 1;
          let child = index;
          if (left < this.items.length && this.compare(this.items[left], this.items[child]) < 0) child = left;
          if (right < this.items.length && this.compare(this.items[right], this.items[child]) < 0) child = right;
          if (child === index) break;
          this.items[index] = this.items[child];
          index = child;
        }
        this.items[index] = last;
      }
      return first;
    }
  }

  // PART 3 — Dijkstra shortest path and nearest-specialist selection.
  function shortestPaths(start) {
    const distances = Object.fromEntries(ZONES.map((zone) => [zone.name, Infinity]));
    const previous = {};
    const visited = new Set();
    if (!zoneByName(start)) return { distances, previous };
    distances[start] = 0;
    while (visited.size < ZONES.length) {
      let current = null;
      for (const zone of ZONES) {
        if (!visited.has(zone.name) && (current === null || distances[zone.name] < distances[current])) current = zone.name;
      }
      if (current === null || distances[current] === Infinity) break;
      visited.add(current);
      for (const [from, to, distance] of EDGES) {
        const neighbor = from === current ? to : to === current ? from : null;
        if (!neighbor || visited.has(neighbor)) continue;
        const candidate = distances[current] + distance;
        if (candidate < distances[neighbor]) {
          distances[neighbor] = candidate;
          previous[neighbor] = current;
        }
      }
    }
    return { distances, previous };
  }

  function pathTo(previous, target) {
    if (!zoneByName(target)) return [];
    const path = [target];
    while (previous[path[0]]) path.unshift(previous[path[0]]);
    return path;
  }

  function nearestTeam(type, destination, availableOnly = true) {
    const candidates = state.teams.filter((team) => team.type === type && (availableOnly ? team.status === 'available' : team.status !== 'offline'));
    let best = null;
    for (const team of candidates) {
      const { distances, previous } = shortestPaths(team.position);
      const distance = distances[destination];
      if (!Number.isFinite(distance)) continue;
      if (!best || distance < best.distance) best = { team, distance, path: pathTo(previous, destination) };
    }
    return best;
  }

  function addActivity(text, icon = '↗') {
    state.activity.unshift({ text, time: Date.now(), icon });
    state.activity = state.activity.slice(0, 9);
  }

  function processQueue() {
    const waiting = new PriorityQueue((a, b) => (PRIORITY[b.severity] - PRIORITY[a.severity]) || (a.createdAt - b.createdAt));
    state.requests.filter((request) => request.status === 'queued').forEach((request) => waiting.push(request));
    let assigned = 0;
    while (waiting.size) {
      const request = waiting.pop();
      const choice = nearestTeam(request.type, request.zone, true);
      if (!choice) continue;
      request.status = 'dispatched';
      request.teamId = choice.team.id;
      choice.team.status = 'deployed';
      choice.team.position = request.zone;
      choice.team.requestId = request.id;
      assigned += 1;
      addActivity(`${request.id} assigned to ${choice.team.name} · ${prettyDistance(choice.distance)}`, '↗');
    }
    return assigned;
  }

  function getQueue() {
    const queue = new PriorityQueue((a, b) => (PRIORITY[b.severity] - PRIORITY[a.severity]) || (a.createdAt - b.createdAt));
    state.requests.filter((request) => request.status === 'queued').forEach((request) => queue.push(request));
    const sorted = [];
    while (queue.size) sorted.push(queue.pop());
    return sorted;
  }

  function openRequests() { return state.requests.filter((request) => request.status !== 'resolved'); }
  function activeRequestsAt(zone) { return state.requests.filter((request) => request.status !== 'resolved' && request.zone === zone); }

  function getPreviewRequest() {
    return getQueue()[0] || state.requests.filter((request) => request.status === 'dispatched').sort((a, b) => PRIORITY[b.severity] - PRIORITY[a.severity])[0] || null;
  }

  function routeForRequest(request) {
    if (!request) return null;
    const assigned = request.teamId && teamById(request.teamId);
    if (assigned) {
      const { distances, previous } = shortestPaths(assigned.position);
      return { team: assigned, distance: distances[request.zone], path: pathTo(previous, request.zone), available: assigned.status === 'available' };
    }
    return nearestTeam(request.type, request.zone, false);
  }

  function getMapPath() {
    const requestsHere = activeRequestsAt(selectedZone);
    const request = getQueue().find((item) => item.zone === selectedZone)
      || requestsHere.find((item) => item.status === 'dispatched')
      || requestsHere[0]
      || getPreviewRequest();
    const route = request ? routeForRequest(request) : nearestTeam('Rescue', selectedZone, false);
    if (route?.path?.length > 1) return route.path;
    if (selectedZone) {
      const options = state.teams.filter((team) => team.status !== 'offline');
      let best = null;
      for (const team of options) {
        const { distances, previous } = shortestPaths(team.position);
        if (!best || distances[selectedZone] < best.distance) best = { distance: distances[selectedZone], path: pathTo(previous, selectedZone) };
      }
      return best?.path || [];
    }
    return [];
  }

  function edgeKey(a, b) { return [a, b].sort().join('|'); }

  // PART 3 — Interactive network visualization and route preview.
  function renderMap(containerId, large = false) {
    const container = document.getElementById(containerId);
    if (!container) return;
    const width = 620;
    const height = 380;
    const activePath = getMapPath();
    const activeEdges = new Set(activePath.slice(1).map((zone, i) => edgeKey(activePath[i], zone)));
    const edges = EDGES.map(([from, to, distance]) => {
      const a = zoneByName(from), b = zoneByName(to);
      const isActive = activeEdges.has(edgeKey(from, to));
      const mx = (a.x + b.x) / 2, my = (a.y + b.y) / 2;
      return `<g><line class="map-edge ${isActive ? 'route-active' : ''}" x1="${a.x}" y1="${a.y}" x2="${b.x}" y2="${b.y}"/><text class="edge-label" x="${mx}" y="${my - 5}" text-anchor="middle">${distance.toFixed(1)}</text></g>`;
    }).join('');
    const nodes = ZONES.map((zone) => {
      const incidents = activeRequestsAt(zone.name).length;
      const teams = state.teams.filter((team) => team.position === zone.name && team.status !== 'offline');
      const available = teams.filter((team) => team.status === 'available').length;
      const deployed = teams.length - available;
      const selected = selectedZone === zone.name;
      const countTag = incidents ? `<circle cx="${zone.x + 18}" cy="${zone.y - 17}" r="9" fill="#d5684d"/><text class="node-count" x="${zone.x + 18}" y="${zone.y - 17}">${incidents}</text>` : '';
      const teamTags = [deployed ? `<circle class="team-marker" cx="${zone.x - 17}" cy="${zone.y - 17}" r="8"/><text class="team-count" x="${zone.x - 17}" y="${zone.y - 17}">${deployed}</text>` : '', available ? `<circle class="team-marker available" cx="${zone.x - 17}" cy="${zone.y + 17}" r="8"/><text class="team-count" x="${zone.x - 17}" y="${zone.y + 17}">${available}</text>` : ''].join('');
      const classes = `zone-node ${incidents ? 'has-incident' : ''} ${selected ? 'selected' : ''}`;
      return `<g class="${classes}" data-zone="${esc(zone.name)}" tabindex="0" role="button" aria-label="Select ${esc(zone.name)}, ${incidents} open requests"><title>${esc(zone.name)} · ${incidents} open request${incidents === 1 ? '' : 's'}</title><circle class="node-halo" cx="${zone.x}" cy="${zone.y}" r="16"/><circle class="node-point" cx="${zone.x}" cy="${zone.y}" r="10"/><text class="node-code" x="${zone.x}" y="${zone.y}">${zone.code}</text>${countTag}${teamTags}<text class="node-label" x="${zone.x}" y="${zone.y + 34}">${esc(zone.name)}</text></g>`;
    }).join('');
    const aria = large ? 'Interactive zone network map. Lines show distances in kilometres.' : 'Interactive response map of disaster zones.';
    container.innerHTML = `<svg viewBox="0 0 ${width} ${height}" role="img" aria-label="${aria}" preserveAspectRatio="xMidYMid meet"><path class="map-coast" d="M44 232 C111 171 103 110 185 99 S294 115 347 84 S462 73 560 196 S507 300 445 328"/><path class="map-roads" d="M72 92 225 126 383 72 536 139 548 306 364 287 140 292Z"/>${edges}${nodes}</svg>`;
    $$('[data-zone]', container).forEach((node) => {
      node.addEventListener('click', () => selectZone(node.dataset.zone));
      node.addEventListener('keydown', (event) => { if (event.key === 'Enter' || event.key === ' ') { event.preventDefault(); selectZone(node.dataset.zone); } });
    });
  }

  function selectZone(zone) {
    selectedZone = zoneByName(zone)?.name || selectedZone;
    const selector = $('#zoneSelect');
    if (selector) selector.value = selectedZone;
    renderMap('networkMap');
    renderMap('largeNetworkMap', true);
    renderRoutePanel();
    renderZoneTable();
    showToast(`${selectedZone} selected · New requests will use this zone`);
  }

  // PART 4 — Command center, request/team views, activity feed, and query assistant.
  function renderMetrics() {
    const assignedCount = state.requests.filter((request) => request.status === 'dispatched').length;
    const waitingCount = state.requests.filter((request) => request.status === 'queued').length;
    const openCount = openRequests().length;
    const zoneAlerts = new Set(openRequests().map((request) => request.zone)).size;
    const availableTeams = state.teams.filter((team) => team.status === 'available').length;
    const metrics = [
      { label: 'Open requests', value: openCount, foot: `<strong>${waitingCount} waiting</strong> for a suitable unit`, icon: '≋', tint: 'alert' },
      { label: 'Teams in the field', value: assignedCount, foot: `<strong>${availableTeams} available</strong> to respond`, icon: '↗', tint: 'orange' },
      { label: 'Waiting in queue', value: waitingCount, foot: waitingCount ? `<strong>${getQueue()[0]?.severity || '—'}</strong> is next by priority` : 'All requests have a team', icon: '◷', tint: 'blue' },
      { label: 'Zones with incidents', value: zoneAlerts, foot: `<strong>${ZONES.length} zones</strong> in the response network`, icon: '⌖', tint: '' }
    ];
    $('#metricGrid').innerHTML = metrics.map((metric) => `<article class="metric-card"><div class="metric-top"><span>${metric.label}</span><span class="metric-icon ${metric.tint}">${metric.icon}</span></div><div class="metric-value">${metric.value}</div><div class="metric-foot">${metric.foot}</div></article>`).join('');
    $('#navRequestCount').textContent = openCount;
    $('#mapIncidentCount').textContent = openCount;
    $('#zoneCount').textContent = ZONES.length;
    $('#queueCount').textContent = waitingCount;
  }

  function queueItem(request) {
    return `<article class="queue-item"><span class="priority-mark ${request.severity.toLowerCase()}">${typeIcon(request.type)}</span><div class="queue-main"><div class="queue-line"><span class="queue-id">${esc(request.id)}</span><span class="severity-label ${request.severity.toLowerCase()}">${esc(request.severity)}</span><span class="queue-type">${esc(request.type)}</span></div><div class="queue-zone">${esc(request.zone)}</div><div class="queue-meta"><span>${esc(fmtAge(request.createdAt))}</span><span>·</span><span>${esc(request.description)}</span></div></div><button class="queue-action" data-request-route="${esc(request.id)}" type="button">Review</button></article>`;
  }

  function renderQueue() {
    const queue = getQueue();
    $('#queueList').innerHTML = queue.length ? queue.slice(0, 4).map(queueItem).join('') : '<div class="queue-empty"><strong>Queue clear</strong><span>All incoming requests have a suitable team.</span></div>';
    $('#queueList').querySelectorAll('[data-request-route]').forEach((button) => button.addEventListener('click', () => {
      const request = state.requests.find((item) => item.id === button.dataset.requestRoute);
      if (!request) return;
      goView('network');
      selectZone(request.zone);
    }));
  }

  function renderTeamStrip() {
    const list = state.teams.slice(0, 3);
    $('#teamStrip').innerHTML = list.map((team) => `<div class="team-mini"><div class="team-mini-top"><span class="team-avatar ${team.type.toLowerCase()}">${typeIcon(team.type)}</span><span class="team-mini-copy"><strong>${esc(team.id)} · ${esc(team.name)}</strong><small>${esc(team.type)} response</small></span></div><div class="team-state"><i class="state-dot ${team.status === 'available' ? 'available' : team.status === 'offline' ? 'offline' : ''}"></i>${team.status === 'deployed' ? `Responding · ${esc(team.position)}` : team.status === 'available' ? `Available · ${esc(team.position)}` : 'Out of service'}</div></div>`).join('');
  }

  function renderActivity() {
    $('#activityList').innerHTML = state.activity.slice(0, 4).map((item) => `<div class="activity-item"><span class="activity-icon ${item.icon === '✓' ? 'resolved' : ''}">${esc(item.icon || '•')}</span><div class="activity-copy">${esc(item.text)}</div><span class="activity-time">${esc(fmtAge(item.time))}</span></div>`).join('') || '<div class="empty-list">No dispatch activity yet.</div>';
  }

  function statusBadge(status) {
    return `<span class="status-badge ${status === 'queued' ? 'queued' : status === 'resolved' ? 'resolved' : ''}"><i class="state-dot"></i>${esc(statusLabel(status))}</span>`;
  }

  function renderRequests() {
    const active = openRequests();
    const assigned = state.requests.filter((request) => request.status === 'dispatched').length;
    const waiting = state.requests.filter((request) => request.status === 'queued').length;
    const resolved = state.requests.filter((request) => request.status === 'resolved').length;
    $('#requestSummary').innerHTML = `<div class="summary-tile"><span>Total reports</span><strong>${state.requests.length}</strong></div><div class="summary-tile"><span>Need a team</span><strong>${waiting}</strong></div><div class="summary-tile"><span>In the field</span><strong>${assigned}</strong></div><div class="summary-tile"><span>Resolved</span><strong>${resolved}</strong></div>`;
    $('#filterAllCount').textContent = active.length;
    $('#filterQueuedCount').textContent = waiting;
    $('#filterDispatchedCount').textContent = assigned;
    $('#filterResolvedCount').textContent = resolved;
    $$('#requestFilters [data-filter]').forEach((button) => button.classList.toggle('selected', button.dataset.filter === currentFilter));
    const records = state.requests.filter((request) => currentFilter === 'all' ? true : request.status === currentFilter).sort((a, b) => b.createdAt - a.createdAt);
    const filtered = records.filter((request) => {
      const term = requestSearchTerm.toLowerCase();
      return !term || [request.id, request.type, request.severity, request.zone, request.description, request.reporter].join(' ').toLowerCase().includes(term);
    });
    $('#requestList').innerHTML = filtered.length ? filtered.map((request) => {
      const team = request.teamId && teamById(request.teamId);
      const action = request.status === 'dispatched' ? `<button class="row-action" data-resolve="${esc(request.id)}" type="button">Resolve</button>` : `<button class="row-action" data-request-zone="${esc(request.zone)}" type="button">View zone</button>`;
      return `<article class="request-row"><div class="req-primary"><span class="priority-mark ${request.severity.toLowerCase()}">${typeIcon(request.type)}</span><div class="req-details"><strong>${esc(request.id)} · ${esc(request.description)}</strong><small>${esc(request.reporter || 'Community report')} · ${esc(fmtAge(request.createdAt))}</small></div><span class="severity-label ${request.severity.toLowerCase()}">${esc(request.severity)}</span></div><div class="req-zone"><strong>${esc(request.zone)}</strong><small>Reported zone</small></div><div class="req-assigned"><strong>${team ? `${esc(team.id)} · ${esc(team.name)}` : request.status === 'queued' ? 'Awaiting suitable team' : '—'}</strong><small>${team ? `${esc(team.type)} response` : request.status === 'queued' ? `No ${esc(request.type.toLowerCase())} team available` : 'No team assigned'}</small></div>${statusBadge(request.status)}<div class="req-actions">${action}</div></article>`;
    }).join('') : '<div class="empty-list"><strong>No matching requests</strong>Try another filter or search term.</div>';
    $('#requestList').querySelectorAll('[data-resolve]').forEach((button) => button.addEventListener('click', () => resolveRequest(button.dataset.resolve)));
    $('#requestList').querySelectorAll('[data-request-zone]').forEach((button) => button.addEventListener('click', () => { goView('network'); selectZone(button.dataset.requestZone); }));
  }

  function renderTeams() {
    const counts = ['Medical', 'Rescue', 'Supply'].map((type) => {
      const units = state.teams.filter((team) => team.type === type);
      return { type, total: units.length, available: units.filter((team) => team.status === 'available').length };
    });
    $('#teamSummary').innerHTML = counts.map((item) => `<div class="summary-tile"><span>${item.type} units · ${item.available} available</span><strong>${item.total}</strong></div>`).join('');
    $('#teamGroups').innerHTML = ['Medical', 'Rescue', 'Supply'].map((type) => {
      const teams = state.teams.filter((team) => team.type === type);
      return `<section class="panel team-group"><div class="team-group-head"><span class="team-type-icon ${type.toLowerCase()}">${typeIcon(type)}</span><div><h2>${type} response</h2><p>Specialty-matched dispatch</p></div><span class="group-count">${teams.length} units</span></div>${teams.map((team) => {
        const request = team.requestId && state.requests.find((item) => item.id === team.requestId && item.status === 'dispatched');
        const statusText = team.status === 'available' ? 'Available' : team.status === 'offline' ? 'Out of service' : 'In the field';
        const statusClass = team.status === 'available' ? '' : team.status === 'offline' ? 'resolved' : 'queued';
        const action = team.status === 'available' ? `<button data-team-offline="${esc(team.id)}" type="button">Set unavailable</button>` : team.status === 'offline' ? `<button data-team-available="${esc(team.id)}" type="button">Set available</button>` : `<button data-team-request="${esc(team.requestId || '')}" type="button">View request</button>`;
        return `<div class="team-card"><div class="team-card-top"><span class="team-id-avatar ${type.toLowerCase()}">${esc(team.id)}</span><div class="team-card-copy"><strong>${esc(team.name)}</strong><small>${esc(team.type)} specialist · ${esc(team.position)}</small></div><span class="status-badge ${statusClass}"><i class="state-dot"></i>${statusText}</span></div><div class="team-card-bottom"><span>${request ? `Responding to <b>${esc(request.id)}</b>` : team.status === 'available' ? 'Ready for dispatch' : 'Temporarily unavailable'}</span>${action}</div></div>`;
      }).join('')}</section>`;
    }).join('');
    $$('[data-team-offline]').forEach((button) => button.addEventListener('click', () => changeTeamStatus(button.dataset.teamOffline, 'offline')));
    $$('[data-team-available]').forEach((button) => button.addEventListener('click', () => changeTeamStatus(button.dataset.teamAvailable, 'available')));
    $$('[data-team-request]').forEach((button) => button.addEventListener('click', () => {
      const request = state.requests.find((item) => item.id === button.dataset.teamRequest);
      if (request) { goView('requests'); requestSearchTerm = request.id; $('#requestSearch').value = request.id; renderRequests(); }
    }));
  }

  function renderRoutePanel() {
    const zone = zoneByName(selectedZone);
    if (!zone) return;
    $('#routeZoneTitle').textContent = zone.name;
    const requests = activeRequestsAt(zone.name);
    const preferredRequest = getQueue().find((request) => request.zone === zone.name) || requests.find((request) => request.status === 'dispatched') || { type: 'Rescue', zone: zone.name, teamId: null };
    const route = routeForRequest(preferredRequest);
    $('#routeIntro').textContent = requests.length ? `${requests.length} active request${requests.length === 1 ? '' : 's'} in this zone. ${zone.condition}.` : `${zone.condition}. Preview the nearest ${preferredRequest.type.toLowerCase()} route for this location.`;
    if (!route) {
      $('#routeDetails').innerHTML = `<div class="route-empty">No ${esc(preferredRequest.type.toLowerCase())} team is currently registered in the network. Requests of this type will wait in the queue.</div>`;
    } else {
      const isBusy = route.team.status === 'deployed';
      const routeText = route.path.length > 1 ? route.path.map(esc).join(' → ') : 'Already at this zone';
      $('#routeDetails').innerHTML = `<div class="route-metric"><span>Shortest road distance</span><strong>${prettyDistance(route.distance)}</strong><small>${route.team.status === 'available' ? 'Available now' : 'Nearest suitable team is currently deployed'}</small></div><div class="route-detail"><span class="route-step-num">1</span><div class="route-copy"><strong>${esc(route.team.id)} · ${esc(route.team.name)}</strong><small>${esc(route.team.type)} team · ${esc(route.team.status === 'available' ? 'Available' : 'In the field')} at ${esc(route.team.position)}</small></div></div><div class="route-detail"><span class="route-step-num">2</span><div class="route-copy"><strong>${esc(routeText)}</strong><small>${isBusy ? 'Preview route only · this team must be released before it can take another request.' : 'Dijkstra shortest path over the simulated zone links.'}</small></div></div>`;
    }
    $('#requestFromZoneBtn').disabled = false;
  }

  function renderZoneTable() {
    const rows = ZONES.map((zone) => {
      const incidents = activeRequestsAt(zone.name).length;
      const teams = state.teams.filter((team) => team.position === zone.name && team.status !== 'offline');
      const available = teams.filter((team) => team.status === 'available').length;
      return `<div class="zone-row" data-zone-row="${esc(zone.name)}"><div class="zone-name"><span class="zone-code">${zone.code}</span><strong>${esc(zone.name)}</strong></div><span>${esc(zone.condition)}</span><span>${teams.length ? `${teams.length} unit${teams.length === 1 ? '' : 's'} · ${available} free` : 'No team stationed'}</span><span class="zone-incidents">${incidents ? `${incidents} open request${incidents === 1 ? '' : 's'}` : 'No active reports'}</span></div>`;
    });
    $('#zoneTable').innerHTML = `<div class="zone-row zone-heading"><span>ZONE</span><span>AREA STATUS</span><span>TEAM PRESENCE</span><span>ACTIVE INCIDENTS</span></div>${rows.join('')}`;
    $$('[data-zone-row]').forEach((row) => row.addEventListener('click', () => selectZone(row.dataset.zoneRow)));
  }

  function renderAll() {
    saveState();
    renderMetrics();
    renderQueue();
    renderTeamStrip();
    renderActivity();
    renderRequests();
    renderTeams();
    renderMap('networkMap');
    renderMap('largeNetworkMap', true);
    renderRoutePanel();
    renderZoneTable();
    const date = new Intl.DateTimeFormat(undefined, { weekday: 'long', month: 'long', day: 'numeric' }).format(new Date());
    $('#dateLabel').textContent = date.toUpperCase();
  }

  function showToast(message) {
    const toast = $('#toast');
    toast.textContent = message;
    toast.classList.add('visible');
    clearTimeout(toastTimer);
    toastTimer = setTimeout(() => toast.classList.remove('visible'), 2600);
  }

  function goView(view) {
    if (!document.getElementById(`view-${view}`)) return;
    currentView = view;
    $$('.page-view').forEach((section) => section.classList.toggle('active', section.id === `view-${view}`));
    $$('.nav-item').forEach((button) => button.classList.toggle('active', button.dataset.view === view));
    window.scrollTo({ top: 0, behavior: 'smooth' });
  }

  // PART 1 — Request form intake and report lifecycle.
  function openRequestDialog() {
    const modal = $('#requestModal');
    $('#zoneSelect').value = selectedZone;
    modal.hidden = false;
    document.body.style.overflow = 'hidden';
    setTimeout(() => $('select[name="type"]', modal).focus(), 10);
  }

  function closeRequestDialog() {
    $('#requestModal').hidden = true;
    document.body.style.overflow = '';
  }

  function submitRequest(formData) {
    const type = String(formData.get('type') || 'Medical');
    const severity = String(formData.get('severity') || 'Medium');
    const zone = String(formData.get('zone') || selectedZone);
    const description = String(formData.get('description') || '').trim();
    const reporter = String(formData.get('reporter') || '').trim() || 'Community report';
    if (!zoneByName(zone) || !PRIORITY[severity] || !['Medical', 'Rescue', 'Supply'].includes(type) || description.length < 4) {
      showToast('Check the request details and try again.');
      return;
    }
    const request = { id: `D-${state.nextId++}`, type, severity, zone, description, reporter, status: 'queued', teamId: null, createdAt: Date.now() };
    state.requests.unshift(request);
    const dispatched = processQueue();
    if (request.status === 'dispatched') {
      const team = teamById(request.teamId);
      addActivity(`${request.id} received and assigned to ${team.name}`, '↗');
    } else {
      addActivity(`${request.id} received · waiting for a ${type.toLowerCase()} team`, '!');
    }
    renderAll();
    closeRequestDialog();
    $('#requestForm').reset();
    if (request.status === 'dispatched') showToast(`${request.id} received · ${teamById(request.teamId).id} dispatched`);
    else showToast(`${request.id} added to the priority queue`);
    if (dispatched > 1) setTimeout(() => showToast(`${dispatched} waiting request${dispatched === 1 ? '' : 's'} dispatched`), 2800);
  }

  // PART 2 — Release a dispatched team and retry the matching priority queue.
  function resolveRequest(id) {
    const request = state.requests.find((item) => item.id === id && item.status === 'dispatched');
    if (!request) return;
    const team = teamById(request.teamId);
    request.status = 'resolved';
    if (team) { team.status = 'available'; team.requestId = null; }
    addActivity(`${request.id} marked resolved${team ? ` · ${team.name} is available` : ''}`, '✓');
    const assigned = processQueue();
    renderAll();
    if (assigned) showToast(`${request.id} resolved · ${assigned} queued request${assigned === 1 ? '' : 's'} dispatched`);
    else showToast(`${request.id} marked resolved · team returned to service`);
  }

  function changeTeamStatus(id, status) {
    const team = teamById(id);
    if (!team || !['available', 'offline'].includes(status) || team.status === 'deployed') return;
    team.status = status;
    if (status === 'available') {
      addActivity(`${team.name} is available for dispatch`, '✓');
      const assigned = processQueue();
      renderAll();
      showToast(assigned ? `${team.id} available · ${assigned} waiting request${assigned === 1 ? '' : 's'} dispatched` : `${team.id} is ready for dispatch`);
    } else {
      addActivity(`${team.name} set out of service`, '•');
      renderAll();
      showToast(`${team.id} set out of service`);
    }
  }

  function openQueryDrawer() {
    $('#queryDrawer').classList.add('open');
    $('#queryDrawer').setAttribute('aria-hidden', 'false');
    $('#drawerScrim').hidden = false;
    if (!chatStarted) {
      chatStarted = true;
      addChatMessage('assistant', 'Hi, I can look up this simulation’s requests, teams, zones and routes. Try “Which requests are waiting?” or “How are requests prioritized?”');
    }
    setTimeout(() => $('#queryInput').focus(), 130);
  }

  function closeQueryDrawer() {
    $('#queryDrawer').classList.remove('open');
    $('#queryDrawer').setAttribute('aria-hidden', 'true');
    $('#drawerScrim').hidden = true;
  }

  function addChatMessage(role, text) {
    const message = document.createElement('div');
    message.className = `chat-message ${role}`;
    message.textContent = text;
    $('#chatMessages').append(message);
    $('#chatMessages').scrollTop = $('#chatMessages').scrollHeight;
  }

  // PART 4 — Local query assistant; responses use only the current demo state.
  function assistantAnswer(question) {
    const q = question.toLowerCase();
    const waiting = getQueue();
    if (/how|priorit|algorithm|dijkstra|sort|work/.test(q)) {
      return 'Requests are ordered by severity (Critical, High, Medium, Low). Requests with the same severity are ordered by the time received. A binary min-heap represents this priority queue. For each waiting request, the simulator checks for an available team of the matching type, then uses Dijkstra’s algorithm on the zone-distance graph to select the nearest one. If none is free, the request stays queued.';
    }
    if (/wait|queue|pending|next|urgent|critical|highest|priority/.test(q)) {
      if (!waiting.length) return 'There are no requests waiting for a team right now.';
      const next = waiting[0];
      const rest = waiting.slice(1).map((request) => `${request.id} (${request.severity} ${request.type}, ${request.zone})`);
      return `Next by priority: ${next.id} · ${next.severity} ${next.type} · ${next.zone}. ${next.description}${rest.length ? `\nAlso waiting: ${rest.join('; ')}.` : ''}`;
    }
    if (/team|unit|volunteer|available|free|busy/.test(q)) {
      const available = state.teams.filter((team) => team.status === 'available');
      const byType = ['Medical', 'Rescue', 'Supply'].map((type) => `${type}: ${available.filter((team) => team.type === type).map((team) => `${team.id} at ${team.position}`).join(', ') || 'none available'}`);
      return `Available teams (${available.length}): ${byType.join(' · ')}. In-field units return to the available pool when their request is resolved.`;
    }
    if (/route|distance|nearest|travel|path|km/.test(q)) {
      const mentionedZone = ZONES.find((item) => q.includes(item.name.toLowerCase()));
      const zone = mentionedZone?.name || selectedZone || 'Cedar Crossing';
      const requestedType = /medical|medic|health/.test(q) ? 'Medical' : /supply|food|water/.test(q) ? 'Supply' : /rescue/.test(q) ? 'Rescue' : 'Rescue';
      const route = nearestTeam(requestedType, zone, false);
      if (!route) return `There is no ${requestedType.toLowerCase()} team registered in the network.`;
      return `Nearest ${requestedType.toLowerCase()} team to ${zone}: ${route.team.id} at ${route.team.position}, ${prettyDistance(route.distance)} by the shortest network route (${route.path.join(' → ')}). ${route.team.status === 'available' ? 'This unit is available.' : 'This is a route preview; the unit is currently deployed.'}`;
    }
    if (/zone|area|location|incident/.test(q)) {
      const entries = ZONES.map((zone) => `${zone.name}: ${activeRequestsAt(zone.name).length} open`).join(' · ');
      return `Current zone report: ${entries}. Zone conditions are illustrative scenario labels, not live hazard data.`;
    }
    const matching = state.requests.filter((request) => [request.id, request.type, request.severity, request.zone, request.description, request.reporter].join(' ').toLowerCase().includes(q));
    if (matching.length) return matching.slice(0, 4).map((request) => `${request.id} · ${request.severity} ${request.type} · ${request.zone} · ${statusLabel(request.status)}${request.teamId ? ` (${request.teamId})` : ''}`).join('\n');
    return 'I can answer questions about waiting requests, urgency, teams, zones and shortest routes. Try a request ID or one of those topics.';
  }

  function askQuestion(question) {
    const trimmed = question.trim();
    if (!trimmed) return;
    addChatMessage('user', trimmed);
    $('#queryInput').value = '';
    setTimeout(() => addChatMessage('assistant', assistantAnswer(trimmed)), 160);
  }

  function resetDemo() {
    state = freshDemo();
    requestSearchTerm = '';
    globalSearchTerm = '';
    currentFilter = 'all';
    selectedZone = 'Cedar Crossing';
    $('#globalSearch').value = '';
    $('#requestSearch').value = '';
    renderAll();
    goView('overview');
    showToast('Demo data restored');
  }

  function submitGlobalSearch() {
    globalSearchTerm = $('#globalSearch').value.trim();
    if (!globalSearchTerm) return;
    currentFilter = 'all';
    requestSearchTerm = globalSearchTerm;
    $('#requestSearch').value = globalSearchTerm;
    renderRequests();
    goView('requests');
  }

  function updateClock() {
    $('#clockText').textContent = new Intl.DateTimeFormat(undefined, { hour: '2-digit', minute: '2-digit' }).format(new Date());
  }

  function init() {
    $('#zoneSelect').innerHTML = ZONES.map((zone) => `<option value="${esc(zone.name)}">${zone.code} · ${esc(zone.name)}</option>`).join('');
    $('#zoneSelect').value = selectedZone;
    renderAll();
    updateClock();
    setInterval(updateClock, 30000);

    $$('.nav-item').forEach((button) => button.addEventListener('click', () => goView(button.dataset.view)));
    $$('[data-goto]').forEach((button) => button.addEventListener('click', () => goView(button.dataset.goto)));
    $$('[data-open-request]').forEach((button) => button.addEventListener('click', openRequestDialog));
    $('#newRequestBtn').addEventListener('click', openRequestDialog);
    $('#mobileNewRequestBtn').addEventListener('click', openRequestDialog);
    $$('[data-close-request]').forEach((button) => button.addEventListener('click', closeRequestDialog));
    $('#requestModal').addEventListener('click', (event) => { if (event.target === $('#requestModal')) closeRequestDialog(); });
    $('#requestForm').addEventListener('submit', (event) => { event.preventDefault(); submitRequest(new FormData(event.currentTarget)); });
    $('#requestFilters').addEventListener('click', (event) => {
      const button = event.target.closest('[data-filter]');
      if (!button) return;
      currentFilter = button.dataset.filter;
      renderRequests();
    });
    $('#requestSearch').addEventListener('input', (event) => { requestSearchTerm = event.target.value.trim(); renderRequests(); });
    $('#globalSearch').addEventListener('keydown', (event) => { if (event.key === 'Enter') submitGlobalSearch(); });
    $('#globalSearch').addEventListener('search', () => { if (!$('#globalSearch').value) { globalSearchTerm = ''; requestSearchTerm = ''; renderRequests(); } });
    $('#queryOpenBtn').addEventListener('click', openQueryDrawer);
    $('#queryCloseBtn').addEventListener('click', closeQueryDrawer);
    $('#drawerScrim').addEventListener('click', closeQueryDrawer);
    $('#queryForm').addEventListener('submit', (event) => { event.preventDefault(); askQuestion($('#queryInput').value); });
    $$('.suggestion-row [data-query]').forEach((button) => button.addEventListener('click', () => askQuestion(button.dataset.query)));
    $('#requestFromZoneBtn').addEventListener('click', () => { goView('overview'); openRequestDialog(); });
    $('#queueInfoBtn').addEventListener('click', () => { openQueryDrawer(); askQuestion('How are requests prioritized?'); });
    $('#resetDemoBtn').addEventListener('click', resetDemo);
    document.addEventListener('keydown', (event) => {
      if (event.key === 'Escape') {
        if (!$('#requestModal').hidden) closeRequestDialog();
        closeQueryDrawer();
      }
      if ((event.ctrlKey || event.metaKey) && event.key.toLowerCase() === 'k') { event.preventDefault(); $('#globalSearch').focus(); }
    });
  }

  init();
})();
