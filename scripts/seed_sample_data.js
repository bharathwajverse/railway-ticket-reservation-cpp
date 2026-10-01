// =============================================================================
// seed_sample_data.js - MongoDB Shell Seed Script
// Usage: mongosh "<MONGO_URI>" seed_sample_data.js
// =============================================================================

const dbName = 'datadb';
const targetDb = db.getSiblingDB(dbName);

print(`[MongoDB] Initializing database: ${dbName}`);

// Create unique indexes
targetDb.trains.createIndex({ trainNo: 1 }, { unique: true });
targetDb.passengers.createIndex({ pnr: 1 }, { unique: true });
targetDb.waiting_list.createIndex({ waitId: 1 }, { unique: true });

print(`[MongoDB] Unique indexes verified for collections: trains, passengers, waiting_list.`);

// Sample train documents
const sampleTrains = [
  {
    trainNo: 10101,
    name: "Rajdhani Express",
    source: "Delhi",
    destination: "Mumbai",
    departure: "06:00 AM",
    totalSeats: 4,
    availableSeats: 4,
    fare: 1500.00
  },
  {
    trainNo: 10202,
    name: "Vande Bharat",
    source: "Chennai",
    destination: "Bangalore",
    departure: "05:50 AM",
    totalSeats: 5,
    availableSeats: 5,
    fare: 950.00
  },
  {
    trainNo: 10303,
    name: "Shatabdi Express",
    source: "Kolkata",
    destination: "Patna",
    departure: "02:15 PM",
    totalSeats: 3,
    availableSeats: 3,
    fare: 750.00
  },
  {
    trainNo: 10404,
    name: "Tejas Express",
    source: "Ahmedabad",
    destination: "Mumbai",
    departure: "06:40 AM",
    totalSeats: 4,
    availableSeats: 4,
    fare: 1100.00
  }
];

sampleTrains.forEach(train => {
  targetDb.trains.updateOne(
    { trainNo: train.trainNo },
    { $set: train },
    { upsert: true }
  );
  print(`  -> Upserted train ${train.trainNo}: ${train.name}`);
});

print(`[MongoDB] Seeding completed successfully.`);
