#include <GL/glut.h>
#include <iostream>
#include <vector>
#include <algorithm>
#include <cmath>
using namespace std;

// Profundidad definida por defecto
int DEPTH = 4;

class Piece {
    bool isQueen;
    int color; // 0: blancas (jugador), 1: negras (IA), -1: vacío
public:
    Piece(int c = -1, bool q = false) : color(c), isQueen(q) {}
    int getColor() const { return color; }
    bool isQueenPiece() const { return isQueen; }
    void crown() { isQueen = true; }
    bool isEmpty() const { return color == -1; }
    void clear() {
        color = -1;
        isQueen = false;
    }
    void setColor(int c) { color = c; }
};

struct Move {
    int fromRow, fromCol, toRow, toCol;
    bool isCapture;
    int capturedRow, capturedCol;

    Move(int fr = -1, int fc = -1, int tr = -1, int tc = -1, bool cap = false, int cr = -1, int cc = -1)
        : fromRow(fr), fromCol(fc), toRow(tr), toCol(tc), isCapture(cap), capturedRow(cr), capturedCol(cc) {
    }
};

class Board {
public:
    Piece grid[8][8];

    Board() {
        initializeBoard();
    }

    void initializeBoard() {

        for (int r = 0; r < 8; r++) {
            for (int c = 0; c < 8; c++) {
                grid[r][c] = Piece(-1);
            }
        }

        for (int r = 0; r < 3; r++) {
            for (int c = 0; c < 8; c++) {
                if ((r + c) % 2 == 1) {
                    grid[r][c] = Piece(0); // blancas
                }
            }
        }

        for (int r = 5; r < 8; r++) {
            for (int c = 0; c < 8; c++) {
                if ((r + c) % 2 == 1) {
                    grid[r][c] = Piece(1); // negras
                }
            }
        }
    }

    Board(const Board& other) {
        for (int r = 0; r < 8; r++) {
            for (int c = 0; c < 8; c++) {
                grid[r][c] = other.grid[r][c];
            }
        }
    }

    Board& operator=(const Board& other) {
        if (this != &other) {
            for (int r = 0; r < 8; r++) {
                for (int c = 0; c < 8; c++) {
                    grid[r][c] = other.grid[r][c];
                }
            }
        }
        return *this;
    }
};

struct Node {
    Board state;
    Move move;
    int value;
    std::vector<Node*> children;

    Node(const Board& b, const Move& m) : state(b), move(m), value(0) {}
};

// Variables globales
Board currentBoard;
int turn = 0; // 0 jugador (blancas), 1 IA (negras)
bool pieceSelected = false;
int selRow = -1, selCol = -1;
vector<Move> possibleMoves;
bool gameOver = false;
string gameMessage = "Tu turno - Blancas";

vector<Move> getValidMoves(const Board& b, int color) {
    vector<Move> moves;

    for (int r = 0; r < 8; r++) {
        for (int c = 0; c < 8; c++) {
            if (!b.grid[r][c].isEmpty() && b.grid[r][c].getColor() == color) {

                vector<pair<int, int>> directions;

                if (color == 0 || b.grid[r][c].isQueenPiece()) {
                    directions.push_back({ 1, -1 });
                    directions.push_back({ 1, 1 });
                }
                if (color == 1 || b.grid[r][c].isQueenPiece()) {
                    directions.push_back({ -1, -1 });
                    directions.push_back({ -1, 1 });
                }

                for (auto& dir : directions) {
                    int newR = r + dir.first;
                    int newC = c + dir.second;

                    //Movimiento simple
                    if (newR >= 0 && newR < 8 && newC >= 0 && newC < 8 &&
                        b.grid[newR][newC].isEmpty()) {
                        moves.push_back(Move(r, c, newR, newC, false));
                    }

                    // Captura
                    if (newR >= 0 && newR < 8 && newC >= 0 && newC < 8 &&
                        !b.grid[newR][newC].isEmpty() &&
                        b.grid[newR][newC].getColor() != color) {
                    
                        int jumpR = newR + dir.first;
                        int jumpC = newC + dir.second;
                    
                        if (jumpR >= 0 && jumpR < 8 && jumpC >= 0 && jumpC < 8 &&
                            b.grid[jumpR][jumpC].isEmpty()) {
                            moves.push_back(Move(r, c, jumpR, jumpC, true, newR, newC));
                        }
                    }
                }
            }
        }
    }

    return moves;
}

void applyMove(Board& b, const Move& move) {
    // Mover la pieza
    b.grid[move.toRow][move.toCol] = b.grid[move.fromRow][move.fromCol];
    b.grid[move.fromRow][move.fromCol].clear();

    // Si fue captura, eliminar pieza capturada
    if (move.isCapture) {
        b.grid[move.capturedRow][move.capturedCol].clear();
    }

    // Coronar si llega al final
    if (move.toRow == 0 && b.grid[move.toRow][move.toCol].getColor() == 1) {
        b.grid[move.toRow][move.toCol].crown(); 
    }
    if (move.toRow == 7 && b.grid[move.toRow][move.toCol].getColor() == 0) {
        b.grid[move.toRow][move.toCol].crown();
    }
}

int evaluateBoard(const Board& b) {
    int score = 0;
    for (int r = 0; r < 8; r++) {
        for (int c = 0; c < 8; c++) {
            if (!b.grid[r][c].isEmpty()) {
                int pieceValue = b.grid[r][c].isQueenPiece() ? 3 : 1;
                if (b.grid[r][c].getColor() == 1) {
                    score += pieceValue; // IA
                }
                else {
                    score -= pieceValue; // jugador
                }
            }
        }
    }
    return score;
}

Node* buildTree(Board b, int depth, bool isMaximizing) {
    Node* root = new Node(b, Move());

    if (depth == 0) {
        root->value = evaluateBoard(b);
        return root;
    }

    int color = isMaximizing ? 1 : 0;
    vector<Move> moves = getValidMoves(b, color);

    if (moves.empty()) {
        root->value = isMaximizing ? -1000 : 1000;
        return root;
    }

    for (const Move& move : moves) {
        Board tempBoard = b;
        applyMove(tempBoard, move);

        Node* child = buildTree(tempBoard, depth - 1, !isMaximizing);
        child->move = move;
        root->children.push_back(child);
    }

    if (isMaximizing) {
        int best = -1000;
        for (Node* child : root->children)
            best = max(best, child->value);
        root->value = best;
    }
    else {
        int best = 1000;
        for (Node* child : root->children)
            best = min(best, child->value);
        root->value = best;
    }

    return root;
}

// Liberar memoria del árbol recursivamente
void deleteTree(Node* node) {
    if (!node) return;
    for (Node* ch : node->children) {
        deleteTree(ch);
    }
    delete node;
}

Move getBestMove(Board& b) {
    vector<Move> moves = getValidMoves(b, 1);
    if (moves.empty()) return Move();

    // Construir árbol físico y elegir mejor hijo del root
    Node* root = buildTree(b, DEPTH, true);

    Move bestMove = moves[0];
    int bestScore = -100000;
    // Cada hijo del root corresponde a un movimiento del jugador IA
    for (Node* child : root->children) {
        if (child->value > bestScore) {
            bestScore = child->value;
            bestMove = child->move;
        }
    }

    // liberar memoria
    deleteTree(root);

    return bestMove;
}

bool isGameOver() {
    vector<Move> playerMoves = getValidMoves(currentBoard, 0);
    vector<Move> aiMoves = getValidMoves(currentBoard, 1);

    if (playerMoves.empty()) {
        gameMessage = "¡IA gana! No tienes movimientos";
        return true;
    }
    if (aiMoves.empty()) {
        gameMessage = "¡Ganaste! IA sin movimientos";
        return true;
    }

    // Verificar si quedan piezas
    int playerPieces = 0, aiPieces = 0;
    for (int r = 0; r < 8; r++) {
        for (int c = 0; c < 8; c++) {
            if (!currentBoard.grid[r][c].isEmpty()) {
                if (currentBoard.grid[r][c].getColor() == 0) playerPieces++;
                else aiPieces++;
            }
        }
    }

    if (playerPieces == 0) {
        gameMessage = "¡IA gana! Te quedaste sin piezas";
        return true;
    }
    if (aiPieces == 0) {
        gameMessage = "¡Ganaste! IA sin piezas";
        return true;
    }

    return false;
}

void aiMove() {
    if (turn != 1 || gameOver) return;

    Move bestMove = getBestMove(currentBoard);
    if (bestMove.fromRow != -1) {
        applyMove(currentBoard, bestMove);
        cout << "IA mueve de (" << bestMove.fromRow << "," << bestMove.fromCol
            << ") a (" << bestMove.toRow << "," << bestMove.toCol << ")" << endl;
    }

    turn = 0;
    gameOver = isGameOver();
    if (!gameOver) {
        gameMessage = "Tu turno - Blancas";
    }
}

void drawCircle(float cx, float cy, float r) {
    int num_segments = 30;
    glBegin(GL_POLYGON);
    for (int i = 0; i < num_segments; i++) {
        float theta = 2.0f * 3.1415926f * float(i) / float(num_segments);
        float x = r * cos(theta);
        float y = r * sin(theta);
        glVertex2f(x + cx, y + cy);
    }
    glEnd();
}

void drawText(float x, float y, const string& text) {
    glRasterPos2f(x, y);
    for (char c : text) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, c);
    }
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT);
    float cellSize = 2.0f / 8.0f;

    for (int r = 0; r < 8; r++) {
        for (int c = 0; c < 8; c++) {
            if ((r + c) % 2 == 0) {
                glColor3f(1.0f, 1.0f, 1.0f); // casillas claras
            }
            else {
                glColor3f(0.5f, 0.5f, 0.5f); // casillas oscuras
            }

            float x = -1.0f + c * cellSize;
            float y = -1.0f + r * cellSize;

            glBegin(GL_QUADS);
            glVertex2f(x, y);
            glVertex2f(x + cellSize, y);
            glVertex2f(x + cellSize, y + cellSize);
            glVertex2f(x, y + cellSize);
            glEnd();
        }
    }

    if (pieceSelected) {
        possibleMoves = getValidMoves(currentBoard, 0);
        glColor3f(0.0f, 0.0f, 0.6f); // azul para movimientos válidos
        for (const Move& move : possibleMoves) {
            if (move.fromRow == selRow && move.fromCol == selCol) {
                float x = -1.0f + move.toCol * cellSize;
                float y = -1.0f + move.toRow * cellSize;

                glBegin(GL_LINE_LOOP);
                glVertex2f(x + 0.02f, y + 0.02f);
                glVertex2f(x + cellSize - 0.02f, y + 0.02f);
                glVertex2f(x + cellSize - 0.02f, y + cellSize - 0.02f);
                glVertex2f(x + 0.02f, y + cellSize - 0.02f);
                glEnd();
            }
        }
    }

    for (int r = 0; r < 8; r++) {
        for (int c = 0; c < 8; c++) {
            if (!currentBoard.grid[r][c].isEmpty()) {
                float x = -1.0f + (c + 0.5f) * cellSize;
                float y = -1.0f + (r + 0.5f) * cellSize;

                if (currentBoard.grid[r][c].getColor() == 0) {
                    glColor3f(1.0f, 1.0f, 1.0f); // blancas
                }
                else {
                    glColor3f(0.1f, 0.1f, 0.1f); // negras
                }

                drawCircle(x, y, cellSize / 2.5f);

                if (currentBoard.grid[r][c].isQueenPiece()) {
                    glColor3f(1.0f, 1.0f, 0.0f); // dorado
                    drawCircle(x, y, cellSize / 4.0f);
                }

                if (pieceSelected && selRow == r && selCol == c) {
                    glColor3f(1.0f, 0.0f, 0.0f); // rojo
                    glLineWidth(3.0f);

                    glBegin(GL_LINE_LOOP);
                    int num_segments = 30;
                    for (int i = 0; i < num_segments; i++) {
                        float theta = 2.0f * 3.1415926f * float(i) / float(num_segments);
                        float dx = (cellSize / 2.3f) * cos(theta);
                        float dy = (cellSize / 2.3f) * sin(theta);
                        glVertex2f(x + dx, y + dy);
                    }
                    glEnd();

                    glLineWidth(1.0f);
                }
            }
        }
    }

    // Dibujar mensaje de estado
    glColor3f(0.0f, 0.0f, 0.0f);
    drawText(-0.9f, 0.9f, gameMessage);

    glutSwapBuffers();
}

void mouse(int button, int state, int x, int y) {
    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN && turn == 0 && !gameOver) {
        int winW = glutGet(GLUT_WINDOW_WIDTH);
        int winH = glutGet(GLUT_WINDOW_HEIGHT);

        float cellSizeW = winW / 8.0f;
        float cellSizeH = winH / 8.0f;

        int col = (int)(x / cellSizeW);
        int row = 7 - (int)(y / cellSizeH);

        if (row < 0 || row >= 8 || col < 0 || col >= 8) return;

        cout << "Click en fila " << row << ", columna " << col << endl;

        if (!pieceSelected) {
            if (!currentBoard.grid[row][col].isEmpty() &&
                currentBoard.grid[row][col].getColor() == 0) {

                vector<Move> moves = getValidMoves(currentBoard, 0);
                bool hasValidMoves = false;
                for (const Move& move : moves) {
                    if (move.fromRow == row && move.fromCol == col) {
                        hasValidMoves = true;
                        break;
                    }
                }

                if (hasValidMoves) {
                    selRow = row;
                    selCol = col;
                    pieceSelected = true;
                    gameMessage = "Pieza seleccionada - Elige destino";
                }
            }
        }
        else {
            vector<Move> validMoves = getValidMoves(currentBoard, 0);
            Move selectedMove;
            bool validMoveFound = false;

            for (const Move& move : validMoves) {
                if (move.fromRow == selRow && move.fromCol == selCol &&
                    move.toRow == row && move.toCol == col) {
                    selectedMove = move;
                    validMoveFound = true;
                    break;
                }
            }

            if (validMoveFound) {
                applyMove(currentBoard, selectedMove);
                cout << "Movimiento aplicado: (" << selRow << "," << selCol
                    << ") -> (" << row << "," << col << ")" << endl;

                pieceSelected = false;
                turn = 1; // turno de la IA
                gameMessage = "Turno de IA - Negras";

                gameOver = isGameOver();
                if (!gameOver) {
                    glutTimerFunc(500, [](int) { aiMove(); glutPostRedisplay(); }, 0);
                }
            }
            else {
                if (!currentBoard.grid[row][col].isEmpty() &&
                    currentBoard.grid[row][col].getColor() == 0) {
                    selRow = row;
                    selCol = col;
                    gameMessage = "Nueva pieza seleccionada";
                }
                else {
                    pieceSelected = false;
                    gameMessage = "Tu turno - Blancas";
                }
            }
        }

        glutPostRedisplay();
    }
}

void keyboard(unsigned char key, int x, int y) {
    if (key == 'r' || key == 'R') {
        // Reiniciar juego
        currentBoard.initializeBoard();
        turn = 0;
        pieceSelected = false;
        selRow = selCol = -1;
        gameOver = false;
        gameMessage = "Juego reiniciado - Tu turno";
        glutPostRedisplay();
    }
    else if (key == 27) { // ESC
        exit(0);
    }
}

void reshape(int w, int h) {
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-1.0, 1.0, -1.0, 1.0, -1.0, 1.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

int main(int argc, char** argv) {
    cout << "=== JUEGO DE DAMAS ===" << endl;
    cout << "Selecciona el nivel de dificultad (profundidad del algoritmo):" << endl;
    cout << "1. Muy Facil (Profundidad = 1)\n2. Facil (Profundidad = 2)\n3. Normal (Profundidad = 3)\n4. Dificil (Profundidad = 4)\n5. Muy Difícil (Profundidad = 5)\n6. Extremo (Profundidad = 6)\nIngresa tu opcion(1 - 6) o un valor personalizado : " << endl;

    int opcion;
    cin >> opcion;

    if (opcion >= 1 && opcion <= 6) {
        DEPTH = opcion;
    }
    else if (opcion > 6 && opcion <= 10) {
        DEPTH = opcion;
        cout << "¡Advertencia! Profundidad muy alta, la IA puede tardar mucho en responder." << endl;
    }
    else {
        cout << "Opcion invalida. Usando dificultad normal (Profundidad = 3)." << endl;
        DEPTH = 3;
    }

    cout << "\nNivel seleccionado: Profundidad = " << DEPTH << endl;
    cout << "Iniciando juego..." << endl << endl;

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(600, 600);
    glutInitWindowPosition(100, 100);
    glutCreateWindow("Juego de Damas");

    glutDisplayFunc(display);
    glutMouseFunc(mouse);
    glutKeyboardFunc(keyboard);
    glutReshapeFunc(reshape);

    glClearColor(0.2f, 0.3f, 0.4f, 1.0f);
    glEnable(GL_LINE_SMOOTH);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    cout << "Controles:" << endl;
    cout << "- Click izquierdo: Seleccionar y mover piezas" << endl;
    cout << "- R: Reiniciar juego" << endl;
    cout << "- ESC: Salir" << endl;
    cout << "Que empiece el juego!" << endl;

    glutMainLoop();
    return 0;
}
