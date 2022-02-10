#include <SDL2/SDL.h>
#include <iostream>
#include <list>
#include <string>
#include <cstdlib>
#include <ctime>
#include <vector>
//For windows
#include <windows.h>

using namespace std;

#define TARGET_FPS 60 
#define MS_PER_FRAME 1000/ TARGET_FPS
#define MS_PER_UPDATE 1000/12
#define SCREEN_WIDTH 640 
#define SCREEN_HEIGHT 480 
#define WALL_THICKNESS 20
#define SNAKEPIECE_WIDTH WALL_THICKNESS
#define SNAKEPIECE_HEIGHT WALL_THICKNESS
#define INIT_TIME 70
#define GRID_START_X WALL_THICKNESS
#define GRID_START_Y wALL_THICKNESS
#define INTERVAL_X SNAKEPIECEWIDTH
#define INTERVAL_Y SNAKEPIECEHEIGHT
#define GRID_COL_NUM (SCREEN_WIDTH - WALL_THICKNESS*2)/SNAKEPIECE_WIDTH
#define GRID_ROW_NUM (SCREEN_HEIGHT - WALL_THICKNESS*2)/SNAKEPIECE_HEIGHT
#define WALL 1

typedef enum{
    UP,
    DOWN,
    LEFT,
    RIGHT,
} DIRECTION;

typedef enum{
    VERTICAL,
    HORIZONTAL
} PIECE_DIR;

typedef struct SnakePiece{
    int row;
    int col;
    PIECE_DIR dir;
} SnakePiece;

list<SnakePiece> snake;
vector<SnakePiece> turning_points;


DIRECTION curr_dir = RIGHT;
DIRECTION buf_dir = RIGHT;
SDL_Event buf;

typedef struct apple{
    int row;
    int col;
} Apple;

SDL_Window *win = NULL;
SDL_Renderer *ren = NULL;

bool isRunning = true;
bool can_move = true;
bool canWiggle = true;
Apple apple;

int delay_time = INIT_TIME;
int score = 0;

int init();
void close();
void handle_input();
void draw_wall();
void init_snake();
void draw_snake();
void move_snake();
void new_apple();
void draw_apple();
bool isRepeated(Apple a);
int check_next_block(SnakePiece next);
void change_direction(SDL_Event);

int main(int argv, char** args){
    //For windows console handle
    HWND windowhandle = GetConsoleWindow();
    ShowWindow(windowhandle, SW_HIDE);
    //
    if(init()){
        cout << "SDL init failed" << endl;
        close();
    }
    init_snake();
    new_apple();
    //Variables to keep track of frame rate
    Uint32 previous = SDL_GetTicks();
    Uint32 lag = 0;
    //Game Loop
    while(isRunning){
        Uint32 current = SDL_GetTicks();
        Uint32 elapsed = current - previous;
        previous = current;
        lag += elapsed;
        //Set draw color to black before any game loop iteration
        //Set Color -> Clear the whole render with the drawing color
        SDL_SetRenderDrawColor(ren, 0, 0, 0, 255); //(0,0,0) is all black
        SDL_RenderClear(ren);

        handle_input();

        //I cannot understand this lag loop
        while(lag >= MS_PER_UPDATE){
            move_snake();
            lag -= MS_PER_UPDATE;
            canWiggle = true;
        }


        draw_snake();
        draw_apple();
        draw_wall();


        SDL_RenderPresent(ren);
        Uint32 frame_end = SDL_GetTicks();
        Uint32 frame_duration = frame_end - current;
        if(frame_duration < MS_PER_FRAME){
            SDL_Delay(MS_PER_FRAME - frame_duration);
        }
    }
    cout << "Game Ends" << endl;
    cout << "Score: " << score << endl;
    SDL_Delay(1000);
    close();
    return 0;
}

//If error return nonzero value
int init(){
    //Init SDL
    if(SDL_Init(SDL_INIT_VIDEO) != 0){
        cout <<"SDL init error : " << SDL_GetError() << endl;
        return 1;
    }
    //Create Window
    win = SDL_CreateWindow("Snake Game", 100, 100, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if(win == nullptr){
        cout <<"SDL create window error : " << SDL_GetError() << endl;
        return 1;
    }
    ren = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (ren == nullptr){
        SDL_DestroyWindow(win);
        std::cout << "SDL_CreateRenderer Error: " << SDL_GetError() << std::endl;
        return 1;
    }
    return 0;
}

//Clean up SDL objects
void close(){
    SDL_DestroyRenderer(ren);
    SDL_DestroyWindow(win);
    SDL_Quit();
}

void handle_input(){
    SDL_Event e;
    while(SDL_PollEvent(&e)){
        if(e.type == SDL_QUIT){
            isRunning = false;
        }else if(e.type == SDL_KEYDOWN &&  (e.key.keysym.sym == SDLK_RIGHT ||
                                            e.key.keysym.sym == SDLK_LEFT  ||
                                            e.key.keysym.sym == SDLK_UP    ||
                                            e.key.keysym.sym == SDLK_DOWN)){
                                                change_direction(e);
                                            }
    }
    /*
    const Uint8* keystates = SDL_GetKeyboardState(NULL);
    if(keystates[SDL_SCANCODE_UP] && curr_dir != DOWN){
        curr_dir = UP;
    }else if(keystates[SDL_SCANCODE_DOWN] && curr_dir != UP){
        curr_dir = DOWN;
    }else if(keystates[SDL_SCANCODE_LEFT] && curr_dir != RIGHT){
        curr_dir = LEFT;
    }else if(keystates[SDL_SCANCODE_RIGHT] && curr_dir != LEFT){
        curr_dir = RIGHT;
    }
    */
    return;
}

void draw_wall(){
    SDL_SetRenderDrawColor(ren, 30, 144, 255, 255);
    //SDL_Rect 
    //(x, y) specifies the top left location of the rect
    SDL_Rect block = {
        .x = 0,
        .y = 0,
        .w = WALL_THICKNESS,
        .h = SCREEN_HEIGHT
    };
    //Draw left wall
    SDL_RenderFillRect(ren, &block);
    //Right wall
    block.x = SCREEN_WIDTH - WALL_THICKNESS;
    SDL_RenderFillRect(ren, &block);
    //Up wall
    block.x = 0;
    block.y = 0;
    block.w = SCREEN_WIDTH;
    block.h = WALL_THICKNESS;
    SDL_RenderFillRect(ren, &block);
    //Down wall
    block.y = SCREEN_HEIGHT - WALL_THICKNESS;
    SDL_RenderFillRect(ren, &block);
}

void init_snake(){
    //Initial length of the snake will be 4
    //push them into the queue in reverse order
    //(3,3), (3,4), (3,5), (3,6)
    //Initial direction will be RIGHT
    SnakePiece s = {.row = 3, .col = 3};
    s.dir = HORIZONTAL;
    curr_dir = RIGHT;
    snake.push_back(s);
    s.col = 4;
    snake.push_back(s);
    s.col = 5;
    snake.push_back(s);
    s.col = 6;
    snake.push_back(s);
    buf.key.keysym.sym = SDLK_ESCAPE;
    return;
}

void draw_snake(){
    //iterate through the whole snake and draw
    SDL_Rect block = {
        .w = SNAKEPIECE_WIDTH + SNAKEPIECE_WIDTH/10,
        .h = SNAKEPIECE_HEIGHT + SNAKEPIECE_HEIGHT/10,
    };
    int j = 0;
    static int wiggle = 0;
    bool is_turning_point = false;
    for(list<SnakePiece>::iterator it = snake.begin(); it !=  snake.end(); it++){
        //check is turning point, change snakepiece.direction
        for(vector<SnakePiece>::iterator i = turning_points.begin(); i != turning_points.end(); i++){
            if(it->col == i->col && it->row == i->row){
                it->dir = i->dir;
                is_turning_point = true;
                //Delete the turning point if its the tail
                if(it->row == snake.back().row && it->col == snake.back().col){
                    turning_points.erase(i);
                    break;
                }
            }
        }
        block.x = it->col * SNAKEPIECE_WIDTH + SNAKEPIECE_WIDTH;
        block.y = it->row * SNAKEPIECE_HEIGHT + SNAKEPIECE_HEIGHT;

        //Wiggling deviation
        canWiggle = false;
        if(!is_turning_point && canWiggle){
            if(it->dir == HORIZONTAL){
                //Deviate up and down
                if(wiggle % 2 == 0){
                    if(j % 2 == 0)
                        block.y += SNAKEPIECE_HEIGHT/10;
                    else
                        block.y -= SNAKEPIECE_HEIGHT/10;
                }else{
                    if(j % 2 == 0)
                        block.y -= SNAKEPIECE_HEIGHT/10;
                    else
                        block.y += SNAKEPIECE_HEIGHT/10;
                }
            }else{
                //Deviate left and right
                if(wiggle % 2 == 0){
                    if(j % 2 == 0)
                        block.x += SNAKEPIECE_HEIGHT/8;
                    else
                        block.x -= SNAKEPIECE_HEIGHT/8;
                }else{
                    if(j % 2 == 0)
                        block.x -= SNAKEPIECE_HEIGHT/8;
                    else
                        block.x += SNAKEPIECE_HEIGHT/8;
                }
            }
        }
        j++;
        //Load skin bmp
        string imagePath = "snake_skin.bmp";
        SDL_Surface *sur = SDL_LoadBMP(imagePath.c_str());
        if(!sur){
            cout << "Load BMP file failed" << endl;
            close();
            exit(1);
        }
        SDL_Texture *tex = SDL_CreateTextureFromSurface(ren, sur);
        SDL_FreeSurface(sur);
        if(!tex){
            cout << "Create texture from surface failed" << endl;
            close();
            exit(1);
        }
        SDL_Rect temp = {
            .x = 0,
            .y = 0,
            .w = 25,
            .h = 25
        };
        //Draw the skin bmp to block
        SDL_RenderCopy(ren, tex, &temp, &block);
        SDL_DestroyTexture(tex);
        //SDL_SetRenderDrawColor(ren, 34, 139, 34, 255);
        if(it->row == snake.back().row && it->col == snake.back().col){
            block.x = block.x - SNAKEPIECE_WIDTH/10;
            block.y = block.y - SNAKEPIECE_HEIGHT/10;
            block.w = block.w + SNAKEPIECE_WIDTH/5;
            block.h = block.h + SNAKEPIECE_HEIGHT/5;
            SDL_SetRenderDrawColor(ren, 0, 100, 0, 255);
            SDL_RenderFillRect(ren, &block);
        }
    }
    wiggle++;
    canWiggle = false;
    return;
}

bool is_turning_point(SnakePiece s){
    return false;
}

void draw_apple(){
    SDL_Rect block = {
        .x = apple.col * SNAKEPIECE_WIDTH + SNAKEPIECE_WIDTH,
        .y = apple.row * SNAKEPIECE_HEIGHT + SNAKEPIECE_HEIGHT,
        .w = SNAKEPIECE_WIDTH,
        .h = SNAKEPIECE_HEIGHT
    };
    SDL_SetRenderDrawColor(ren, 255, 0 ,0 , 255);
    SDL_RenderFillRect(ren, &block);
    //Draw leaf
    block.w = SNAKEPIECE_WIDTH/5*2;
    block.h = SNAKEPIECE_HEIGHT/5*2;
    block.x = block.x + SNAKEPIECE_WIDTH/5*2;
    block.y = block.y - block.h/2;
    SDL_SetRenderDrawColor(ren, 34, 139, 34, 255);
    SDL_RenderFillRect(ren, &block);

    return;
}

//Move snake works as follow:
//  1)Check next block(apple, wall, body)
//  2)Do the corresponding action
void move_snake(){
    SnakePiece next;
    SnakePiece head = snake.back();
    //Calculate next piece by accessing the head of the snake, which is the last element of the snake list
    switch(curr_dir){
        case LEFT:
            next.row = head.row;
            if(head.col - 1 < 0 && !WALL)
                next.col = GRID_COL_NUM - 1;
            else
                next.col = head.col - 1;
            break;
        case RIGHT:
            next.row = head.row;
            if(head.col + 1 >= GRID_COL_NUM && !WALL)
                next.col = 0;
            else
                next.col = head.col + 1;
            break;
        case UP:
            next.col = head.col;
            if(head.row - 1 < 0  && !WALL)
                next.row = GRID_ROW_NUM - 1;
            else
                next.row = head.row - 1;
            break;
        case DOWN:
            next.col = head.col;
            if(head.row + 1 >= GRID_ROW_NUM && !WALL)
                next.row = 0;
            else
                next.row = head.row + 1;
            break;
        default:
            cout << "Direction Error." << endl;
            next.row = head.row;
            next.col = head.col;
    }
    if(curr_dir == UP || curr_dir == DOWN)
        next.dir = VERTICAL;
    else
        next.dir = HORIZONTAL;
    if(buf_dir != curr_dir){
        turning_points.push_back(next);
        buf_dir = curr_dir;
    }
    
    //Check next block
    switch(check_next_block(next)){
        case 2:
            //Next block is apple , push next to queue, tail doesn't need to change
            //Alocate new apple
            snake.push_back(next);
            new_apple();
            score++;
            SDL_SetWindowTitle(win, ("Score: " + to_string(score)).c_str());
            break;
        case 1:
        case 3:
            //Body, Game ends here
            //temporary handling 
            isRunning = false;
            break;
        case 0:
            //Normal, push new head to the list and pop the tail piece
            snake.push_back(next);
            snake.pop_front();
            break;
    }
    can_move = true;

    if(buf.key.keysym.sym != SDLK_ESCAPE){
        change_direction(buf);
        buf.key.keysym.sym = SDLK_ESCAPE;
    }
    return;
}

//return 3 for wall, return 2 for apple, return 1 for body
int check_next_block(SnakePiece next){
    if((next.row < 0 || next.row >= GRID_ROW_NUM || next.col < 0 || next.col >= GRID_COL_NUM) && WALL)
        return 3;
    if(next.row == apple.row && next.col == apple.col){
        return 2;
    } 
    for(list<SnakePiece>::iterator it = snake.begin(); it != snake.end(); it++){
        if(next.row == it->row && next.col == it->col)
            return 1;
    }
    return 0;
}

void new_apple(){
    //Select a block other than the snake body 
    srand(time(0));
    SnakePiece s;
    apple.col = rand() % GRID_COL_NUM;
    apple.row = rand() % GRID_ROW_NUM;
    while(isRepeated(apple)){
        apple.col = rand() % GRID_COL_NUM;
        apple.row = rand() % GRID_ROW_NUM;
    }
    return;
}

bool isRepeated(Apple a){
    for(list<SnakePiece>::iterator it = snake.begin(); it != snake.end(); it++){
        if(it->col == a.col && it->row == a.row){
            return true;
        }
    }
    return false;
}

void change_direction(SDL_Event e){
    if(can_move == false){
        cout << "Skip current input" << endl;
        buf = e;
        return ;
    }
    switch(e.key.keysym.sym){
        case SDLK_UP:
            if(curr_dir == DOWN)
                break;
            curr_dir = UP;
            cout << "UP" << endl;
            break;
        case SDLK_DOWN:
            if(curr_dir == UP)
                break;
            curr_dir = DOWN;
            cout << "DOWN" << endl;
            break;
        case SDLK_RIGHT:
            if(curr_dir == LEFT)
                break;
            curr_dir = RIGHT;
            cout << "RIGHT" << endl;
            break;
        case SDLK_LEFT:
            if(curr_dir == RIGHT)
                break;
            curr_dir = LEFT;
            cout << "LEFT" << endl;
            break;
    }
    can_move = false;
    return ;
}