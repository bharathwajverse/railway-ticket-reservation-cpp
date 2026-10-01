# =============================================================================
# Stage 1: Build Stage
# =============================================================================
FROM gcc:13-bookworm AS builder

WORKDIR /app

# Copy backend source files
COPY backend/ ./backend/

# Compile railway_api binary (Linux uses standard pthread / socket networking)
RUN mkdir -p /app/backend/database/data && \
    g++ -std=c++17 -O2 -Wall -Wextra \
    -Ibackend/include \
    -Ibackend/dsa \
    -Ibackend/database \
    -Ibackend/api \
    backend/api/server.cpp \
    backend/api/routes.cpp \
    backend/dsa/booking_ops.cpp \
    backend/dsa/seat_map.cpp \
    backend/dsa/train_ops.cpp \
    backend/dsa/validation.cpp \
    backend/dsa/waiting_queue.cpp \
    backend/database/db_connection.cpp \
    backend/database/passenger_repo.cpp \
    backend/database/train_repo.cpp \
    backend/database/waiting_repo.cpp \
    -lpthread \
    -o /app/railway_api

# =============================================================================
# Stage 2: Runtime Stage
# =============================================================================
FROM debian:bookworm-slim AS runner

WORKDIR /app

# Install standard C++ runtime library
RUN apt-get update && apt-get install -y --no-install-recommends \
    libstdc++6 \
    ca-certificates \
    && rm -rf /var/lib/apt/lists/*

# Copy built binary from builder
COPY --from=builder /app/railway_api /app/railway_api

# Copy static frontend directory
COPY frontend/ /app/frontend/

# Create database storage directory
RUN mkdir -p /app/backend/database/data

# Expose REST API and Web Dashboard port
EXPOSE 8080

# Environment variables
ENV API_PORT=8080
ENV DB_NAME=datadb

# Start the server
CMD ["/app/railway_api"]
