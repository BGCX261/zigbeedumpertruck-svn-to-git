#ifndef GUI_CORE_H
#define GUI_CORE_H

#include <iostream>
#include "SDL/SDL.h"
#include "SDL/SDL_thread.h"
#include "SDL/SDL_ttf.h"
#include "rs232.h"





class GUI_Core {
public:
    GUI_Core();
    ~GUI_Core();

    void Init();
    void Start();

private:
    long lastTick;
    int scrWidth;
    int scrHeight;
//    static SDL_Surface* scr;
  
//    static TTF_Font* font;

    //    Rs232Interface* rs;


    static int InputHandler(void* unused);
    static int HeartBeat(void* unused);
};

#endif
