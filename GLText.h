#ifndef GLTEXT_H_
#define GLTEXT_H_

#include <sstream>
#include <iostream>
#include <stdio.h>
#include <SDL2/SDL.h>
#ifdef __APPLE__
#include <SDL2_ttf/SDL_ttf.h>
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#else
#include <SDL2/SDL_ttf.h>
#include <GL/gl.h>
#include <GL/glu.h>
#endif

void preRenderText(int font_id,TTF_Font* font,char* text,int foreground_r,int foreground_g,int foreground_b,int *w,int *h);

class GLText
{
public:
	GLText(int id,TTF_Font *font);
	void setText(char* characters, int foreground_r,int foreground_g,int foreground_b);
	void setPosition(int x, int y);
	void render();
	~GLText();
private:
	TTF_Font  *_font;
	int  _id,_x,_y,_w,_h;
};

#endif

