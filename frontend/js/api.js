// =============================================================================
// api.js - REST API Client for Railway Ticket Reservation System
// Clean fetch wrapper handling uniform { success, data, error } backend shape
// =============================================================================

const API_BASE = '/api';

/**
 * Common request helper handling JSON parsing and API contract
 * @param {string} endpoint - API route (e.g. '/trains')
 * @param {object} options - Fetch options (method, headers, body)
 * @returns {Promise<any>} data field from response
 */
async function request(endpoint, options = {}) {
    const config = {
        headers: {
            'Content-Type': 'application/json',
            ...(options.headers || {})
        },
        ...options
    };

    try {
        const response = await fetch(`${API_BASE}${endpoint}`, config);
        const result = await response.json();

        if (!response.ok || !result.success) {
            const errorMsg = result.error || `HTTP error ${response.status}`;
            throw new Error(errorMsg);
        }

        return result.data;
    } catch (err) {
        console.error(`API Error on [${options.method || 'GET'} ${endpoint}]:`, err.message);
        throw err;
    }
}

// =============================================================================
// API Service Methods
// =============================================================================

export const api = {
    /**
     * Get all trains, optionally sorted
     * @param {'fare' | 'name' | ''} sort 
     */
    async getTrains(sort = '') {
        const query = sort ? `?sort=${encodeURIComponent(sort)}` : '';
        return request(`/trains${query}`);
    },

    /**
     * Search trains by number or destination
     * @param {{ number?: number, destination?: string }} params 
     */
    async searchTrains(params) {
        if (params.number) {
            const data = await request(`/trains/search?number=${encodeURIComponent(params.number)}`);
            return [data]; // Return array for uniform handling
        } else if (params.destination) {
            return request(`/trains/search?destination=${encodeURIComponent(params.destination)}`);
        }
        return [];
    },

    /**
     * Add a new train (Admin)
     */
    async addTrain(train) {
        return request('/trains', {
            method: 'POST',
            body: JSON.stringify(train)
        });
    },

    /**
     * Get 2D seat availability map and stats for a train
     * @param {number} trainNo 
     */
    async getSeats(trainNo) {
        return request(`/trains/${encodeURIComponent(trainNo)}/seats`);
    },

    /**
     * Book a ticket or queue onto waiting list
     */
    async bookTicket(booking) {
        return request('/bookings', {
            method: 'POST',
            body: JSON.stringify(booking)
        });
    },

    /**
     * Cancel confirmed ticket by PNR
     * @param {number} pnr 
     */
    async cancelTicket(pnr) {
        return request(`/bookings/${encodeURIComponent(pnr)}`, {
            method: 'DELETE'
        });
    },

    /**
     * Get passengers filtered by PNR, train number, or all
     */
    async getPassengers(params = {}) {
        let query = '';
        if (params.pnr) query = `?pnr=${encodeURIComponent(params.pnr)}`;
        else if (params.trainNo) query = `?trainNo=${encodeURIComponent(params.trainNo)}`;
        
        const res = await request(`/passengers${query}`);
        // If searched by PNR, single object is returned; wrap in array for uniform rendering
        return Array.isArray(res) ? res : [res];
    },

    /**
     * Get all waiting lists grouped by train
     */
    async getWaiting() {
        return request('/waiting');
    }
};
