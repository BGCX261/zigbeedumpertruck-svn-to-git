#include "gui_core.h"
#include <sstream>

bool quitSignal = false;
Rs232Interface* rs;
TTF_Font* font;
SDL_Surface* scr;

void GetTemp(Rs232Interface* rs)
{
	int temp;
	double temp_d;
	char sig=4;
	rs->putRaw(&sig, 1);
	SDL_Delay(500);
	std::string str = rs->getString(8);
	
	temp = ((int)str[0] << 8) | ((int)str[1] & 0xFF);
	temp_d = (double)temp / 16.0;

	std::cout << "Temp: " << temp_d << std::endl;

	SDL_Color fontcolor = {255,0,0, 0};
    	SDL_Color fontHighLightColor = {255,255,255, 0};
    	SDL_Color bgcolor = {127,127,127, 0};
	SDL_Surface *text;

	std::ostringstream ss;
  ss << "Temperature: ";
	ss << temp_d;
	SDL_Rect text_rect;

	text = TTF_RenderText_Shaded(font, ss.str().c_str(), fontHighLightColor, bgcolor);


	SDL_BlitSurface(text, NULL, scr, NULL);
  SDL_Flip(scr);
}

void GetSpeed(Rs232Interface* rs)
{
	char sig = 9;
	rs->putRaw(&sig, 1);
	SDL_Delay(200);
	std::string str = rs->getString(8);
	std::cout << "Speed: " << (int)str[0] << std::endl;
}



GUI_Core::GUI_Core()
{
	lastTick = 0;
	scrWidth = 300;
	scrHeight = 200;
	scr = 0;
	rs = new Rs232Interface("/dev/ttyUSB0", B9600);
	//    quitSignal = false;
}

GUI_Core::~GUI_Core()
{
	delete(rs);
	TTF_CloseFont(font);
	TTF_Quit();
	SDL_Quit();
}

void GUI_Core::Init()
{
	atexit ( SDL_Quit );

	if(SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		std::cout << "Error while initing video" << std::endl;
		exit(1);
	}

	scr = SDL_SetVideoMode(scrWidth, scrHeight, 16, SDL_DOUBLEBUF);

	if(scr == NULL)
	{
		std::cout << "Error while setting screen mode" << std::endl;
		exit(1);
	}

	if(!rs->init())
	{
		std::cout << "Error while initing rs232 interface" << std::endl;
		exit(1);
	}


  if(TTF_Init()==-1) std::cout << "Error with TTF_Init()" << std::endl;
	
	font = TTF_OpenFont("Vera.ttf", 16);
    	if (font == NULL){
      		printf("Unable to load font");
    	}

}

void GUI_Core::Start()
{
	lastTick = SDL_GetTicks();
	quitSignal = false;

	SDL_Thread* inputLoop;
	SDL_Thread* heartBeat;


	inputLoop = SDL_CreateThread(&InputHandler, this);
	if (inputLoop == NULL ) {
		std::cout << "Error creating input handler" << std::endl;
		return;
	}

	heartBeat = SDL_CreateThread(&HeartBeat, this);
	if (heartBeat == NULL) {
		std::cout << "Error creating heart beat timer" << std::endl;
		return;
	}

	int retVal;
	SDL_WaitThread(inputLoop, &retVal);
	quitSignal = true;
}

int GUI_Core::HeartBeat(void* unused)
{
	while(!quitSignal) //get quit signal here
	{
		std::cout << "Sending heart beat..." << std::endl;
		char beat = 0;
		rs->putChar(beat);
		SDL_Delay(200);
	}
	return 0;
}

int GUI_Core::InputHandler(void* unused)
{
	SDL_Event keyevent;
	bool running = true;   //Run signal

	char left[2], right[2], back[2], forward[2], stop[2], straight[2], beat;

	beat=8;
	left[0] = 2;
	left[1] = -10;

	right[0] = 2;
	right[1] = 10;

	back[0] = 1;
	back[1] = -2;

	forward[0] = 1;
	forward[1] = 2;

	stop[0] = 1;
	stop[1] = 0;

	straight[0] = 2;
	straight[1] = 0;


	char stds[2], stdf[2];

	stds[0] = 2;
	stds[1] = 10;

	stdf[0] = 1;
	stdf[1] = 2;




	while(running) {
		while (SDL_PollEvent(&keyevent))   //Poll our SDL key event for any keystrokes.
		{
			if (keyevent.type == SDL_KEYDOWN)
			{
				switch(keyevent.key.keysym.sym){
					case SDLK_LEFT:
						rs->putRaw(left, 2);                        
						break;
					case SDLK_RIGHT:
						rs->putRaw(right, 2);
						break;
					case SDLK_UP:
						rs->putRaw(forward, 2);
						break;
					case SDLK_DOWN:
						rs->putRaw(back, 2);
						break;
					case SDLK_h:
						rs->putRaw(&beat, 1);
						break;
					case SDLK_ESCAPE:
				//		rs->putRaw(stop, 2);
				//		rs->putRaw(straight, 2);
						running = false;
						break;
					case SDLK_t:
						GetTemp(rs);
						break;
					case SDLK_s:
						GetSpeed(rs);
						break;
					case SDLK_1:
						forward[1] = 2;
						break;
					case SDLK_2:
						forward[1] = 4;
						break;
					case SDLK_3:
						forward[1] = 6;
						break;
					case SDLK_4:
						back[1] = -3;
						break;
					case SDLK_5:
						back[1] = -5;
						break;
					case SDLK_p:
						rs->putRaw(stds, 2);
						rs->putRaw(stdf, 2);
					default:
						break;
				}
			}
			if (keyevent.type == SDL_KEYUP)
			{
				switch(keyevent.key.keysym.sym){
					case SDLK_LEFT:
					case SDLK_RIGHT:
						rs->putRaw(straight, 2);
						break;
					case SDLK_UP:
					case SDLK_DOWN:
						rs->putRaw(stop, 2);
						break;
					default:
						break;
				}
			}
		}
	}
	quitSignal = true;
	return 0;
}





