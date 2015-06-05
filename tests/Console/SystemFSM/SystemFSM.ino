#include <Metro.h>
#include <Streaming.h>
#include <FiniteStateMachine.h>

void setup();
void loop();

class System {
  public:
    void begin();
    void loop();
    
    // probes for current state
    boolean inGame();
    boolean inIdle();
    boolean inSetup();
  private:
  
    static void idleEnter(), idleUpdate(), idleExit();
    static State *idle;
    
    FSM *s;
};

FSM *sp;
System system;

void System::begin() {
  Serial << "begin" << endl;
  
  idle = new State(idleEnter, idleUpdate, idleExit);
  
  this->s = new FSM(idle);
  sp = s;
}

void System::loop() {
  Serial << "loop" << endl;
  s->update();
}

void System::idleEnter() {
  Serial << "idleEnter" << endl;
}
void System::idleUpdate() {
  Serial << "idleUpdate" << endl;
  static Metro timeHere(1000UL);
  if( timeHere.check() ) sp->transitionTo(idle);
}
void System::idleExit() {
  Serial << "idleExit" << endl;
}


void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  
  system.begin();
}

void loop() {
  // put your main code here, to run repeatedly:
  system.loop();
  
  delay(100);
}
