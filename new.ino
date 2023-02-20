#include <Servo.h>
#include <NewPing.h>
#define TRIGGER_PIN 8
#define ECHO_PIN 9
#define MAX_DISTANCE 400

Servo push;                                             //сервопривод, толкающий кубики
Servo roll;                                             //сервопривод, вращающий ленту
Servo move;
NewPing distance(TRIGGER_PIN, ECHO_PIN, MAX_DISTANCE);  //дальнометр

short _counter;
class ColorSensor {
  int _s0, _s1, _s2, _s3;
  int _flag;
  char _color_flag = 'r';
  public:
  short r, g, b;
  ColorSensor(int s0, int s1, int s2, int s3) {
    _s0 = s0;
    pinMode(s0, OUTPUT);
    digitalWrite(s0, HIGH);
    _s1 = s1;
    pinMode(s1, OUTPUT);
    digitalWrite(s1, HIGH);
    _s2 = s2;
    pinMode(s2, OUTPUT);
    digitalWrite(s2, LOW);
    _s3 = s3;
    pinMode(s3, OUTPUT);
    digitalWrite(s3, LOW);
    _counter = 0;
    r = 0;
    g = 0;
    b = 0;
    _flag = 0;
    attachInterrupt(0, _CounterAdd, CHANGE);
  }
  static void _CounterAdd() {
    _counter++;
  }
  public:
  void NextColor() {
    switch (_color_flag) {
      case 'r':
        r = _counter;
        _color_flag = 'g';
        digitalWrite(_s2, HIGH);
        digitalWrite(_s3, HIGH);
        break;
      case 'g':
        g = _counter;
        _color_flag = 'b';
        digitalWrite(_s2, LOW);
        digitalWrite(_s3, HIGH);
        break;
      case 'b':
        b = _counter;
        _color_flag = 'r';
        digitalWrite(_s2, LOW);
        digitalWrite(_s3, LOW);
        break;
    }
    _counter = 0;
  }
  public:
  short R() {
    return r;
  }
  public:
  short G() {
    return g;
  }
  public:
  short B() {
    return b;
  }
};
void timer_init() {
  TCCR2A = 0x00;
  TCCR2B = 0x07;  //the clock frequency source 1024 points
  TCNT2 = 100;    //10 ms overflow again
  TIMSK2 = 0x01;  //allow interrupt
}


ColorSensor sensor(3, 4, 5, 6);

ISR(TIMER2_OVF_vect)  //the timer 2, 10ms interrupt overflow again. Internal overflow interrupt executive function
{
  sensor.NextColor();
  TCNT2 = 100;
}

void setup() {
  Serial.begin(115200);
  roll.attach(10);
  push.attach(11);
  move.attach(12);
  Serial.begin(115200);
  timer_init();
}

int DeterColor(int r, int g, int b){
  int h;
  int max = max(r,max(g,b));
  int min = min(r,min(g,b));
  if (max==r)
    if (g>=b) h=60*(g-b)/(max-min);
    else h=60*(g-b)/(max-min)+360;
  else 
    if (max==g) h=60*(b-r)/(max-min)+120;
    else h=60*(r-g)/(max-min)+240;
  float s=min;
  s=1-s/max;
  Serial.println(s);
  if (h<30) return 3;
  if (h>=300) return 1;
  if (s>=20) return 6;
  return 2;
  // 0-чёрный, 1-красный, 2-зелёный, 3-жёлтый, 4-синий, 5-сиреневый, 6-голубой, 7-белый
}

const int disMax=6; //ширина дорожки
const int color[4] = {1,2,3,6}; //цвета: красный, зелёный, жёлтый, синий
char port[10] = {0};

void loop() {
  int val, c[4]={0}, cnew, dis, sr, sg, sb;
  if (Serial.available()){
    val = Serial.read();
    //Приём сообщениий из приложения
    switch (val){
      case '1': port[1] = 1; break;
      case '2': port[2] = 1; break;
      case '3': port[3] = 1; break;
      case '4': port[4] = 1; break;
      case '6': port[6] = 1; break;
      case '7': port[7] = 1; break;
      case '8': port[8] = 1; break;
      case '9': port[9] = 1; break;
      case '5': //запуск
         int num=0; 
         for (int i=1; i<10; i++)
          if (port[i] == 1){
            c[num]=color[i%5];
            num++;
          }
        break;
      case '0': 
         //Отключение абсолютно всего 
         for (int i=0; i<10; i++) port[i]=0;
      }
    }
  if (val=='5'){
    roll.write(180);
    dis=distance.ping_cm();
    //Serial.println(dis);
    if ((dis<disMax)and(dis!=0)){
      roll.write(180);
      delay(200);
      roll.write(0);
      sr=0; sg=0; sb=0;
      for (int i=0; i<25; i++) {
        sr+=sensor.r; 
        sg+=sensor.g;
        sb+=sensor.b;
        delay(100);
      }
      cnew=DeterColor(sr/25, sg/25, sb/25);
      if ((cnew==c[0]) or (cnew==c[1])) {
        push.write(180);
        delay(200);
        push.write(0);
        delay(200);
        push.write(90);
      }
      else {
        move.write(180);
        delay(100);
        move.write(90);
        push.write(180);
        delay(200);
        push.write(0);
        delay(200);
        push.write(90);
        move.write(0);
        delay(100);
        move.write(90);
      }
    }
  }
  delay(200);
}