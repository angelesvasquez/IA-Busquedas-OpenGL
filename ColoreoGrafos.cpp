#include <GL/glut.h>
#include <iostream>
#include <vector>
#include <random>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <string>
#include <set>

class grafos {
public:
    std::vector<std::pair<std::vector<int>, int>> grafoinicial;
    int cantidadnode;
    std::vector<int> numerodenodo = { 2,3,4 };
    std::vector<std::string> coloresausar = { "rojo", "verde", "azul" };
    std::vector<int> coloresennumero = { 3,4,5 };

    std::vector<std::pair<int, std::string>> coloracion_final;
    void recibircantnodo(int x);
    void print_cantvecinos();
    bool verificarnodo(int nodo, int vecino);
    void insertar(int nodo, int vecinos);
    void rellenar();
    void descendente(std::vector<std::pair<int, int >>& xd);
    std::vector<std::pair<int, std::string >> Coloracionmasrestrictiva();
    std::vector<std::pair<int, std::string >> Coloracionmasrestringida();
    void dibujar_nodos();
    void dibujar_conexiones();
    void dibujar();
};
//////////////////////////////////////

grafos asd;

void grafos::recibircantnodo(int x) {
    grafoinicial.resize(x);
    cantidadnode = x;
    std::srand(static_cast<unsigned>(std::time(0)));
    if (x > 4) {
        numerodenodo = { 2,3,4 };
    }
    else if (x == 2) {
        numerodenodo = { 1 };
    }
    else {
        for (int i = 2; i < x; i++) {
            numerodenodo.push_back(i);
        }
    }
    for (int m = 0; m < x; m++) {
        int indice_aleatorio = std::rand() % numerodenodo.size();
        int valor_aleatorio = numerodenodo[indice_aleatorio];
        grafoinicial[m].second = valor_aleatorio;

    }
}
void grafos::print_cantvecinos() {
    for (int i = 0; i < grafoinicial.size(); i++) {
        std::cout << "nodo" << i << " ->";
        for (int j = 0; j < grafoinicial[i].first.size(); j++) {
            std::cout << grafoinicial[i].first[j] << " ";
        }
        std::cout << "\n";
    }
}
bool grafos::verificarnodo(int nodo, int vecino) {
    for (int a = 0; a < grafoinicial[nodo].first.size(); a++) {
        if (grafoinicial[nodo].first[a] == vecino) {
            return true;
        }
    }
    return false;
}
void grafos::insertar(int nodo, int vecinos) {
    if (!verificarnodo(nodo, vecinos) && !verificarnodo(vecinos, nodo) && (grafoinicial[nodo].second >= grafoinicial[nodo].first.size() && grafoinicial[vecinos].second >= grafoinicial[vecinos].first.size())) {
        grafoinicial[nodo].first.push_back(vecinos);
        grafoinicial[vecinos].first.push_back(nodo);
    }
}


void grafos::rellenar() {
    if (cantidadnode == 0) { return; }
    for (int nodo = 0; nodo < cantidadnode; nodo++) {
        while (grafoinicial[nodo].first.size() < grafoinicial[nodo].second) {
            int vecino = std::rand() % cantidadnode;
            if (vecino != nodo && vecino < cantidadnode && grafoinicial[vecino].first.size() < (grafoinicial[vecino].second + 1)) {
                insertar(nodo, vecino);
            }
        }
    }
}
std::vector<std::pair<int, std::string >> grafos::Coloracionmasrestrictiva() {
    std::vector <std::pair<int, int>> nodoycantidad;
    nodoycantidad.resize(grafoinicial.size());
    for (int i = 0; i < grafoinicial.size(); i++) {
        nodoycantidad[i].first = i;
        nodoycantidad[i].second = grafoinicial[i].first.size();
    }
    descendente(nodoycantidad);
    std::vector<std::pair<int, std::string >> coloresyaterminados;
    std::vector<std::string> colores_asignados(grafoinicial.size(), "ninguno");
    for (int i = 0; i < nodoycantidad.size(); i++) {

        int nodo_actual = nodoycantidad[i].first;
        std::vector<bool> colores_utilizados(coloresausar.size(), false);
        for (int j = 0; j < grafoinicial[nodo_actual].first.size(); j++) {
            int vecino = grafoinicial[nodo_actual].first[j];
            std::string color_vecino = colores_asignados[vecino];

            std::cout << "  Vecino " << vecino << " tiene color " << color_vecino << std::endl;
            if (color_vecino != "ninguno") {
                for (int k = 0; k < coloresausar.size(); ++k) {
                    if (coloresausar[k] == color_vecino) {
                        colores_utilizados[k] = true;
                        break;
                    }
                }
            }
        }
        for (int m = 0; m < coloresausar.size(); ++m) {
            if (!colores_utilizados[m]) {
                colores_asignados[nodo_actual] = coloresausar[m];
                break;
            }
        }
        coloresyaterminados.push_back({ nodo_actual, colores_asignados[nodo_actual] });
    }

    for (int i = 0; i < coloresyaterminados.size(); i++) {
        std::cout << coloresyaterminados[i].first << coloresyaterminados[i].second << " ";
    }std::cout << "\n";

    return coloresyaterminados;
}
std::vector<std::pair<int, std::string>> grafos::Coloracionmasrestringida() {
    int n = grafoinicial.size();
    std::vector<std::string> colores_asignados(n, "ninguno");
    std::vector<int> saturacion(n, 0);
    std::vector<std::pair<int, int>> nodos_y_grados;

    for (int i = 0; i < n; ++i) {
        nodos_y_grados.push_back({ i, grafoinicial[i].first.size() });
    }

    descendente(nodos_y_grados);
    std::vector<std::set<int>> colores_posibles(n);
    for (int i = 0; i < n; ++i) {
        colores_posibles[i] = { 3, 4, 5 };
    }

    int primer_nodo = nodos_y_grados[0].first;
    colores_asignados[primer_nodo] = "verde";
    colores_posibles[primer_nodo].clear();
    saturacion[primer_nodo] = 1;

    auto seleccionar_nodo = [&]() -> int {
        int mejor_nodo = -1, mejor_saturacion = -1, mejor_grado = -1;
        for (int i = 0; i < n; ++i) {
            if (colores_asignados[i] == "ninguno") {
                if (saturacion[i] > mejor_saturacion ||
                    (saturacion[i] == mejor_saturacion && nodos_y_grados[i].second > mejor_grado)) {
                    mejor_nodo = i;
                    mejor_saturacion = saturacion[i];
                    mejor_grado = nodos_y_grados[i].second;
                }
            }
        }
        return mejor_nodo;
        };

    for (int i = 1; i < n; ++i) {
        int nodo = seleccionar_nodo();

        for (int color : colores_posibles[nodo]) {
            bool color_asignado = false;
            for (int vecino : grafoinicial[nodo].first) {
                if (colores_asignados[vecino] == coloresausar[color - 3]) {
                    color_asignado = true;
                    break;
                }
            }

            if (!color_asignado) {
                colores_asignados[nodo] = coloresausar[color - 3];
                colores_posibles[nodo].clear();
                colores_posibles[nodo].insert(color);
                saturacion[nodo] += 1;

                for (int vecino : grafoinicial[nodo].first) {
                    if (colores_asignados[vecino] == "ninguno") {
                        saturacion[vecino] = std::max(saturacion[vecino], (int)colores_posibles[vecino].size());
                    }
                }
                break;
            }
        }
    }
    std::vector<std::pair<int, std::string>> resultado;
    for (int i = 0; i < n; ++i) {
        resultado.push_back({ i, colores_asignados[i] });
    }
    return resultado;
}
void grafos::descendente(std::vector<std::pair<int, int >>& xd) {
    for (int i = 0; i < xd.size() - 1; i++) {
        for (int j = i; j < xd.size() - 1; j++) {
            if (xd[j].second < xd[j + 1].second) {
                std::pair<int, int> temp = std::make_pair(xd[j].first, xd[j].second);
                xd[j] = xd[j + 1];
                xd[j + 1] = temp;
            }
        }
    }
}

void grafos::dibujar_nodos() {
    float radio = 250.0f;
    float angulo_inicial = 2 * 3.14159 / cantidadnode;

    if (coloracion_final.size() != grafoinicial.size()) {
        std::cerr << " " << std::endl;
        return;
    }

    for (int i = 0; i < grafoinicial.size(); i++) {

        float x = 300 + cos(angulo_inicial * i) * radio;
        float y = 300 + sin(angulo_inicial * i) * radio;
        float size = 20.0f;
        glColor3f(1.0f, 1.0f, 1.0f);
        for (int j = 0; j < coloracion_final.size(); j++) {
            if (i == coloracion_final[j].first) {
                if (coloracion_final[j].second == "rojo") { glColor3f(1.0f, 0.0f, 0.0f); }
                if (coloracion_final[j].second == "verde") { glColor3f(0.0f, 1.0f, 0.0f); }
                if (coloracion_final[j].second == "azul") { glColor3f(0.0f, 0.0f, 1.0f); }
                break;
            }
        }


        glBegin(GL_POLYGON);
        for (int j = 0; j < 360; j++) {
            float angle = 2 * 3.14159 * j / 360;
            glVertex2f(x + cos(angle) * size, y + sin(angle) * size);
        }
        glEnd();


        glColor3f(0.0f, 0.0f, 0.0f);
        glRasterPos2f(x - 5.0f, y - 5.0f);
        std::string num = std::to_string(i);
        for (char c : num) {
            glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, c);
        }
    }
}


void grafos::dibujar_conexiones() {
    float radio = 250.0f;
    float angulo_inicial = 2 * 3.14159 / cantidadnode;

    for (int i = 0; i < grafoinicial.size(); i++) {
        for (int j = 0; j < grafoinicial[i].first.size(); j++) {
            int vecino = grafoinicial[i].first[j];
            float x = 300 + cos(angulo_inicial * i) * radio;
            float y = 300 + sin(angulo_inicial * i) * radio;
            float x2 = 300 + cos(angulo_inicial * vecino) * radio;
            float y2 = 300 + sin(angulo_inicial * vecino) * radio;
            glColor3f(0.0f, 0.0f, 0.0f);
            glBegin(GL_LINES);
            glVertex2f(x, y);
            glVertex2f(x2, y2);
            glEnd();
        }
    }
}

void grafos::dibujar() {
    std::cout << "Dibujando..." << std::endl;
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    dibujar_nodos();
    dibujar_conexiones();
    glFlush();
}

void keyPressed(unsigned char key, int x, int y) {
    if (key == '1') {
        asd.coloracion_final = asd.Coloracionmasrestrictiva(); \

            for (int i = 0; i < asd.coloracion_final.size(); i++) {
                std::cout << asd.coloracion_final[i].first << asd.coloracion_final[i].second << " ";
            }std::cout << "\n";
    }
    if (key == '2') {
        asd.coloracion_final = asd.Coloracionmasrestringida();
        for (int i = 0; i < asd.coloracion_final.size(); i++) {
            std::cout << asd.coloracion_final[i].first << asd.coloracion_final[i].second << " ";
        }std::cout << "\n";
    }

    glutPostRedisplay();
}

int main(int argc, char** argv) {
    std::cout << "Dame el numero de nodos para dibujar\n";
    int asasd;
    std::cin >> asasd;
    asd.recibircantnodo(asasd);
    asd.rellenar();
    asd.print_cantvecinos();

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
    glutInitWindowSize(600, 600);
    glutCreateWindow("Coloracion de Grafos");
    glClearColor(0.5f, 0.0f, 0.5f, 1.0f);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, 600, 600, 0, -1, 1);
    glutKeyboardFunc(keyPressed);
    glutDisplayFunc([]() { asd.dibujar();  });



    glutMainLoop();

    return 0;
}
