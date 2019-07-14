

#include <stdio.h>
#include <math.h>

#include <GL/gl.h>
#include <GL/glu.h>

#include <vector> // la classe vector di STL 

#include "car.h"
#include "point3.h"
#include "mesh.h"
#include "glm.h"


GLMmodel*  modelMotrice;        /* glm model data structure */
GLfloat    scale;		        /* original scale factor */
GLfloat    smoothing_angle = 90.0;	/* smoothing angle */
GLuint     modelListMotrice = 0; /* display list for object */

GLMmodel*  modelRimorchio;
GLuint     modelListRimorchio = 0;

GLMmodel*  modelRuoteA1;
GLuint     modelListRuoteA1 = 0;

GLMmodel*  modelRuoteP1A;
GLuint     modelListRuoteP1A = 0;

GLMmodel*  modelRuoteP1B;
GLuint     modelListRuoteP1B = 0;

GLMmodel*  modelRuoteR1A;
GLuint     modelListRuoteR1A = 0;

GLMmodel*  modelRuoteR1B;
GLuint     modelListRuoteR1B = 0;

GLMmodel*  modelCerchioneA1;
GLuint     modelListCerchioneA1 = 0;

GLMmodel*  modelCerchioneP1A;
GLuint     modelListCerchioneP1A = 0;

GLMmodel*  modelCerchioneP1B;
GLuint     modelListCerchioneP1B = 0;

GLMmodel*  modelCerchioneR1A;
GLuint     modelListCerchioneR1A = 0;

GLMmodel*  modelCerchioneR1B;
GLuint     modelListCerchioneR1B = 0;

GLfloat centerMotrice[3];
GLfloat centerRimorchio[3];
GLfloat centerA1[3];
GLfloat centerP1A[3];
GLfloat centerP1B[3];
GLfloat centerR1A[3];
GLfloat centerR1B[3];

// var globale di tipo mesh

Mesh start((char *)"start.obj");
Mesh pista((char *)"pista.obj");
Mesh traguardo((char *)"traguardo.obj");
extern bool morning;
extern bool demo;
extern bool demox;
extern bool freeRun;
extern bool useHeadlight; // var globale esterna: per usare i fari
extern bool useShadow; // var globale esterna: per generare l'ombra
extern bool useWireframe;
extern bool srcW;
extern bool srcH;
// da invocare quando e' stato premuto/rilasciato il tasto numero "keycode"
void Controller::EatKey(int keycode, int* keymap, bool pressed_or_released)
{
  for (int i=0; i<NKEYS; i++){
    if (keycode==keymap[i]) key[i]=pressed_or_released;
  }
}

// da invocare quando e' stato premuto/rilasciato un jbutton
void Controller::Joy(int keymap, bool pressed_or_released)
{
    key[keymap]=pressed_or_released;
}

// Funzione che prepara tutto per usare un env map
void SetupEnvmapTexture()
{
  // facciamo binding con la texture 1
  glBindTexture(GL_TEXTURE_2D,1);
   
  glEnable(GL_TEXTURE_2D);
  glEnable(GL_TEXTURE_GEN_S); // abilito la generazione automatica delle coord texture S e T
  glEnable(GL_TEXTURE_GEN_T);
  glTexGeni(GL_S, GL_TEXTURE_GEN_MODE , GL_SPHERE_MAP); // Env map
  glTexGeni(GL_T, GL_TEXTURE_GEN_MODE , GL_SPHERE_MAP);
  glColor3f(1,1,1); // metto il colore neutro (viene moltiplicato col colore texture, componente per componente)
  glDisable(GL_LIGHTING); // disabilito il lighting OpenGL standard (lo faccio con la texture)
}

// funzione che prepara tutto per creare le coordinate texture (s,t) da (x,y,z)
// Mappo l'intervallo [ minY , maxY ] nell'intervallo delle T [0..1]
//     e l'intervallo [ minZ , maxZ ] nell'intervallo delle S [0..1]
void SetupWheelTexture(Point3 min, Point3 max){
  glBindTexture(GL_TEXTURE_2D,0);
  glEnable(GL_TEXTURE_2D);
  glEnable(GL_TEXTURE_GEN_S);
  glEnable(GL_TEXTURE_GEN_T);

  // ulilizzo le coordinate OGGETTO
  // cioe' le coordnate originali, PRIMA della moltiplicazione per la ModelView
  // in modo che la texture sia "attaccata" all'oggetto, e non "proiettata" su esso
  glTexGeni(GL_S, GL_TEXTURE_GEN_MODE , GL_OBJECT_LINEAR);
  glTexGeni(GL_T, GL_TEXTURE_GEN_MODE , GL_OBJECT_LINEAR);
  float sz=1.0/(max.Z() - min.Z());
  float ty=1.0/(max.Y() - min.Y());
  float s[4]={0,0,sz,  - min.Z()*sz };
  float t[4]={0,ty,0,  - min.Y()*ty };
  glTexGenfv(GL_S, GL_OBJECT_PLANE, s);
  glTexGenfv(GL_T, GL_OBJECT_PLANE, t);
}

// DoStep: facciamo un passo di fisica (a delta_t costante)
//
// Indipendente dal rendering.
//
// ricordiamoci che possiamo LEGGERE ma mai SCRIVERE
// la struttura controller da DoStep
void Car::DoStep(){
  // computiamo l'evolversi della macchina
  
  float vxm, vym, vzr,vxr, vzm; // velocita' in spazio macchina
  
  // da vel frame mondo a vel frame macchina
  float cosf = cos(facing*M_PI/180.0);
  float sinf = sin(facing*M_PI/180.0);
  float cosr = cos(facingR*M_PI/180.0);
  float sinr = sin(facingR*M_PI/180.0);
  vxm = +cosf*vx - sinf*vz;
  vym = vy;
  vzm = +sinf*vx + cosf*vz;
  vxr = +cosr*vrx - sinr*vrz;
  vzr= +sinr*vrx + cosr*vrz;
  
  // gestione dello sterzo
  if ((controller.key[Controller::LEFT]&&!demox)||(giraSx&&demox)){
	  sterzo+=velSterzo; sterzoR+=velSterzoR;
	  if(vzm<=-0.05||capovolgimentoNewR>INCLINAZIONEMAX||capovolgimentoNewR<-INCLINAZIONEMAX) giroR+=velGiroR;
  }
  if ((controller.key[Controller::RIGHT]&&!demox)||(giraDx&&demox)){
	  sterzo-=velSterzo; sterzoR-=velSterzoR;
	  if(vzm<=-0.05||capovolgimentoNewR>INCLINAZIONEMAX||capovolgimentoNewR<-INCLINAZIONEMAX) giroR-=velGiroR;
  }
  if (!capovolto&&((!controller.key[Controller::LEFT]&&!demox)||(!giraSx&&demox))&&((!controller.key[Controller::RIGHT]&&!demox)||(!giraDx&&demox))&&sterzo<0.9&&(facing<(facingNewR))){
	  sterzoR-=velSterzoR/riduzioneVSR;
	  if (facing>facingNewR+ritornoRmax){
		  ritornoRmax=ritornoRmax/2;
		  riduzioneVSR=riduzioneVSR/2;
	  }
  }
  if (!capovolto&&((!controller.key[Controller::LEFT]&&!demox)||(!giraSx&&demox))&&((!controller.key[Controller::RIGHT]&&!demox)||(!giraDx&&demox))&&sterzo>-0.9&&(facing>(facingNewR))){
  	  sterzoR+=velSterzoR/riduzioneVSR;
  	if (facing<facingNewR-ritornoRmax){
  			  ritornoRmax=ritornoRmax/2;
  			  riduzioneVSR=riduzioneVSR/2;
  		  }
  }
  sterzo*=velRitornoSterzo; // ritorno a volante fermo
  sterzoR*=velRitornoSterzoR;

  if ((((!controller.key[Controller::LEFT]&&!demox)||(!giraSx&&demox)) && ((!controller.key[Controller::RIGHT]&&!demox)||(!giraDx&&demox)))||vzm>=-0.03){
	  if (capovolgimentoNewR>INCLINAZIONEMAX)
		  giroR+=velGiroR;
	  else if(capovolgimentoNewR<-INCLINAZIONEMAX)
		  giroR-=velGiroR;
	  else if (capovolgimentoNewR>0) giroR-=velGiroR;
		  else if (capovolgimentoNewR<0) giroR+=velGiroR;
  }
  giroR*=velRitornoGiroR;


  if ((controller.key[Controller::ACC]&&!demox)||(avanti&&demox)) {if (vzm>=-0.05) vzm-=accMax; else vzm=-0.05;} // accelerazione in avanti
  if ((controller.key[Controller::DEC]&&!demox)||(indietro&&demox)) {if (vzm<=+0.02) vzm+=accMax; else vzm=+0.02;} // accelerazione indietro

  // attriti (semplificando)
  vxm*=attritoX;
  vym*=attritoY;
  vzm*=attritoZ;
  vxr*=attritoX;
  vzr*=attritoZ;
  

  // l'orientamento della macchina segue quello dello sterzo
  // (a seconda della velocita' sulla z)
  facing = facing - (vzm*grip)*sterzo;
  
  facingNewR= facingNewR - (vzm*grip)*sterzoR;

  if (facing<facingR+0.01&&facing>facingR-0.01 && (((facingR<facingNewR && facingNewR-facingR<0.05) || (facingR>facingNewR && facingR-facingNewR<0.05)) )){
  	  facingNewR=facing;
  	  sterzoR=sterzo;
  	  riduzioneVSR=1;
  	  ritornoRmax=10;
    }
  if (facingNewR>facing+40 || facingNewR<facing-40){
	  if (facingNewR>facing){
		  facingNewR=facing+40;
	  }
	  else if(facingNewR<facing){
		  facingNewR=facing-40;
	  }
  }

  if (capovolgimentoNewR>INCLINAZIONEMAX){
	  if (capovolgimentoNewR<90){
		  capovolgimentoNewR=capovolgimentoNewR-(-0.05*grip)*(giroR*2);
	  }
	  else{
		  capovolgimentoNewR=90;
	  }
  }
  else if (capovolgimentoNewR<-INCLINAZIONEMAX){
	  if (capovolgimentoNewR>-90){
		  capovolgimentoNewR=capovolgimentoNewR-(-0.05*grip)*(giroR*2);
	  }
	  else{
		  capovolgimentoNewR=-90;
	  }
  }
  else{
	  capovolgimentoNewR=capovolgimentoNewR-(-0.05*grip)*giroR;
  }
  if (capovolgimentoR<=-90||capovolgimentoR>=90){
	  capovolto=true;
  }
  if ((capovolgimentoNewR>=-0.1)&&
		  (capovolgimentoNewR<=0.1)){
  		  capovolgimentoNewR=0;
  }
  if (!capovolto){
	  facingR=facingRR[ritardo];




	  facingRR[ritardo]=facingNewR;
	  capovolgimentoR=capovolgimentoRR[ritardo];
	  capovolgimentoRR[ritardo]=capovolgimentoNewR;
	  ritardo=(ritardo+1)%RITARDOMAX;
	  energia=(energia+1)%ENERGIAMAX;

	  facingOld=facing;
  }

  // rotazione mozzo ruote (a seconda della velocita' sulla z)
  float da ; //delta angolo
  da=(360.0*vzm)/(2.0*M_PI*raggioRuotaA);
  mozzoA+=da;
  da=(360.0*vzm)/(2.0*M_PI*raggioRuotaP);
  mozzoP+=da;
  da=(360.0*vzr)/(2.0*M_PI*raggioRuotaP);
  mozzoPR+=da;

  if (!capovolto)
	  mozzoPR=mozzoP;
  
  // ritorno a vel coord mondo
  vx = +cosf*vxm + sinf*vzm;
  vy = vym;
  vz = -sinf*vxm + cosf*vzm;
  vrx = +cosr*vxr + sinr*vzr;
  vrz= +sinr*vxr + cosr*vzr;
  if (!capovolto){
	  vrz=vz;
	  vrx=vx;
  }
  
  // posizione = posizione + velocita * delta t (ma delta t e' costante)
  px+=vx;
  py+=vy;
  pz+=vz;
  prz+=vrz;
  prx+=vrx;
  if ((px>=traguardo.bbmin.X()*0.75&&px<=traguardo.bbmax.X()*0.75)&&
		  (pz<=traguardo.bbmax.Z()*0.75&&pz>=traguardo.bbmin.Z()*0.75)){
	  fineGiro=true;

  }
  else{
	  fineGiro=false;
  }
} 

void Car::DoDemox(float dritto, float sx, float dx, float dietro){
	avanti=dritto;
	giraSx=sx;
	giraDx=dx;
	indietro=dietro;
	DoStep();
}
/*
void Car::DoDemo(){

	//angolo=0;
    float demoarcNext=0;
    avanti=true;
    demo=true;
    pxNext=lineaGuida.v[(demoVertexi+1)%demoVertexSize].p.X()*0.75;
    pzNext=lineaGuida.v[(demoVertexi+1)%demoVertexSize].p.Z()*0.75;
    pxNN=lineaGuida.v[(demoVertexi+2)%demoVertexSize].p.X()*0.75;
    pzNN=lineaGuida.v[(demoVertexi+2)%demoVertexSize].p.Z()*0.75;
    if ((facing>=0.0 && facing<90.0)){
    	if (px<pxNext && pz<pzNext){
    		demoVertexi=(demoVertexi+1)%demoVertexSize;
    		pxNext=lineaGuida.v[(demoVertexi+1)%demoVertexSize].p.X()*0.75;
    		pzNext=lineaGuida.v[(demoVertexi+1)%demoVertexSize].p.Z()*0.75;
    		pxNN=lineaGuida.v[(demoVertexi+2)%demoVertexSize].p.X()*0.75;
    		pzNN=lineaGuida.v[(demoVertexi+2)%demoVertexSize].p.Z()*0.75;
    		demogammaNN=atan((pzNext-pz)/(pxNext-px));
    	    demogamma=atan((pzNN-pzNext)/(pxNN-pxNext));

    	    demogamma=demogammaNN;
    	    demogammaNN=atan((pzNN-pzNext)/(pxNN-pxNext));

    	    angolo=facing+demogamma;

    	}

    }
    else if (facing>=90.0 && facing<180.0){
    	if (px<pzNext && pz<pzNext){
    		//printf("\nDENTRO\n");
			demoVertexi=(demoVertexi+1)%demoVertexSize;
			pxNext=lineaGuida.v[(demoVertexi+1)%demoVertexSize].p.X()*0.75;
			pzNext=lineaGuida.v[(demoVertexi+1)%demoVertexSize].p.Z()*0.75;
			pxNN=lineaGuida.v[(demoVertexi+2)%demoVertexSize].p.X()*0.75;
			pzNN=lineaGuida.v[(demoVertexi+2)%demoVertexSize].p.Z()*0.75;


    		demogammaNN=abs(atan((pzNext-pz)/(pxNext-px)));
		    demogamma=demogammaNN;
		    demogammaNN=abs(atan((pzNN-pzNext)/(pxNN-pxNext)));
    	    angolo=facing+demogamma;


    	}
    }
    else if (facing>=180.0 && facing<270.0){
    	if (px>pxNext && pz>pzNext ){
    		//printf("\nDENTRAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAnDENTRAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\n");
    		//printf("\nDENTRAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAnDENTRAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\n");
    		//printf("\nDENTRAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAnDENTRAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\n");

			demoVertexi=(demoVertexi+1)%demoVertexSize;
			pxNext=lineaGuida.v[(demoVertexi+1)%demoVertexSize].p.X()*0.75;
			pzNext=lineaGuida.v[(demoVertexi+1)%demoVertexSize].p.Z()*0.75;
			pxNN=lineaGuida.v[(demoVertexi+2)%demoVertexSize].p.X()*0.75;
			pzNN=lineaGuida.v[(demoVertexi+2)%demoVertexSize].p.Z()*0.75;


    		demogammaNN=abs(atan((pzNext-pz)/(pxNext-px)));
		    demogamma=abs(atan((pzNN-pz)/(pxNN-px)));

		    demogamma=demogammaNN;

		    demogammaNN=abs(atan((pzNN-pzNext)/(pxNN-pxNext)));
    	    angolo=facing+demogamma;
    	    //demogammaLast=abs(demogamma-demogammaLast);



    	}
    }
    else if (facing>=270.0 && facing<360.0){
    	if (px>pxNext && pz>pzNext && demogamma>0){
    		//printf("\nDENTROOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOnDENTROOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO\n");
    		//printf("\nDENTROOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOnDENTROOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO\n");
    		//printf("\nDENTROOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOnDENTROOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO\n");

			demoVertexi=(demoVertexi+1)%demoVertexSize;
			pxNext=lineaGuida.v[(demoVertexi+1)%demoVertexSize].p.X()*0.75;
			pzNext=lineaGuida.v[(demoVertexi+1)%demoVertexSize].p.Z()*0.75;
			pxNN=lineaGuida.v[(demoVertexi+2)%demoVertexSize].p.X()*0.75;
			pzNN=lineaGuida.v[(demoVertexi+2)%demoVertexSize].p.Z()*0.75;

			demogammaNN=atan((pzNext-pz)/-(pxNext-px));
		    demogamma=(atan((pzNN-pzNext)/-(pxNN-pxNext)));
		    demogamma=demogammaNN;
		    demogammaNN=atan((pzNN-pzNext)/-(pxNN-pxNext));
    	    angolo=facing+demogamma;
    	    //demogammaLast=abs(demogamma-demogammaLast);



		}
    }



    if (facing<0)
    	facing=360+facing;
    facing=fmod(facing,360.0);

    //printf("facing= %f , pz= %f , px= %f , pzNext= %f , pxNext= %f , demogammaNN= %f , demogamma= %f , angolo= %f\n",facing,pz,px,pzNext,pxNext,demogammaNN,demogamma,angolo);
    if ((abs(demogamma-demogammaNN)>=0.5 ||(demogamma<0 && demogammaNN<0 && abs(demogamma+demogammaNN)>=0.5 ))){
    	indietro=true;
    	avanti=false;
    }
    else{
    	avanti=true;
    	indietro=false;
    }
    if (((facing>=0.0 && facing<180.0))){
    	if (facing>=0 && facing<90 && demogammaNN>0.0 && pz<pzNext && ((facing<angolo && demogammaNN<demogamma && pxNN<pxNext)||(facing>angolo && demogammaNN>demogamma))){
    		giraSx=true;
    		giraDx=false;
    	}
    	else if ((facing>90 && demogammaNN>0 && demogammaNN>demogamma)||(facing<90 &&  ((((demogammaNN<demogamma && demogammaNN>0)||(demogammaNN>demogamma && demogammaNN<0)||(demogammaNN<0&&demogamma<0)) && facing<45 && ((px>pxNext && pxNN<pxNext)||(pxNN>pxNext)) && demogammaNN<0)||(facing>45 &&  (((pxNN>pxNext||px<pxNext) && demogamma>0 && demogammaNN>0)||(pxNN>pxNext && demogamma<0 && demogammaNN<0)))))){
    		giraSx=false;
    		giraDx=true;
    	}
    	else{
    		giraSx=false;
    		giraDx=false;
    	}
    }
    else{

		if (((facing<angolo) && demogamma>0.0 && (((facing<315&& facing>225 && demogammaNN<demogamma&&pzNN<pzNext&&pz>pzNext&& demogamma>0.0)||(facing>315&&demogammaNN<demogamma&&pzNN<pzNext && pz>pzNext&& demogamma>0.0))||px<0 || ((facing<225 && demogammaNN<demogamma && ((px<pxNext && demogamma>0.0)||(px>pxNext&&demogamma<0))))))||(facing>angolo && ((((pzNext<pzNN && pxNext<pxNN)||(pzNN<pzNext && pz>pzNext&& pxNext<pxNN) || (pz<pzNext && px>pxNext)) && (((facing>225 && facing<270)) && px>0 && demogammaNN<demogamma)&&((demogamma>0.0)||(demogamma<0 && demogammaNN>0)))||(facing>315&&px>0&&((demogammaNN<0&&demogammaNN<demogamma)||(demogamma<0 && demogammaNN>0))&&pz>pzNext&&pzNN<pzNext&&pxNext<pxNN)||(facing<225&&demogammaNN<demogamma&&((pz>pzNext&&px<pxNext)||(pz<pzNext&&px>pxNext&&demogamma<0)))||(facing<315 && facing>270 &&pz>pzNext&&px>pxNext&&demogammaNN>0)))){
			giraSx=true;
			giraDx=false;
		}
		else if (( ((demogammaNN>0 && pzNN>pzNext&& pxNN>pxNext && demogammaNN>demogamma)||( facing<315 && ((demogammaNN<0 && pz<pzNext &&  pxNN>pxNext && (facing<angolo+180 || facing>225) )||(demogammaNN>0 && demogammaNN<demogamma && pxNN>pxNext && ((pz<pzNext && (facing<angolo||facing>(180+angolo)) && px>pxNext)||(px>pxNext && pz>pzNext&& pzNN>pzNext && (facing>angolo&&!(facing>(180+angolo))))))))||(facing>315 && demogammaNN<demogamma && ((pzNN>pzNext )||(pzNN<pzNext&&pz<pzNext)) && pxNN>pxNext)) && px>0  && pz>0 && facing>225)||(facing<225 && demogammaNN>demogamma && px>pxNext && pz>pzNext&& facing<angolo)||(facing>315&&demogamma<0&&demogammaNN<0&&px<pxNext)){
			giraSx=false;
			giraDx=true;
		}

    	else{
    		giraSx=false;
    		giraDx=false;
    	}
    }
    DoStep();






}
*/


void drawPista () {

        glPushMatrix();

       	glColor3f(0.8,0.8,0.8);
        //glScalef(0.2, 1.0, 0.2);
        glScalef(0.75, 1.0, 0.75);
        glTranslatef(0,0.01,0);
        //pista.RenderNxV();
        pista.RenderNxF();
        glPushMatrix();
		glTranslatef(0,0.01,0);
		glColor3f(0.2,0.2,0.2);
		traguardo.RenderNxF();
		start.RenderNxF();
        glPopMatrix();

        glPopMatrix();
}



void Controller::Init(){
  for (int i=0; i<NKEYS; i++) key[i]=false;
}

void Car::Init(){
	modelMotrice=glmReadOBJ((char*)"motriceScaled1.obj");
	glmFacetNormals(modelMotrice);
	glmVertexNormals(modelMotrice, smoothing_angle);
	glmComputeBoundingBox(modelMotrice);
	glmCenter(modelMotrice,centerMotrice);


	modelRimorchio=glmReadOBJ((char*)"rimorchioScaled1.obj");
	glmFacetNormals(modelRimorchio);
	glmVertexNormals(modelRimorchio, smoothing_angle);
	glmComputeBoundingBox(modelRimorchio);
	glmCenter(modelRimorchio,centerRimorchio);


	modelRuoteA1=glmReadOBJ((char*)"ruoteA1.obj");
	glmFacetNormals(modelRuoteA1);
	glmVertexNormals(modelRuoteA1, smoothing_angle);
	glmComputeBoundingBox(modelRuoteA1);
	glmCenter(modelRuoteA1,centerA1);

	modelRuoteP1A=glmReadOBJ((char*)"ruoteP1A.obj");
	glmFacetNormals(modelRuoteP1A);
	glmVertexNormals(modelRuoteP1A, smoothing_angle);
	glmComputeBoundingBox(modelRuoteP1A);
	glmCenter(modelRuoteP1A,centerP1A);

	modelRuoteP1B=glmReadOBJ((char*)"ruoteP1B.obj");
	glmFacetNormals(modelRuoteP1B);
	glmVertexNormals(modelRuoteP1B, smoothing_angle);
	glmComputeBoundingBox(modelRuoteP1B);
	glmCenter(modelRuoteP1B,centerP1B);

	modelRuoteR1A=glmReadOBJ((char*)"ruoteR1A.obj");
	glmFacetNormals(modelRuoteR1A);
	glmVertexNormals(modelRuoteR1A, smoothing_angle);
	glmComputeBoundingBox(modelRuoteR1A);
	glmCenter(modelRuoteR1A,centerR1A);

	modelRuoteR1B=glmReadOBJ((char*)"ruoteR1B.obj");
	glmFacetNormals(modelRuoteR1B);
	glmVertexNormals(modelRuoteR1B, smoothing_angle);
	glmComputeBoundingBox(modelRuoteR1B);
	glmCenter(modelRuoteR1B,centerR1B);

	modelCerchioneA1=glmReadOBJ((char*)"cerchioneAnteriore1.obj");
	glmFacetNormals(modelCerchioneA1);
	glmVertexNormals(modelCerchioneA1, smoothing_angle);
	glmComputeBoundingBox(modelCerchioneA1);

	modelCerchioneP1A=glmReadOBJ((char*)"cerchionePosteriore1A.obj");
	glmFacetNormals(modelCerchioneP1A);
	glmVertexNormals(modelCerchioneP1A, smoothing_angle);
	glmComputeBoundingBox(modelCerchioneP1A);

	modelCerchioneP1B=glmReadOBJ((char*)"cerchionePosteriore1B.obj");
	glmFacetNormals(modelCerchioneP1B);
	glmVertexNormals(modelCerchioneP1B, smoothing_angle);
	glmComputeBoundingBox(modelCerchioneP1B);

	modelCerchioneR1A=glmReadOBJ((char*)"cerchioneR1A.obj");
	glmFacetNormals(modelCerchioneR1A);
	glmVertexNormals(modelCerchioneR1A, smoothing_angle);
	glmComputeBoundingBox(modelCerchioneR1A);

	modelCerchioneR1B=glmReadOBJ((char*)"cerchioneR1B.obj");
	glmFacetNormals(modelCerchioneR1B);
	glmVertexNormals(modelCerchioneR1B, smoothing_angle);
	glmComputeBoundingBox(modelCerchioneR1B);

  // inizializzo lo stato della macchina
	px=pz=prz=prx=facing=facingR=facingNewR=capovolgimentoR=capovolgimentoNewR=0; // posizione e orientamento
	px=prx=(traguardo.Center().X()*0.75); py=0; pz=prz=traguardo.bbmin.Z()*0.75; facing=facingOld=facingR=facingNewR=195;
	if (demox || freeRun){
		px=prx=0;
		pz=prz=0;
		facing=facingOld=facingR=facingNewR=0;
	}
    py=0.03;
	mozzoA=mozzoP=mozzoPR=sterzo=sterzoR=giroR=0;   // stato
	ritornoRmax=10;
	riduzioneVSR=1;
	  vx=vy=vz=vrz=vrx=0;      // velocita' attuale
	  capovolto=false;
	  if (demox || freeRun){
		  for (int i=0;i<RITARDOMAX;i++){
			facingRR[i]=0;
			capovolgimentoRR[i]=0;
		  }
	  }
	  else{
		  for (int i=0;i<RITARDOMAX;i++){
			facingRR[i]=195;
			capovolgimentoRR[i]=0;
		  }
	  }
	  for (int i=0;i<ENERGIAMAX;i++){
		  energiaAcc[i]=0;
	  }
	  ritardo=0;
	  energia=0;
  // inizializzo la struttura di controllo
	  demovz=demovx=demoSubi=demogammaNN=demogamma=0;
	  demogamma=0;

	  numsub=30;
  fineGiro=false;
  nuovoGiro=true;
  controller.Init();
  
  //velSterzo=3.4;         // A
  velSterzo=2.4;         // A
  velRitornoSterzo=0.93; // B, sterzo massimo = A*B / (1-B)
  
  velSterzoR=1.4;
  velRitornoSterzoR=0.97;

  velGiroR=1.5;
  velRitornoGiroR=0.9;

  accMax = 0.0011;
  
  // attriti: percentuale di velocita' che viene mantenuta
  // 1 = no attrito
  // <<1 = attrito grande
  attritoZ = 0.991;  // piccolo attrito sulla Z (nel senso di rotolamento delle ruote)
  attritoX = 0.8;  // grande attrito sulla X (per non fare slittare la macchina)
  attritoY = 1.0;  // attrito sulla y nullo
  
  // Nota: vel max = accMax*attritoZ / (1-attritoZ)
  
  raggioRuotaA = 0.25;
  raggioRuotaP = 0.35;
  //demoSubi=0;
  grip = 0.45; // quanto il facing macchina si adegua velocemente allo sterzo
  avanti=giraDx=giraSx=indietro=false;
}



// attiva una luce di openGL per simulare un faro della macchina
void Car::DrawHeadlight(float x, float y, float z, int lightN, bool useHeadlight) const{
  int usedLight=GL_LIGHT1 + lightN;
  
  if(useHeadlight)
  {
  glEnable(usedLight);
  
  float col0[4]= {1,1,0.0,  1};
  glLightfv(usedLight, GL_DIFFUSE, col0);
  
  float col1[4]= {1,1,0.0,  1};
  glLightfv(usedLight, GL_AMBIENT, col1);
   
  float tmpPos[4] = {x,y,z,  1}; // ultima comp=1 => luce posizionale
  glLightfv(usedLight, GL_POSITION, tmpPos );
  
  float tmpDir[4] = {0,0.1,1,  0}; // ultima comp=1 => luce posizionale
  glLightfv(usedLight, GL_SPOT_DIRECTION, tmpDir );
  
  glLightf (usedLight, GL_SPOT_CUTOFF,30);
  glLightf (usedLight, GL_SPOT_EXPONENT,5);
  
  glLightf(usedLight,GL_CONSTANT_ATTENUATION,0);
  glLightf(usedLight,GL_LINEAR_ATTENUATION,1);
  }
  else
   glDisable(usedLight);
}



void Car::RenderAllParts(bool usecolor) const{
	if (useWireframe) {
	            glDisable(GL_TEXTURE_2D);
	            glColor3f(1,0,0);
	            glDisable(GL_LIGHTING);
	            glPolygonMode(GL_FRONT_AND_BACK,GL_LINE);
	  }
 glPushMatrix();


 glPushMatrix();

 glTranslatef(px,py,pz);
 if (!usecolor && useShadow){

	glColor3f(0.3,0.3,0.3); // colore fisso
 	glTranslatef(0,0.04,0); // alzo l'ombra di un epsilon per evitare z-fighting con il pavimento
 	glScalef(1.01,0,1.01);  // appiattisco sulla Y, ingrandisco dell'1% sulla Z e sulla X
 	glDisable(GL_LIGHTING); // niente lighing per l'ombra
  }

 glScalef(-0.05,0.05,-0.05);

 glPushMatrix();


	glRotatef(facing, 0,1,0);


//fari
 DrawHeadlight(-9,14,32, 0, useHeadlight);
 DrawHeadlight(+9,14,32, 1, useHeadlight); // accendi faro destro
 glPushAttrib(GL_ALL_ATTRIB_BITS );

 if (modelListMotrice)
         glDeleteLists(modelListMotrice, 1);
	modelListMotrice = glmList(modelMotrice, GLM_SMOOTH | GLM_MATERIAL);
 	glCallList(modelListMotrice);





    if (useWireframe)
    	glColor3f(0.6,0.6,0.6);
 for (int i=0; i<2; i++) {
   // i==0 -> disegno ruote destre.
   // i==1 -> disegno ruote sinistre.
	int sign;
	if (i==0) sign=1; else sign=-1;


    glPushMatrix();
   	if (i==1){
   		glTranslatef(0,+centerA1[1], 0);
			glRotatef(180, 0,0,1 );
			glTranslatef(0,-centerA1[1], 0);
   	}

   	glTranslatef( centerA1[0],centerA1[1],centerA1[2]);
   	glRotatef( sign*sterzo,0,1,0);
   	glRotatef(-sign*mozzoA,1,0,0);
   	glTranslatef(  -centerA1[0],-centerA1[1],-centerA1[2]  );

   	if (modelListRuoteA1)
			glDeleteLists(modelListRuoteA1, 1);
		modelListRuoteA1 = glmList(modelRuoteA1, GLM_SMOOTH | GLM_MATERIAL);
		glCallList(modelListRuoteA1);

		if (modelListCerchioneA1)
				glDeleteLists(modelListCerchioneA1, 1);
		modelListCerchioneA1 = glmList(modelCerchioneA1, GLM_SMOOTH | GLM_MATERIAL);
		glCallList(modelListCerchioneA1);
		glPopMatrix();

   glPushMatrix();
     if (i==1) {
   	  glTranslatef(0,+centerP1A[1], 0);
		  glRotatef(180, 0,0,1 );
		  glTranslatef(0,-centerP1A[1], 0);
     }

     glTranslatef( centerP1A[0],centerP1A[1],centerP1A[2]);
	  glRotatef(-sign*mozzoP,1,0,0);
     glTranslatef(  -centerP1A[0],-centerP1A[1],-centerP1A[2]  );

     	if (modelListRuoteP1A)
			glDeleteLists(modelListRuoteP1A, 1);
     	modelListRuoteP1A = glmList(modelRuoteP1A, GLM_SMOOTH | GLM_MATERIAL);
		glCallList(modelListRuoteP1A);

		if (modelListCerchioneP1A)
				glDeleteLists(modelListCerchioneP1A, 1);
		modelListCerchioneP1A = glmList(modelCerchioneP1A, GLM_SMOOTH | GLM_MATERIAL);
		glCallList(modelListCerchioneP1A);


   glPopMatrix();

   glPushMatrix();
     if (i==1) {
   	  glTranslatef(0,+centerP1B[1], 0);
		  glRotatef(180, 0,0,1 );
		  glTranslatef(0,-centerP1B[1], 0);
     }

     glTranslatef( centerP1B[0],centerP1B[1],centerP1B[2]);
	  glRotatef(-sign*mozzoP,1,0,0);
     glTranslatef(  -centerP1B[0],-centerP1B[1],-centerP1B[2]  );

		if (modelListRuoteP1B)
			glDeleteLists(modelListRuoteP1B, 1);
		modelListRuoteP1B = glmList(modelRuoteP1B, GLM_SMOOTH | GLM_MATERIAL);
		glCallList(modelListRuoteP1B);

		if (modelListCerchioneP1B)
				glDeleteLists(modelListCerchioneP1B, 1);
		modelListCerchioneP1B = glmList(modelCerchioneP1B, GLM_SMOOTH | GLM_MATERIAL);
		glCallList(modelListCerchioneP1B);
	  glPopMatrix();
 }


 glPopMatrix();
 glPopMatrix();

if (useWireframe)
	glColor3f(0, 0, 0.4);
 glPushMatrix();
 glTranslatef(prx,py,prz);
 if (!usecolor && useShadow){

  	glColor3f(0.3,0.3,0.3); // colore fisso
 	glTranslatef(0,0.04,0); // alzo l'ombra di un epsilon per evitare z-fighting con il pavimento
 	glScalef(1.01,0,1.01);  // appiattisco sulla Y, ingrandisco dell'1% sulla Z e sulla X
 	glDisable(GL_LIGHTING); // niente lighing per l'ombra
  }
 glScalef(-0.05,0.05,-0.05);

 glPushMatrix();

 glRotatef(facingR, 0,1,0);

   if (capovolgimentoR<0){
		glTranslatef(-modelRuoteR1A->bbmin[0],-(modelRuoteR1A->bbmin[1]),0);
		glRotatef(+capovolgimentoR, 0,0,1);
		glTranslatef(+modelRuoteR1A->bbmin[0],+(modelRuoteR1A->bbmin[1]),0);
   }
   else {
		glTranslatef(+modelRuoteR1A->bbmin[0],-(modelRuoteR1A->bbmin[1]),0);
		glRotatef(+capovolgimentoR, 0,0,1);
		glTranslatef(-modelRuoteR1A->bbmin[0],-(modelRuoteR1A->bbmin[1]),0);
   }

   if (modelListRimorchio)
           glDeleteLists(modelListRimorchio, 1);
   modelListRimorchio = glmList(modelRimorchio, GLM_SMOOTH | GLM_MATERIAL);
   glCallList(modelListRimorchio);

   if (useWireframe)
       	glColor3f(0.6,0.6,0.6);
   for (int i=0; i<2; i++) {
   		int sign;
   	    if (i==0) sign=1; else sign=-1;

   	    glPushMatrix();
			  if (i==1) {
				  glTranslatef(0,+centerR1A[1], 0);
				  glRotatef(180, 0,0,1 );
				  glTranslatef(0,-centerR1A[1], 0);
			  }

			  glTranslatef( centerR1A[0],centerR1A[1],centerR1A[2]);
			  glRotatef(-sign*mozzoPR,1,0,0);
			  glTranslatef(  -centerR1A[0],-centerR1A[1],-centerR1A[2]  );

				if (modelListRuoteR1A)
					glDeleteLists(modelListRuoteR1A, 1);
				modelListRuoteR1A = glmList(modelRuoteR1A, GLM_SMOOTH | GLM_MATERIAL);
				glCallList(modelListRuoteR1A);

				if (modelListCerchioneR1A)
						glDeleteLists(modelListCerchioneR1A, 1);
				modelListCerchioneR1A = glmList(modelCerchioneR1A, GLM_SMOOTH | GLM_MATERIAL);
				glCallList(modelListCerchioneR1A);
			  glPopMatrix();


			glPushMatrix();
			  if (i==1) {
				  glTranslatef(0,+centerR1B[1], 0);
				  glRotatef(180, 0,0,1 );
				  glTranslatef(0,-centerR1B[1], 0);
			  }

			  glTranslatef( centerR1B[0],centerR1B[1],centerR1B[2]);
			  glRotatef(-sign*mozzoPR,1,0,0);
			  glTranslatef(  -centerR1B[0],-centerR1B[1],-centerR1B[2]  );

				if (modelListRuoteR1B)
					glDeleteLists(modelListRuoteR1B, 1);
				modelListRuoteR1B = glmList(modelRuoteR1B, GLM_SMOOTH | GLM_MATERIAL);
				glCallList(modelListRuoteR1B);

				if (modelListCerchioneR1B)
						glDeleteLists(modelListCerchioneR1B, 1);
				modelListCerchioneR1B = glmList(modelCerchioneR1B, GLM_SMOOTH | GLM_MATERIAL);
				glCallList(modelListCerchioneR1B);
			  glPopMatrix();
   	}
	glPopMatrix();
 glPopMatrix();
 glPopMatrix();
glPopAttrib();
if (useWireframe){
    	   glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);
    	       glColor3f(1,1,1);
    	       glEnable(GL_LIGHTING);
    }

 
}
 
// disegna a schermo
void Car::Render() const{
  

  RenderAllParts(true);
  
  // ombra!
  if(morning)
  {

    RenderAllParts(false);  // disegno la macchina appiattita


  }

  glEnable(GL_LIGHTING);

  
}
