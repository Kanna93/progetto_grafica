
#include <SDL2/SDL.h>

#include <SDL2/SDL_image.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "car.h"
#include "GLText.h"
#include <iostream>
#include <fstream>
using namespace std;

#define CAMERA_BACK_CAR 0
#define CAMERA_TOP_FIXED 1
#define CAMERA_TOP_CAR 2
#define CAMERA_PILOT 3
#define CAMERA_MOUSE 4
#define CAMERA_TYPE_MAX 5

#define MODE_FREE 1
#define MODE_DEMO 0
#define MODE_RUN 2
SDL_Window *win;
SDL_Window *winB;
SDL_GLContext mainContext;
Uint32 windowID;
SDL_Joystick *joystick;
static int keymap[Controller::NKEYS] = {SDLK_a, SDLK_d, SDLK_w, SDLK_s};

fstream txtFile;
bool demo=false;
bool demox=true;
bool freeRun=false;
bool dx=false,sx=false,dritto=false,dietro=false;
bool morning=true;
TTF_Font *font;
TTF_Font *font_models;
TTF_Font *fontAvvisi;
TTF_Font *fontTasti;
TTF_Font *fontTastiAttivi;
GLText *countdown;
GLText *secondi;
GLText *tempoGiro;
GLText *tempoMigliore;
GLText *fps_text;
GLText *avvisi;
GLText *tasti;
GLText *tastoW;
GLText *tastoA;
GLText *tastoS;
GLText *tastoD;
GLText *tastiAttivi;
GLText *stats_text;
char bufferSecondi[20];
char bufferTempoGiro[20];
char bufferTempoMigliore[20];
char bufferFps[20];
float viewAlpha=20, viewBeta=40; // angoli che definiscono la vista
float eyeDist=10.0; // distanza dell'occhio dall'origine
int scrH=750, scrW=750; // altezza e larghezza viewport (in pixels)
bool useWireframe=false;
bool useHeadlight=false;
bool useShadow=true;
bool done=0;
int modeType=0;
int cameraType=2;
int centesimiGara=0;
int secondiGara=0;
int minutiGara=0;
int centesimiGiro=0;
int secondiGiro=0;
int minutiGiro=0;
int centesimiGiroV=0;
int secondiGiroV=0;
int minutiGiroV=0;
int numeroGiro=0;
int giroVeloce=0;
int countdownMs=0;
int corsaMs=0;
int giroMs=0;
int tempoAvvisi=0;
bool nuovoGiro=false;
bool startGiro=true;
Car car; // la nostra macchina
int nstep=0; // numero di passi di FISICA fatti fin'ora
const int PHYS_SAMPLING_STEP=10; // numero di millisec che un passo di fisica simula

// Frames Per Seconds
const int fpsSampling = 3000; // lunghezza intervallo di calcolo fps
float fps=0; // valore di fps dell'intervallo precedente
int fpsNow=0; // quanti fotogrammi ho disegnato fin'ora nell'intervallo attuale
Uint32 timeLastInterval=0; // quando e' cominciato l'ultimo intervallo

extern void drawPista();
void options();
void init();


void  SetCoordToPixel(){
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  glTranslatef(-1,-1,0);
  glScalef(2.0/scrW, 2.0/scrH, 1);
}

bool LoadTexture(int textbind,char *filename){
GLenum texture_format;

  SDL_Surface *s = IMG_Load(filename);
  if (!s) return false;

  if (s->format->BytesPerPixel == 4){     // contiene canale alpha
    if (s->format->Rmask == 0x000000ff){
      texture_format = GL_RGBA;
    }
    else{
    	//CAMBIO texture_format = GL_BGRA;
      texture_format = GL_RGBA;
    }
  } else if (s->format->BytesPerPixel == 3){     // non contiene canale alpha
     if (s->format->Rmask == 0x000000ff)
       texture_format = GL_RGB;
     else{
    	 //CAMBIO texture_format = GL_BGR;
       texture_format = GL_RGB;
     }
     } else {
        printf("[ERROR] the image is not truecolor\n");
        exit(1);
      }

  glBindTexture(GL_TEXTURE_2D, textbind);
  gluBuild2DMipmaps(
    GL_TEXTURE_2D, 
    3,
    s->w, s->h, 
    texture_format,
    GL_UNSIGNED_BYTE,
    s->pixels
  );
  glTexParameteri(
  GL_TEXTURE_2D, 
  GL_TEXTURE_MAG_FILTER,
  GL_LINEAR ); 
  glTexParameteri(
  GL_TEXTURE_2D, 
  GL_TEXTURE_MIN_FILTER,
  GL_LINEAR_MIPMAP_LINEAR ); 
  return true;
}

// disegna gli assi nel sist. di riferimento
void drawAxis(){
  const float K=0.10;
  glColor3f(0,0,1);
  glBegin(GL_LINES);
    glVertex3f( -1,0,0 );
    glVertex3f( +1,0,0 );

    glVertex3f( 0,-1,0 );
    glVertex3f( 0,+1,0 );

    glVertex3f( 0,0,-1 );
    glVertex3f( 0,0,+1 );
  glEnd();
  
  glBegin(GL_TRIANGLES);
    glVertex3f( 0,+1  ,0 );
    glVertex3f( K,+1-K,0 );
    glVertex3f(-K,+1-K,0 );

    glVertex3f( +1,   0, 0 );
    glVertex3f( +1-K,+K, 0 );
    glVertex3f( +1-K,-K, 0 );

    glVertex3f( 0, 0,+1 );
    glVertex3f( 0,+K,+1-K );
    glVertex3f( 0,-K,+1-K );
  glEnd();

}



void drawSphere(double r, int lats, int longs) {
int i, j;
  for(i = 0; i <= lats; i++) {
     double lat0 = M_PI * (-0.5 + (double) (i - 1) / lats);
     double z0  = sin(lat0);
     double zr0 =  cos(lat0);
   
     double lat1 = M_PI * (-0.5 + (double) i / lats);
     double z1 = sin(lat1);
     double zr1 = cos(lat1);
    
     glBegin(GL_QUAD_STRIP);
     for(j = 0; j <= longs; j++) {
        double lng = 2 * M_PI * (double) (j - 1) / longs;
        double x = cos(lng);
        double y = sin(lng);
    
//le normali servono per l'EnvMap
        glNormal3f(x * zr0, y * zr0, z0);
        glVertex3f(r * x * zr0, r * y * zr0, r * z0);
        glNormal3f(x * zr1, y * zr1, z1);
        glVertex3f(r * x * zr1, r * y * zr1, r * z1);
     }
     glEnd();
  }
}

void drawFloor()
{
  const float S=100; // size
  const float H=0;   // altezza
  const int K=150; //disegna K x K quads
  

  if (useWireframe) {
          //glDisable(GL_TEXTURE_2D);
          glColor3f(0,0.8,0);
          glDisable(GL_LIGHTING);
          glPolygonMode(GL_FRONT_AND_BACK,GL_LINE);
  }
  glPushMatrix();

  // disegna KxK quads
  glBegin(GL_QUADS);
  if (!useWireframe){
          	glEnable(GL_TEXTURE_2D);
          	glColor3f(0.13, 0.54, 0.13); // colore uguale x tutti i quads
     }
    glNormal3f(0,1,0);       // normale verticale uguale x tutti
    for (int x=0; x<K; x++) 
    for (int z=0; z<K; z++) {
      float x0=-S + 2*(x+0)*S/K;
      float x1=-S + 2*(x+1)*S/K;
      float z0=-S + 2*(z+0)*S/K;
      float z1=-S + 2*(z+1)*S/K;
      glVertex3d(x0, H, z0);
      glVertex3d(x1, H, z0);
      glVertex3d(x1, H, z1);
      glVertex3d(x0, H, z1);
    }

  glEnd();
  glPopMatrix();
  if (useWireframe){
  	   glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);
  	       glColor3f(1,1,1);
  	       glEnable(GL_LIGHTING);
     }
}

// setto la posizione della camera
void setCamera(){

        double px = car.px;
        double py = car.py;
        double pz = car.pz;
        double angle = car.facing;
        double cosf = cos(angle*M_PI/180.0);
        double sinf = sin(angle*M_PI/180.0);
        double camd, camh, ex, ey, ez, cx, cy, cz;
        double cosff, sinff;

// controllo la posizione della camera a seconda dell'opzione selezionata
        switch (cameraType) {
        case CAMERA_BACK_CAR:
                camd = 10.5;
                camh = 1.0;
                ex = px + camd*sinf;
                ey = py + camh;
                ez = pz + camd*cosf;
                cx = px - camd*sinf;
                cy = py + camh;
                cz = pz - camd*cosf;
                gluLookAt(ex,ey,ez,cx,cy,cz,0.0,1.0,0.0);
                break;
        case CAMERA_TOP_FIXED:
                camd = 5.5;
                camh = 1.0;
                angle = car.facing + 40.0;
                cosff = cos(angle*M_PI/180.0);
                sinff = sin(angle*M_PI/180.0);
                ex = px + camd*sinff;
                ey = py + camh;
                ez = pz + camd*cosff;
                cx = px - camd*sinf;
                cy = py + camh;
                cz = pz - camd*cosf;
                gluLookAt(ex,ey,ez,cx,cy,cz,0.0,1.0,0.0);
                break;
        case CAMERA_TOP_CAR:
                camd = 10.5;
                camh = 1.0;
                ex = px + camd*sinf;
                ey = py + camh;
                ez = pz + camd*cosf;
                cx = px - camd*sinf;
                cy = py + camh;
                cz = pz - camd*cosf;
                gluLookAt(ex,ey+5,ez,cx,cy,cz,0.0,1.0,0.0);
                break;
        case CAMERA_PILOT:
                camd = 0.1;
                camh = 2.55;
                ex = px + camd*sinf;
                ey = py + camh;
                ez = pz + camd*cosf;
                cx = px - camd*sinf;
                cy = py + camh;
                cz = pz - camd*cosf;
                gluLookAt(ex,ey,ez,cx,cy,cz,0.0,1.0,0.0);
                break;
        case CAMERA_MOUSE:
                glTranslatef(0,0,-eyeDist);
                glRotatef(viewBeta,  1,0,0);
                glRotatef(viewAlpha, 0,1,0);
                break;
        }
}

void drawSky() {
int H = 100;

  if (useWireframe) {
    glDisable(GL_TEXTURE_2D);
    glColor3f(0.4, 0.8, 1);
    glDisable(GL_LIGHTING);
    glPolygonMode(GL_FRONT_AND_BACK,GL_LINE);
    drawSphere(100.0, 20, 20);
    glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);
    glColor3f(1,1,1);
    glEnable(GL_LIGHTING);
  }
  else
  {
	  	if (morning)
		  glBindTexture(GL_TEXTURE_2D,2);
	  	else
	  	  glBindTexture(GL_TEXTURE_2D,6);
        glEnable(GL_TEXTURE_2D);
        glEnable(GL_TEXTURE_GEN_S);
        glEnable(GL_TEXTURE_GEN_T);
        glTexGeni(GL_S, GL_TEXTURE_GEN_MODE , GL_SPHERE_MAP); // Env map
        glTexGeni(GL_T, GL_TEXTURE_GEN_MODE , GL_SPHERE_MAP);
        glColor3f(1,1,1);
        glDisable(GL_LIGHTING);

        drawSphere(100.0, 20, 20);

        glDisable(GL_TEXTURE_GEN_S);
        glDisable(GL_TEXTURE_GEN_T);
        glDisable(GL_TEXTURE_2D);
        glEnable(GL_LIGHTING);
  }

}

/* Esegue il Rendering della scena */
void rendering(SDL_Window *win){
  
  // un frame in piu'!!!
  fpsNow++;
  
  glLineWidth(3); // linee larghe
     
  // settiamo il viewport
  glViewport(0,0, scrW, scrH);
  
  // colore sfondo = bianco
  glClearColor(1,1,1,1);


  // settiamo la matrice di proiezione
  glMatrixMode( GL_PROJECTION );
  glLoadIdentity();
  gluPerspective( 70, //fovy,
		((float)scrW) / scrH,//aspect Y/X,
		0.2,//distanza del NEAR CLIPPING PLANE in coordinate vista
		1000  //distanza del FAR CLIPPING PLANE in coordinate vista
  );

  glMatrixMode( GL_MODELVIEW ); 
  glLoadIdentity();
  
  // riempe tutto lo screen buffer di pixel color sfondo
  glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

  //drawAxis(); // disegna assi frame VISTA
  // setto la posizione luce
  float tmpv [4]= {0,1,2,  0};
  float tmpvNotte[4] = {0,1,0, 1};
  float tmpvNotte1[4] = {3,1,3, 0};

  if (morning)
	  glLightfv(GL_LIGHT0, GL_POSITION, tmpv );
  else{
	  glLightfv(GL_LIGHT0, GL_POSITION, tmpvNotte );
	  glLightfv(GL_LIGHT0, GL_CONSTANT_ATTENUATION , tmpvNotte1 );
  }
  
  // settiamo matrice di vista
  setCamera();

  glEnable(GL_LIGHT0);
  
  

  drawSky(); // disegna il cielo come sfondo
  drawPista(); // disegna la pista
  drawFloor(); // disegna il suolo


  car.Render(); // disegna la macchina


  // attendiamo la fine della rasterizzazione di 
  // tutte le primitive mandate 
  glDisable(GL_DEPTH_TEST);
  glDisable(GL_LIGHTING);
 
// disegnamo i fps (frame x sec) come una barra a sinistra.
// (vuota = 0 fps, piena = 100 fps)
  SetCoordToPixel();
  if (modeType==MODE_RUN){
	  if (countdownMs>=300&&countdownMs<400){
			countdown->setText((char *)"3",255,0,0);
			countdown->setPosition(scrW/2-20,scrH/2+120);
			countdown->render();
		}
		else if (countdownMs>=400&&countdownMs<500){
			countdown->setText((char *)"2",255,0,0);
			countdown->setPosition(scrW/2-20,scrH/2+120);
			countdown->render();
		}
		else if (countdownMs>=500&&countdownMs<600){
			countdown->setText((char *)"1",255,0,0);
			countdown->setPosition(scrW/2-20,scrH/2+120);
			countdown->render();
		}
		else if (countdownMs>=600&&corsaMs<100){
			countdown->setText((char *)"VIA!",255,0,0);
			countdown->setPosition(scrW/2-100,scrH/2+120);
			countdown->render();
	  }
	  int tmp=0;
	  if (countdownMs>=600){
		  centesimiGara=(corsaMs-giroMs)%100;
		  tmp=(corsaMs-giroMs)/100;
		  secondiGara=tmp%60;
		  minutiGara=tmp/60;
	
	  }
	  sprintf(bufferSecondi,"%02d : %02d . %02d",minutiGara,secondiGara,centesimiGara);
	  secondi->setText(bufferSecondi,0,0,255);
	  secondi->setPosition(5,scrH-5);
	  secondi->render();
	  if (car.fineGiro && nuovoGiro && !startGiro){
		  nuovoGiro=false;
		  giroMs=corsaMs;
		  numeroGiro++;
		  centesimiGiro=centesimiGara;
		  secondiGiro=secondiGara;
		  minutiGiro=minutiGara;
		  if (numeroGiro==1 && (centesimiGiroV==0 && secondiGiroV==0 && minutiGiroV==0)){
			  centesimiGiroV=centesimiGiro;
			  secondiGiroV=secondiGiro;
			  minutiGiroV=minutiGiro;
			  giroVeloce=numeroGiro;
			  txtFile.open("par.txt", ios::out);
			  if (txtFile.is_open()){
				  txtFile<<std::to_string(minutiGiroV)<<"\n"<<std::to_string(secondiGiroV)<<"\n"<<std::to_string(centesimiGiroV)<<"\n";
				  txtFile.close();
			  }
		  }
		  else if ((centesimiGiro+secondiGiro*100+minutiGiro*60*100)<(centesimiGiroV+secondiGiroV*100+minutiGiroV*60*100)){
			  centesimiGiroV=centesimiGiro;
			  secondiGiroV=secondiGiro;
			  minutiGiroV=minutiGiro;
			  giroVeloce=numeroGiro;
			  txtFile.open("par.txt", ios::out);
			  if (txtFile.is_open()){
				  txtFile<<std::to_string(minutiGiroV)<<"\n"<<std::to_string(secondiGiroV)<<"\n"<<std::to_string(centesimiGiroV)<<"\n";
				  txtFile.close();
			  }
		  }
	  }
	  else if (car.fineGiro && nuovoGiro && startGiro){
		  nuovoGiro=false;
		  startGiro=false;
	  }
	  else if (!car.fineGiro){
		  nuovoGiro=true;
	  }
	
	  sprintf(bufferTempoGiro,"%02d : %02d . %02d - %d",minutiGiro,secondiGiro,centesimiGiro,numeroGiro);
	  tempoGiro->setText(bufferTempoGiro,0,0,255);
	  tempoGiro->setPosition(scrW-220,scrH-5);
	  tempoGiro->render();
	  sprintf(bufferTempoMigliore,"%02d : %02d . %02d - %d",minutiGiroV,secondiGiroV,centesimiGiroV,giroVeloce);
	  tempoMigliore->setText(bufferTempoMigliore,0,0,255);
	  tempoMigliore->setPosition(scrW-220,scrH-40);
	  tempoMigliore->render();
  }

  if (modeType==MODE_DEMO){
	  if (tempoAvvisi>=300&&tempoAvvisi<500){
			  if (tempoAvvisi<450){
			  avvisi->setText((char *)"Benvenuti nel tutorial",255,0,0);
			  avvisi->setPosition(scrW/2-180,scrH-30);
			  avvisi->render();
		  }
	      dritto=sx=dx=dietro=false;
	  }
	  else if (tempoAvvisi>=500&&tempoAvvisi<800){
		  if (tempoAvvisi<750){
			  avvisi->setText((char *)"Verranno mostrati i comandi principali",255,0,0);
			  avvisi->setPosition(scrW/2-300,scrH-30);
			  avvisi->render();
			  avvisi->setText((char *)"per muovere il camion",255,0,0);
			  avvisi->setPosition(scrW/2-200,scrH-65);
			  avvisi->render();
		  }
		  dritto=sx=dx=dietro=false;
	  }
	  else if (tempoAvvisi>=800&&tempoAvvisi<1400){
		  if (tempoAvvisi<1350){
			  avvisi->setText((char *)"Per andare avanti premere W",255,0,0);
			  avvisi->setPosition(scrW/2-250,scrH-30);
			  avvisi->render();
			  if (tempoAvvisi>=900 && tempoAvvisi<=1100){
				  sx=dx=dietro=false; dritto=true;
			  }
			  else if (tempoAvvisi>=1100 && tempoAvvisi<1300){
				  dritto=sx=dx=dietro=false;
			  }
			  else if (tempoAvvisi>=1300 && tempoAvvisi<1350){
				  sx=dx=dietro=false; dritto=true;
			  }

		  }
		  else{
			  sx=dx=dietro=false; dritto=true;
		  }
	  }
	  else if (tempoAvvisi>=1400&&tempoAvvisi<2000){
		  if (tempoAvvisi<1950){
			  avvisi->setText((char *)"Per curvare verso sinistra premere A",255,0,0);
			  avvisi->setPosition(scrW/2-280,scrH-30);
			  avvisi->render();
			  if (tempoAvvisi>=1500 && tempoAvvisi<1700){
				  dx=dietro=false; sx=dritto=true;
			  }
			  else if(tempoAvvisi>=1700 && tempoAvvisi<1800){
				  sx=dx=dietro=false; dritto=true;
			  }
			  else if(tempoAvvisi>=1800 && tempoAvvisi<1900){
				  dx=dietro=false; sx=dritto=true;
			  }
			  else if (tempoAvvisi>=1900 && tempoAvvisi<1950){
				  sx=dx=dietro=false; dritto=true;
			  }
		  }
		  else{
			  sx=dx=dietro=false; dritto=true;
		  }
	  }
	  else if (tempoAvvisi>=2000&&tempoAvvisi<2700){
		  if (tempoAvvisi<2600){
			  avvisi->setText((char *)"Per curvare verso destra premere D",255,0,0);
			  avvisi->setPosition(scrW/2-270,scrH-30);
			  avvisi->render();
			  if (tempoAvvisi>=2100 && tempoAvvisi<2300){
				  sx=dietro=false; dx=dritto=true;
			  }
			  else if (tempoAvvisi>=2300 && tempoAvvisi<2400){
				  sx=dx=dietro=false; dritto=true;
			  }
			  else if (tempoAvvisi>=2400 && tempoAvvisi<2500){
				  sx=dietro=false; dx=dritto=true;
			  }
			  else if (tempoAvvisi>=2500 && tempoAvvisi<2650){
				  sx=dx=dietro=false; dritto=true;
			  }
		  }
		  else{
			  dritto=sx=dx=dietro=false;
		  }
	  }
	  else if (tempoAvvisi>=2700&&tempoAvvisi<3500){
		  if (tempoAvvisi<3450){
			  avvisi->setText((char *)"Per andare indietro premere S",255,0,0);
			  avvisi->setPosition(scrW/2-250,scrH-30);
			  avvisi->render();
			  if (tempoAvvisi>=2800 && tempoAvvisi<3000){
				  dritto=sx=dx=false; dietro=true;
			  }
			  else if (tempoAvvisi>=3000 && tempoAvvisi<3100){
				  dritto=sx=dx=dietro=false;
			  }
			  else if (tempoAvvisi>=3100 && tempoAvvisi<3200){
				  dritto=sx=dx=false; dietro=true;
			  }
			  else if (tempoAvvisi>=3200 && tempoAvvisi<3300){
				  dritto=dx=false; dietro=sx=true;
			  }
			  else if (tempoAvvisi>=3300 && tempoAvvisi<3400){
				  dritto=sx=false; dietro=dx=true;
			  }
			  else if (tempoAvvisi>=3400 && tempoAvvisi<3450){
				  dritto=sx=false; dietro=true;
			  }
		  }
		  else{
			 sx=dx=dietro=false; dritto=true;
		  }
	  }
	  else if (tempoAvvisi>=3500&&tempoAvvisi<4400){
		  if (tempoAvvisi<4350){
			  avvisi->setText((char *)"ATTENZIONE!",255,0,0);
			  avvisi->setPosition(scrW/2-90,scrH-30);
			  avvisi->render();
			  avvisi->setText((char *)"Non esagerare con le curve:",255,0,0);
			  avvisi->setPosition(scrW/2-205,scrH-65);
			  avvisi->render();
			  avvisi->setText((char *)"potrebbe ribaltarsi il rimorchio",255,0,0);
			  avvisi->setPosition(scrW/2-240,scrH-100);
			  avvisi->render();
			  if (tempoAvvisi>=3600 && tempoAvvisi<4200){
				  dx=dietro=false; sx=dritto=true;
			  }
			  else if (tempoAvvisi>=4200 && tempoAvvisi<4350){
				  sx=dx=dietro=false; dritto=true;
			  }
		  }
		  else{
			  dritto=sx=dx=dietro=false;
		  }
	  }
	  else if (tempoAvvisi>=4400 && tempoAvvisi<5400){
		  if(tempoAvvisi<5300){
			  if (tempoAvvisi>=4400 && tempoAvvisi<4600){
				  avvisi->setText((char *)"Nella modalità run saranno aggiunte",255,0,0);
				  avvisi->setPosition(scrW/2-290,scrH-30);
				  avvisi->render();
				  avvisi->setText((char *)"le seguenti informazioni",255,0,0);
				  avvisi->setPosition(scrW/2-210,scrH-65);
				  avvisi->render();
			  }
			  if (tempoAvvisi>=4650){
				  secondi->setText("00 : 00 . 00",0,0,255);
				  secondi->setPosition(5,scrH-5);
				  secondi->render();
				  tempoGiro->setText("00 : 00 . 00 - 1",0,0,255);
				  tempoGiro->setPosition(scrW-220,scrH-5);
				  tempoGiro->render();
				  tempoMigliore->setText("00 : 00 . 00 - 1",0,0,255);
				  tempoMigliore->setPosition(scrW-220,scrH-40);
				  tempoMigliore->render();
				  if (tempoAvvisi>=4700 && tempoAvvisi<4850){
					  avvisi->setText((char *)" <- Tempo Attuale",255,0,0);
					  avvisi->setPosition(200,scrH-5);
					  avvisi->render();
				  }
				  else if (tempoAvvisi>=4900 && tempoAvvisi<5050){
					  avvisi->setText((char *)"Tempo ultimo giro -> ",255,0,0);
					  avvisi->setPosition(scrW-550,scrH-5);
					  avvisi->render();
					  avvisi->setText((char *)"e nr giro attuale",255,0,0);
					  avvisi->setPosition(scrW-550,scrH-35);
					  avvisi->render();
				  }
				  else if (tempoAvvisi>=5100 && tempoAvvisi<5250){
					  avvisi->setText((char *)"Tempo giro migliore -> ",255,0,0);
					  avvisi->setPosition(scrW-550,scrH-40);
					  avvisi->render();
					  avvisi->setText((char *)"e nr giro migliore ",255,0,0);
					  avvisi->setPosition(scrW-550,scrH-75);
					  avvisi->render();
				  }
			  }
		  }
	  }
	  else if (tempoAvvisi>=5400 && tempoAvvisi<9400){
		  if (tempoAvvisi>=5400 && tempoAvvisi<5600){
			  if (tempoAvvisi<5550){
				  avvisi->setText((char *)"In ogni modalità si possono",255,0,0);
				  avvisi->setPosition(scrW/2-240,scrH-30);
				  avvisi->render();
				  avvisi->setText((char *)"uilizzare diversi comandi",255,0,0);
				  avvisi->setPosition(scrW/2-220,scrH-65);
				  avvisi->render();
			  }
		  }
		  else if (tempoAvvisi>=5600 && tempoAvvisi<6900){
			  if (tempoAvvisi<6800){
				  avvisi->setText((char *)"Con F1 è possibile cambiare camera:",255,0,0);
				  avvisi->setPosition(scrW/2-290,scrH-30);
				  avvisi->render();
				  if (tempoAvvisi>=5800 && tempoAvvisi<6000){
					  cameraType=CAMERA_PILOT;
					  avvisi->setText((char *)"- camera guidatore",255,0,0);
					  avvisi->setPosition(scrW/2-290,scrH-65);
					  avvisi->render();
				  }
				  else if (tempoAvvisi>=6000 && tempoAvvisi<6200){
					  cameraType=CAMERA_MOUSE;
					  avvisi->setText((char *)"- camera fissa col mondo",255,0,0);
					  avvisi->setPosition(scrW/2-290,scrH-65);
					  avvisi->render();
					  avvisi->setText((char *)"  (si può spostare la visuale col mouse)",255,0,0);
					  avvisi->setPosition(scrW/2-290,scrH-100);
					  avvisi->render();
				  }
				  else if (tempoAvvisi>=6200 && tempoAvvisi<6400){
					  cameraType=CAMERA_BACK_CAR;
					  avvisi->setText((char *)"- camera da dietro",255,0,0);
					  avvisi->setPosition(scrW/2-290,scrH-65);
					  avvisi->render();
				  }
				  else if (tempoAvvisi>=6400 && tempoAvvisi<6600){
					  cameraType=CAMERA_TOP_FIXED;
					  avvisi->setText((char *)"- camera angolare",255,0,0);
					  avvisi->setPosition(scrW/2-290,scrH-65);
					  avvisi->render();
				  }
				  else if (tempoAvvisi>=6600 && tempoAvvisi<6800){
					  cameraType=CAMERA_TOP_CAR;
					  avvisi->setText((char *)"- camera dall'alto",255,0,0);
					  avvisi->setPosition(scrW/2-290,scrH-65);
					  avvisi->render();
				  }
			  }
		  }
		  else if (tempoAvvisi>=6900 && tempoAvvisi<7400){
			  if (tempoAvvisi<7350){
				  avvisi->setText((char *)"Con F2 si può attivare/disattivare la rappresentazione",255,0,0);
				  avvisi->setPosition(scrW/2-370,scrH-30);
				  avvisi->render();
				  avvisi->setText((char *)"tramite wireframe",255,0,0);
				  avvisi->setPosition(scrW/2-110,scrH-65);
				  avvisi->render();
				  if (tempoAvvisi>=7100 && tempoAvvisi<7200){
					  useWireframe=true;
				  }
				  else if (tempoAvvisi>=7200 && tempoAvvisi<7300){
					  useWireframe=false;
				  }
			  }
		  }
		  else if (tempoAvvisi>=7400 && tempoAvvisi<7900){
			  if (tempoAvvisi<7850){
				  avvisi->setText((char *)"Con F3 si possono accendere/spegnere i fanali",255,0,0);
				  avvisi->setPosition(scrW/2-370,scrH-30);
				  avvisi->render();
				  if (tempoAvvisi>=7600 && tempoAvvisi<7700){
					  if (morning)
						  useHeadlight=true;
					  else
						  useHeadlight=false;
				  }
				  else if (tempoAvvisi>=7700 && tempoAvvisi<7800){
					  if (morning)
						  useHeadlight=false;
					  else
						  useHeadlight=true;				  }
			  }
		  }
		  else if (tempoAvvisi>=7900 && tempoAvvisi<8400){
			  if (tempoAvvisi<8350){
				  avvisi->setText((char *)"Con F4 si possono attivare/disattivare le ombre",255,0,0);
				  avvisi->setPosition(scrW/2-370,scrH-30);
				  avvisi->render();
				  if (tempoAvvisi>=8100 && tempoAvvisi<8200){
					  if (morning)
						  useShadow=false;
					  else
						  useShadow=true;
				  }
				  else if (tempoAvvisi>=8200 && tempoAvvisi<8300){
					  if (morning)
						  useShadow=true;
					  else
						  useShadow=false;				  }
			  }
		  }
		  else if (tempoAvvisi>=8400 && tempoAvvisi<8900){
			  if (tempoAvvisi<8850){
				  avvisi->setText((char *)"Infine con O si può scegliere la modalità fra:",255,0,0);
				  avvisi->setPosition(scrW/2-370,scrH-30);
				  avvisi->render();
				  avvisi->setText((char *)"-> Giro Libero",255,0,0);
				  avvisi->setPosition(scrW/2-370,scrH-65);
				  avvisi->render();
				  avvisi->setText((char *)"-> Corsa",255,0,0);
				  avvisi->setPosition(scrW/2-370,scrH-100);
				  avvisi->render();
				  avvisi->setText((char *)"-> Demo",255,0,0);
				  avvisi->setPosition(scrW/2-370,scrH-135);
				  avvisi->render();
				  avvisi->setText((char *)"Sia di giorno che di notte",255,0,0);
				  avvisi->setPosition(scrW/2-370,scrH-170);
				  avvisi->render();
			  }
		  }
		  else if (tempoAvvisi>=8900 && tempoAvvisi<9400){
			  avvisi->setText((char *)"BUON DIVERTIMENTO!",255,0,0);
			  avvisi->setPosition(scrW/2-150,scrH-30);
			  avvisi->render();
			  avvisi->setText((char *)"(Premere Q per uscire dal gioco)",255,0,0);
			  avvisi->setPosition(scrW/2-250,scrH-65);
			  avvisi->render();
		  }

	  }
	  else if (tempoAvvisi>=9400){
		  options();
	  }
	  if (tempoAvvisi<4400){
		  if (dritto){
			  tastiAttivi->setText((char *)"W",0,0,0);
			  tastiAttivi->setPosition(80,scrH/2-50);
			  tastiAttivi->render();
		  }
		  else{
			  tasti->setText((char *)"W",0,0,0);
			  tasti->setPosition(80,scrH/2-50);
			  tasti->render();
		  }
		  if (sx){
			  tastiAttivi->setText((char *)"A",0,0,0);
			  tastiAttivi->setPosition(40,scrH/2-100);
			  tastiAttivi->render();
		  }
		  else{
			  tasti->setText((char *)"A",0,0,0);
			  tasti->setPosition(40,scrH/2-100);
			  tasti->render();
		  }
		  if (dietro){
			  tastiAttivi->setText((char *)"S",0,0,0);
			  tastiAttivi->setPosition(80,scrH/2-100);
			  tastiAttivi->render();
		  }
		  else{
			  tasti->setText((char *)"S",0,0,0);
			  tasti->setPosition(80,scrH/2-100);
			  tasti->render();
		  }
		  if (dx){
			  tastiAttivi->setText((char *)"D",0,0,0);
			  tastiAttivi->setPosition(120,scrH/2-100);
			  tastiAttivi->render();
		  }
		  else{
			  tasti->setText((char *)"D",0,0,0);
			  tasti->setPosition(120,scrH/2-100);
			  tasti->render();
		  }
	  }


  }



  sprintf(bufferFps,"FPS: %d",(int)fps);
  fps_text->setText(bufferFps,0,0,255);
  fps_text->setPosition(10,50);
  fps_text->render();
  glBegin(GL_QUADS);
  float y=scrH*fps/100;
  float ramp=fps/100;
  glColor3f(1-ramp,0,ramp);
  glVertex2d(20,70);
  glVertex2d(20,70+y);
  glVertex2d(10,70+y);
  glVertex2d(10,70);
  glEnd();
  
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_LIGHTING);
  
  glFinish();
  // ho finito: buffer di lavoro diventa visibile
  SDL_GL_SwapWindow(win);
}

void redraw(){
  // ci automandiamo un messaggio che (s.o. permettendo)
  // ci fara' ridisegnare la finestra
  SDL_Event e;
  e.type=SDL_WINDOWEVENT;
  e.window.event=SDL_WINDOWEVENT_EXPOSED;
  SDL_PushEvent(&e);
}



void RunLoop(){

	  while (!done) {

	    SDL_Event e;

	    // guardo se c'e' un evento:
	    if (SDL_PollEvent(&e)) {
	     // se si: processa evento
	     switch (e.type) {
	      case SDL_KEYDOWN:
	        car.controller.EatKey(e.key.keysym.sym, keymap , true);
	        if (e.key.keysym.sym==SDLK_F1) cameraType=(cameraType+1)%CAMERA_TYPE_MAX;
	        if (e.key.keysym.sym==SDLK_F2) useWireframe=!useWireframe;
	        if (e.key.keysym.sym==SDLK_F3) useHeadlight=!useHeadlight;
	        if (e.key.keysym.sym==SDLK_F4) useShadow=!useShadow;
	        if (e.key.keysym.sym==SDLK_o) options();
	        if (e.key.keysym.sym==SDLK_q) done=1;
	        break;
	      case SDL_KEYUP:
	        car.controller.EatKey(e.key.keysym.sym, keymap , false);
	        break;
	      case SDL_QUIT:
	          done=1;   break;
	      case SDL_WINDOWEVENT:
	         // dobbiamo ridisegnare la finestra
	          if (e.window.event==SDL_WINDOWEVENT_EXPOSED)
	            rendering(win);
	          else{
	           windowID = SDL_GetWindowID(win);
	           if (e.window.windowID == windowID)  {
	             switch (e.window.event)  {
	                  case SDL_WINDOWEVENT_SIZE_CHANGED:  {
	                     scrW = e.window.data1;
	                     scrH = e.window.data2;
	                     glViewport(0,0,scrW,scrH);
	                     rendering(win);
	                     //redraw(); // richiedi ridisegno
	                     break;
	                  }
	             }
	           }
	         }
	      break;

	      case SDL_MOUSEMOTION:
	        if (e.motion.state & SDL_BUTTON(1) & cameraType==CAMERA_MOUSE) {
	          viewAlpha+=e.motion.xrel;
	          viewBeta +=e.motion.yrel;
	          if (viewBeta<+5) viewBeta=+5; //per non andare sotto la macchina
	          if (viewBeta>+90) viewBeta=+90;

	        }
	        break;

	     case SDL_MOUSEWHEEL:
	       if (e.wheel.y < 0 ) {
	         // avvicino il punto di vista (zoom in)
	         eyeDist=eyeDist*0.9;
	         if (eyeDist<1) eyeDist = 1;
	       };
	       if (e.wheel.y > 0 ) {
	         // allontano il punto di vista (zoom out)
	         eyeDist=eyeDist/0.9;
	       };
	     break;

	     case SDL_JOYAXISMOTION: /* Handle Joystick Motion */
	        if( e.jaxis.axis == 0)
	         {
	            if ( e.jaxis.value < -3200  )
	             {
	              car.controller.Joy(0 , true);
	              car.controller.Joy(1 , false);
	             }
	            if ( e.jaxis.value > 3200  )
	            {
	              car.controller.Joy(0 , false);
	              car.controller.Joy(1 , true);
	            }
	            if ( e.jaxis.value >= -3200 && e.jaxis.value <= 3200 )
	             {
	              car.controller.Joy(0 , false);
	              car.controller.Joy(1 , false);
	             }
		    rendering(win);
	        }
	        break;
	      case SDL_JOYBUTTONDOWN: /* Handle Joystick Button Presses */
	        if ( e.jbutton.button == 0 )
	        {
	           car.controller.Joy(2 , true);
	        }
	        if ( e.jbutton.button == 2 )
	        {
	           car.controller.Joy(3 , true);
	        }
	        break;
	      case SDL_JOYBUTTONUP: /* Handle Joystick Button Presses */
	           car.controller.Joy(2 , false);
	           car.controller.Joy(3 , false);
	        break;
	     }
	    } else {
	      // nessun evento: siamo IDLE

	      Uint32 timeNow=SDL_GetTicks(); // che ore sono?

	      if (timeLastInterval+fpsSampling<timeNow) {
	        fps = 1000.0*((float)fpsNow) /(timeNow-timeLastInterval);
	        fpsNow=0;
	        timeLastInterval = timeNow;
	      }

	      bool doneSomething=false;
	      int guardia=0; // sicurezza da loop infinito

	      // finche' il tempo simulato e' rimasto indietro rispetto
	      // al tempo reale...
	      while (nstep*PHYS_SAMPLING_STEP < timeNow) {
	    	switch (modeType){
	    	case MODE_DEMO:
	    			//demo=true;
	    			demox=true;
					car.DoDemox(dritto,sx,dx,dietro);
					if (tempoAvvisi<9401)
						tempoAvvisi++;
					break;
	    	case MODE_RUN:
					if (countdownMs>=600){
						corsaMs++;
						car.DoStep();
					}
					else
						countdownMs++;
					break;
	    	case MODE_FREE:
				car.DoStep();
				break;

	    	}

	        nstep++;
	        doneSomething=true;
	        timeNow=SDL_GetTicks();
	        if (guardia++>1000) {done=true; printf("STOP\nguardia: %d\ntimeNow= %d\ntimeLast= %d\nnstep= %d",guardia, timeNow, timeLastInterval, nstep); break;} // siamo troppo lenti!
	      }

	      if (doneSomething)
	    	  rendering(win);
	      //redraw();
	      else {
	        // tempo libero!!!
	      }
	    }
	  }
}
void optRendering(SDL_Window *win){
	  glViewport(0,0, scrW, scrH);
	char modeNames[5][256];
	glDisable(GL_DEPTH_TEST);

	glClearColor(0,0,0,0);
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

	SetCoordToPixel();

	glDisable(GL_LIGHTING);

	glBegin(GL_QUADS);
	glColor3f(0.13, 0.54, 0.13);

	glVertex2i(0, 0);
	glVertex2i(scrW, 0);
	glVertex2i(scrW,  scrH);
	glVertex2i(0,  scrH);
	glEnd();
	SetCoordToPixel();


	sprintf(modeNames[0], "Premi un numero per selezionare la modalità:");
	sprintf(modeNames[1], "(SHIFT+NUMERO per la modalità notte)");
	sprintf(modeNames[2], "1 : Giro Libero");
	sprintf(modeNames[3], "2 : Corsa a tempo");
	sprintf(modeNames[4], "3 : Tutorial");

	   for(int i= 0; i<5;i++)
	   {
			stats_text-> setText(modeNames[i], 0,0,255);
			stats_text-> setPosition(10,scrH - 30* i); //12 Ã¨ Pixel dimension per il font
			stats_text->render();
	   }

	glFinish();
	SDL_GL_SwapWindow(win);
}
void options(){
	optRendering(win);
	bool loop=true;
	bool esc=false;


	bool lshift=false;
	bool rshift=false;
	morning=true;
	demo=false;
	while (loop){

		SDL_Event event;
		if (SDL_PollEvent(&event))
		{
			switch(event.type)
			{
					case SDL_KEYDOWN:

						switch (event.key.keysym.sym)
						{
							case SDLK_LSHIFT:
								lshift=true;
								break;
							case SDLK_RSHIFT:
								rshift=true;
								break;
							case SDLK_1:
								modeType = MODE_FREE;
								demox=false; freeRun=true;
								if (lshift||rshift){
									morning=false;
									useHeadlight=true;
								}
								else{
									morning=true;
									useHeadlight=false;
								}
								loop = false;
								break;
							case SDLK_3:
								modeType = MODE_DEMO;
								//demo=true;
								demox=true; freeRun=false;
								if (lshift||rshift){
									morning=false;
									useHeadlight=true;
								}
								else{
									morning=true;
									useHeadlight=false;
								}
								loop = false;
								break;
							case SDLK_2:
								modeType = MODE_RUN;
								demox=false;  freeRun=false;
								if (lshift||rshift){
									morning=false;
									useHeadlight=true;
								}
								else{
									morning=true;
									useHeadlight=false;
								}
								loop = false;
								break;
							case SDLK_ESCAPE:
								esc=true; loop=false;
								break;
							case SDLK_q:
								loop=false; done=1; break;


						}
						break;
						case SDL_KEYUP:
							switch (event.key.keysym.sym){
							case SDLK_LSHIFT:
								lshift=false;
								break;
							case SDLK_RSHIFT:
								rshift=false;
								break;
							}


						break;
						case SDL_WINDOWEVENT:
						 // dobbiamo ridisegnare la finestra
						  if (event.window.event==SDL_WINDOWEVENT_EXPOSED)
							optRendering(win);
						  else{
						   windowID = SDL_GetWindowID(win);
						   if (event.window.windowID == windowID)  {
							 switch (event.window.event)  {
								  case SDL_WINDOWEVENT_SIZE_CHANGED:  {
									 scrW = event.window.data1;
									 scrH = event.window.data2;
									 glViewport(0,0,scrW,scrH);
									 optRendering(win);
									 //redraw(); // richiedi ridisegno
									 break;
								  }
							 }
						   }
						 }
						 break;
						case SDL_QUIT:
						loop=false; done=1; break;

			}
		}
		Uint32 timeNow=SDL_GetTicks(); // che ore sono?
		timeLastInterval = timeNow;
		while (nstep*PHYS_SAMPLING_STEP < timeNow) {
			nstep++;
		}
	}

	if (!done && !esc){
		init();
	}
}

void init(){
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);
	centesimiGara=0;
	secondiGara=0;
	minutiGara=0;
	centesimiGiro=0;
	secondiGiro=0;
	minutiGiro=0;
	numeroGiro=0;
	giroVeloce=0;
	countdownMs=0;
	corsaMs=0;
	giroMs=0;
	tempoAvvisi=0;
	nuovoGiro=false;
	startGiro=true;
	car.Init();
}
int main(int argv, char** args)
{
	string line;
	txtFile.open("par.txt", ios::in);
	if (txtFile.is_open()){
		demox=false;
		freeRun=true;
		dx=sx=dritto=dietro=true;
		modeType=MODE_FREE;
		if (getline(txtFile, line)){
			minutiGiroV=std::stoi(line);
		}
		if (getline(txtFile, line)){
			secondiGiroV=std::stoi(line);
		}
		if (getline(txtFile, line)){
			centesimiGiroV=std::stoi(line);
		}
		txtFile.close();
	}
	else {
		txtFile.open("par.txt", ios::out);
		txtFile << "0\n0\n0";
		txtFile.close();
	}

  // inizializzazione di SDL
  SDL_Init( SDL_INIT_VIDEO | SDL_INIT_JOYSTICK);
  if (TTF_Init () < 0)
  {
	  fprintf (stderr, "Couldn't initialize TTF: %s\n", SDL_GetError ());
	  SDL_Quit ();
	  return (2);
  }

  font = TTF_OpenFont ("OpenSans-Regular.ttf", 30);

  if (font == NULL)
  {
	  fprintf (stderr, "Couldn't load font\n");
  }

  font_models = TTF_OpenFont ("OpenSans-ExtraBold.ttf", 120);


  if (font_models == NULL)
  {
	  fprintf (stderr, "Couldn't load font\n");
  }

  fontAvvisi = TTF_OpenFont ("OpenSans-Regular.ttf", 30);
  if (fontAvvisi == NULL)
  {
	  fprintf (stderr, "Couldn't load font\n");
  }

  fontTasti=TTF_OpenFont ("OpenSans-Regular.ttf", 30);
  if (fontTasti == NULL)
  {
	  fprintf (stderr, "Couldn't load font\n");
  }

  fontTastiAttivi=TTF_OpenFont ("OpenSans-ExtraBold.ttf", 36);
  if (fontTasti == NULL)
  {
	  fprintf (stderr, "Couldn't load font\n");
  }

  countdown=new GLText(1,font_models);
  secondi=new GLText(1,font);
  tempoGiro=new GLText(1,font);
  tempoMigliore=new GLText(1,font);
  fps_text=new GLText(1,font);
  avvisi=new GLText(1,fontAvvisi);
  tasti=new GLText(1,fontTasti);
  tastoW=new GLText(1,fontTasti);
  tastoA=new GLText(1,fontTasti);
  tastoS=new GLText(1,fontTasti);
  tastoD=new GLText(1,fontTasti);
  stats_text = new GLText(1,font);
  tastiAttivi=new GLText(1,fontTastiAttivi);


  SDL_JoystickEventState(SDL_ENABLE);
  joystick = SDL_JoystickOpen(0);

  SDL_GL_SetAttribute( SDL_GL_DEPTH_SIZE, 16 ); 
  SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );

  // facciamo una finestra di scrW x scrH pixels
  win=SDL_CreateWindow("progettocar4", 0, 0, scrW, scrH, SDL_WINDOW_OPENGL|SDL_WINDOW_RESIZABLE);

  //Create our opengl context and attach it to our window
  mainContext=SDL_GL_CreateContext(win);
  SDL_GL_SetSwapInterval(1);


  glEnable(GL_LIGHT0);
  glEnable(GL_NORMALIZE); // opengl, per favore, rinormalizza le normali 
                          // prima di usarle
  glFrontFace(GL_CW); // consideriamo Front Facing le facce ClockWise
  glEnable(GL_COLOR_MATERIAL);
  glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
  glEnable(GL_POLYGON_OFFSET_FILL); // caro openGL sposta i 
                                    // frammenti generati dalla
                                    // rasterizzazione poligoni
  glPolygonOffset(1,1);             // indietro di 1
  
  if (!LoadTexture(0,(char *)"logo.jpg")) return 0;
  if (!LoadTexture(1,(char *)"envmap_flipped.jpg")) return 0;
  if (!LoadTexture(2,(char *)"sky_ok.jpg")) return -1;
  if (!LoadTexture(6,(char *)"night_sky.jpg")) return -1;
 init();
  RunLoop();
  SDL_GL_DeleteContext(mainContext);
  SDL_DestroyWindow(win);
  SDL_Quit();
return (0);
}

