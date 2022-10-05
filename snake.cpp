#include <vector>
#include <random>
#include <algorithm>
#include <emscripten/val.h>
#include <emscripten.h>
#include <emscripten/html5.h>
#include <string>
#include <iostream>

using emscripten::val;

thread_local val const document = val::global("document");
val canvas;
val ctx;

enum eDirecton {LEFT, RIGHT, UP, DOWN};
auto dir = eDirecton::RIGHT;

struct coordinates {
    bool operator==(coordinates rhs) {
        return (x == rhs.x && y == rhs.y);
    }
    int x, y;
};

bool gameover = false;
int score = 0;

int constexpr width = 20, height = 20, size = 20;
auto constexpr snakeColor = "green", fruitColor = "red";

coordinates fruit;
std::vector<coordinates> body{coordinates{(width / 2), (height / 2)}};
std::random_device dev;
std::mt19937 rng(dev());
std::uniform_int_distribution<std::mt19937::result_type> randX(0, width-1);
std::uniform_int_distribution<std::mt19937::result_type> randY(0, height-1);

EM_BOOL keyCallback(int eventType, EmscriptenKeyboardEvent const *keyEvent, void *userData) {
    switch(keyEvent->key[0]) {
    case 'A':
    case 'a':
        if (dir == LEFT)
            dir = DOWN;
        else if (dir == DOWN)
            dir = RIGHT;
        else if (dir == RIGHT)
            dir = UP;
        else if (dir == UP)
            dir = LEFT;
        break;
    case 'D':
    case 'd':
        if (dir == LEFT)
            dir = UP;
        else if (dir == DOWN)
            dir = LEFT;
        else if (dir == RIGHT)
            dir = DOWN;
        else if (dir == UP)
            dir = RIGHT;
        break;
    case 'X':
    case 'x':
        gameover = true;
        break;
    }
    return EM_TRUE;
}

void algorithm() {
    auto head = body[0];
    switch(dir) {
    case LEFT:
        --head.x;
        break;
    case RIGHT:
        ++head.x;
        break;
    case UP:
        --head.y;
        break;
    case DOWN:
        ++head.y;
    }
    
    if (head.x < 0 or head.x == width || head.y < 0 or head.y == height) {
        gameover = true;
        return;
    }
    body.insert(body.begin(), head);

    if (head.x == fruit.x && head.y == fruit.y) {
        ++score;
        std::cout << "Score: " << score << std::endl;
        
        if (score == height * width) {
            gameover = true;
            return;
        }
        do {
            fruit.x = randX(rng);
            fruit.y = randY(rng);
        }
        while (std::find(body.begin(), body.end(), fruit) != body.end());
    }
    else
        body.pop_back();

    if (std::find(body.begin()+1, body.end(), head) != body.end()) {
        gameover = true;
        return;
    }
}

void draw() {
    ctx.call<void>("clearRect", 0, 0, width * size, height * size);
    ctx.set("fillStyle", snakeColor);
    for (auto const &tile: body) {
        ctx.call<void>("fillRect", tile.x * size, tile.y * size, size, size);
    }
    ctx.set("fillStyle", fruitColor);
    ctx.call<void>("fillRect", fruit.x * size, fruit.y * size, size, size);
}

void mainLoop() {
    draw();
    algorithm();
    if (gameover) {
        std::cout << "Gameover!" << std::endl;
        emscripten_cancel_main_loop();
    }
}

int main() {
    canvas = document.call<val, std::string>("getElementById", "canvas");
    ctx = canvas.call<val, std::string>("getContext", "2d");


    emscripten_set_keypress_callback("#canvas", nullptr, true, keyCallback);
    canvas.set("width", width * size);
    canvas.set("height", height * size);
    
    do {
        fruit.x = randX(rng);
        fruit.y = randY(rng);
    }
    while (std::find(body.begin(), body.end(), fruit) != body.end());
    emscripten_set_main_loop(mainLoop, 6, true);

}