#include <iostream>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include <cstdlib>

bool gameOver;
const int width = 20;
const int height = 10;

int x, y;
int fruitX, fruitY;
int score;

constexpr int maxTail = 100;
int tailX[maxTail], tailY[maxTail];
int nTail;

enum Direction { STOP = 0, LEFT, RIGHT, UP, DOWN };
Direction dir;

// === Функции для неблокирующего ввода ===
void enableRawMode() {
    termios term;
    tcgetattr(STDIN_FILENO, &term);
    term.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &term);
    fcntl(STDIN_FILENO, F_SETFL, O_NONBLOCK);
}

void disableRawMode() {
    termios term;
    tcgetattr(STDIN_FILENO, &term);
    term.c_lflag |= (ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &term);
}
// ========================================

void Setup() {
    gameOver = false;
    dir = STOP;
    x = width / 2;
    y = height / 2;
    fruitX = rand() % width;
    fruitY = rand() % height;
    score = 0;
}

void Draw() {
    std::cout << "\033[2J\033[H";
    for (int i = 0; i < width + 2; i++) std::cout << "#";
    std::cout << "\n";

    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            if (j == 0) std::cout << "#";

            if (j == x && i == y) {
                switch (dir) {
                    case LEFT: std::cout << "<"; break;
                    case RIGHT: std::cout << ">"; break;
                    case UP: std::cout << "^"; break;
                    case DOWN: std::cout << "V"; break;
                    default: std::cout << "S"; break;
                }
            }
            else {
                bool printTail = false;
                for (int k = 0; k < nTail; k++) {
                    if (tailX[k] == j && tailY[k] == i) {
                        std::cout << "o";
                        printTail = true;
                        break;
                    }
                }
                if (!printTail) {
                    if (j == fruitX && i == fruitY) {
                        std::cout << "@";
                    } else std::cout << " ";
                }
            }

            if (j == width - 1) std::cout << "#";
        }
        std::cout << "\n";
    }

    for (int i = 0; i < width + 2; i++) std::cout << "#";
    std::cout << "\n";
    std::cout << "Score: " << score << "\n";
}

void Input() {
    char buf = 0;
    if (read(STDIN_FILENO, &buf, 1) > 0) {
        if (buf == '\033') {
            char seq[2];
            if (read(STDIN_FILENO, &seq[0], 1) > 0 && read(STDIN_FILENO, &seq[1], 1) > 0) {
                if (seq[0] == '[') {
                    switch (seq[1]) {
                        case 'A': if (dir != DOWN) dir = UP; break;
                        case 'B': if (dir != UP) dir = DOWN; break;
                        case 'C': if (dir != LEFT) dir = RIGHT; break;
                        case 'D': if (dir != RIGHT) dir = LEFT; break;
                        default: break;
                    }
                }
            }
        } else {
            switch (buf) {
                case 'a': if (dir != RIGHT) dir = LEFT; break;
                case 'd': if (dir != LEFT) dir = RIGHT; break;
                case 'w': if (dir != DOWN) dir = UP; break;
                case 's': if (dir != UP) dir = DOWN; break;
                case 'x': gameOver = true; break;
                default: break;
            }
        }
    }
}

void Logic() {
    int prevX = tailX[0];
    int prevY = tailY[0];
    tailX[0] = x;
    tailY[0] = y;

    switch (dir) {
        case LEFT: x--; break;
        case RIGHT: x++; break;
        case UP: y--; break;
        case DOWN: y++; break;
        default: break;
    }

    if (x >= width) x = 0; else if (x < 0) x = width - 1;
    if (y >= height) y = 0; else if (y < 0) y = height - 1;

    for (int i = 0; i < nTail; i++) {
        if (tailX[i] == x && tailY[i] == y) {
            gameOver = true;
        }
    }

    if (x == fruitX && y == fruitY) {
        score += 1;
        nTail++;

        fruitX = rand() % width;
        fruitY = rand() % height;
    }

    for (int i = 1; i < nTail; i++) {
        int prev2X = tailX[i];
        int prev2Y = tailY[i];
        tailX[i] = prevX;
        tailY[i] = prevY;
        prevX = prev2X;
        prevY = prev2Y;
    }
}

int main() {
    enableRawMode();
    Setup();

    while (!gameOver) {
        Draw();
        Input();
        Logic();
        usleep(100000);
    }

    disableRawMode();
    return 0;
}
