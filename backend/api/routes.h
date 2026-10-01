// =============================================================================
// routes.h - REST API Route Handlers
// Declares route registration function connecting endpoints to DSA operations
// =============================================================================
#ifndef ROUTES_H
#define ROUTES_H

#include "third_party/httplib.h"
#include "structures.h"

// Register all REST endpoints on the server instance
void registerRoutes(httplib::Server& svr, RailwaySystem& sys);

#endif // ROUTES_H
