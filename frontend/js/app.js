// =============================================================================
// app.js - Single Page Application Driver for RailReserve Pro
// Pure Vanilla JavaScript (ES Modules) - No external libraries
// =============================================================================

import { api } from './api.js';

// Global application state
const state = {
    trains: [],
    selectedTrainNo: null,
    pendingCancelPnr: null
};

// =============================================================================
// Theme Handling (Light / Dark)
// =============================================================================

function initTheme() {
    const savedTheme = localStorage.getItem('rail_theme') || 
        (window.matchMedia('(prefers-color-scheme: dark)').matches ? 'dark' : 'light');
    document.documentElement.setAttribute('data-theme', savedTheme);

    const toggleBtn = document.getElementById('theme-toggle');
    toggleBtn.addEventListener('click', () => {
        const current = document.documentElement.getAttribute('data-theme');
        const next = current === 'dark' ? 'light' : 'dark';
        document.documentElement.setAttribute('data-theme', next);
        localStorage.setItem('rail_theme', next);
    });
}

// =============================================================================
// Toast Notifications
// =============================================================================

function showToast(message, type = 'info') {
    const container = document.getElementById('toast-container');
    const toast = document.createElement('div');
    toast.className = `toast toast-${type}`;
    toast.textContent = message;
    container.appendChild(toast);

    setTimeout(() => {
        toast.style.opacity = '0';
        toast.style.transform = 'translateY(10px)';
        setTimeout(() => toast.remove(), 250);
    }, 3500);
}

// =============================================================================
// Tab Switching
// =============================================================================

function switchTab(tabId) {
    document.querySelectorAll('.tab-btn').forEach(btn => {
        btn.classList.toggle('active', btn.dataset.tab === tabId);
    });

    document.querySelectorAll('.tab-content').forEach(content => {
        content.classList.toggle('active', content.id === `${tabId}-section`);
    });

    window.location.hash = tabId;

    if (tabId === 'trains') loadTrains();
    if (tabId === 'waiting') loadWaitingList();
    if (tabId === 'passengers') loadPassengers();
}

function initTabs() {
    document.querySelectorAll('.tab-btn').forEach(btn => {
        btn.addEventListener('click', () => switchTab(btn.dataset.tab));
    });

    const initialTab = window.location.hash.replace('#', '') || 'trains';
    switchTab(initialTab);
}

// =============================================================================
// Trains View (Tab 1)
// =============================================================================

async function loadTrains() {
    const grid = document.getElementById('trains-grid');
    grid.innerHTML = '<div style="color: var(--text-muted); padding: 1rem;">Loading available trains...</div>';

    try {
        const sortSelect = document.getElementById('train-sort-select');
        const sort = sortSelect.value;
        const trains = await api.getTrains(sort);
        state.trains = trains;

        renderTrainsGrid(trains);
        populateTrainSelectDropdown(trains);
    } catch (err) {
        grid.innerHTML = `<div style="color: var(--danger); padding: 1rem;">Failed to load trains: ${err.message}</div>`;
    }
}

function renderTrainsGrid(trains) {
    const grid = document.getElementById('trains-grid');
    if (!trains || trains.length === 0) {
        grid.innerHTML = '<div style="color: var(--text-muted); padding: 1rem;">No trains available matching criteria.</div>';
        return;
    }

    grid.innerHTML = trains.map(t => {
        let badgeClass = 'badge-success';
        let badgeText = `${t.availableSeats} Seats Available`;
        if (t.availableSeats === 0) {
            badgeClass = 'badge-danger';
            badgeText = 'Full (Waiting List)';
        } else if (t.availableSeats <= 2) {
            badgeClass = 'badge-warning';
            badgeText = `${t.availableSeats} Left (Fast Filling)`;
        }

        return `
            <div class="train-card">
              <div>
                <div class="train-header">
                  <span class="train-number">${t.trainNo}</span>
                  <span class="badge ${badgeClass}">${badgeText}</span>
                </div>
                <h3 class="train-name">${escapeHtml(t.name)}</h3>
                <div class="train-route">
                  <span>${escapeHtml(t.source)}</span>
                  <span class="route-arrow">➔</span>
                  <span>${escapeHtml(t.destination)}</span>
                </div>
                <div class="train-meta">
                  <div class="meta-item">
                    <span class="meta-label">Departure</span>
                    <span class="meta-value">${escapeHtml(t.departure)}</span>
                  </div>
                  <div class="meta-item">
                    <span class="meta-label">Fare</span>
                    <span class="meta-value">Rs. ${parseFloat(t.fare).toFixed(2)}</span>
                  </div>
                </div>
              </div>
              <button class="btn btn-primary book-train-btn" data-train-no="${t.trainNo}" style="width: 100%;">
                ${t.availableSeats > 0 ? 'Book Ticket' : 'Join Waiting List'}
              </button>
            </div>
        `;
    }).join('');

    // Attach click events to "Book Ticket" buttons
    document.querySelectorAll('.book-train-btn').forEach(btn => {
        btn.addEventListener('click', () => {
            const trainNo = parseInt(btn.dataset.trainNo, 10);
            selectTrainForBooking(trainNo);
        });
    });
}

function selectTrainForBooking(trainNo) {
    switchTab('book');
    const select = document.getElementById('book-train-select');
    select.value = trainNo;
    handleTrainSelectionChange(trainNo);
}

function populateTrainSelectDropdown(trains) {
    const select = document.getElementById('book-train-select');
    select.innerHTML = '<option value="">-- Choose a Train --</option>' +
        trains.map(t => `<option value="${t.trainNo}">${t.trainNo} - ${escapeHtml(t.name)} (${t.availableSeats}/${t.totalSeats} seats)</option>`).join('');
}

// Live search with debounce
let searchDebounceTimer = null;
function initTrainSearch() {
    const searchInput = document.getElementById('train-search-input');
    const sortSelect = document.getElementById('train-sort-select');
    const refreshBtn = document.getElementById('refresh-trains-btn');

    searchInput.addEventListener('input', (e) => {
        clearTimeout(searchDebounceTimer);
        searchDebounceTimer = setTimeout(async () => {
            const query = e.target.value.trim();
            if (!query) {
                loadTrains();
                return;
            }

            try {
                let results = [];
                if (/^\d+$/.test(query)) {
                    results = await api.searchTrains({ number: parseInt(query, 10) });
                } else {
                    results = await api.searchTrains({ destination: query });
                }
                renderTrainsGrid(results);
            } catch {
                renderTrainsGrid([]);
            }
        }, 250);
    });

    sortSelect.addEventListener('change', () => loadTrains());
    refreshBtn.addEventListener('click', () => loadTrains());
}

// =============================================================================
// Book Ticket View (Tab 2) & 2D Seat Map Preview
// =============================================================================

async function handleTrainSelectionChange(trainNo) {
    const previewContainer = document.getElementById('seat-map-preview');
    const titleEl = document.getElementById('seat-map-title');
    const gridEl = document.getElementById('seat-grid');
    const submitBtn = document.getElementById('submit-booking-btn');

    if (!trainNo) {
        previewContainer.style.display = 'none';
        return;
    }

    try {
        const seatData = await api.getSeats(trainNo);
        previewContainer.style.display = 'block';
        titleEl.textContent = `2D Coach Seat Map: Train ${seatData.trainNo} (${seatData.available} of ${seatData.totalSeats} seats available)`;

        gridEl.innerHTML = seatData.seats.map((state, idx) => {
            const isBooked = state === 1;
            const cls = isBooked ? 'seat-booked' : 'seat-avail';
            const title = isBooked ? `Seat ${idx + 1} Booked` : `Seat ${idx + 1} Available`;
            return `<div class="seat-box ${cls}" title="${title}">${idx + 1}</div>`;
        }).join('');

        if (seatData.available === 0) {
            submitBtn.textContent = 'Join Waiting List (FIFO)';
            submitBtn.className = 'btn btn-secondary';
        } else {
            submitBtn.textContent = 'Confirm Booking';
            submitBtn.className = 'btn btn-primary';
        }
    } catch (err) {
        showToast(`Could not load seat map: ${err.message}`, 'danger');
    }
}

function initBookingForm() {
    const form = document.getElementById('booking-form');
    const trainSelect = document.getElementById('book-train-select');
    const dateInput = document.getElementById('book-travel-date');

    // Default travel date to tomorrow
    const tomorrow = new Date();
    tomorrow.setDate(tomorrow.getDate() + 1);
    dateInput.value = tomorrow.toISOString().split('T')[0];

    trainSelect.addEventListener('change', (e) => {
        handleTrainSelectionChange(parseInt(e.target.value, 10));
    });

    form.addEventListener('submit', async (e) => {
        e.preventDefault();
        const trainNo = parseInt(trainSelect.value, 10);
        const name = document.getElementById('book-passenger-name').value.trim();
        const age = parseInt(document.getElementById('book-passenger-age').value, 10);
        const gender = document.getElementById('book-passenger-gender').value;
        const dateVal = dateInput.value;

        if (!dateVal) {
            showToast('Please select a valid travel date.', 'warning');
            return;
        }

        const [year, month, day] = dateVal.split('-').map(Number);

        try {
            const bookingPayload = {
                trainNo,
                name,
                age,
                gender,
                travelDate: { day, month, year }
            };

            const result = await api.bookTicket(bookingPayload);
            const receiptContainer = document.getElementById('ticket-receipt-container');
            receiptContainer.style.display = 'block';

            if (result.status === 'CONFIRMED') {
                showToast(`Booking Confirmed! PNR: ${result.pnr}`, 'success');
                receiptContainer.innerHTML = `
                  <div class="ticket-receipt">
                    <div class="ticket-header">
                      <h3>INDIAN RAILWAYS E-TICKET CONFIRMATION</h3>
                      <div class="ticket-pnr-badge">PNR: ${result.pnr}</div>
                      <span class="badge badge-success">CONFIRMED</span>
                    </div>
                    <div class="ticket-details-grid">
                      <div><strong>Passenger:</strong> ${escapeHtml(result.name)}</div>
                      <div><strong>Train No:</strong> ${result.trainNo}</div>
                      <div><strong>Assigned Seat:</strong> Seat #${result.seatNo}</div>
                      <div><strong>Fare Paid:</strong> Rs. ${parseFloat(result.fare).toFixed(2)}</div>
                      <div><strong>Travel Date:</strong> ${day}/${month}/${year}</div>
                    </div>
                    <div style="display: flex; gap: 0.75rem; justify-content: center;" class="no-print">
                      <button class="btn btn-primary" onclick="window.print()">🖨️ Print Ticket</button>
                      <button class="btn btn-secondary" id="book-another-btn">Book Another</button>
                    </div>
                  </div>
                `;
            } else if (result.status === 'WAITING') {
                showToast(`Added to Waiting List (Position: ${result.position})`, 'warning');
                receiptContainer.innerHTML = `
                  <div class="ticket-receipt" style="border-color: var(--accent);">
                    <div class="ticket-header">
                      <h3>WAITING LIST NOTIFICATION (FIFO)</h3>
                      <div class="ticket-pnr-badge" style="color: var(--accent);">Wait ID: ${result.waitId}</div>
                      <span class="badge badge-warning">WAITING - POSITION #${result.position}</span>
                    </div>
                    <div class="ticket-details-grid">
                      <div><strong>Passenger:</strong> ${escapeHtml(result.name)}</div>
                      <div><strong>Train No:</strong> ${result.trainNo}</div>
                      <div><strong>Queue Position:</strong> #${result.position} in line</div>
                      <div><strong>Status:</strong> Will auto-promote on cancellation</div>
                    </div>
                    <div style="text-align: center;" class="no-print">
                      <button class="btn btn-secondary" id="book-another-btn">Back to Form</button>
                    </div>
                  </div>
                `;
            }

            form.reset();
            handleTrainSelectionChange(trainNo);

            document.getElementById('book-another-btn')?.addEventListener('click', () => {
                receiptContainer.style.display = 'none';
            });

        } catch (err) {
            showToast(`Booking Failed: ${err.message}`, 'danger');
        }
    });
}

// =============================================================================
// My Booking & Cancellation (Tab 3)
// =============================================================================

function initMyBooking() {
    const searchBtn = document.getElementById('search-pnr-btn');
    const pnrInput = document.getElementById('pnr-search-input');
    const resultContainer = document.getElementById('pnr-result-container');

    const searchAction = async () => {
        const pnr = parseInt(pnrInput.value.trim(), 10);
        if (!pnr) {
            showToast('Please enter a valid PNR number.', 'warning');
            return;
        }

        try {
            const passengers = await api.getPassengers({ pnr });
            if (!passengers || passengers.length === 0) {
                resultContainer.innerHTML = '<div style="color: var(--danger); text-align: center;">PNR not found in system.</div>';
                return;
            }

            const p = passengers[0];
            const isConfirmed = p.status === 'CONFIRMED';
            const badgeClass = isConfirmed ? 'badge-success' : 'badge-danger';

            resultContainer.innerHTML = `
              <div class="ticket-receipt">
                <div class="ticket-header">
                  <h3>BOOKING STATUS</h3>
                  <div class="ticket-pnr-badge">PNR: ${p.pnr}</div>
                  <span class="badge ${badgeClass}">${p.status}</span>
                </div>
                <div class="ticket-details-grid">
                  <div><strong>Passenger:</strong> ${escapeHtml(p.name)}</div>
                  <div><strong>Age / Gender:</strong> ${p.age} / ${p.gender}</div>
                  <div><strong>Train Number:</strong> ${p.trainNo}</div>
                  <div><strong>Seat Number:</strong> ${isConfirmed ? '#' + p.seatNo : 'N/A (Cancelled)'}</div>
                  <div><strong>Date:</strong> ${p.travelDate.day}/${p.travelDate.month}/${p.travelDate.year}</div>
                </div>
                ${isConfirmed ? `
                  <div style="text-align: center;" class="no-print">
                    <button class="btn btn-danger" id="init-cancel-btn" data-pnr="${p.pnr}">Cancel Ticket</button>
                  </div>
                ` : ''}
              </div>
            `;

            document.getElementById('init-cancel-btn')?.addEventListener('click', () => {
                state.pendingCancelPnr = p.pnr;
                const dialog = document.getElementById('cancel-dialog');
                document.getElementById('cancel-dialog-text').textContent = 
                    `Are you sure you want to cancel PNR ${p.pnr} for ${p.name}? This will free the seat.`;
                dialog.showModal();
            });

        } catch (err) {
            resultContainer.innerHTML = `<div style="color: var(--danger); text-align: center;">${err.message}</div>`;
        }
    };

    searchBtn.addEventListener('click', searchAction);
    pnrInput.addEventListener('keydown', (e) => { if (e.key === 'Enter') searchAction(); });

    // Cancellation Dialog Listeners
    const dialog = document.getElementById('cancel-dialog');
    document.getElementById('cancel-dialog-no').addEventListener('click', () => dialog.close());
    document.getElementById('cancel-dialog-yes').addEventListener('click', async () => {
        dialog.close();
        if (!state.pendingCancelPnr) return;

        try {
            const cancelRes = await api.cancelTicket(state.pendingCancelPnr);
            showToast(`Ticket PNR ${state.pendingCancelPnr} cancelled successfully.`, 'success');

            if (cancelRes.promoted) {
                showToast(`Waiting List Promotion: ${cancelRes.promoted.name} auto-confirmed into Seat ${cancelRes.promoted.seatNo}!`, 'warning');
            }

            searchAction(); // Refresh current card
        } catch (err) {
            showToast(`Cancellation error: ${err.message}`, 'danger');
        }
    });
}

// =============================================================================
// Waiting List View (Tab 4)
// =============================================================================

async function loadWaitingList() {
    const container = document.getElementById('waiting-list-container');
    container.innerHTML = '<div style="color: var(--text-muted);">Loading waiting queues...</div>';

    try {
        const groups = await api.getWaiting();
        if (!groups || groups.length === 0) {
            container.innerHTML = '<div style="color: var(--text-muted); padding: 1rem;">All waiting queues are currently empty.</div>';
            return;
        }

        container.innerHTML = groups.map(g => `
          <div class="train-card" style="margin-bottom: 1.5rem;">
            <div class="train-header">
              <span class="train-number">${g.trainNo}</span>
              <span class="badge badge-warning">${g.count} In Queue</span>
            </div>
            <h3 class="train-name">${escapeHtml(g.trainName)}</h3>
            <div style="margin-top: 1rem;">
              <ol style="padding-left: 1.5rem; display: flex; flex-direction: column; gap: 0.5rem;">
                ${g.entries.map((w, i) => `
                  <li>
                    <strong>${i === 0 ? 'Next in line' : '#' + (i + 1)}:</strong> 
                    ${escapeHtml(w.name)} (Age: ${w.age}, Gender: ${w.gender}) 
                    <span style="color: var(--text-muted); font-size: 0.8rem;">[WaitID: ${w.waitId}]</span>
                  </li>
                `).join('')}
              </ol>
            </div>
          </div>
        `).join('');
    } catch (err) {
        container.innerHTML = `<div style="color: var(--danger);">${err.message}</div>`;
    }
}

function initWaitingList() {
    document.getElementById('refresh-waiting-btn').addEventListener('click', loadWaitingList);
}

// =============================================================================
// Passengers Manifest (Tab 5)
// =============================================================================

async function loadPassengers(filterTrain = null) {
    const tbody = document.getElementById('passengers-table-body');
    tbody.innerHTML = '<tr><td colspan="7" style="color: var(--text-muted);">Loading passenger records...</td></tr>';

    try {
        const params = filterTrain ? { trainNo: filterTrain } : {};
        const passengers = await api.getPassengers(params);

        if (!passengers || passengers.length === 0) {
            tbody.innerHTML = '<tr><td colspan="7" style="text-align: center; color: var(--text-muted);">No passenger records found.</td></tr>';
            return;
        }

        tbody.innerHTML = passengers.map(p => `
          <tr>
            <td><strong>${p.pnr}</strong></td>
            <td>${escapeHtml(p.name)}</td>
            <td>${p.age} / ${p.gender}</td>
            <td>${p.trainNo}</td>
            <td>${p.seatNo}</td>
            <td>${p.travelDate.day}/${p.travelDate.month}/${p.travelDate.year}</td>
            <td><span class="badge ${p.status === 'CONFIRMED' ? 'badge-success' : 'badge-danger'}">${p.status}</span></td>
          </tr>
        `).join('');
    } catch (err) {
        tbody.innerHTML = `<tr><td colspan="7" style="color: var(--danger);">${err.message}</td></tr>`;
    }
}

function initPassengersView() {
    const filterInput = document.getElementById('passenger-filter-train');
    const filterBtn = document.getElementById('filter-passengers-btn');
    const resetBtn = document.getElementById('reset-passengers-btn');

    filterBtn.addEventListener('click', () => {
        const val = parseInt(filterInput.value.trim(), 10);
        if (val) loadPassengers(val);
    });

    resetBtn.addEventListener('click', () => {
        filterInput.value = '';
        loadPassengers();
    });
}

// =============================================================================
// Admin View: Add Train (Tab 6)
// =============================================================================

function initAdminView() {
    const form = document.getElementById('add-train-form');
    form.addEventListener('submit', async (e) => {
        e.preventDefault();

        const trainPayload = {
            trainNo: parseInt(document.getElementById('admin-train-no').value, 10),
            name: document.getElementById('admin-train-name').value.trim(),
            source: document.getElementById('admin-train-source').value.trim(),
            destination: document.getElementById('admin-train-destination').value.trim(),
            departure: document.getElementById('admin-train-departure').value.trim(),
            totalSeats: parseInt(document.getElementById('admin-train-seats').value, 10),
            fare: parseFloat(document.getElementById('admin-train-fare').value)
        };

        try {
            await api.addTrain(trainPayload);
            showToast(`Train ${trainPayload.trainNo} added successfully!`, 'success');
            form.reset();
            switchTab('trains');
        } catch (err) {
            showToast(`Error adding train: ${err.message}`, 'danger');
        }
    });
}

// =============================================================================
// XSS Safety Utility
// =============================================================================

function escapeHtml(str) {
    if (!str) return '';
    return String(str)
        .replace(/&/g, '&amp;')
        .replace(/</g, '&lt;')
        .replace(/>/g, '&gt;')
        .replace(/"/g, '&quot;')
        .replace(/'/g, '&#39;');
}

// =============================================================================
// Application Initialization
// =============================================================================

document.addEventListener('DOMContentLoaded', () => {
    initTheme();
    initTabs();
    initTrainSearch();
    initBookingForm();
    initMyBooking();
    initWaitingList();
    initPassengersView();
    initAdminView();
});
