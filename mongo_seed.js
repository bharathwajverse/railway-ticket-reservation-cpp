// =============================================================================
// MongoDB Shell Initialization Script (mongosh compatible)
// Database: railway_reservation
// Usage: mongosh railway_reservation mongo_seed.js
// =============================================================================

use('railway_reservation');

// 1. Reset Collections
db.trains.drop();
db.passengers.drop();
db.waiting_list.drop();

// 2. Create Unique Indexes
db.trains.createIndex({ "train_no": 1 }, { unique: true });
db.passengers.createIndex({ "pnr": 1 }, { unique: true });
db.waiting_list.createIndex({ "wait_id": 1 }, { unique: true });

// 3. Insert Trains Collection
db.trains.insertMany([
  { "train_no": 10101, "name": "Rajdhani Express", "source": "Delhi", "destination": "Mumbai", "departure": "06:00 AM", "total_seats": 4, "available_seats": 3, "fare": 1500.00 },
  { "train_no": 10202, "name": "Vande Bharat", "source": "Chennai", "destination": "Bangalore", "departure": "05:50 AM", "total_seats": 5, "available_seats": 5, "fare": 950.00 },
  { "train_no": 10303, "name": "Shatabdi Express", "source": "Kolkata", "destination": "Patna", "departure": "02:15 PM", "total_seats": 3, "available_seats": 3, "fare": 750.00 },
  { "train_no": 10404, "name": "Tejas Express", "source": "Ahmedabad", "destination": "Mumbai", "departure": "06:40 AM", "total_seats": 4, "available_seats": 4, "fare": 1100.00 }
]);

// 4. Insert Passengers Collection
db.passengers.insertMany([
  { "pnr": 1001, "name": "John Doe", "age": 25, "gender": "M", "train_no": 10101, "seat_no": 1, "travel_date": { "day": 15, "month": 11, "year": 2026 }, "status": "CONFIRMED", "concession": "GENERAL", "fare_paid": 1500.00 }
]);

print('>>> MongoDB railway_reservation collections loaded successfully!');
