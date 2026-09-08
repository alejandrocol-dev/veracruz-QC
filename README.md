# Sistema de Gestión - S.A. Veracruz

Sistema por consola en C para el control de calidad y trazabilidad de lotes de jugo concentrado y aceite esencial. Proyecto desarrollado como Trabajo Final Integrador (TFI).

## Características

- **Autenticación:** inicio de sesión y registro de usuarios con validaciones de credenciales (almacenado en `Usuarios.dat`).
- **Gestión de lotes (CRUD):** alta, baja lógica, baja física (purga), modificación y consulta de análisis de calidad (pH, acidez, inocuidad y estado del lote).
- **Estructuras en memoria:** ordenamiento y visualización de registros mediante listas enlazadas dinámicas en RAM.
- **Interfaz por consola:** menús navegables con teclado y ajuste dinámico al tamaño de ventana mediante Windows API.

## Compilación y ejecución

Requiere un compilador de C (como GCC / MinGW) en Windows:

```bash
gcc VeracruzQuality.c -o VeracruzQuality.exe
.\VeracruzQuality.exe
```

## Controles

- `↑` / `↓`: navegar por las opciones
- `Enter`: seleccionar
- `ESC`: volver / cancelar

## Archivos

- `VeracruzQuality.c`: código fuente principal.
- `Usuarios.dat`: base de datos binaria de usuarios.
- `Analisis.dat`: base de datos binaria de análisis de lotes.
