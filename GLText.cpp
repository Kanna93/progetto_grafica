#include "GLText.h"

void preRenderText(int font_id, TTF_Font* font, char* text,int foreground_r,int foreground_g,int foreground_b,int *w,int *h)
{
SDL_Color tmpfontcolor = { foreground_r,foreground_g,foreground_b,255 };
SDL_Surface *initial;
SDL_Surface *intermediary;

	/* Use SDL_TTF to render our text */
	initial = NULL;
	initial = TTF_RenderText_Blended(font, text, tmpfontcolor);

	/* Convert the rendered text to a known format */
	*w = initial->w;
	*h = initial->h;

	intermediary = SDL_CreateRGBSurface(0, *w, *h, 32,
		0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000);

	SDL_BlitSurface(initial, 0, intermediary, 0);

	/* Tell GL about our new texture */
	glBindTexture(GL_TEXTURE_2D, font_id);
	glTexImage2D(GL_TEXTURE_2D, 0, 4, *w, *h, 0, GL_RGBA,
		GL_UNSIGNED_BYTE, intermediary->pixels);
}

GLText::GLText(int id, TTF_Font *font)
{
	_font = font;
	_id = id;
	_x = 0;
	_y = 0;
	_h = 0;
	_w = 0;
}

void GLText::setText(char * characters, int foreground_r,int foreground_g,int foreground_b)
{
	//RenderText
	preRenderText(_id, _font, characters, foreground_r,foreground_g,foreground_b,&_w,&_h);
}

void GLText::setPosition(int x, int y)
{
	_x = x;
	_y = y;
}

void GLText::render()
{
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);
	glDisable(GL_LIGHTING);
	/* prepare to render our texture */
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, _id);
	glColor3f(1.0f, 1.0f, 1.0f);

	/* Draw a quad at location */
	//std::cout<<"screen: "<<_w<< " "<<_h<<std::endl;
	glBegin(GL_QUADS);
		glTexCoord2f(0.0f, 0.0f);
		glVertex2f(_x, _y);   //angolo in alto a s
		glTexCoord2f(1.0f, 0.0f);
		glVertex2f(_x + _w, _y);
		glTexCoord2f(1.0f, 1.0f);
		glVertex2f(_x + _w, _y - _h);
		glTexCoord2f(0.0f, 1.0f);
		glVertex2f(_x, _y - _h);
	glEnd();
	
	/* Clean up */
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_BLEND);
	glEnable(GL_LIGHTING);
}

GLText::~GLText()
{
}
