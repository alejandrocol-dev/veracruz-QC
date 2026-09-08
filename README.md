# VeracruzQuality

Citrus quality control and batch tracking in C. Final Algorithms project with S.A. Veracruz.

## Features

- **Authentication:** user registration and login with credential checks (`Usuarios.dat`).
- **Batch Management (CRUD):** create, soft delete, hard delete (purge), update, and query quality tests (pH, acidity, food safety, and batch status).
- **In-Memory Structures:** sorting and risk management using dynamic linked lists in RAM.
- **Terminal UI:** keyboard-driven menus and dynamic console resizing via Windows API.

## Build & Run

Requires a C compiler (GCC / MinGW) on Windows:

```bash
gcc VeracruzQuality.c -o VeracruzQuality.exe
.\VeracruzQuality.exe
```

## Controls

- `↑` / `↓`: navigate options
- `Enter`: select
- `ESC`: back / exit

## Files

- `VeracruzQuality.c`: main source code.
- `Usuarios.dat`: binary storage for users.
- `Analisis.dat`: binary storage for batch quality records.
