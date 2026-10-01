# Security Policy

## Supported Versions

| Version | Supported          |
| ------- | ------------------ |
| 2.0.x   | :white_check_mark: |
| 1.0.x   | :x:                |

## Reporting a Vulnerability

If you discover a security vulnerability within this project, please report it by opening a
[confidential issue](https://github.com/bharathwajverse/railway-ticket-reservation-cpp/issues/new)
or emailing the repository owner directly.

## Security Considerations

- The Administrator Portal is protected by a PIN-based authentication mechanism.
- The embedded HTTP server (port 8080) is designed for **local development and demonstration only**.
- MongoDB configuration files (`database/config/mongodb.conf`) containing credentials are excluded from version control via `.gitignore`.
- Generated e-ticket files (`ticket_*.txt`) are excluded from version control.
- MongoDB Atlas operations use secure connection strings (`mongodb+srv://`) with TLS enabled by default.

## Disclaimer

This project is an academic mini-project developed as part of a B.Tech CSE (AI/ML) Data Structures course.
It is **not intended for production deployment**. The embedded web server does not include TLS/SSL encryption
or production-grade authentication.
