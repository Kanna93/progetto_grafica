class Controller{
public:
  enum { LEFT=0, RIGHT=1, ACC=2, DEC=3, NKEYS=4};
  bool key[NKEYS];
  void Init();
  void EatKey(int keycode, int* keymap, bool pressed_or_released);
  void Joy(int keymap, bool pressed_or_released);
  Controller(){Init();} // costruttore
};


class Car{

  void RenderAllParts(bool usecolor) const; 
                         // disegna tutte le parti della macchina
                         // invocato due volte: per la car e la sua ombra

public:
  // Metodi
  void Init(); // inizializza variabili
  void Render() const; // disegna a schermo
  void DoStep(); // computa un passo del motore fisico
  void DoDemo();
  void DoDemox(float,float,float,float);
  Car(){Init();} // costruttore
 
  Controller controller;  
  
  // STATO DELLA MACCHINA
  // (DoStep fa evolvere queste variabili nel tempo)
  float px,py,pz,prz,prx,facing,facingOld,facingR,facingNewR,capovolgimentoR,capovolgimentoNewR,ritornoRmax,riduzioneVSR; // posizione e orientamento
  float mozzoA, mozzoP, mozzoPR, sterzo, sterzoR, giroR; // stato interno
  float vx,vy,vz,vrz,vrx; // velocita' attuale
  float vzm;
  float pxNext,pzNext,pxNN,pzNN,angolo;
  bool nuovoGiro;
  bool fineGiro;
  bool capovolto;
  bool giraDx,giraSx,avanti,dritto,indietro;
  const static int RITARDOMAX=30;
  const static int INCLINAZIONEMAX=45;
  const static int ENERGIAMAX=15;
  float facingRR[RITARDOMAX],capovolgimentoRR[RITARDOMAX],energiaAcc[ENERGIAMAX];
  int ritardo,energia;
  float demovz, demovx, demogamma, demogammaNext, demogammaNN;
  int demoVertexSize,demoSubi, demoVertexi, numsub;

  // STATS DELLA MACCHINA
  // (di solito rimangono costanti)
  float velSterzo, velRitornoSterzo,velSterzoR,velRitornoSterzoR,velGiroR,velRitornoGiroR, accMax, attrito,
  raggioRuotaA, raggioRuotaP, grip,
  attritoX, attritoY, attritoZ; // attriti
  
private:
  void DrawHeadlight(float x, float y, float z, int lightN, bool useHeadlight) const;
};
