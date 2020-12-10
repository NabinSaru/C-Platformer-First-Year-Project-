#include <stdio.h>
#include <SDL.h>
#include <SDL_image.h>
#include <math.h>
#include <stdlib.h>
#include <SDL_mixer.h>
#include <time.h>
#define GRAVITY .98f
#define STATUS_TITLE 0
#define STATUS_GAME 1
#define STATUS_WIN 2
#define STATUS_GAMEOVER 3



typedef struct
{
  int x, y, baseX, baseY, mode, animFrame;
  float phase;
} Bee;

typedef struct
{
    float x, y, dx, dy;
    int onLedge, slowingDown, facingLeft, isDead, animFrame;
}Man;

typedef struct
{
    float x, y, w, h;
}Ledge;

typedef struct
{
  Man man;
  float scrollX, bgScrollX;
  Ledge ledges[100];
  Bee bees[200];

  int time, status, deathCount;

  SDL_Renderer *renderer;
  SDL_Texture *manFrames[11];
  SDL_Texture *platform;
  SDL_Texture  *bg;
  SDL_Texture *bee[3];
  SDL_Texture *title;
  SDL_Texture *over;
  SDL_Texture *win;

  int musicChannel;
  Mix_Chunk *bgMusic, *jumpSound, *landSound, *deathSound, *titlemusic, *beepSound, *exitSound;
} GameState;

void loadGame(GameState *game);

void process(GameState *game);

int collide2d(float x1, float y1, float x2, float y2, float wt1, float ht1, float wt2, float ht2);

void collisionDetect(GameState *game);

int processEvents(SDL_Window *window, GameState *game);

void doRender(SDL_Renderer *renderer, GameState  *game);

void titlescreen(GameState *game);

void gameOver(GameState *game);

void gameWin(GameState *game);

int main(int argc, char* args[])
{
    GameState gameState;
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);

    Mix_OpenAudio(MIX_DEFAULT_FREQUENCY, MIX_DEFAULT_FORMAT, MIX_DEFAULT_CHANNELS, 4096);

    srand((int)time(NULL));

    window = SDL_CreateWindow("Dungeon of Death", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 640, 480, SDL_WINDOW_FULLSCREEN);
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    gameState.renderer = renderer;
    loadGame(&gameState);

    int done = 0;
    while(!done)
    {
        done = processEvents(window, &gameState);
        process(&gameState);
        collisionDetect(&gameState);
        doRender(renderer, &gameState);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}

void loadGame(GameState *game)
{

  game->bgMusic = Mix_LoadWAV("music.wav");
  if(game->bgMusic!=NULL)
  {
      Mix_VolumeChunk(game->bgMusic, 64);
  }
  game->titlemusic = Mix_LoadWAV("titlemusic.wav");
  if(game->titlemusic!=NULL)
  {
      Mix_VolumeChunk(game->titlemusic, 8);
  }
  game->deathSound = Mix_LoadWAV("death.wav");
  Mix_VolumeChunk(game->deathSound, 32);
  game->jumpSound = Mix_LoadWAV("jump.wav");
  Mix_VolumeChunk(game->jumpSound, 24);
  game->landSound = Mix_LoadWAV("thud.wav");
  Mix_VolumeChunk(game->landSound, 24);
  game->beepSound = Mix_LoadWAV("selection.wav");
  Mix_VolumeChunk(game->beepSound, 32);
  game->exitSound = Mix_LoadWAV("exit.wav");
  Mix_VolumeChunk(game->beepSound, 32);


  SDL_Surface *surface = NULL;

  surface = IMG_Load("win.png");
  if(surface == NULL)
  {
    printf("Cannot find win.png!\n\n");
    SDL_Quit();
    exit(1);
  }
  game->win = SDL_CreateTextureFromSurface(game->renderer, surface);

  surface = IMG_Load("title.png");
  if(surface == NULL)
  {
    printf("Cannot find title.png!\n\n");
    SDL_Quit();
    exit(1);
  }
  game->title = SDL_CreateTextureFromSurface(game->renderer, surface);

  SDL_FreeSurface(surface);

  surface = IMG_Load("over.png");
  if(surface == NULL)
  {
    printf("Cannot find over.png!\n\n");
    SDL_Quit();
    exit(1);
  }
  game->over = SDL_CreateTextureFromSurface(game->renderer, surface);

  SDL_FreeSurface(surface);

  surface = IMG_Load("bg1.png");
  if(surface == NULL)
  {
    printf("Cannot find bg1.png!\n\n");
    SDL_Quit();
    exit(1);
  }
  game->bg = SDL_CreateTextureFromSurface(game->renderer, surface);

  SDL_FreeSurface(surface);

  surface = IMG_Load("bat1.png");
  if(surface == NULL)
  {
    printf("Cannot find bat1.png!\n\n");
    SDL_Quit();
    exit(1);
  }
  game->bee[0] = SDL_CreateTextureFromSurface(game->renderer, surface);


  SDL_FreeSurface(surface);

  surface = IMG_Load("bat2.png");
  if(surface == NULL)
  {
    printf("Cannot find bat2.png!\n\n");
    SDL_Quit();
    exit(1);
  }
  game->bee[1] = SDL_CreateTextureFromSurface(game->renderer, surface);


  SDL_FreeSurface(surface);

  surface = IMG_Load("bat3.png");
  if(surface == NULL)
  {
    printf("Cannot find bat3.png!\n\n");
    SDL_Quit();
    exit(1);
  }
  game->bee[2] = SDL_CreateTextureFromSurface(game->renderer, surface);


  SDL_FreeSurface(surface);

  surface = IMG_Load("ledge.png");
  if(surface == NULL)
  {
    printf("Cannot find ledge.png!\n\n");
    SDL_Quit();
    exit(1);
  }
  game->platform = SDL_CreateTextureFromSurface(game->renderer, surface);

  SDL_FreeSurface(surface);

  surface = IMG_Load("character1.png");
  if(surface == NULL)
  {
    printf("Cannot find character1.png!\n\n");
    SDL_Quit();
    exit(1);
  }
  game->manFrames[0] = SDL_CreateTextureFromSurface(game->renderer, surface);
  SDL_FreeSurface(surface);

  surface = IMG_Load("character2.png");
  if(surface == NULL)
  {
    printf("Cannot find character2.png!\n\n");
    SDL_Quit();
    exit(1);
  }
  game->manFrames[1] = SDL_CreateTextureFromSurface(game->renderer, surface);
  SDL_FreeSurface(surface);

  surface = IMG_Load("character3.png");
  if(surface == NULL)
  {
    printf("Cannot find character3.png!\n\n");
    SDL_Quit();
    exit(1);
  }
  game->manFrames[2] = SDL_CreateTextureFromSurface(game->renderer, surface);
  SDL_FreeSurface(surface);

  surface = IMG_Load("character4.png");
  if(surface == NULL)
  {
    printf("Cannot find character4.png!\n\n");
    SDL_Quit();
    exit(1);
  }
  game->manFrames[3] = SDL_CreateTextureFromSurface(game->renderer, surface);
  SDL_FreeSurface(surface);

  surface = IMG_Load("character5.png");
  if(surface == NULL)
  {
    printf("Cannot find character5.png!\n\n");
    SDL_Quit();
    exit(1);
  }
  game->manFrames[4] = SDL_CreateTextureFromSurface(game->renderer, surface);
  SDL_FreeSurface(surface);

  surface = IMG_Load("character6.png");
  if(surface == NULL)
  {
    printf("Cannot find character6.png!\n\n");
    SDL_Quit();
    exit(1);
  }
  game->manFrames[5] = SDL_CreateTextureFromSurface(game->renderer, surface);
  SDL_FreeSurface(surface);

  surface = IMG_Load("character7.png");
  if(surface == NULL)
  {
    printf("Cannot find character7.png!\n\n");
    SDL_Quit();
    exit(1);
  }
  game->manFrames[6] = SDL_CreateTextureFromSurface(game->renderer, surface);
  SDL_FreeSurface(surface);

  surface = IMG_Load("character8.png");
  if(surface == NULL)
  {
    printf("Cannot find character8.png!\n\n");
    SDL_Quit();
    exit(1);
  }
  game->manFrames[7] = SDL_CreateTextureFromSurface(game->renderer, surface);
  SDL_FreeSurface(surface);

  surface = IMG_Load("character9.png");
  if(surface == NULL)
  {
    printf("Cannot find character9.png!\n\n");
    SDL_Quit();
    exit(1);
  }
  game->manFrames[8] = SDL_CreateTextureFromSurface(game->renderer, surface);
  SDL_FreeSurface(surface);

  surface = IMG_Load("character10.png");
  if(surface == NULL)
  {
    printf("Cannot find character10.png!\n\n");
    SDL_Quit();
    exit(1);
  }
  game->manFrames[9] = SDL_CreateTextureFromSurface(game->renderer, surface);
  SDL_FreeSurface(surface);

  surface = IMG_Load("character11.png");
  if(surface == NULL)
  {
    printf("Cannot find character11.png!\n\n");
    SDL_Quit();
    exit(1);
  }
  game->manFrames[10] = SDL_CreateTextureFromSurface(game->renderer, surface);
  SDL_FreeSurface(surface);



  game->man.x = 100;
  game->man.y = 150;
  game->man.dx = 0;
  game->man.dy = 0;
  game->man.onLedge = 0;
  game->man.slowingDown = 0;
  game->man.facingLeft = 0;
  game->man.isDead = 0;
  game->man.animFrame = 0;
  game->status = STATUS_TITLE;

  game->scrollX = 0;
  game->bgScrollX = 0;
  game->time = 0;
  game->deathCount = -1;


  for(int i = 0; i < 200; i++)
  {
    game->bees[i].baseX = 320+rand()%50000;
    game->bees[i].baseY = rand()%480;
    game->bees[i].mode = rand()%2;
    game->bees[i].phase = 2*3.14*(rand()%360)/360.0f;
  }
  //load ledges
  for(int i = 0; i < 100; i++)
  {
    game->ledges[i].w = 128;
    game->ledges[i].h = 64;
    game->ledges[i].x = i*256;
    if(i % 2 == 0)
      game->ledges[i].y = 200;
    else
      game->ledges[i].y = 350;
  }
  game->ledges[99].x = 350;
  game->ledges[99].y = 200;

  game->ledges[98].x = 350;
  game->ledges[98].y = 350;



}

void process(GameState *game)
{
  game->time++;

  if(game->status == STATUS_TITLE)
  {
    game->musicChannel = Mix_PlayChannel(-1, game->titlemusic, -1);
    const Uint8 *state = SDL_GetKeyboardState(NULL);
    if(state[SDL_SCANCODE_RETURN])
    {
        game->status = STATUS_GAME;
        Mix_HaltChannel(game->musicChannel);
        Mix_PlayChannel(-1, game->beepSound, 0);
        SDL_Delay(500);
        game->musicChannel = Mix_PlayChannel(-1, game->bgMusic, -1);
    }

      //music
  }
  else  if(game->status == STATUS_GAMEOVER)
  {

      const Uint8 *state = SDL_GetKeyboardState(NULL);
      if(state[SDL_SCANCODE_Y])
      {
        Mix_PlayChannel(-1, game->beepSound, 0);
        SDL_Delay(500);
        game->status = STATUS_GAME;
        game->time = 0;
        game->man.isDead = 0;
        game->man.x = 100;
        game->man.y = 150;
        game->man.dx = 0;
        game->man.dy = 0;
        game->man.onLedge = 0;
        game->musicChannel = Mix_PlayChannel(-1, game->bgMusic, -1);
      }


      else if(state[SDL_SCANCODE_N])
      {
          Mix_PlayChannel(-1, game->exitSound, 0);
          SDL_Delay(350);
          SDL_Quit();
          exit(0);
      }
  }
  else if(game->status == STATUS_GAME)
  {
       //box movement
      Man *man = &game->man;
      man->x += man->dx;
      man->y += man->dy;

      if(man->dx != 0 && man->onLedge && !man->slowingDown)
      {
        if(game->time % 2 == 0)
        {
            if(man->animFrame <= 10)
            {
                man->animFrame++;
            }
            else
            {
                man->animFrame = 0;
            }


        }
      }

      if(game->man.x > 10000)
      {
          gameWin(game);
          game->status = STATUS_WIN;
      }

      if(game->man.isDead && game->deathCount < 0)
      {
          game->deathCount = 1;
      }
      if(game->deathCount >= 0)
      {
          game->deathCount--;
          if(game->deathCount <0)
          {
              gameOver(game);
              game->status = STATUS_GAMEOVER;
              game->time = 0;
          }
      }


      man->dy += GRAVITY;

          for(int i = 0; i < 200; i++)
          {
            game->bees[i].x = game->bees[i].baseX;
            game->bees[i].y = game->bees[i].baseY;

            if(game->bees[i].mode == 0)
            {
              game->bees[i].x = game->bees[i].baseX+sinf(game->bees[i].phase+game->time*0.06f)*75;
            }
            else
            {
              game->bees[i].y = game->bees[i].baseY+cosf(game->bees[i].phase+game->time*0.06f)*75;
            }
          }

  }



  game->bgScrollX = -game->man.x-100;

  game->scrollX = -game->man.x+320;
  if(game->scrollX > 0)
    game->scrollX = 0;
  if(game->scrollX < -50000+320)
    game->scrollX = -50000+320;
}

int collide2d(float x1, float y1, float x2, float y2, float wt1, float ht1, float wt2, float ht2)
{
  return (!((x1 > (x2+wt2)) || (x2 > (x1+wt1)) || (y1 > (y2+ht2)) || (y2 > (y1+ht1))));
}

void collisionDetect(GameState *game)
{

  for(int i = 0; i < 200; i++)
  {
    if(collide2d(game->man.x, game->man.y, game->bees[i].x, game->bees[i].y, 36, 52, 24, 24))
    {
      if(!game->man.isDead)
      {
        game->man.isDead = 1;
        Mix_HaltChannel(game->musicChannel);
        Mix_PlayChannel(-1, game->deathSound, 0);
      }
      break;
    }
  }

  if(game->man.y > 480)
  {
    if(!game->man.isDead)
    {
      game->man.isDead = 1;
      Mix_HaltChannel(game->musicChannel);
      Mix_PlayChannel(-1, game->deathSound, 0);
    }
  }

  //Check for collision with any ledges
  for(int i = 0; i < 100; i++)
  {
    float mw = 36, mh = 40;
    float mx = game->man.x, my = game->man.y;
    float bx = game->ledges[i].x, by = game->ledges[i].y, bw = game->ledges[i].w, bh = game->ledges[i].h;

    if(mx+mw/2 > bx && mx+mw/2<bx+bw)
    {
      //hitting ledge from bottom?
      if(my < by+bh && my > by && game->man.dy < 0)
      {
        //correct y
        game->man.y = by+bh;
        my = by+bh;

        //hit ledge from bottom -> stop any jump velocity
        game->man.dy = 0;
        game->man.onLedge = 1;
      }
    }
    if(mx+mw > bx && mx<bx+bw)
    {
      //landing on ledge
      if(my+mh > by && my < by && game->man.dy > 0)
      {
        //correct y
        game->man.y = by-mh;
        my = by-mh;

        //landed on ledge, stop any jump velocity
        game->man.dy = 0;
        if(!game->man.onLedge)
        {
          Mix_PlayChannel(-1, game->landSound, 0);
          game->man.onLedge = 1;
        }
      }
    }

    if(my+mh > by && my<by+bh)
    {
      //hitting right edge
      if(mx < bx+bw && mx+mw > bx+bw && game->man.dx < 0)
      {
        //correct x
        game->man.x = bx+bw;
        mx = bx+bw;

        game->man.dx = 0;
      }
      //hitting left edge
      else if(mx+mw > bx && mx < bx && game->man.dx > 0)
      {
        //correct x
        game->man.x = bx-mw;
        mx = bx-mw;

        game->man.dx = 0;
      }
    }
  }
}



int processEvents(SDL_Window *window, GameState *game)
{
    int done = 0;
    SDL_Event event;

    while(SDL_PollEvent(&event))
    {
        switch(event.type)
        {
        case SDL_WINDOWEVENT_CLOSE:
            if(window)
            {
                SDL_DestroyWindow(window);
                window = NULL;
                done = 1;
            }
            break;

        case SDL_KEYDOWN:
        switch(event.key.keysym.sym)
            {
            case SDLK_ESCAPE:
                done = 1;
                break;
            //initial jump
            case SDLK_w:
                if(game->man.onLedge)
                {
                    game->man.dy = -15;
                    game->man.onLedge = 0;
                    Mix_PlayChannel(-1, game->jumpSound, 0);
                }
                break;
            }
            break;
        case SDL_QUIT:
            done = 1;
            break;
        }

    }
  const Uint8 *state = SDL_GetKeyboardState(NULL);
  //extra jump
  if(state[SDL_SCANCODE_W])
  {
    game->man.dy -= 0.2f;
  }

  //Walking
  if(state[SDL_SCANCODE_A])
  {
    game->man.dx -= 0.5;
    if(game->man.dx < -7)
    {
      game->man.dx = -7;
    }
    game->man.slowingDown = 0;
    game->man.facingLeft = 1;
  }
  else if(state[SDL_SCANCODE_D])
  {
    game->man.dx += 0.5;
    if(game->man.dx > 7)
    {
      game->man.dx = 7;
    }
    game->man.slowingDown = 0;
    game->man.facingLeft = 0;
  }
  else
  {
    game->man.animFrame = 0;
    game->man.dx *= 0.8f;
    game->man.slowingDown = 1;
    if(fabsf(game->man.dx) < 0.1f)
    {
      game->man.dx = 0;
    }
  }

    return done;

}

void doRender(SDL_Renderer *renderer, GameState *game)
{

  if(game->status == STATUS_TITLE)
  {
    titlescreen(game);
  }
  else if(game->status == STATUS_GAMEOVER)
  {
    gameOver(game);
  }
  else if(game->status == STATUS_WIN)
  {
    gameWin(game);
  }
  else if(game->status == STATUS_GAME)
  {
    SDL_SetRenderDrawColor(renderer, 30, 30, 30, 100);
    SDL_RenderClear(renderer);

    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_Rect bgRect = {0,0,640,480};
    SDL_RenderCopy(renderer, game->bg, NULL, &bgRect);
    for(int i = 0; i < 100; i++)
    {
      SDL_Rect ledgeRect = { game->scrollX+game->ledges[i].x, game->ledges[i].y, game->ledges[i].w, game->ledges[i].h };
      SDL_RenderCopy(renderer, game->platform, NULL, &ledgeRect);
      //SDL_RenderFillRect(renderer, &ledgeRect);
    }
    SDL_SetRenderDrawColor(renderer, 159, 199, 203, 255);

    for(int i = 0; i < 200; i++)
    {
      SDL_Rect beeRect = { game->scrollX+game->bees[i].x, game->bees[i].y, 64, 64 };
      if(game->time%3==0)
      {
          SDL_RenderCopy(renderer, game->bee[0], NULL, &beeRect);
      }
      else if(game->time%3==1)
      {
          SDL_RenderCopy(renderer, game->bee[1], NULL, &beeRect);
      }
      else
      {
          SDL_RenderCopy(renderer, game->bee[2], NULL, &beeRect);
      }
    }
    SDL_Rect rect = {game->scrollX+game->man.x, game->man.y, 36, 52};
    SDL_RenderCopyEx(renderer, game->manFrames[game->man.animFrame],
                   NULL, &rect, 0, NULL, (game->man.facingLeft == 1));
  }
    SDL_RenderPresent(renderer);

}

void titlescreen(GameState *game)
{
  SDL_Renderer *renderer = game->renderer;
  SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);

  //Clear the screen
  SDL_RenderClear(renderer);

  SDL_Rect rect = { 0, 0, 640, 480 };
  SDL_RenderCopy(renderer, game->title, NULL, &rect);


  //set the drawing color to white


}

void gameOver(GameState *game)
{
  SDL_Renderer *renderer = game->renderer;
  SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);

  //Clear the screen
  SDL_RenderClear(renderer);

  SDL_Rect rect = { 0, 0, 640, 480 };
  SDL_RenderCopy(renderer, game->over, NULL, &rect);

  //set the drawing color to white

}

void gameWin(GameState *game)
{
  SDL_Renderer *renderer = game->renderer;
  SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);

  //Clear the screen
  SDL_RenderClear(renderer);

  SDL_Rect rect = { 0, 0, 640, 480 };
  SDL_RenderCopy(renderer, game->win, NULL, &rect);


}
