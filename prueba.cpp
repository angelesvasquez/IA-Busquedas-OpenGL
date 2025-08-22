#include <GL/freeglut.h>
#include <iostream>
#include <vector>
#include <queue>
#include <stack>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <algorithm>
using namespace std;

const int N = 20;  // tamaño de la grilla
int grid[N][N];
bool visited[N][N];
//pair<int, bool> grid[N][N];
pair<int, int> parent[N][N];
pair<int, int> startNode = { 0,0 };
pair<int, int> goalNode = { N - 1, N - 1 };
bool pathFound = false;

enum CellType { FREE = 0, OBSTACLE = 1, VISITED = 2, PATH = 3, START = 5, GOAL = 5 };

int dx[4] = { 1,-1,0,0 };
int dy[4] = { 0,0,1,-1 };

void resetGrid(bool keepObstacles = true) {
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            if (grid[i][j] != OBSTACLE || !keepObstacles)
                grid[i][j] = FREE;
            visited[i][j] = false;
            parent[i][j] = { -1,-1 };
        }
    }
    grid[startNode.first][startNode.second] = START;
    grid[goalNode.first][goalNode.second] = GOAL;
    pathFound = false;
}

void drawSquare(int x, int y, float r, float g, float b) {
    glColor3f(r, g, b);
    glBegin(GL_QUADS);
    glVertex2f(x, y);
    glVertex2f(x + 1, y);
    glVertex2f(x + 1, y + 1);
    glVertex2f(x, y + 1);
    glEnd();
}

void drawGrid() {
    glClear(GL_COLOR_BUFFER_BIT);
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            switch (grid[i][j]) {
            case FREE: drawSquare(j, N - 1 - i, 1, 1, 1); break;
            case OBSTACLE: drawSquare(j, N - 1 - i, 0.3, 0.3, 0.3); break;
            case VISITED: drawSquare(j, N - 1 - i, 0, 0, 1); break;
            case PATH: drawSquare(j, N - 1 - i, 1, 0, 0); break;
            case START | GOAL: drawSquare(j, N - 1 - i, 0, 0, 0); break;
            }
        }
    }
    glutSwapBuffers();
}

bool inBounds(int x, int y) {
    return x >= 0 && x < N && y >= 0 && y < N;
}

// --- reconstrucción del camino ---
void reconstructPath() {
    pair<int, int> cur = goalNode;
    while (cur != startNode && parent[cur.first][cur.second].first != -1) {
        if (grid[cur.first][cur.second] != GOAL)
            grid[cur.first][cur.second] = PATH;
        cur = parent[cur.first][cur.second];
    }
    pathFound = true;
}

// --- BFS ---
void BFS() {
    resetGrid();
    queue<pair<int, int>>q;
    q.push(startNode);
    visited[startNode.first][startNode.second] = true;
    while (!q.empty()) {
        auto node = q.front();
        q.pop();
        int x = node.first;
        int y = node.second;

        if (node == goalNode) {
            reconstructPath();
            return;
        }
        for (int k = 0; k < 4; k++) {
            int nx = x + dx[k], ny = y + dy[k];
            if (inBounds(nx, ny) && !visited[nx][ny] && grid[nx][ny] != OBSTACLE) {
                visited[nx][ny] = true;
                parent[nx][ny] = { x,y };
                if (grid[nx][ny] != GOAL) grid[nx][ny] = VISITED;
                q.push({ nx,ny });
            }
        }
    }
    cout << "[BFS] No se encontró camino\n";
}

// --- DFS ---
void DFS() {
    resetGrid();
    stack<pair<int, int>>st;
    st.push(startNode);
    visited[startNode.first][startNode.second] = true;
    while (!st.empty()) {
        auto node = st.top();
        st.pop();
        int x = node.first;
        int y = node.second;
        if (node == goalNode) {
            reconstructPath();
            return;
        }
        for (int k = 0; k < 4; k++) {
            int nx = x + dx[k], ny = y + dy[k];
            if (inBounds(nx, ny) && !visited[nx][ny] && grid[nx][ny] != OBSTACLE) {
                visited[nx][ny] = true;
                parent[nx][ny] = { x,y };
                if (grid[nx][ny] != GOAL) grid[nx][ny] = VISITED;
                st.push({ nx,ny });
            }
        }
    }
    cout << "[DFS] No se encontró camino\n";
}

// --- A* ---
struct Node { int x, y; double f, g, h; };
struct cmp { bool operator()(Node a, Node b) { return a.f > b.f; } };

double manhattan(int x, int y) { return abs(x - goalNode.first) + abs(y - goalNode.second); }

void Astar() {
    resetGrid();
    priority_queue<Node, vector<Node>, cmp>pq;
    pq.push({ startNode.first,startNode.second,0,0,0 });
    visited[startNode.first][startNode.second] = true;
    while (!pq.empty()) {
        Node cur = pq.top(); pq.pop();
        if (make_pair(cur.x, cur.y) == goalNode) { reconstructPath(); return; }
        for (int k = 0; k < 4; k++) { // 4 direcciones
            int nx = cur.x + dx[k], ny = cur.y + dy[k];
            if (inBounds(nx, ny) && grid[nx][ny] != OBSTACLE) {
                double g2 = cur.g + 1;
                double h2 = manhattan(nx, ny);
                double f2 = g2 + h2;
                if (!visited[nx][ny] || g2 < cur.g) {
                    visited[nx][ny] = true;
                    parent[nx][ny] = { cur.x,cur.y };
                    if (grid[nx][ny] != GOAL) grid[nx][ny] = VISITED;
                    pq.push({ nx,ny,f2,g2,h2 });
                }
            }
        }
    }
    cout << "[A*] No se encontró camino\n";
}

// --- Hill Climbing ---
void HillClimbing() {
    resetGrid();
    pair<int, int> cur = startNode;
    visited[cur.first][cur.second] = true;
    while (cur != goalNode) {
        vector<pair<int, int>>neighbors;
        for (int k = 0; k < 4; k++) {
            int nx = cur.first + dx[k], ny = cur.second + dy[k];
            if (inBounds(nx, ny) && grid[nx][ny] != OBSTACLE && !visited[nx][ny]) {
                neighbors.push_back({ nx,ny });
            }
        }
        if (neighbors.empty()) {
            cout << "[Hill Climbing] Atascado, no hay camino\n";
            return;
        }
        auto best = neighbors[0];
        double bestH = manhattan(best.first, best.second);
        for (auto nb : neighbors) {
            double h = manhattan(nb.first, nb.second);
            if (h < bestH) {
                best = nb; bestH = h;
            }
        }
        parent[best.first][best.second] = cur;
        if (grid[best.first][best.second] != GOAL) grid[best.first][best.second] = VISITED;
        visited[best.first][best.second] = true;
        cur = best;
    }
    reconstructPath();
}

void keyboard(unsigned char key, int, int) {
    if (key == '1') BFS();
    else if (key == '2') Astar();
    else if (key == '3') HillClimbing();
    else if (key == '4') DFS();
    glutPostRedisplay();
}

void initObstacles() {
    srand(time(0));
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            if ((rand() % 100) < 30 && !(i == 0 && j == 0) && !(i == N - 1 && j == N - 1))
                grid[i][j] = OBSTACLE;
        }
    }
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(600, 600);
    glutCreateWindow("Busquedas (1=BFS, 2=A*, 3=HillClimbing, 4=DFS)");
    glMatrixMode(GL_PROJECTION);
    gluOrtho2D(0, N, 0, N);

    initObstacles();
    resetGrid();

    glutDisplayFunc(drawGrid);
    glutKeyboardFunc(keyboard);
    glutMainLoop();
    return 0;
}
