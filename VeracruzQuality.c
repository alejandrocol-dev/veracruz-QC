/**
 * =====================================================================================
 * SISTEMA DE GESTION Y CONTROL DE CALIDAD - S.A. VERACRUZ
 * =====================================================================================
 * PROPOSITO: Gestionar el control de calidad de citricos
 * =====================================================================================
 */

/**
 * SECCIÓN 1: BIBLIOTECAS (LIBRARIES)
 */
#include <conio.h>
#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>

/**
 * SECCIÓN 2: CONSTANTES Y CONFIGURACIÓN (CONSTANTS)
 */
#define FILE_USUARIOS "Usuarios.dat" // Archivo binario de usuarios
#define FILE_ANALISIS "Analisis.dat" // Archivo binario de registros de calidad
#define TEMP_ANALISIS "TempAnalisis.dat" // Archivo temporal para bajas físicas

// Códigos de teclas ASCII especiales
#define TECLA_ARRIBA 72
#define TECLA_ABAJO 80
#define TECLA_ENTER 13
#define TECLA_ESC 27
#define TECLA_BACKSPACE 8

// Códigos de Colores de Consola
#define COLOR_FONDO 0             // Negro
#define COLOR_TEXTO_BLANCO 15     // Blanco Fuerte
#define COLOR_TEXTO_PRINCIPAL 10  // Verde (Color corporativo)
#define COLOR_TEXTO_SECUNDARIO 14 // Amarillo (Destacados/Advertencias)
#define COLOR_TEXTO_ROJO 12       // Alertas de error
#define COLOR_TEXTO_GRIS 8        // Guías de ayuda

/**
 * SECCIÓN 3: ESTRUCTURAS DE DATOS (DATA STRUCTURES)
 */

// Estructura para el sistema de login
typedef struct {
  int id;                   // ID autoincremental
  char nombreUsuario[13];   // Max 12 caracteres + null terminator
  char contrasena[33];      // Max 32 caracteres + null terminator
  char nombreCompleto[100]; // Nombre y Apellido del responsable
} Usuario;

// Estructura principal del negocio (Control de Calidad)
typedef struct {
  int idLote;       // Identificador único del lote (Trazabilidad)
  int tipoProducto; // 1: Jugo Concentrado, 2: Aceite Esencial
  float ph;         // Nivel de acidez (pH)
  float acidez;     // Porcentaje de acidez titulable
  bool inocuidad;   // Cumplimiento fitosanitario (true=Ok, false=Riesgo)
  int estado;       // 1: Aprobado, 2: Reprocesar, 3: Bloqueado
  bool activo;      // Para Baja Lógica (true=Existente, false=Borrado lógico)
} RegistroCalidad;

/**
 * ESTRUCTURA DINÁMICA (LINKED LIST)
 */
typedef struct NodoCalidad {
  RegistroCalidad datos;         // La información del lote
  struct NodoCalidad *siguiente; // Dirección de memoria del próximo elemento
} NodoCalidad;

// Variable global para el usuario que está operando el sistema
Usuario usuarioActual;

/**
 * SECCIÓN 4: PROTOTIPOS DE FUNCIONES (PROTOTYPES)
 */

// Utilidades de Interfaz (UI)
void gotoxy(int x, int y);
void color(int texto, int fondo);
void ocultarCursor();
void mostrarCursor();
void limpiarPantalla();
int obtenerAnchoConsola();
int obtenerAltoConsola();
void centrarTexto(const char *texto, int y, int colorTexto);
void dibujarRecuadro(int x1, int y1, int x2, int y2, int colorLinea);
void mostrarMensaje(const char *mensaje, int colorTexto);
void pausar();
bool leerInput(char *buffer, int longitudMax, bool esPassword);

// Pantallas Estáticas y Menús
void mostrarSplashScreen();
void mostrarEncabezado();
void mostrarDespedida();
int menuNavegable(const char *titulo, const char *opciones[], int numOpciones);

// Lógica de Usuarios (Login/Registro)
bool validarNombreUsuario(const char *usuario);
bool validarContrasena(const char *pass);
int obtenerSiguienteIdUsuario();
void menuAcceso();
void registrarUsuario();
bool iniciarSesion();

// Lógica de Gestión de Calidad (CRUD y Archivos)
void menuPrincipal();
void crearArchivoAnalisis();
void grabarAnalisis();
void bajaLogicaAnalisis();
void bajaFisicaAnalisis();
void modificarAnalisis();
void listarAnalisisArchivo();
void consultarAnalisis();

// Gestión Dinámica (RAM - Listas Enlazadas)
void crearYMostrarEstructuraDinamica(NodoCalidad **listaGlobal);
void insertarOrdenado(NodoCalidad **cabeza, NodoCalidad *nuevo);
NodoCalidad *crearNodo(RegistroCalidad reg);
void mostrarLista(NodoCalidad *cabeza);
void liberarLista(NodoCalidad **cabeza);

/**
 * =====================================================================================
 * SECCIÓN 5: FUNCIÓN PRINCIPAL (ENTRY POINT)
 * =====================================================================================
 */
int main() {
  // Configuración de la consola para soportar caracteres especiales (UTF-8)
  SetConsoleOutputCP(65001);
  ocultarCursor();
  system("title S.A. VERACRUZ - Calidad y Despacho");

  mostrarSplashScreen(); // Pantalla de bienvenida decorativa
  menuAcceso();          // Entrada al sistema (Login/Register)

  return 0;
}

/**
 * =====================================================================================
 * SECCIÓN 6: UTILIDADES DE INTERFAZ (UI UTILS)
 * =====================================================================================
 */

// Obtiene el ancho actual de la ventana de comandos
int obtenerAnchoConsola() {
  CONSOLE_SCREEN_BUFFER_INFO csbi;
  if (GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi)) {
    return csbi.srWindow.Right - csbi.srWindow.Left + 1;
  }
  return 120;
}

// Obtiene el alto actual de la ventana de comandos
int obtenerAltoConsola() {
  CONSOLE_SCREEN_BUFFER_INFO csbi;
  if (GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi)) {
    return csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
  }
  return 30;
}

// Ubica el cursor en una coordenada específica (Columna x, Fila y)
void gotoxy(int x, int y) {
  HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
  COORD pos = {(SHORT)x, (SHORT)y};
  SetConsoleCursorPosition(hConsole, pos);
}

// Cambia la combinación de colores de texto y fondo en la consola
void color(int texto, int fondo) {
  HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
  SetConsoleTextAttribute(hConsole, (WORD)((fondo << 4) | texto));
}

void ocultarCursor() {
  HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
  CONSOLE_CURSOR_INFO cursorInfo;
  GetConsoleCursorInfo(hConsole, &cursorInfo);
  cursorInfo.bVisible = FALSE;
  SetConsoleCursorInfo(hConsole, &cursorInfo);
}

void mostrarCursor() {
  HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
  CONSOLE_CURSOR_INFO cursorInfo;
  GetConsoleCursorInfo(hConsole, &cursorInfo);
  cursorInfo.bVisible = TRUE;
  SetConsoleCursorInfo(hConsole, &cursorInfo);
}

void limpiarPantalla() { system("cls"); }

// Dibuja un marco decorativo usando caracteres ASCII de línea doble
void dibujarRecuadro(int x1, int y1, int x2, int y2, int colorLinea) {
  color(colorLinea, COLOR_FONDO);
  for (int i = x1 + 1; i < x2; i++) {
    gotoxy(i, y1);
    printf("═");
    gotoxy(i, y2);
    printf("═");
  }
  for (int i = y1 + 1; i < y2; i++) {
    gotoxy(x1, i);
    printf("║");
    gotoxy(x2, i);
    printf("║");
  }
  gotoxy(x1, y1);
  printf("╔");
  gotoxy(x2, y1);
  printf("╗");
  gotoxy(x1, y2);
  printf("╚");
  gotoxy(x2, y2);
  printf("╝");
}

// Centra un texto horizontalmente basándose en el ancho de la consola
void centrarTexto(const char *texto, int y, int colorTexto) {
  int visualLen = 0;
  const unsigned char *p = (const unsigned char *)texto;
  while (*p) {
    if ((*p & 0xc0) != 0x80)
      visualLen++; // Salta bytes de continuación UTF-8
    p++;
  }
  int ancho = obtenerAnchoConsola();
  int x = (ancho - visualLen) / 2;
  if (x < 0)
    x = 0;
  gotoxy(x, y);
  color(colorTexto, COLOR_FONDO);
  printf("%s", texto);
}

// Muestra un mensaje de estado en la parte inferior de la pantalla
void mostrarMensaje(const char *mensaje, int colorTexto) {
  int ancho = obtenerAnchoConsola();
  int alto = obtenerAltoConsola();
  int yMensaje = alto - 2;

  gotoxy(0, yMensaje);
  color(COLOR_FONDO, COLOR_FONDO);
  for (int i = 0; i < ancho; i++)
    printf(" ");

  centrarTexto(mensaje, yMensaje, colorTexto);
}

void pausar() {
  mostrarMensaje("Presione cualquier tecla para continuar...",
                 COLOR_TEXTO_BLANCO);
  getch();
  mostrarMensaje("", COLOR_FONDO);
}

/**
 * FUNCIÓN LEER INPUT
 */
bool leerInput(char *buffer, int longitudMax, bool esPassword) {
  int pos = 0;
  char c;
  color(COLOR_TEXTO_BLANCO,
        COLOR_FONDO); // Color para el texto que escribe el usuario
  while (true) {
    c = getch();
    if (c == TECLA_ESC)
      return false; // Permite cancelar la entrada con ESC
    else if (c == TECLA_ENTER) {
      buffer[pos] = '\0';
      return true;
    } else if (c == TECLA_BACKSPACE) {
      if (pos > 0) {
        pos--;
        printf("\b \b"); // Retrocede, imprime espacio, retrocede de nuevo
      }
    } else {
      if (pos < longitudMax - 1 && c >= 32 && c <= 126) {
        buffer[pos++] = c;
        if (esPassword)
          printf("*");
        else
          printf("%c", c);
      }
    }
  }
}

/**
 * =====================================================================================
 * SECCIÓN 7: PRESENTACIÓN Y MENÚS (PRESENTATION LAYER)
 * =====================================================================================
 */

// Pantalla de Logo ASCII con centrado dinámico (Responsive)
void mostrarSplashScreen() {
  int anchoPre = 0, altoPre = 0;
  while (true) {
    int ancho = obtenerAnchoConsola();
    int alto = obtenerAltoConsola();

    if (ancho != anchoPre || alto != altoPre) {
      limpiarPantalla();
      anchoPre = ancho;
      altoPre = alto;

      const char *logo[] = {
          "██╗   ██╗███████╗██████╗  █████╗  ██████╗██████╗ ██╗   ██╗███████╗",
          "██║   ██║██╔════╝██╔══██╗██╔══██╗██╔════╝██╔══██╗██║   ██║╚══███╔╝",
          "██║   ██║█████╗  ██████╔╝███████║██║     ██████╔╝██║   ██║  ███╔╝ ",
          "╚██╗ ██╔╝██╔══╝  ██╔══██╗██╔══██║██║     ██╔══██╗██║   ██║ ███╔╝  ",
          " ╚████╔╝ ███████╗██║  ██║██║  ██║╚██████╗██║  ██║╚██████╔╝███████╗",
          "  ╚═══╝  ╚══════╝╚═╝  ╚═╝╚═╝  ╚═╝ ╚═════╝╚═╝  ╚═╝ ╚═════╝ ╚══════╝",
          "                                                                  ",
          "     ██████╗ ██╗   ██╗ █████╗ ██╗     ██╗████████╗██╗   ██╗       ",
          "    ██╔═══██╗██║   ██║██╔══██╗██║     ██║╚══██╔══╝╚██╗ ██╔╝       ",
          "    ██║   ██║██║   ██║███████║██║     ██║   ██║    ╚████╔╝        ",
          "    ██║▄▄ ██║██║   ██║██╔══██║██║     ██║   ██║     ╚██╔╝         ",
          "    ╚██████╔╝╚██████╔╝██║  ██║███████╗██║   ██║      ██║          ",
          "     ╚══▀▀═╝  ╚═════╝ ╚═╝  ╚═╝╚══════╝╚═╝   ╚═╝      ╚═╝          "};

      int numLineasLogo = 13;
      int yInicio = (alto - (numLineasLogo + 16)) / 2;
      if (yInicio < 2)
        yInicio = 2;

      color(COLOR_TEXTO_PRINCIPAL, COLOR_FONDO);

      // DECORACIÓN PREMIUM: Gran marco exterior
      if (ancho > 90 && alto > 30) {
        dibujarRecuadro(2, 1, ancho - 3, alto - 2, COLOR_TEXTO_PRINCIPAL);
      }

      for (int i = 0; i < numLineasLogo; i++) {
        int x = ((ancho - 74) / 2) + 4;
        gotoxy(x, yInicio + i);
        printf("%s", logo[i]);
      }

      centrarTexto("SISTEMA DE GESTION Y CONTROL DE CALIDAD", yInicio + 15,
                   COLOR_TEXTO_BLANCO);

      // DECORACIÓN PREMIUM: Cuadro de Créditos
      int yCreditos = yInicio + 17;
      int wMarco = 80;
      if (wMarco > ancho - 4)
        wMarco = ancho - 6;
      int xMarco = (ancho - wMarco) / 2;

      color(COLOR_TEXTO_SECUNDARIO, COLOR_FONDO);
      gotoxy(xMarco, yCreditos);
      for (int k = 0; k < wMarco; k++)
        printf("═");
      centrarTexto("♦ ♦ ♦", yCreditos + 3, COLOR_TEXTO_PRINCIPAL);
      gotoxy(xMarco, yCreditos + 6);
      for (int k = 0; k < wMarco; k++)
        printf("═");

      gotoxy(xMarco, yCreditos + 2);
      printf(">>");
      gotoxy(xMarco + wMarco - 2, yCreditos + 2);
      printf("<<");

      centrarTexto("DESARROLLO POR", yCreditos + 2,
                   COLOR_TEXTO_PRINCIPAL);
      centrarTexto("Alejandro Colchi", yCreditos + 4, COLOR_TEXTO_BLANCO);

      centrarTexto("Presione cualquier tecla para continuar...", alto - 4,
                   COLOR_TEXTO_GRIS);
    }
    if (kbhit()) {
      getch();
      break;
    }
    Sleep(100);
  }
}

// Encabezado estándar para todas las pantallas del sistema
void mostrarEncabezado() {
  limpiarPantalla();
  int ancho = obtenerAnchoConsola();
  dibujarRecuadro(1, 1, ancho - 2, 5, COLOR_TEXTO_PRINCIPAL);
  centrarTexto("S.A. VERACRUZ", 2, COLOR_TEXTO_PRINCIPAL);
  centrarTexto("SISTEMA DE GESTION DE CALIDAD", 3, COLOR_TEXTO_SECUNDARIO);

  if (strlen(usuarioActual.nombreUsuario) > 0) {
    char usrBuffer[150];
    sprintf(usrBuffer, "Responsable: %s", usuarioActual.nombreCompleto);
    gotoxy(ancho - strlen(usrBuffer) - 4, 3);
    color(COLOR_TEXTO_BLANCO, 0);
    printf("%s", usrBuffer);
  }
}

/**
 * FUNCIÓN MENÚ NAVEGABLE
 */
int menuNavegable(const char *titulo, const char *opciones[], int numOpciones) {
  int seleccion = 0;
  int anchoPre = 0, altoPre = 0;

  while (true) {
    int ancho = obtenerAnchoConsola();
    int alto = obtenerAltoConsola();
    if (ancho != anchoPre || alto != altoPre) {
      anchoPre = ancho;
      altoPre = alto;
      mostrarEncabezado();
      int maxLen = 0;
      for (int i = 0; i < numOpciones; i++)
        if ((int)strlen(opciones[i]) > maxLen)
          maxLen = strlen(opciones[i]);
      int anchoBloque = (maxLen + 15 < 40) ? 40 : maxLen + 15;
      int xMenu = (ancho - anchoBloque) / 2;
      int yBase = 7 + ((alto - 7 - (numOpciones + 4)) / 2);
      centrarTexto(titulo, yBase - 2, COLOR_TEXTO_BLANCO);
      dibujarRecuadro(xMenu, yBase, xMenu + anchoBloque + 2,
                      yBase + numOpciones + 2, COLOR_TEXTO_PRINCIPAL);
      centrarTexto("[Flechas]: Navegar   [Enter]: Seleccionar   [ESC]: Volver",
                   yBase + numOpciones + 4, COLOR_TEXTO_GRIS);
    }

    int maxLen = 0;
    for (int i = 0; i < numOpciones; i++)
      if ((int)strlen(opciones[i]) > maxLen)
        maxLen = strlen(opciones[i]);
    int anchoBloque = (maxLen + 15 < 40) ? 40 : maxLen + 15;
    int xMenu = (ancho - anchoBloque) / 2;
    int yBase = 7 + ((alto - 7 - (numOpciones + 4)) / 2);

    for (int i = 0; i < numOpciones; i++) {
      gotoxy(xMenu + 1, yBase + i + 1);
      if (i == seleccion)
        color(COLOR_TEXTO_BLANCO, COLOR_TEXTO_PRINCIPAL);
      else
        color(COLOR_TEXTO_BLANCO, COLOR_FONDO);
      int espacios = anchoBloque - strlen(opciones[i]);
      for (int k = 0; k < espacios / 2; k++)
        printf(" ");
      printf("%s", opciones[i]);
      for (int k = 0; k < (espacios - espacios / 2); k++)
        printf(" ");
    }

    while (true) {
      if (kbhit()) {
        int tecla = getch();
        if (tecla == 0 || tecla == 224) {
          tecla = getch();
          if (tecla == TECLA_ARRIBA)
            seleccion = (seleccion <= 0) ? numOpciones - 1 : seleccion - 1;
          else if (tecla == TECLA_ABAJO)
            seleccion = (seleccion >= numOpciones - 1) ? 0 : seleccion + 1;
          break;
        } else if (tecla == TECLA_ENTER)
          return seleccion;
        else if (tecla == TECLA_ESC)
          return -1;
      }
      if (obtenerAnchoConsola() != anchoPre)
        break;
      Sleep(50);
    }
  }
}

/**
 * =====================================================================================
 * SECCIÓN 8: LÓGICA DE USUARIOS (AUTHENTICATION)
 * =====================================================================================
 */

bool validarNombreUsuario(const char *usuario) {
  int len = strlen(usuario);
  if (len < 6 || len > 12)
    return false;
  if (!islower(usuario[0]))
    return false; // Debe iniciar con minúscula
  int mayus = 0, num = 0;
  for (int i = 0; i < len; i++) {
    if (isupper(usuario[i]))
      mayus++;
    if (isdigit(usuario[i]))
      num++;
  }
  return (mayus >= 2 && num <= 3); // Regla: Min 2 Mayus y Max 3 números
}

bool validarContrasena(const char *pass) {
  int len = strlen(pass);
  if (len < 6 || len > 32)
    return false;
  bool m = false, M = false, N = false;
  for (int i = 0; i < len; i++) {
    if (islower(pass[i]))
      m = true;
    if (isupper(pass[i]))
      M = true;
    if (isdigit(pass[i]))
      N = true;
  }
  return (m && M && N); // Regla: Al menos una minúscula, mayúscula y número
}

// Obtiene el último ID del archivo para simular un autoincremental
int obtenerSiguienteIdUsuario() {
  FILE *f = fopen(FILE_USUARIOS, "rb");
  int id = 1;
  Usuario u;
  if (f) {
    fseek(f, 0, SEEK_END);
    if (ftell(f) > 0) {
      fseek(f, -((long)sizeof(Usuario)), SEEK_END);
      fread(&u, sizeof(Usuario), 1, f);
      id = u.id + 1;
    }
    fclose(f);
  }
  return id;
}

void registrarUsuario() {
  mostrarEncabezado();
  centrarTexto("ALTA DE USUARIO", 7, COLOR_TEXTO_PRINCIPAL);
  int xC = (obtenerAnchoConsola() - 110) / 2;
  dibujarRecuadro(xC, 9, xC + 110, 21, COLOR_TEXTO_SECUNDARIO);

  gotoxy(xC + 4, 11);
  color(COLOR_TEXTO_SECUNDARIO, 0);
  printf("Reglas para el Usuario: 6-12 caracteres, primera minuscula, minimo 2 "
         "Mayusculas y Maximo 3 numeros.");
  gotoxy(xC + 4, 12);
  printf("Reglas para la Contrasena: 6-32 caracteres, 1 minuscula, 1 "
         "mayuscula, 1 numero, sin secuencias obvias.");

  Usuario u;
  memset(&u, 0, sizeof(Usuario));
  u.id = obtenerSiguienteIdUsuario();
  mostrarCursor();
  int xF = xC + 20, yF = 15;
  color(COLOR_TEXTO_BLANCO, 0);
  gotoxy(xF, yF);
  printf("Usuario: ");
  gotoxy(xF, yF + 2);
  printf("Contrasena: ");
  gotoxy(xF, yF + 4);
  printf("Nombre y Apellido: ");

  while (true) {
    gotoxy(xF + 9, yF);
    printf("                     ");
    gotoxy(xF + 9, yF);
    if (!leerInput(u.nombreUsuario, 13, false))
      return;
    if (validarNombreUsuario(u.nombreUsuario))
      break;
    mostrarMensaje("Usuario invalido (ver reglas).", COLOR_TEXTO_SECUNDARIO);
  }
  mostrarMensaje("", 0);
  while (true) {
    gotoxy(xF + 12, yF + 2);
    printf("                     ");
    gotoxy(xF + 12, yF + 2);
    if (!leerInput(u.contrasena, 33, false))
      return;
    if (validarContrasena(u.contrasena))
      break;
    mostrarMensaje("Contrasena insegura (ver reglas).", COLOR_TEXTO_SECUNDARIO);
  }
  mostrarMensaje("", 0);
  gotoxy(xF + 19, yF + 4);
  if (!leerInput(u.nombreCompleto, 100, false))
    return;

  FILE *f =
      fopen(FILE_USUARIOS, "ab"); // "ab" = Append Binary (Agrega al final)
  if (!f)
    f = fopen(FILE_USUARIOS, "wb");
  if (f) {
    fwrite(&u, sizeof(Usuario), 1, f);
    fclose(f);
    mostrarMensaje("Exito: Usuario registrado.", COLOR_TEXTO_PRINCIPAL);
  }
  ocultarCursor();
  getch();
}

bool iniciarSesion() {
  mostrarEncabezado();
  centrarTexto("INICIO DE SESION", 7, COLOR_TEXTO_PRINCIPAL);
  char user[50], pass[50];
  mostrarCursor();
  int xC = (obtenerAnchoConsola() - 50) / 2;
  dibujarRecuadro(xC, 10, xC + 50, 16, COLOR_TEXTO_PRINCIPAL);
  gotoxy(xC + 5, 12);
  printf("Usuario: ");
  if (!leerInput(user, 50, false))
    return false;
  gotoxy(xC + 5, 14);
  printf("Contrasena: ");
  if (!leerInput(pass, 50, false))
    return false;

  FILE *f = fopen(FILE_USUARIOS, "rb");
  if (!f) {
    mostrarMensaje("Error: No existen usuarios.", COLOR_TEXTO_SECUNDARIO);
    getch();
    return false;
  }
  Usuario u;
  bool find = false;
  while (fread(&u, sizeof(Usuario), 1, f) > 0) {
    if (strcmp(u.nombreUsuario, user) == 0 && strcmp(u.contrasena, pass) == 0) {
      usuarioActual = u;
      find = true;
      break;
    }
  }
  fclose(f);
  if (find) {
    mostrarMensaje("Bienvenido!", COLOR_TEXTO_PRINCIPAL);
    getch();
    return true;
  }
  mostrarMensaje("Error: Credenciales incorrectas.", COLOR_TEXTO_SECUNDARIO);
  getch();
  return false;
}

void menuAcceso() {
  const char *op[] = {"INICIO DE SESION", "REGISTRARSE", "SALIR"};
  while (true) {
    int s = menuNavegable("AUTENTICACION", op, 3);
    if (s == 0) {
      if (iniciarSesion())
        menuPrincipal();
    } else if (s == 1)
      registrarUsuario();
    else
      break;
  }
}

/**
 * =====================================================================================
 * SECCIÓN 9: LÓGICA DE NEGOCIO (QUALITY CONTROL OPERATIONS)
 * =====================================================================================
 */

void crearArchivoAnalisis() {
  mostrarEncabezado();
  centrarTexto("FORMATEO DE BASE DE DATOS", 7, COLOR_TEXTO_SECUNDARIO);
  mostrarMensaje("¿Confirmar limpieza total del archivo? (s/n)",
                 COLOR_TEXTO_SECUNDARIO);
  char b[2];
  if (!leerInput(b, 2, false))
    return;
  if (tolower(b[0]) == 's') {
    FILE *f = fopen(FILE_ANALISIS, "wb");
    if (f) {
      fclose(f);
      mostrarMensaje("Base de datos reiniciada.", COLOR_TEXTO_PRINCIPAL);
    }
  }
  getch();
}

// Panel lateral de ayuda contextual
void ayudaContextual(const char *t, int c) {
  int x = (obtenerAnchoConsola() / 2) + 12, y = 8, w = 40, h = 8;
  dibujarRecuadro(x, y, x + w, y + h, 8);
  gotoxy(x + 2, y);
  color(8, 0);
  printf(" AYUDA ");
  color(c, 0);
  gotoxy(x + 2, y + 2);
  if (strlen(t) < 35)
    printf("%s", t);
  else {
    char temp[100];
    strcpy(temp, t);
    temp[35] = '\0';
    printf("%s", temp);
    gotoxy(x + 2, y + 3);
    printf("%s", &t[35]);
  }
}

void grabarAnalisis() {
  mostrarEncabezado();
  centrarTexto("NUEVO ANALISIS DE CALIDAD", 6, COLOR_TEXTO_PRINCIPAL);
  RegistroCalidad r;
  char b[50];
  mostrarCursor();
  int x = (obtenerAnchoConsola() / 2) - 50;
  dibujarRecuadro(x, 8, x + 55, 22, COLOR_TEXTO_PRINCIPAL);
  color(COLOR_TEXTO_BLANCO, 0);

  ayudaContextual("ID unico de trazabilidad.", 14);
  gotoxy(x + 4, 10);
  printf("ID Lote: ");
  if (!leerInput(b, 10, false))
    return;
  r.idLote = atoi(b);
  ayudaContextual("1: Jugo / 2: Aceite Esencial.", 14);
  gotoxy(x + 4, 12);
  printf("Producto: ");
  if (!leerInput(b, 5, false))
    return;
  r.tipoProducto = atoi(b);
  ayudaContextual("Escala 0.0-14.0. Optimo: 2.2-2.8", 14);
  gotoxy(x + 4, 14);
  printf("pH:");
  if (!leerInput(b, 10, false))
    return;
  r.ph = atof(b);
  ayudaContextual("Porcentaje acidez. Optimo: 40-48", 14);
  gotoxy(x + 4, 16);
  printf("Acidez:");
  if (!leerInput(b, 10, false))
    return;
  r.acidez = atof(b);
  ayudaContextual("Cumple fitosanitario? (1:Si/0:No).", 14);
  gotoxy(x + 4, 18);
  printf("Inocuidad:");
  if (!leerInput(b, 5, false))
    return;
  r.inocuidad = (atoi(b) == 1);

  // LOGICA DE ESTADOS
  if (!r.inocuidad)
    r.estado = 3; // BLOQUEADO
  else {
    bool phOk = (r.ph >= 2.2 && r.ph <= 2.8);
    bool acOk = (r.acidez >= 40.0 && r.acidez <= 48.0);
    r.estado = (phOk && acOk) ? 1 : 2; // 1:APROBADO, 2:REPROCESAR
  }
  r.activo = true;
  FILE *f = fopen(FILE_ANALISIS, "ab");
  if (f) {
    fwrite(&r, sizeof(RegistroCalidad), 1, f);
    fclose(f);
    mostrarMensaje("Registro Guardado.", 10);
  }
  getch();
}

void bajaLogicaAnalisis() {
  mostrarEncabezado();
  centrarTexto("BORRADO LOGICO", 6, COLOR_TEXTO_PRINCIPAL);
  char b[15];
  mostrarCursor();
  int an = obtenerAnchoConsola();
  gotoxy(an / 2 - 15, 8);
  printf("ID a dar de baja:");
  if (!leerInput(b, 10, false))
    return;
  int id = atoi(b);
  FILE *f =
      fopen(FILE_ANALISIS, "r+b"); // "r+b" = Modo lectura/escritura binaria
  if (f) {
    RegistroCalidad r;
    bool ok = false;
    while (fread(&r, sizeof(RegistroCalidad), 1, f) > 0) {
      if (r.idLote == id && r.activo) {
        r.activo = false;
        fseek(f, -((long)sizeof(RegistroCalidad)),
              SEEK_CUR); // Retrocede el puntero para sobrescribir
        fwrite(&r, sizeof(RegistroCalidad), 1, f);
        ok = true;
        break;
      }
    }
    fclose(f);
    mostrarMensaje(ok ? "Lote dado de baja (Logicamente)." : "No encontrado.",
                   ok ? 10 : 12);
  }
  getch();
}

void bajaFisicaAnalisis() {
  mostrarEncabezado();
  centrarTexto("PURGA DEFINITIVA", 7, COLOR_TEXTO_PRINCIPAL);
  mostrarMensaje("¿Eliminar registros inactivos definitivamente? (s/n)",
                 COLOR_TEXTO_SECUNDARIO);
  char b[2];
  if (!leerInput(b, 2, false))
    return;
  if (tolower(b[0]) == 's') {
    FILE *o = fopen(FILE_ANALISIS, "rb"), *t = fopen(TEMP_ANALISIS, "wb");
    if (o && t) {
      RegistroCalidad r;
      while (fread(&r, sizeof(RegistroCalidad), 1, o) > 0)
        if (r.activo)
          fwrite(&r, sizeof(RegistroCalidad), 1,
                 t); // Solo copia los activos al nuevo archivo
      fclose(o);
      fclose(t);
      remove(FILE_ANALISIS);
      rename(TEMP_ANALISIS, FILE_ANALISIS);
      mostrarMensaje("Purga terminada.", 10);
    }
  }
  getch();
}

void modificarAnalisis() {
  mostrarEncabezado();
  centrarTexto("MODIFICACION DE LOTE", 6, 10);
  int an = obtenerAnchoConsola();
  char b[50];
  mostrarCursor();
  gotoxy(an / 2 - 15, 8);
  printf("ID Lote:");
  if (!leerInput(b, 10, false))
    return;
  int id = atoi(b);
  FILE *f = fopen(FILE_ANALISIS, "r+b");
  if (f) {
    RegistroCalidad r;
    bool found = false;
    while (fread(&r, sizeof(RegistroCalidad), 1, f) > 0) {
      if (r.idLote == id && r.activo) {
        found = true;
        mostrarEncabezado();
        centrarTexto("MODIFICACION", 6, 10);
        dibujarRecuadro(an / 2 - 35, 9, an / 2 + 35, 23, 14);
        gotoxy(an / 2 - 30, 11);
        color(8, 0);
        printf("Actual:");
        color(15, 0);
        printf(" ID:%d PH:%.2f AC:%.1f%% IN:%d", r.idLote, r.ph, r.acidez,
               r.inocuidad);
        gotoxy(an / 2 - 30, 14);
        printf("pH (Enter=Mantener): ");
        if (leerInput(b, 10, false) && strlen(b) > 0)
          r.ph = atof(b);
        gotoxy(an / 2 - 30, 16);
        printf("Acidez (Enter=Mantener): ");
        if (leerInput(b, 10, false) && strlen(b) > 0)
          r.acidez = atof(b);
        gotoxy(an / 2 - 30, 18);
        printf("Inocuidad (1=Si, 0=No): ");
        if (leerInput(b, 5, false) && strlen(b) > 0)
          r.inocuidad = (atoi(b) == 1);

        if (!r.inocuidad)
          r.estado = 3;
        else
          r.estado = (r.ph >= 2.2 && r.ph <= 2.8 && r.acidez >= 40.0 &&
                      r.acidez <= 48.0)
                         ? 1
                         : 2;
        fseek(f, -((long)sizeof(RegistroCalidad)), SEEK_CUR);
        fwrite(&r, sizeof(RegistroCalidad), 1, f);
        break;
      }
    }
    fclose(f);
    mostrarMensaje(found ? "Modificado con exito." : "ID no hallado.",
                   found ? 10 : 12);
  }
  getch();
}

void listarAnalisisArchivo() {
  mostrarEncabezado();
  centrarTexto("LOTES EN BASE DE DATOS", 7, 10);
  FILE *f = fopen(FILE_ANALISIS, "rb");
  if (f) {
    RegistroCalidad r;
    int y = 9, xS = (obtenerAnchoConsola() - 74) / 2;
    color(10, 0);
    gotoxy(xS, y++);
    printf("╔══════╦══════════════╦══════════╦══════════╦═══════════════╦══════"
           "════╗");
    gotoxy(xS, y++);
    printf("║ ID   ║ PRODUCTO     ║ PH       ║ ACIDEZ   ║ ESTADO        ║ "
           "COND.    ║");
    gotoxy(xS, y++);
    printf("╠══════╬══════════════╬══════════╬══════════╬═══════════════╬══════"
           "════╣");
    while (fread(&r, sizeof(RegistroCalidad), 1, f) > 0) {
      color(15, 0);
      gotoxy(xS, y);
      printf("║ %-5d║ %-12s ║ %-8.2f ║ %-7.1f%% ║", r.idLote,
             (r.tipoProducto == 1 ? "JUGO" : "ACEITE"), r.ph, r.acidez);
      if (r.estado == 1) {
        color(11, 0);
        printf(" APROBADO      ");
      } else if (r.estado == 2) {
        color(14, 0);
        printf(" REPROCESAR    ");
      } else {
        color(12, 0);
        printf(" BLOQUEADO     ");
      }
      color(15, 0);
      printf("║ %-8s ║", r.activo ? "ACTIVO" : "BAJA");
      y++;
      if (y > obtenerAltoConsola() - 4) {
        pausar();
        mostrarEncabezado();
        y = 9;
      }
    }
    color(10, 0);
    gotoxy(xS, y);
    printf("╚══════╩══════════════╩══════════╩══════════╩═══════════════╩══════"
           "════╝");
    fclose(f);
  } else
    mostrarMensaje("Archivo vacio.", 14);
  getch();
}

void consultarAnalisis() {
  mostrarEncabezado();
  char b[10];
  mostrarCursor();
  int an = obtenerAnchoConsola();
  gotoxy(an / 2 - 10, 8);
  printf("ID:");
  if (!leerInput(b, 10, false))
    return;
  int id = atoi(b);
  ocultarCursor();
  mostrarEncabezado();
  centrarTexto("BUSQUEDA EN BASE DE DATOS", 6, 10);
  FILE *f = fopen(FILE_ANALISIS, "rb");
  if (f) {
    RegistroCalidad r;
    bool find = false;
    while (fread(&r, sizeof(RegistroCalidad), 1, f) > 0) {
      if (r.idLote == id && r.activo) {
        int xF = (an - 60) / 2, yF = 10;
        dibujarRecuadro(xF, yF, xF + 60, yF + 12, 10);
        color(14, 0);
        gotoxy(xF + 2, yF + 2);
        printf("FICHA TECNICA - LOTE #%d", id);
        color(15, 0);
        gotoxy(xF + 4, yF + 5);
        printf("Producto: %s", (r.tipoProducto == 1 ? "Jugo" : "Aceite"));
        gotoxy(xF + 4, yF + 6);
        printf("Inocuidad:%s", r.inocuidad ? "OK" : "RIESGO");
        gotoxy(xF + 32, yF + 5);
        printf("pH:      %.2f", r.ph);
        gotoxy(xF + 32, yF + 6);
        printf("Acidez:  %.1f%%", r.acidez);
        gotoxy(xF + 4, yF + 10);
        printf("RESULTADO: ");
        if (r.estado == 1) {
          color(11, 0);
          printf("APROBADO");
        } else if (r.estado == 2) {
          color(14, 0);
          printf("REPROCESAR");
        } else {
          color(12, 0);
          printf("BLOQUEADO");
        }
        find = true;
        break;
      }
    }
    fclose(f);
    if (!find)
      mostrarMensaje("Error: ID no encontrado.", 12);
  }
  getch();
}

/**
 * =====================================================================================
 * SECCIÓN 10: MEMORIA DINÁMICA - LISTAS ENLAZADAS (RAM MANAGEMENT)
 * =====================================================================================
 */

// Carga los datos del archivo a la RAM y los ordena automáticamente
void crearYMostrarEstructuraDinamica(NodoCalidad **lG) {
  liberarLista(lG); // Importante: Limpiar memoria anterior antes de recargar
  FILE *f = fopen(FILE_ANALISIS, "rb");
  if (f) {
    RegistroCalidad r;
    int c = 0;
    while (fread(&r, sizeof(RegistroCalidad), 1, f) > 0) {
      if (r.activo) {
        NodoCalidad *n = crearNodo(r);
        insertarOrdenado(lG, n);
        c++;
      }
    }
    fclose(f);
    char m[100];
    sprintf(m, "Atencion: Se cargaron %d registros a la RAM.", c);
    mostrarMensaje(m, 10);
  }
  getch();
}

/**
 * FUNCIÓN INSERTAR ORDENADO
 */
void insertarOrdenado(NodoCalidad **c, NodoCalidad *n) {
  if (*c == NULL) {
    *c = n;
    return;
  } // Si la lista está vacía, el primero es el nodo nuevo
  NodoCalidad *act = *c, *ant = NULL;
  while (act != NULL) {
    // Compara el campo 'estado'. Al ser mayor el número (3=Bloqueado), queda
    // arriba.
    if (n->datos.estado > act->datos.estado)
      break;
    else if (n->datos.estado == act->datos.estado &&
             n->datos.acidez > act->datos.acidez)
      break;
    ant = act;
    act = act->siguiente;
  }
  if (ant == NULL) {
    n->siguiente = *c;
    *c = n;
  } // Insertar al principio
  else {
    ant->siguiente = n;
    n->siguiente = act;
  } // Insertar en medio o al final
}

// Crea un nuevo nodo reservando memoria física en RAM
NodoCalidad *crearNodo(RegistroCalidad r) {
  NodoCalidad *n = (NodoCalidad *)malloc(sizeof(NodoCalidad));
  n->datos = r;
  n->siguiente = NULL;
  return n;
}

void mostrarLista(NodoCalidad *c) {
  mostrarEncabezado();
  centrarTexto("ESTRUCTURA DINAMICA - ANALISIS DE RIESGOS", 7, 10);
  if (!c) {
    mostrarMensaje("Informacion: La RAM esta vacia. Cargue datos primero.", 14);
    getch();
    return;
  }
  int y = 10, xS = (obtenerAnchoConsola() - 62) / 2;
  color(10, 0);
  gotoxy(xS, y++);
  printf("╔══════╦══════════════╦══════════╦══════════╦═══════════════╗");
  gotoxy(xS, y++);
  printf("║ ID   ║ PRODUCTO     ║ PH       ║ ACIDEZ   ║ ESTADO        ║");
  gotoxy(xS, y++);
  printf("╠══════╬══════════════╬══════════╬══════════╬═══════════════╣");
  while (c) {
    color(15, 0);
    gotoxy(xS, y++);
    printf("║ %-5d║ %-12s ║ %-8.2f ║ %-7.1f%% ║ ", c->datos.idLote,
           (c->datos.tipoProducto == 1 ? "JUGO" : "ACEITE"), c->datos.ph,
           c->datos.acidez);

    char st[20];
    int cl;
    if (c->datos.estado == 3) {
      cl = 12;
      strcpy(st, "BLOQUEADO");
    } else if (c->datos.estado == 2) {
      cl = 14;
      strcpy(st, "REPROCESAR");
    } else {
      cl = 11;
      strcpy(st, "APROBADO");
    }
    color(cl, 0);
    printf("%-13s", st);
    color(15, 0);
    printf(" ║");
    c = c->siguiente;
  }
  color(10, 0);
  gotoxy(xS, y);
  printf("╚══════╩══════════════╩══════════╩══════════╩═══════════════╝");
  pausar();
}

// Libera la memoria ocupada por la lista para evitar fugas (Memory Leaks)
void liberarLista(NodoCalidad **c) {
  NodoCalidad *a;
  while (*c) {
    a = *c;
    *c = (*c)->siguiente;
    free(a);
  }
}

/**
 * =====================================================================================
 * SECCIÓN 11: CONTROL DEL PROGRAMA (GUI & MAIN LOOP)
 * =====================================================================================
 */

void menuPrincipal() {
  const char *op[] = {"CREAR ARCHIVO (RESET)",
                      "GRABAR LOTE (ALTA)",
                      "BAJA LOGICA",
                      "BAJA FISICA (PURGA)",
                      "MODIFICACION TECNICA",
                      "LISTADO GENERAL",
                      "CONSULTA POR ID",
                      "CREAR ESTRUCTURA (RAM)",
                      "MOSTRAR GESTION RIESGOS",
                      "SALIR"};
  NodoCalidad *lC = NULL;
  while (true) {
    int s = menuNavegable("MENU DE GESTION OPERATIVA", op, 10);
    if (s == 0)
      crearArchivoAnalisis();
    else if (s == 1)
      grabarAnalisis();
    else if (s == 2)
      bajaLogicaAnalisis();
    else if (s == 3)
      bajaFisicaAnalisis();
    else if (s == 4)
      modificarAnalisis();
    else if (s == 5)
      listarAnalisisArchivo();
    else if (s == 6)
      consultarAnalisis();
    else if (s == 7)
      crearYMostrarEstructuraDinamica(&lC);
    else if (s == 8)
      mostrarLista(lC);
    else
      break;
  }
  liberarLista(&lC);
  mostrarDespedida();
}

void mostrarDespedida() {
  color(15, 0);
  limpiarPantalla();
  centrarTexto("S.A. VERACRUZ - SESION FINALIZADA CORRECTAMENTE",
               obtenerAltoConsola() / 2, 10);
  Sleep(2000);
}
