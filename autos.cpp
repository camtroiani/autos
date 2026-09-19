#include <iostream>
#include <cstdlib>
#include <ctime>
#include <unistd.h>
#include <termios.h>
#include <fcntl.h>

using namespace std;

const int ANCHO = 60;
const int ALTO = 20;

// -----------------------------
// ESTRUCTURA DE LOS AUTOS
// -----------------------------

struct Auto {
    int x;
    int y;
    int direccion;
};

// -----------------------------
// LIMPIAR PANTALLA
// -----------------------------

void limpiarPantalla() {
    cout << "\033[2J\033[H";
}

// -----------------------------
// MENÚ PRINCIPAL
// -----------------------------

void menuInicio() {

    limpiarPantalla();

    cout << "\n\n";
    cout << "============================================================\n";
    cout << "                                                            \n";
    cout << "                 🚗  ULTIMO SEMAFORO  🚗                    \n";
    cout << "                                                            \n";
    cout << "============================================================\n";
    cout << "\n\n";

    cout << "                         JUGAR\n";
    cout << "\n";
    cout << "                       SALIR\n";

    cout << "\n\n";
    cout << "------------------------------------------------------------\n";
    cout << "                  Presiona ENTER para jugar\n";
    cout << "------------------------------------------------------------\n";

    cin.get();
}

// -----------------------------
// DIBUJAR JUEGO
// -----------------------------

void dibujar(Auto auto1, Auto auto2, int jugadorX, int jugadorY) {

    limpiarPantalla();

    cout << "============================================================\n";
    cout << "                    🚗 ULTIMO SEMAFORO 🚗\n";
    cout << "============================================================\n";

    cout << "\nMETA\n";

    cout << "------------------------------------------------------------\n";

    for (int y = 0; y < ALTO; y++) {

        cout << "|";

        for (int x = 0; x < ANCHO; x++) {

            char objeto = ' ';

            // AUTO 1
            if (x == auto1.x && y == auto1.y) {
                objeto = 'C';
            }

            // AUTO 2
            if (x == auto2.x && y == auto2.y) {
                objeto = 'C';
            }

            // JUGADOR
            if (x == jugadorX && y == jugadorY) {
                objeto = 'A';
            }

            cout << objeto;
        }

        cout << "|\n";
    }

    cout << "------------------------------------------------------------\n";
    cout << "VIDAS: ♥ ♥ ♥                 NIVEL: 1                 META: 0/5\n";
    cout << "------------------------------------------------------------\n";

    cout << "\nW A S D para moverte\n";
}

// -----------------------------
// MAIN
// -----------------------------

int main() {

    srand(time(NULL));

    // Primero mostramos el menú
    menuInicio();

    // -------------------------
    // POSICIÓN DEL JUGADOR
    // -------------------------

    int jugadorX = ANCHO / 2;
    int jugadorY = ALTO - 2;

    // -------------------------
    // SOLO DOS AUTOS
    // -------------------------

    Auto auto1;
    auto1.x = 10;
    auto1.y = 5;
    auto1.direccion = 1;

    Auto auto2;
    auto2.x = 40;
    auto2.y = 10;
    auto2.direccion = -1;

    // -------------------------
    // JUEGO
    // -------------------------

    while (true) {

        dibujar(
            auto1,
            auto2,
            jugadorX,
            jugadorY
        );

        // Mover auto 1
        auto1.x += auto1.direccion;

        if (auto1.x >= ANCHO)
            auto1.x = 0;

        // Mover auto 2
        auto2.x += auto2.direccion;

        if (auto2.x < 0)
            auto2.x = ANCHO - 1;

        // Pequeña pausa
        usleep(150000);
    }

    return 0;
}