#include <GL/glut.h>
#include <cmath>
#include <vector>
#include <queue>
#include <iostream>
#include <utility>
#include <stack>
#include <algorithm>
using namespace std;

const int GRID_SIZE = 100;
const float NODE_SIZE = 0.2f;

struct Node {
    int x, y;
    bool isActive = true;
    bool isStart = false;
    bool isEnd = false;
    bool isPath = false;
    bool isExplored = false;
};

struct NodeA {
    int x, y;
    double f, g, h;
    bool operator>(const NodeA& other) const { return f > other.f; }
};

Node grid[GRID_SIZE][GRID_SIZE];
bool startSelected = false;
pair<int, int> startNode, endNode;
int porcentaje;

// Direcciones (8 movimientos: ortogonales y diagonales)
int dx[8] = { 0, 1, 0, -1, 1, 1, -1, -1 };
int dy[8] = { 1, 0, -1, 0, 1, -1, 1, -1 };
double cost[8] = { 1, 1, 1, 1, sqrt(2), sqrt(2), sqrt(2), sqrt(2) };

void generarObstaculos() {
    cout << "Ingrese el porcentaje de nodos a eliminar (0-100): ";
    cin >> porcentaje;
    srand(time(0));
    int totalNodos = GRID_SIZE * GRID_SIZE;
    int nodosAEliminar = totalNodos * porcentaje / 100;

    int count = 0;
    while (count < nodosAEliminar) {
        int i = rand() % GRID_SIZE;
        int j = rand() % GRID_SIZE;

        if (grid[i][j].isActive && !grid[i][j].isStart && !grid[i][j].isEnd) {
            grid[i][j].isActive = false;
            count++;
        }
    }
    cout << "Obstaculos eliminados: " << nodosAEliminar << " nodos (" << porcentaje << "%)\n";
}

void drawCircle(float cx, float cy, float r, float red, float green, float blue) {
    glColor3f(red, green, blue);
    glBegin(GL_TRIANGLE_FAN);
    for (int i = 0; i < 360; i++) {
        float theta = i * 3.1415926f / 180;
        glVertex2f(cx + cos(theta) * r, cy + sin(theta) * r);
    }
    glEnd();
}

void drawGrid() {

    glColor3f(0.7f, 0.7f, 0.7f);
    glBegin(GL_LINES);
    for (int i = 0; i < GRID_SIZE; i++) {
        for (int j = 0; j < GRID_SIZE; j++) {
            if (!grid[i][j].isActive) continue;
            for (int d = 0; d < 8; d++) {
                int nx = i + dx[d], ny = j + dy[d];
                if (nx >= 0 && nx < GRID_SIZE && ny >= 0 && ny < GRID_SIZE && grid[nx][ny].isActive) {
                    glVertex2f(i + 0.5f, j + 0.5f);
                    glVertex2f(nx + 0.5f, ny + 0.5f);
                }
            }
        }
    }
    glEnd();

    for (int i = 0; i < GRID_SIZE; i++) {
        for (int j = 0; j < GRID_SIZE; j++) {
            if (!grid[i][j].isActive) continue;
            if (grid[i][j].isStart)
                drawCircle(i + 0.5f, j + 0.5f, NODE_SIZE, 0.0f, 1.0f, 0.0f); // verde
            else if (grid[i][j].isEnd)
                drawCircle(i + 0.5f, j + 0.5f, NODE_SIZE, 1.0f, 0.0f, 0.0f); // rojo
            else if (grid[i][j].isPath)
                drawCircle(i + 0.5f, j + 0.5f, NODE_SIZE, 1.0f, 0.5f, 0.0f); // naranja
            else if (grid[i][j].isExplored)
                drawCircle(i + 0.5f, j + 0.5f, NODE_SIZE, 0.0f, 0.7f, 1.0f); // azul brillante
            else
                drawCircle(i + 0.5f, j + 0.5f, NODE_SIZE, 0.8f, 0.8f, 0.8f); // gris claro
        }
    }
}

void clearPathOnly() {
    for (int i = 0; i < GRID_SIZE; ++i) {
        for (int j = 0; j < GRID_SIZE; ++j) {
            grid[i][j].isExplored = false;
            grid[i][j].isPath = false;
        }
    }
}


//Búsqueda por amplitud 
void bfsPath() {
    vector<vector<bool>> visited(GRID_SIZE, vector<bool>(GRID_SIZE, false));
    vector<vector<pair<int, int>>> parent(GRID_SIZE, vector<pair<int, int>>(GRID_SIZE, make_pair(-1, -1)));
    queue<pair<int, int>> q; q.push(startNode);
    visited[startNode.first][startNode.second] = true;
    bool found = false;
    while (!q.empty() && !found) {
        pair<int, int> current = q.front();
        q.pop(); int x = current.first;
        int y = current.second;
        for (int d = 0; d < 8; d++) {
            int nx = x + dx[d];
            int ny = y + dy[d];
            if (nx >= 0 && nx < GRID_SIZE && ny >= 0 && ny < GRID_SIZE && !visited[nx][ny] && grid[nx][ny].isActive) {
                visited[nx][ny] = true;
                grid[nx][ny].isExplored = true;
                parent[nx][ny] = make_pair(x, y);
                q.push(make_pair(nx, ny));
                if (nx == endNode.first && ny == endNode.second) {
                    found = true;
                    break;
                }
            }
        }
    } if (found) {
        int cx = endNode.first;
        int cy = endNode.second;
        while (!(cx == startNode.first && cy == startNode.second)) {
            if (!(cx == endNode.first && cy == endNode.second) && !(cx == startNode.first && cy == startNode.second)) {
                grid[cx][cy].isPath = true;
            }
            pair<int, int> p = parent[cx][cy];
            cx = p.first; cy = p.second;
        }
        cout << "Camino encontrado con BFS.\n";
    }
    else {
        cout << "No se encontro camino con BFS.\n";
    }
}
//Búsqueda por profundidad 
void dfsPath() {
    vector<vector<bool>> visited(GRID_SIZE, vector<bool>(GRID_SIZE, false));
    vector<vector<pair<int, int>>> parent(GRID_SIZE, vector<pair<int, int>>(GRID_SIZE, make_pair(-1, -1)));
    stack<pair<int, int>> st;
    st.push(startNode);
    visited[startNode.first][startNode.second] = true;
    bool found = false;
    while (!st.empty() && !found) {
        pair<int, int> current = st.top();
        st.pop();
        int x = current.first;
        int y = current.second;
        for (int d = 0; d < 8; d++) {
            int nx = x + dx[d];
            int ny = y + dy[d];
            if (nx >= 0 && nx < GRID_SIZE && ny >= 0 && ny < GRID_SIZE && !visited[nx][ny] && grid[nx][ny].isActive) {
                visited[nx][ny] = true;
                grid[nx][ny].isExplored = true;
                parent[nx][ny] = make_pair(x, y);
                st.push(make_pair(nx, ny));
                if (nx == endNode.first && ny == endNode.second) {
                    found = true; break;
                }
            }
        }
    } if (found) {
        int cx = endNode.first;
        int cy = endNode.second;
        while (!(cx == startNode.first && cy == startNode.second)) {
            if (!(cx == endNode.first && cy == endNode.second) && !(cx == startNode.first && cy == startNode.second)) {
                grid[cx][cy].isPath = true;
            }
            pair<int, int> p = parent[cx][cy];
            cx = p.first; cy = p.second;
        }
        cout << "Camino encontrado con DFS.\n";
    }
    else {
        cout << "No se encontro camino con DFS.\n";
    }
}

double euclidean(int x, int y) {
    int dx = x - endNode.first;
    int dy = y - endNode.second;
    return sqrt(dx * dx + dy * dy);
}
//Búsqueda por A*
void aStarPath() {
    vector<vector<double>> gCost(GRID_SIZE, vector<double>(GRID_SIZE, 1e9));
    vector<vector<pair<int, int>>> parent(GRID_SIZE, vector<pair<int, int>>(GRID_SIZE, make_pair(-1, -1)));
    vector<vector<bool>> visited(GRID_SIZE, vector<bool>(GRID_SIZE, false));

    priority_queue<NodeA, vector<NodeA>, greater<NodeA>> pq;

    int sx = startNode.first, sy = startNode.second;
    int gx = endNode.first, gy = endNode.second;

    gCost[sx][sy] = 0;
    pq.push({ sx, sy, euclidean(sx, sy), 0, euclidean(sx, sy) });

    bool found = false;

    while (!pq.empty() && !found) {
        NodeA cur = pq.top();
        pq.pop();

        if (visited[cur.x][cur.y]) continue;
        visited[cur.x][cur.y] = true;
        grid[cur.x][cur.y].isExplored = true;

        if (cur.x == gx && cur.y == gy) {
            found = true;
            break;
        }

        for (int d = 0; d < 8; d++) {
            int nx = cur.x + dx[d];
            int ny = cur.y + dy[d];
            if (nx >= 0 && nx < GRID_SIZE && ny >= 0 && ny < GRID_SIZE &&
                grid[nx][ny].isActive && !visited[nx][ny]) {

                double tentativeG = gCost[cur.x][cur.y] + cost[d];
                if (tentativeG < gCost[nx][ny]) {
                    gCost[nx][ny] = tentativeG;
                    double h = euclidean(nx, ny);
                    double f = tentativeG + h;
                    parent[nx][ny] = make_pair(cur.x, cur.y);
                    pq.push({ nx, ny, f, tentativeG, h });
                }
            }
        }
    }

    if (found) {
        int cx = gx, cy = gy;
        while (!(cx == sx && cy == sy)) {
            if (!(cx == gx && cy == gy) && !(cx == sx && cy == sy)) {
                grid[cx][cy].isPath = true;
            }
            pair<int, int> p = parent[cx][cy];
            cx = p.first;
            cy = p.second;
        }
        cout << "Camino encontrado con A*.\n";
    }
    else {
        cout << "No se encontro camino con A*.\n";
    }
}

void hillClimbingPath() {
    vector<vector<bool>> visited(GRID_SIZE, vector<bool>(GRID_SIZE, false));
    vector<vector<pair<int, int>>> parent(GRID_SIZE, vector<pair<int, int>>(GRID_SIZE, { -1, -1 }));

    int sx = startNode.first, sy = startNode.second;
    int gx = endNode.first, gy = endNode.second;
    int exploredCount = 0;
    bool found = false;

    stack<pair<int, int>> st;
    st.push({ sx, sy });
    visited[sx][sy] = true;

    while (!st.empty() && !found) {
        pair<int, int> current = st.top();
        int x = current.first;
        int y = current.second;

        if (x == gx && y == gy) {
            found = true;
            break;
        }

        vector<pair<double, pair<int, int>>> neighbors;
        for (int d = 0; d < 8; d++) {
            int nx = x + dx[d], ny = y + dy[d];
            if (nx >= 0 && nx < GRID_SIZE && ny >= 0 && ny < GRID_SIZE &&
                !visited[nx][ny] && grid[nx][ny].isActive) {

                double h = euclidean(nx, ny);

                if (!grid[nx][ny].isExplored) {
                    grid[nx][ny].isExplored = true;
                    exploredCount++;
                }

                neighbors.push_back({ h, {nx, ny} });
            }
        }

        sort(neighbors.begin(), neighbors.end());

        bool foundNeighbor = false;
        for (auto& nb : neighbors) {
            int nx = nb.second.first, ny = nb.second.second;
            if (!visited[nx][ny]) {
                visited[nx][ny] = true;
                parent[nx][ny] = { x, y };
                st.push({ nx, ny });
                foundNeighbor = true;
                break;
            }
        }

        if (!foundNeighbor) {
            st.pop();
        }
    }

    if (found) {
        int cx = gx, cy = gy;
        while (!(cx == sx && cy == sy)) {
            if (!(cx == gx && cy == gy) && !(cx == sx && cy == sy)) {
                grid[cx][cy].isPath = true;
            }
            pair<int, int> p = parent[cx][cy];
            cx = p.first;
            cy = p.second;
        }
        cout << "[HillClimbing] Camino encontrado.\n";
    }
    else {
        cout << "[HillClimbing] No se encontro camino.\n";
    }
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT);
    drawGrid();
    glutSwapBuffers();
}

void reshape(int w, int h) {
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(-1, GRID_SIZE, -1, GRID_SIZE);
    glMatrixMode(GL_MODELVIEW);
}

void resetGrid() {
    for (int i = 0; i < GRID_SIZE; i++) {
        for (int j = 0; j < GRID_SIZE; j++) {
            grid[i][j].isExplored = false;
            grid[i][j].isStart = false;
            grid[i][j].isEnd = false;
            grid[i][j].isPath = false;

        }
    }
    startSelected = false;
}

void keyboard(unsigned char key, int x, int y) {
    if (!grid[startNode.first][startNode.second].isStart ||
        !grid[endNode.first][endNode.second].isEnd) return;

    switch (key) {
    case '1': // BFS
        clearPathOnly();
        bfsPath();
        grid[startNode.first][startNode.second].isStart = true;
        grid[endNode.first][endNode.second].isEnd = true;
        glutPostRedisplay();
        break;
    case '2': // DFS
        clearPathOnly();
        dfsPath();
        grid[startNode.first][startNode.second].isStart = true;
        grid[endNode.first][endNode.second].isEnd = true;
        glutPostRedisplay();
        break;
    case '3': // A*
        clearPathOnly();
        aStarPath();
        grid[startNode.first][startNode.second].isStart = true;
        grid[endNode.first][endNode.second].isEnd = true;
        glutPostRedisplay();
        break;
    case '4': // HillClimbing
        clearPathOnly();
        hillClimbingPath();
        grid[startNode.first][startNode.second].isStart = true;
        grid[endNode.first][endNode.second].isEnd = true;
        glutPostRedisplay();
        break;
    case 'r':
    case 'R':
        resetGrid();
        generarObstaculos();
        glutPostRedisplay();
        break;
    }
}


void mouse(int button, int state, int x, int y) {
    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
        int mx = (x * GRID_SIZE) / glutGet(GLUT_WINDOW_WIDTH);
        int my = GRID_SIZE - (y * GRID_SIZE) / glutGet(GLUT_WINDOW_HEIGHT);

        if (!grid[mx][my].isActive) return;

        if (startSelected && grid[endNode.first][endNode.second].isEnd) {
            resetGrid();
        }

        if (!startSelected) {
            startNode = make_pair(mx, my);
            grid[mx][my].isStart = true;
            startSelected = true;
            cout << "Inicio: (" << mx << ", " << my << ")\n";
        }
        else {
            endNode = make_pair(mx, my);
            grid[mx][my].isEnd = true;
            cout << "Destino: (" << mx << ", " << my << ")\n";
            bfsPath();
        }

        glutPostRedisplay();
    }
}

int main(int argc, char** argv) {
    for (int i = 0; i < GRID_SIZE; i++) {
        for (int j = 0; j < GRID_SIZE; j++) {
            grid[i][j] = { i, j, true, false, false, false };
        }
    }

    generarObstaculos();

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(1200, 800);
    glutCreateWindow("Busqueda en matriz 100x100 con obstaculos");

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutMouseFunc(mouse);
    glutKeyboardFunc(keyboard);

    glClearColor(1.0, 1.0, 1.0, 1.0);
    glutMainLoop();
    return 0;
}
