// =============================================================================
// routes.cpp - REST API Route Implementations
// Connects HTTP endpoints to pure DSA algorithms and MongoDB data repository
// Library Zone: auto, try/catch, httplib, and nlohmann::json permitted here
// =============================================================================
#include "routes.h"
#include "train_ops.h"
#include "booking_ops.h"
#include "seat_map.h"
#include "waiting_queue.h"
#include "validation.h"
#include "train_repo.h"
#include "passenger_repo.h"
#include "waiting_repo.h"
#include "third_party/json.hpp"
#include <iostream>

using json = nlohmann::json;
using namespace std;

// Helper to construct uniform JSON API responses: { success, data, error }
static string makeResponse(bool success, const json& data, const string& error = "") {
    json res;
    res["success"] = success;
    res["data"] = data;
    res["error"] = error;
    return res.dump(2);
}

// Helper to serialize Train struct to json
static json trainToJson(const Train& t) {
    return json{
        {"trainNo", t.trainNo},
        {"name", t.name},
        {"source", t.source},
        {"destination", t.destination},
        {"departure", t.departure},
        {"totalSeats", t.totalSeats},
        {"availableSeats", t.availableSeats},
        {"fare", t.fare}
    };
}

// Helper to serialize Passenger struct to json
static json passengerToJson(const Passenger& p) {
    return json{
        {"pnr", p.pnr},
        {"name", p.name},
        {"age", p.age},
        {"gender", string(1, p.gender)},
        {"trainNo", p.trainNo},
        {"seatNo", p.seatNo},
        {"travelDate", {
            {"day", p.travelDate.day},
            {"month", p.travelDate.month},
            {"year", p.travelDate.year}
        }},
        {"status", p.status}
    };
}

// Helper to serialize WaitingEntry struct to json
static json waitingToJson(const WaitingEntry& w) {
    return json{
        {"waitId", w.waitId},
        {"name", w.name},
        {"age", w.age},
        {"gender", string(1, w.gender)},
        {"trainNo", w.trainNo},
        {"travelDate", {
            {"day", w.travelDate.day},
            {"month", w.travelDate.month},
            {"year", w.travelDate.year}
        }}
    };
}

void registerRoutes(httplib::Server& svr, RailwaySystem& sys) {

    // CORS preflight support
    svr.Options(".*", [](const httplib::Request&, httplib::Response& res) {
        res.set_header("Access-Control-Allow-Origin", "*");
        res.set_header("Access-Control-Allow-Methods", "GET, POST, DELETE, OPTIONS");
        res.set_header("Access-Control-Allow-Headers", "Content-Type");
        res.status = 200;
    });

    // -------------------------------------------------------------------------
    // 1. GET /api/trains?sort=fare|name
    // -------------------------------------------------------------------------
    svr.Get("/api/trains", [&sys](const httplib::Request& req, httplib::Response& res) {
        res.set_header("Access-Control-Allow-Origin", "*");
        res.set_header("Content-Type", "application/json");

        vector<Train> list = sys.trains;
        if (req.has_param("sort")) {
            string s = req.get_param_value("sort");
            if (s == "fare") {
                list = sortTrainsByFare(sys.trains);
            } else if (s == "name") {
                list = sortTrainsByName(sys.trains);
            }
        }

        json arr = json::array();
        for (const auto& t : list) {
            arr.push_back(trainToJson(t));
        }
        res.set_content(makeResponse(true, arr), "application/json");
    });

    // -------------------------------------------------------------------------
    // 2. GET /api/trains/search?number=10101 OR ?destination=Mumbai
    // -------------------------------------------------------------------------
    svr.Get("/api/trains/search", [&sys](const httplib::Request& req, httplib::Response& res) {
        res.set_header("Access-Control-Allow-Origin", "*");
        res.set_header("Content-Type", "application/json");

        if (req.has_param("number")) {
            try {
                int num = stoi(req.get_param_value("number"));
                int idx = binarySearchTrain(sys.trains, num);
                if (idx >= 0) {
                    res.set_content(makeResponse(true, trainToJson(sys.trains[idx])), "application/json");
                } else {
                    res.status = 404;
                    res.set_content(makeResponse(false, nullptr, "Train number not found."), "application/json");
                }
            } catch (...) {
                res.status = 400;
                res.set_content(makeResponse(false, nullptr, "Invalid train number parameter."), "application/json");
            }
            return;
        }

        if (req.has_param("destination")) {
            string dest = req.get_param_value("destination");
            vector<int> matches = linearSearchByDestination(sys.trains, dest);
            json arr = json::array();
            for (int idx : matches) {
                arr.push_back(trainToJson(sys.trains[idx]));
            }
            res.set_content(makeResponse(true, arr), "application/json");
            return;
        }

        res.status = 400;
        res.set_content(makeResponse(false, nullptr, "Missing query parameter 'number' or 'destination'."), "application/json");
    });

    // -------------------------------------------------------------------------
    // 3. POST /api/trains
    // -------------------------------------------------------------------------
    svr.Post("/api/trains", [&sys](const httplib::Request& req, httplib::Response& res) {
        res.set_header("Access-Control-Allow-Origin", "*");
        res.set_header("Content-Type", "application/json");

        try {
            json body = json::parse(req.body);
            Train t;
            t.trainNo = body.at("trainNo").get<int>();
            t.name = body.at("name").get<string>();
            t.source = body.at("source").get<string>();
            t.destination = body.at("destination").get<string>();
            t.departure = body.at("departure").get<string>();
            t.totalSeats = body.at("totalSeats").get<int>();
            t.availableSeats = t.totalSeats;
            t.fare = body.at("fare").get<float>();

            if (t.trainNo < 10000 || t.trainNo > 99999) {
                res.status = 400;
                res.set_content(makeResponse(false, nullptr, "Train number must be 5 digits (10000-99999)."), "application/json");
                return;
            }
            if (t.totalSeats < 1 || t.totalSeats > MAX_SEATS) {
                res.status = 400;
                res.set_content(makeResponse(false, nullptr, "Seats must be between 1 and 60."), "application/json");
                return;
            }
            if (binarySearchTrain(sys.trains, t.trainNo) != -1) {
                res.status = 409;
                res.set_content(makeResponse(false, nullptr, "Train number already exists."), "application/json");
                return;
            }

            int idx = insertTrainSorted(sys.trains, t);
            if (idx >= 0) {
                dbInsertTrain(t);
                res.status = 201;
                res.set_content(makeResponse(true, trainToJson(t)), "application/json");
            } else {
                res.status = 500;
                res.set_content(makeResponse(false, nullptr, "Failed to insert train."), "application/json");
            }
        } catch (const exception& e) {
            res.status = 400;
            res.set_content(makeResponse(false, nullptr, string("Invalid request body: ") + e.what()), "application/json");
        }
    });

    // -------------------------------------------------------------------------
    // 4. GET /api/trains/:trainNo/seats
    // -------------------------------------------------------------------------
    svr.Get(R"(/api/trains/(\d+)/seats)", [&sys](const httplib::Request& req, httplib::Response& res) {
        res.set_header("Access-Control-Allow-Origin", "*");
        res.set_header("Content-Type", "application/json");

        int trainNo = stoi(req.matches[1]);
        int idx = binarySearchTrain(sys.trains, trainNo);
        if (idx == -1) {
            res.status = 404;
            res.set_content(makeResponse(false, nullptr, "Train not found."), "application/json");
            return;
        }

        const Train& t = sys.trains[idx];
        json seats = json::array();
        for (int j = 0; j < t.totalSeats; j++) {
            seats.push_back(sys.seatMap[idx][j]);
        }
        int booked = countOccupiedSeats(sys.seatMap, idx, t.totalSeats);

        json data = {
            {"trainNo", t.trainNo},
            {"trainName", t.name},
            {"totalSeats", t.totalSeats},
            {"booked", booked},
            {"available", t.availableSeats},
            {"seats", seats}
        };
        res.set_content(makeResponse(true, data), "application/json");
    });

    // -------------------------------------------------------------------------
    // 5. POST /api/bookings
    // -------------------------------------------------------------------------
    svr.Post("/api/bookings", [&sys](const httplib::Request& req, httplib::Response& res) {
        res.set_header("Access-Control-Allow-Origin", "*");
        res.set_header("Content-Type", "application/json");

        try {
            json body = json::parse(req.body);
            int trainNo = body.at("trainNo").get<int>();
            string name = body.at("name").get<string>();
            int age = body.at("age").get<int>();
            string genderStr = body.at("gender").get<string>();
            char gender = genderStr.empty() ? 'O' : toupper(genderStr[0]);

            Date travelDate;
            travelDate.day = body.at("travelDate").at("day").get<int>();
            travelDate.month = body.at("travelDate").at("month").get<int>();
            travelDate.year = body.at("travelDate").at("year").get<int>();

            if (!isValidName(name)) {
                res.status = 400;
                res.set_content(makeResponse(false, nullptr, "Invalid passenger name (letters and spaces only)."), "application/json");
                return;
            }
            if (!isValidAge(age)) {
                res.status = 400;
                res.set_content(makeResponse(false, nullptr, "Age must be between 1 and 120."), "application/json");
                return;
            }
            if (!isValidGender(gender)) {
                res.status = 400;
                res.set_content(makeResponse(false, nullptr, "Gender must be M, F, or O."), "application/json");
                return;
            }
            if (!isValidDate(travelDate.day, travelDate.month, travelDate.year)) {
                res.status = 400;
                res.set_content(makeResponse(false, nullptr, "Invalid travel date."), "application/json");
                return;
            }

            BookingResult bRes = bookTicket(sys, trainNo, name, age, gender, travelDate);

            if (!bRes.success) {
                res.status = 400;
                res.set_content(makeResponse(false, nullptr, bRes.errorMsg), "application/json");
                return;
            }

            if (bRes.status == "CONFIRMED") {
                dbInsertPassenger(bRes.passenger);
                int tIdx = binarySearchTrain(sys.trains, trainNo);
                if (tIdx >= 0) {
                    dbUpdateTrainSeats(trainNo, sys.trains[tIdx].availableSeats);
                }

                json data = {
                    {"status", "CONFIRMED"},
                    {"pnr", bRes.passenger.pnr},
                    {"seatNo", bRes.passenger.seatNo},
                    {"trainNo", trainNo},
                    {"name", bRes.passenger.name},
                    {"fare", sys.trains[tIdx].fare}
                };
                res.set_content(makeResponse(true, data), "application/json");
            } else if (bRes.status == "WAITING") {
                dbInsertWaiting(bRes.waitEntry);

                json data = {
                    {"status", "WAITING"},
                    {"waitId", bRes.waitEntry.waitId},
                    {"position", bRes.waitPosition},
                    {"trainNo", trainNo},
                    {"name", bRes.waitEntry.name}
                };
                res.set_content(makeResponse(true, data), "application/json");
            }
        } catch (const exception& e) {
            res.status = 400;
            res.set_content(makeResponse(false, nullptr, string("Invalid booking request: ") + e.what()), "application/json");
        }
    });

    // -------------------------------------------------------------------------
    // 6. DELETE /api/bookings/:pnr
    // -------------------------------------------------------------------------
    svr.Delete(R"(/api/bookings/(\d+))", [&sys](const httplib::Request& req, httplib::Response& res) {
        res.set_header("Access-Control-Allow-Origin", "*");
        res.set_header("Content-Type", "application/json");

        int pnr = stoi(req.matches[1]);
        CancelResult cRes = cancelTicket(sys, pnr);

        if (!cRes.success) {
            res.status = 404;
            res.set_content(makeResponse(false, nullptr, cRes.errorMsg), "application/json");
            return;
        }

        dbUpdatePassengerStatus(pnr, "CANCELLED");
        int trainNo = cRes.cancelled.trainNo;
        int tIdx = binarySearchTrain(sys.trains, trainNo);
        if (tIdx >= 0) {
            dbUpdateTrainSeats(trainNo, sys.trains[tIdx].availableSeats);
        }

        json data;
        data["cancelled"] = true;
        data["pnr"] = pnr;

        if (cRes.promoted) {
            dbInsertPassenger(cRes.promotedPassenger);
            dbDeleteWaiting(cRes.promotedWaitId);

            data["promoted"] = {
                {"pnr", cRes.promotedPassenger.pnr},
                {"name", cRes.promotedPassenger.name},
                {"seatNo", cRes.promotedPassenger.seatNo}
            };
        } else {
            data["promoted"] = nullptr;
        }

        res.set_content(makeResponse(true, data), "application/json");
    });

    // -------------------------------------------------------------------------
    // 7. GET /api/passengers?pnr=1001 OR ?trainNo=10101
    // -------------------------------------------------------------------------
    svr.Get("/api/passengers", [&sys](const httplib::Request& req, httplib::Response& res) {
        res.set_header("Access-Control-Allow-Origin", "*");
        res.set_header("Content-Type", "application/json");

        if (req.has_param("pnr")) {
            try {
                int pnr = stoi(req.get_param_value("pnr"));
                vector<Passenger> list = getPassengerByPNR(sys.passengers, pnr);
                if (!list.empty()) {
                    res.set_content(makeResponse(true, passengerToJson(list[0])), "application/json");
                } else {
                    res.status = 404;
                    res.set_content(makeResponse(false, nullptr, "PNR not found."), "application/json");
                }
            } catch (...) {
                res.status = 400;
                res.set_content(makeResponse(false, nullptr, "Invalid PNR parameter."), "application/json");
            }
            return;
        }

        if (req.has_param("trainNo")) {
            try {
                int tn = stoi(req.get_param_value("trainNo"));
                vector<Passenger> list = getPassengersByTrain(sys.passengers, tn);
                json arr = json::array();
                for (const auto& p : list) {
                    arr.push_back(passengerToJson(p));
                }
                res.set_content(makeResponse(true, arr), "application/json");
            } catch (...) {
                res.status = 400;
                res.set_content(makeResponse(false, nullptr, "Invalid trainNo parameter."), "application/json");
            }
            return;
        }

        json arr = json::array();
        for (const auto& p : sys.passengers) {
            arr.push_back(passengerToJson(p));
        }
        res.set_content(makeResponse(true, arr), "application/json");
    });

    // -------------------------------------------------------------------------
    // 8. GET /api/waiting
    // -------------------------------------------------------------------------
    svr.Get("/api/waiting", [&sys](const httplib::Request& req, httplib::Response& res) {
        (void)req;
        res.set_header("Access-Control-Allow-Origin", "*");
        res.set_header("Content-Type", "application/json");

        auto all = getAllWaitingLists(sys.waitingLists);
        json groups = json::array();

        for (const auto& pair : all) {
            int trainNo = pair.first;
            string trainName = "Train " + to_string(trainNo);
            int idx = binarySearchTrain(sys.trains, trainNo);
            if (idx >= 0) {
                trainName = sys.trains[idx].name;
            }

            json waitingListJson = json::array();
            for (const auto& w : pair.second) {
                waitingListJson.push_back(waitingToJson(w));
            }

            json group = {
                {"trainNo", trainNo},
                {"trainName", trainName},
                {"count", (int)pair.second.size()},
                {"entries", waitingListJson}
            };
            groups.push_back(group);
        }

        res.set_content(makeResponse(true, groups), "application/json");
    });
}
