

#include <ncurses.h>
#include <cstdlib>
#include <ctime>
#include <string>
#include <vector>
#include <array>

using namespace std;

// -----------------------------------------------------------
// CONSTANTES
// -----------------------------------------------------------

const int ANCHO_ESTANDAR = 120;   // ancho requerido tipografia estandar
const int ALTO_ESTANDAR  = 40;    // alto requerido tipografia estandar
const int ANCHO_SQUARE   = 80;    // ancho requerido tipografia square.ttf
const int ALTO_SQUARE    = 50;    // alto requerido tipografia square.ttf

const int NUM_CARRILES   = 6;     // cantidad de carriles con autos
const int VIDAS_INICIALES = 3;
const int META_POR_NIVEL  = 5;    // veces que hay que cruzar para subir de nivel

// -----------------------------------------------------------
// CONSTANTES ENUMERADAS
// -----------------------------------------------------------

enum class EstadoJuego {
    MENU,
    JUGANDO,
    INSTRUCCIONES,
    CREDITOS,
    GAME_OVER,
    SALIR
};

enum class Direccion {
    IZQUIERDA = -1,
    DERECHA   = 1
};

enum class TipoAuto {
    NORMAL,   // 'C' - velocidad estandar
    RAPIDO,   // 'X' - el doble de rapido
    CAMION    // '=' - mas ancho, mas lento
};

// -----------------------------------------------------------
// CLASE AUTO
// -----------------------------------------------------------

class Auto {
private:
    int x;
    int y;
    Direccion direccion;
    int velocidad;      // cuadros que espera antes de moverse
    int contador;        // contador interno de espera
    TipoAuto tipo;
    char simbolo;

public:
    Auto(int posX, int posY, Direccion dir, int vel, TipoAuto t) {
        x = posX;
        y = posY;
        direccion = dir;
        velocidad = vel;
        contador = 0;
        tipo = t;

        if (tipo == TipoAuto::NORMAL) simbolo = 'C';
        else if (tipo == TipoAuto::RAPIDO) simbolo = 'X';
        else simbolo = '=';
    }

    void mover(int anchoMax) {
        contador++;
        if (contador < velocidad) return;
        contador = 0;

        x += static_cast<int>(direccion);

        if (direccion == Direccion::DERECHA && x > anchoMax) {
            x = 0;
        } else if (direccion == Direccion::IZQUIERDA && x < 0) {
            x = anchoMax;
        }
    }

    int getX() const { return x; }
    int getY() const { return y; }
    char getSimbolo() const { return simbolo; }
    TipoAuto getTipo() const { return tipo; }
};

// -----------------------------------------------------------
// CLASE JUGADOR
// -----------------------------------------------------------

class Jugador {
private:
    int x, y;
    int xInicial, yInicial;
    int vidas;
    int metaAlcanzada;
    string nombre;

public:
    Jugador(int posX, int posY, string nom = "Jugador") {
        x = xInicial = posX;
        y = yInicial = posY;
        vidas = VIDAS_INICIALES;
        metaAlcanzada = 0;
        nombre = nom;
    }

    void mover(int dx, int dy, int anchoMax, int filaSuperior, int filaInferior) {
        int nuevoX = x + dx;
        int nuevoY = y + dy;

        if (nuevoX < 0) nuevoX = 0;
        if (nuevoX > anchoMax) nuevoX = anchoMax;
        if (nuevoY < filaSuperior) nuevoY = filaSuperior;
        if (nuevoY > filaInferior) nuevoY = filaInferior;

        x = nuevoX;
        y = nuevoY;
    }

    void reiniciarPosicion() {
        x = xInicial;
        y = yInicial;
    }

    void perderVida() {
        if (vidas > 0) vidas--;
    }

    void sumarMeta() {
        metaAlcanzada++;
    }

    bool estaVivo() const { return vidas > 0; }
    int getX() const { return x; }
    int getY() const { return y; }
    int getVidas() const { return vidas; }
    int getMeta() const { return metaAlcanzada; }
    int getFilaSuperior() const { return yInicial; } // referencia auxiliar
    string getNombre() const { return nombre; }
};

// -----------------------------------------------------------
// CLASE JUEGO (controlador principal)
// -----------------------------------------------------------

class Juego {
private:
    EstadoJuego estado;
    int filas, columnas;          // tamano real de la terminal
    int anchoJuego, altoJuego;    // area jugable

    int filaMeta;                 // fila superior (linea de meta)
    int filaInicioJugador;        // fila inferior (arranque del jugador)

    vector<Auto> autos;
    Jugador jugador;
    int nivel;
    int metaObjetivo;

    array<int, NUM_CARRILES> carriles; // filas donde circulan los autos
    int opcionMenu;                    // opcion resaltada del menu

public:
    Juego()
        : jugador(0, 0) {
        estado = EstadoJuego::MENU;
        nivel = 1;
        metaObjetivo = META_POR_NIVEL;
        opcionMenu = 0;
    }

    // ---------------------------------------------------
    // INICIALIZACION DE NCURSES
    // ---------------------------------------------------
    void inicializarNcurses() {
        initscr();
        cbreak();
        noecho();
        keypad(stdscr, TRUE);
        curs_set(0);
        nodelay(stdscr, TRUE);
        timeout(120);

        if (has_colors()) {
            start_color();
            init_pair(1, COLOR_WHITE, COLOR_BLACK);   // texto general
            init_pair(2, COLOR_YELLOW, COLOR_BLACK);  // titulo
            init_pair(3, COLOR_RED, COLOR_BLACK);     // autos / peligro
            init_pair(4, COLOR_GREEN, COLOR_BLACK);   // jugador / exito
            init_pair(5, COLOR_CYAN, COLOR_BLACK);    // bordes / meta
            init_pair(6, COLOR_BLACK, COLOR_YELLOW);  // opcion seleccionada
        }

        getmaxyx(stdscr, filas, columnas);
        srand(static_cast<unsigned int>(time(nullptr)));
    }

    // ---------------------------------------------------
    // VERIFICAR TAMANO DE TERMINAL (120x40 u 80x50)
    // ---------------------------------------------------
    bool verificarTamanoTerminal() {
        getmaxyx(stdscr, filas, columnas);

        bool tamanoEstandar = (columnas >= ANCHO_ESTANDAR && filas >= ALTO_ESTANDAR);
        bool tamanoSquare   = (columnas >= ANCHO_SQUARE && filas >= ALTO_SQUARE);

        if (tamanoEstandar || tamanoSquare) {
            return true;
        }

        erase();
        attron(COLOR_PAIR(3));
        mvprintw(1, 2, "El tamano de la terminal no es el requerido.");
        attroff(COLOR_PAIR(3));
        mvprintw(3, 2, "Tamano actual: %d columnas x %d filas", columnas, filas);
        mvprintw(5, 2, "Se necesita uno de estos dos tamanos:");
        mvprintw(6, 4, "- 120 columnas x 40 filas (tipografia estandar)");
        mvprintw(7, 4, "-  80 columnas x 50 filas (tipografia square.ttf)");
        mvprintw(9, 2, "Redimensiona la ventana de la terminal y presiona una tecla...");
        refresh();

        nodelay(stdscr, FALSE);
        getch();
        nodelay(stdscr, TRUE);
        return false;
    }

    // ---------------------------------------------------
    // CONFIGURAR EL AREA JUGABLE SEGUN EL TAMANO DETECTADO
    // ---------------------------------------------------
    void configurarAreaJuego() {
        getmaxyx(stdscr, filas, columnas);

        anchoJuego = columnas - 4;          // margen para bordes
        altoJuego  = filas - 12;            // deja lugar a encabezado y pie

        filaMeta = 6;
        filaInicioJugador = filaMeta + altoJuego - 1;

        for (int i = 0; i < NUM_CARRILES; i++) {
            carriles[i] = filaMeta + 2 + (i * (altoJuego - 3) / (NUM_CARRILES));
        }
    }

    // ---------------------------------------------------
    // GENERAR AUTOS SEGUN EL NIVEL (usa RANDOM)
    // ---------------------------------------------------
    void generarAutos() {
        autos.clear();

        for (int i = 0; i < NUM_CARRILES; i++) {
            Direccion dir = (i % 2 == 0) ? Direccion::DERECHA : Direccion::IZQUIERDA;

            int azar = rand() % 100;
            TipoAuto tipo;
            if (azar < 60) tipo = TipoAuto::NORMAL;
            else if (azar < 85) tipo = TipoAuto::RAPIDO;
            else tipo = TipoAuto::CAMION;

            int velocidadBase = (tipo == TipoAuto::RAPIDO) ? 1 : (tipo == TipoAuto::CAMION ? 3 : 2);
            int velocidad = max(1, velocidadBase - (nivel - 1) / 2);

            int posXInicial = rand() % (anchoJuego > 0 ? anchoJuego : 1);

            autos.push_back(Auto(posXInicial, carriles[i], dir, velocidad, tipo));
        }
    }

    // ---------------------------------------------------
    // REINICIAR NIVEL / PARTIDA
    // ---------------------------------------------------
    void iniciarPartida() {
        configurarAreaJuego();
        jugador = Jugador(anchoJuego / 2, filaInicioJugador, "Jugador");
        nivel = 1;
        generarAutos();
        estado = EstadoJuego::JUGANDO;
    }

    void avanzarNivel() {
        nivel++;
        jugador.reiniciarPosicion();
        generarAutos();
    }

    // ---------------------------------------------------
    // DIBUJAR TITULO (reutilizable en varias pantallas)
    // ---------------------------------------------------
    void dibujarTitulo() {
        string titulo = "U L T I M O   S E M A F O R O";
        int x = max(2, (columnas - static_cast<int>(titulo.size())) / 2);

        attron(COLOR_PAIR(5));
        for (int i = 0; i < columnas - 1; i++) mvaddch(0, i, '=');
        for (int i = 0; i < columnas - 1; i++) mvaddch(2, i, '=');
        attroff(COLOR_PAIR(5));

        attron(COLOR_PAIR(2) | A_BOLD);
        mvprintw(1, x, "%s", titulo.c_str());
        attroff(COLOR_PAIR(2) | A_BOLD);
    }

    // ---------------------------------------------------
    // PANTALLA: MENU PRINCIPAL
    // ---------------------------------------------------
    void dibujarMenu() {
        erase();
        dibujarTitulo();

        // ARREGLO (array) con las opciones del menu
        const string opciones[4] = { "JUGAR", "INSTRUCCIONES", "CREDITOS", "SALIR" };
        int cantidadOpciones = 4;

        int inicioY = filas / 2 - 2;

        for (int i = 0; i < cantidadOpciones; i++) {
            int x = (columnas - static_cast<int>(opciones[i].size()) - 4) / 2;
            if (i == opcionMenu) {
                attron(COLOR_PAIR(6) | A_BOLD);
                mvprintw(inicioY + i * 2, x, "> %s <", opciones[i].c_str());
                attroff(COLOR_PAIR(6) | A_BOLD);
            } else {
                attron(COLOR_PAIR(1));
                mvprintw(inicioY + i * 2, x, "  %s  ", opciones[i].c_str());
                attroff(COLOR_PAIR(1));
            }
        }

        attron(COLOR_PAIR(5));
        mvprintw(filas - 2, 2, "Flechas arriba/abajo para elegir, ENTER para confirmar");
        attroff(COLOR_PAIR(5));

        refresh();
    }

    void manejarMenu(int tecla) {
        switch (tecla) {
            case KEY_UP:
                opcionMenu = (opcionMenu + 3) % 4;
                break;
            case KEY_DOWN:
                opcionMenu = (opcionMenu + 1) % 4;
                break;
            case '\n':
            case KEY_ENTER:
                if (opcionMenu == 0) {
                    iniciarPartida();
                } else if (opcionMenu == 1) {
                    estado = EstadoJuego::INSTRUCCIONES;
                } else if (opcionMenu == 2) {
                    estado = EstadoJuego::CREDITOS;
                } else {
                    estado = EstadoJuego::SALIR;
                }
                break;
            default:
                break;
        }
    }

    // ---------------------------------------------------
    // PANTALLA: INSTRUCCIONES
    // ---------------------------------------------------
    void dibujarInstrucciones() {
        erase();
        dibujarTitulo();

        vector<string> lineas = {
            "COMO SE JUEGA:",
            "",
            "- Tu personaje es la letra  A  y arranca abajo de la pantalla.",
            "- Los autos (C), autos rapidos (X) y camiones (=) cruzan la calle.",
            "- Usa las flechas (o W A S D) para moverte y esquivar los autos.",
            "- Tenes que llegar a la linea de META, arriba de todo.",
            "- Cada vez que llegues a la meta sumas un punto de progreso.",
            "- Al completar 5 cruces subis de nivel y los autos se ponen mas dificiles.",
            "- Si un auto te toca, perdes una vida y volves al punto de partida.",
            "- Si perdes las 3 vidas, se termina la partida.",
            "- Presiona ESC en cualquier momento para volver al menu principal."
        };

        int y = 4;
        attron(COLOR_PAIR(1));
        for (const string &linea : lineas) {
            mvprintw(y, 3, "%s", linea.c_str());
            y++;
        }
        attroff(COLOR_PAIR(1));

        attron(COLOR_PAIR(5));
        mvprintw(filas - 2, 2, "Presiona ESC o ENTER para volver al menu");
        attroff(COLOR_PAIR(5));

        refresh();
    }

    // ---------------------------------------------------
    // PANTALLA: CREDITOS
    // ---------------------------------------------------
    void dibujarCreditos() {
        erase();
        dibujarTitulo();

        vector<string> creditos = {
            "ULTIMO SEMAFORO",
            "",
            "Un videojuego artistico (Art Game) hecho en C++ con NCURSES",
            "para la materia Audiovision - Catedra Saitta",
            "Universidad Nacional de las Artes (UNA)",
            "",
            "Desarrollado con:",
            "  - Constantes enumeradas (enum class)",
            "  - Clases y objetos (Auto, Jugador, Juego)",
            "  - Vectores y arreglos",
            "  - Numeros aleatorios (rand)",
            "  - La libreria NCURSES para graficos ASCII"
        };

        int y = 4;
        attron(COLOR_PAIR(1));
        for (const string &linea : creditos) {
            int x = max(2, (columnas - static_cast<int>(linea.size())) / 2);
            mvprintw(y, x, "%s", linea.c_str());
            y++;
        }
        attroff(COLOR_PAIR(1));

        attron(COLOR_PAIR(5));
        mvprintw(filas - 2, 2, "Presiona ESC o ENTER para volver al menu");
        attroff(COLOR_PAIR(5));

        refresh();
    }

    // ---------------------------------------------------
    // PANTALLA: JUEGO
    // ---------------------------------------------------
    void dibujarJuego() {
        erase();
        dibujarTitulo();

        // Linea de meta
        attron(COLOR_PAIR(4) | A_BOLD);
        mvprintw(filaMeta - 1, 2, "META");
        for (int x = 0; x < anchoJuego; x++) mvaddch(filaMeta, x + 2, '=');
        attroff(COLOR_PAIR(4) | A_BOLD);

        // Autos
        for (const Auto &a : autos) {
            attron(COLOR_PAIR(3) | A_BOLD);
            mvaddch(a.getY(), a.getX() + 2, a.getSimbolo());
            attroff(COLOR_PAIR(3) | A_BOLD);
        }

        // Jugador
        attron(COLOR_PAIR(4) | A_BOLD);
        mvaddch(jugador.getY(), jugador.getX() + 2, 'A');
        attroff(COLOR_PAIR(4) | A_BOLD);

        // Bordes laterales del area jugable
        attron(COLOR_PAIR(5));
        for (int y = filaMeta; y <= filaInicioJugador; y++) {
            mvaddch(y, 1, '|');
            mvaddch(y, anchoJuego + 2, '|');
        }
        attroff(COLOR_PAIR(5));

        // HUD (encabezado inferior)
        string corazones = "";
        for (int i = 0; i < jugador.getVidas(); i++) corazones += "<3 ";

        attron(COLOR_PAIR(1));
        mvprintw(filas - 3, 2, "VIDAS: %-10s  NIVEL: %d   META: %d/%d",
                 corazones.c_str(), nivel, jugador.getMeta() % META_POR_NIVEL == 0 && jugador.getMeta() != 0 ? META_POR_NIVEL : jugador.getMeta() % META_POR_NIVEL, META_POR_NIVEL);
        attroff(COLOR_PAIR(1));

        attron(COLOR_PAIR(5));
        mvprintw(filas - 2, 2, "Flechas / WASD para moverte   -   ESC para volver al menu");
        attroff(COLOR_PAIR(5));

        refresh();
    }

    void manejarJuego(int tecla) {
        int dx = 0, dy = 0;

        switch (tecla) {
            case KEY_UP:
            case 'w':
            case 'W':
                dy = -1;
                break;
            case KEY_DOWN:
            case 's':
            case 'S':
                dy = 1;
                break;
            case KEY_LEFT:
            case 'a':
            case 'A':
                dx = -1;
                break;
            case KEY_RIGHT:
            case 'd':
            case 'D':
                dx = 1;
                break;
            case 27: // ESC
                estado = EstadoJuego::MENU;
                return;
            default:
                break;
        }

        if (dx != 0 || dy != 0) {
            jugador.mover(dx, dy, anchoJuego - 1, filaMeta, filaInicioJugador);
        }

        // Mover autos
        for (Auto &a : autos) {
            a.mover(anchoJuego - 1);
        }

        // Verificar colisiones
        for (const Auto &a : autos) {
            if (a.getX() == jugador.getX() && a.getY() == jugador.getY()) {
                jugador.perderVida();
                jugador.reiniciarPosicion();
                break;
            }
        }

        // Verificar si llego a la meta
        if (jugador.getY() <= filaMeta) {
            jugador.sumarMeta();
            if (jugador.getMeta() % META_POR_NIVEL == 0) {
                avanzarNivel();
            } else {
                jugador.reiniciarPosicion();
            }
        }

        // Verificar game over
        if (!jugador.estaVivo()) {
            estado = EstadoJuego::GAME_OVER;
        }
    }

    // ---------------------------------------------------
    // PANTALLA: GAME OVER
    // ---------------------------------------------------
    void dibujarGameOver() {
        erase();
        dibujarTitulo();

        string mensaje = "SE ACABARON TUS VIDAS";
        string mensaje2 = "Nivel alcanzado: " + to_string(nivel);

        int x1 = max(2, (columnas - static_cast<int>(mensaje.size())) / 2);
        int x2 = max(2, (columnas - static_cast<int>(mensaje2.size())) / 2);

        attron(COLOR_PAIR(3) | A_BOLD);
        mvprintw(filas / 2 - 1, x1, "%s", mensaje.c_str());
        attroff(COLOR_PAIR(3) | A_BOLD);

        attron(COLOR_PAIR(1));
        mvprintw(filas / 2 + 1, x2, "%s", mensaje2.c_str());
        attroff(COLOR_PAIR(1));

        attron(COLOR_PAIR(5));
        mvprintw(filas - 2, 2, "Presiona ENTER o ESC para volver al menu principal");
        attroff(COLOR_PAIR(5));

        refresh();
    }

    void manejarGameOver(int tecla) {
        if (tecla == '\n' || tecla == KEY_ENTER || tecla == 10 || tecla == 27) {
            estado = EstadoJuego::MENU;
            opcionMenu = 0;
        }
    }

    // ---------------------------------------------------
    // BUCLE PRINCIPAL
    // ---------------------------------------------------
    void run() {
        inicializarNcurses();

        while (!verificarTamanoTerminal()) {
            // espera hasta que la terminal tenga el tamano correcto
        }

        while (estado != EstadoJuego::SALIR) {

            switch (estado) {
                case EstadoJuego::MENU:
                    dibujarMenu();
                    break;
                case EstadoJuego::INSTRUCCIONES:
                    dibujarInstrucciones();
                    break;
                case EstadoJuego::CREDITOS:
                    dibujarCreditos();
                    break;
                case EstadoJuego::JUGANDO:
                    dibujarJuego();
                    break;
                case EstadoJuego::GAME_OVER:
                    dibujarGameOver();
                    break;
                default:
                    break;
            }

            int tecla = getch();

            if (tecla == KEY_RESIZE) {
                getmaxyx(stdscr, filas, columnas);
                if (estado == EstadoJuego::JUGANDO) configurarAreaJuego();
                continue;
            }

            if (tecla == ERR) continue; // nada presionado en este ciclo

            switch (estado) {
                case EstadoJuego::MENU:
                    manejarMenu(tecla);
                    break;
                case EstadoJuego::INSTRUCCIONES:
                    if (tecla == 27 || tecla == '\n' || tecla == KEY_ENTER || tecla == 10) {
                        estado = EstadoJuego::MENU;
                    }
                    break;
                case EstadoJuego::CREDITOS:
                    if (tecla == 27 || tecla == '\n' || tecla == KEY_ENTER || tecla == 10) {
                        estado = EstadoJuego::MENU;
                    }
                    break;
                case EstadoJuego::JUGANDO:
                    manejarJuego(tecla);
                    break;
                case EstadoJuego::GAME_OVER:
                    manejarGameOver(tecla);
                    break;
                default:
                    break;
            }
        }

        endwin();
    }
};

// -----------------------------------------------------------
// MAIN
// -----------------------------------------------------------

int main() {
    Juego juego;
    juego.run();
    return 0;
}
