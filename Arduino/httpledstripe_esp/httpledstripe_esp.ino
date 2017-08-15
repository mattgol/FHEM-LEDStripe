#include <Adafruit_NeoPixel.h>
#include <ESP8266WiFi.h>

const char* ssid     = "XXX";
const char* password = "XXX";
WiFiServer server(80);
// Which pin on the Arduino is connected to the NeoPixels?
#define LEDPIN1           14
#define LEDPIN2           12

// How many NeoPixels are attached to the Arduino?
#define NUMPIXELS1     30
#define NUMPIXELS2     30
int xfrom;
int yto;
int myredLevel;
int mygreenLevel;
int myblueLevel;
int myOn;
int myOff;


void reset();
// control special effects
boolean fire=false;
boolean rainbow=false;
boolean blinker=false;
boolean sparks=false;
boolean white_sparks=false;
boolean knightrider = false;
uint16_t rainbowColor=0;


uint16_t delay_interval=50;

int cur_step=0;
uint32_t lasteffect;
uint32_t delaytonext;
int transitionsteps = 20;

// setup network and output pins
void setup() {
// Open serial communications and wait for port to open:
  Serial.begin(9600);
   while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo only
  }
  Serial.println(F("Booting"));

 // Initialize all pixels to 'off'
 stripe_setup();
  WiFi.mode(WIFI_STA);
Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
 
  WiFi.begin(ssid, password);
 
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected"); 
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
    
  lasteffect = millis();
  delaytonext = 0;
  server.begin();
 
}

// request receive loop
void loop() {
// listen for incoming clients
  WiFiClient client = server.available();  // Check if a client has connected
  if (client) {
    Serial.println(F("new client"));
   
    String inputLine = "";
   // an http request ends with a blank line
    boolean currentLineIsBlank = true;
    boolean isGet = false;
    boolean isPost = false;
    boolean isPostData = false;
    int postDataLength;
    int ledix = 0;
    int tupel = 0;
    int redLevel = 0;
    int greenLevel = 0;
    int blueLevel = 0;
   
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        if (isPostData && postDataLength > 0) {
          switch (tupel++) {
            case 0:
              redLevel = colorVal(c);
              break;
            case 1:
              greenLevel = colorVal(c);
              break;
            case 2:
              blueLevel = colorVal(c);
              tupel = 0;
              stripe_setPixelColor(ledix++, stripe_color(redLevel,greenLevel,blueLevel));
              break;
          }
          if (--postDataLength == 0) {
            delaytonext=delay_interval;
            stripe_show(transitionsteps);
            sendOkResponse(client);
            break;
          }
        }
        // if you've gotten to the end of the line (received a newline
        // character) and the line is blank, the http request has ended,
        // so you can send a reply
        if (c == '\n' && currentLineIsBlank) {
          if (isPost) {
            isPostData = true;
            continue;
          } else {
            // send http response
            if (isGet) {
              sendOkResponse(client);
            } else {
              client.println(F("HTTP/1.1 500 Invalid request"));
              client.println(F("Connection: close"));  // the connection will be closed after completion of the response
              client.println();
            }
            break;
          }
        }
        if (c == '\n') {
          // http starting a new line, evaluate current line
          currentLineIsBlank = true;
          Serial.println(inputLine);
         
          // SET SINGLE PIXEL url should be GET /rgb/n/rrr,ggg,bbb
          if (inputLine.length() > 3 && inputLine.substring(0,9) == F("GET /rgb/")) {
            int slash = inputLine.indexOf('/', 9 );
            ledix = inputLine.substring(9,slash).toInt();
            int urlend = inputLine.indexOf(' ', 9 );
            String getParam = inputLine.substring(slash+1,urlend+1);
            int komma1 = getParam.indexOf(',');
            int komma2 = getParam.indexOf(',',komma1+1);
            redLevel = getParam.substring(0,komma1).toInt();
            greenLevel = getParam.substring(komma1+1,komma2).toInt();
            blueLevel = getParam.substring(komma2+1).toInt();
            stripe_setPixelColor(ledix, stripe_color(redLevel,greenLevel,blueLevel));
            delaytonext = delay_interval;
            stripe_show(transitionsteps);
            isGet = true;
          }
          // SET DELAY url should be GET /delay/n
          if (inputLine.length() > 3 && inputLine.substring(0,11) == F("GET /delay/")) {
            delay_interval = inputLine.substring(11).toInt();
            if(delay_interval<20) delay_interval = 20;
            isGet = true;
          }
          // SET DELAY url should be GET /transitionsteps/n
          if (inputLine.length() > 3 && inputLine.substring(0,21) == F("GET /transitionsteps/")) {
            transitionsteps = inputLine.substring(21).toInt();
            isGet = true;
          }
          // SET BRIGHTNESS url should be GET /brightness/n
          if (inputLine.length() > 3 && inputLine.substring(0,16) == F("GET /brightness/")) {
            stripe_setBrightness(inputLine.substring(16).toInt());
            stripe_show(1);
            isGet = true;
          }
          // SET PIXEL RANGE url should be GET /range/x,y/rrr,ggg,bbb
          if (inputLine.length() > 3 && inputLine.substring(0,11) == F("GET /range/")) {
            int slash = inputLine.indexOf('/', 11 );
            int komma1 = inputLine.indexOf(',');
            int x = inputLine.substring(11, komma1).toInt();
            int y = inputLine.substring(komma1+1, slash).toInt();
            int urlend = inputLine.indexOf(' ', 11 );
            String getParam = inputLine.substring(slash+1,urlend+1);
            komma1 = getParam.indexOf(',');
            int komma2 = getParam.indexOf(',',komma1+1);
            redLevel = getParam.substring(0,komma1).toInt();
            greenLevel = getParam.substring(komma1+1,komma2).toInt();
            blueLevel = getParam.substring(komma2+1).toInt();
            for(int i=x; i<=y; i++) {
              stripe_setPixelColor(i, stripe_color(redLevel,greenLevel,blueLevel));
            }
            delaytonext = delay_interval;
            stripe_show(transitionsteps);
            isGet = true;
          }
          // TOGGLE PIXEL RANGE url should be GET /togglerange/x,y/rrr,ggg,bbb
          if (inputLine.length() > 3 && inputLine.substring(0,17) == F("GET /togglerange/")) {
            int slash = inputLine.indexOf('/', 17 );
            int komma1 = inputLine.indexOf(',');
            int x = inputLine.substring(17, komma1).toInt();
            int y = inputLine.substring(komma1+1, slash).toInt();
            int urlend = inputLine.indexOf(' ', 17 );
            String getParam = inputLine.substring(slash+1,urlend+1);
            komma1 = getParam.indexOf(',');
            int komma2 = getParam.indexOf(',',komma1+1);
            redLevel = getParam.substring(0,komma1).toInt();
            greenLevel = getParam.substring(komma1+1,komma2).toInt();
            blueLevel = getParam.substring(komma2+1).toInt();
            uint32_t newcolor = stripe_color(redLevel,greenLevel,blueLevel);
            // get old status
            int ledon = 0;
            int ledoff = 0;
            for(int i=x; i<=y; i++) {
              if (stripe_getPixelColor(i) != 0) ledon++;
              else ledoff++;
              if(ledon>=ledoff) newcolor = 0; // switch off
            }
            for(int i=x; i<=y; i++) {
              stripe_setPixelColor(i, newcolor);
            }
            delaytonext = delay_interval;
            stripe_show(transitionsteps);
            isGet = true;
          }
          // POST PIXEL DATA
          if (inputLine.length() > 3 && inputLine.substring(0,10) == F("POST /leds")) {
            isPost = true;
          }
          if (inputLine.length() > 3 && inputLine.substring(0,16) == F("Content-Length: ")) {
            postDataLength = inputLine.substring(16).toInt();
          }
          // SET ALL PIXELS OFF url should be GET /off
          if (inputLine.length() > 3 && inputLine.substring(0,8) == F("GET /off")) {
            for(int i=0; i<stripe_numPixels(); i++) {
              stripe_setPixelColor(i, 0);
            }
            fire = false;
            rainbow = false;
            blinker = false;
            sparks = false;
            white_sparks = false;
            knightrider = false;
            delaytonext = delay_interval;
            stripe_show(transitionsteps);
            isGet = true;
          }
          // SET ALL PIXELS OFF immediately, url should be GET /offnew
          if (inputLine.length() > 3 && inputLine.substring(0,11) == F("GET /offnow")) {
            reset();
            isGet = true;
          }
         
          // GET STATUS url should be GET /status
          if (inputLine.length() > 3 && inputLine.substring(0,11) == F("GET /status")) {
            isGet = true;
          }
          // SET FIRE EFFECT
          if (inputLine.length() > 3 && inputLine.substring(0,9) == F("GET /fire")) {
            fire = true;
            rainbow = false;
            sparks = false;
            white_sparks = false;
            knightrider = false;
            stripe_setBrightness(128);
            isGet = true;
            delaytonext = 0;
          }
          // SET RAINBOW EFFECT
          if (inputLine.length() > 3 && inputLine.substring(0,12) == F("GET /rainbow")) {
            rainbow = true;
            fire = false;
            sparks = false;
            white_sparks = false;
            knightrider = false;
            stripe_setBrightness(128);
            isGet = true;
            delaytonext = 0;
          }
          // SET WHITE_SPARKS EFFECT
          if (inputLine.length() > 3 && inputLine.substring(0,17) == F("GET /white_sparks")) {
            rainbow = false;
            fire = false;
            sparks = false;
            white_sparks = true;
            knightrider = false;
            stripe_setBrightness(128);
            isGet = true;
            delaytonext = 0;
          }
          // SET SPARKS EFFECT
          if (inputLine.length() > 3 && inputLine.substring(0,11) == F("GET /sparks")) {
            rainbow = false;
            fire = false;
            sparks = true;
            white_sparks = false;
            knightrider = false;
            stripe_setBrightness(128);
            isGet = true;
            delaytonext = 0;
          }
          // SET KNIGHTRIDER EFFECT
          if (inputLine.length() > 3 && inputLine.substring(0,16) == F("GET /knightrider")) {
            rainbow = false;
            fire = false;
            sparks = false;
            white_sparks = false;
            knightrider = true;
            stripe_setBrightness(128);
            isGet = true;
            delaytonext = 0;
          }
          // SET no_effects
          if (inputLine.length() > 3 && inputLine.substring(0,9) == F("GET /nofx")) {
            rainbow = false;
            fire = false;
            sparks = false;
            white_sparks = false;
            blinker = false;
            isGet = true;
            delaytonext = 0;
          }
          if (inputLine.length() > 3 && inputLine.substring(0,11) == F("GET /blink/")) {
            int slash = inputLine.indexOf('/', 11 );
            int komma1 = inputLine.indexOf(',');
            xfrom = inputLine.substring(11, komma1).toInt();
            yto = inputLine.substring(komma1+1, slash).toInt();
            int urlend = inputLine.indexOf(' ', 11 );
            String getParam = inputLine.substring(slash+1,urlend+1);
            komma1 = getParam.indexOf(',');
            int komma2 = getParam.indexOf(',',komma1+1);
            int komma3 = getParam.indexOf(',',komma2+1);
            int komma4 = getParam.indexOf(',',komma3+1);
            myredLevel = getParam.substring(0,komma1).toInt();
            mygreenLevel = getParam.substring(komma1+1, komma2).toInt();
            myblueLevel = getParam.substring(komma2+1, komma3).toInt();
           
            myOn = getParam.substring(komma3+1, komma4).toInt();
            myOff = getParam.substring(komma4+1).toInt();

            blinker = true;
            rainbow = false;
            fire = false;
            sparks = false;
           
            isGet = true;
            delaytonext = 0;
          }
          inputLine = "";
        }
        else if (c != '\r') {
          // add character to the current line
          currentLineIsBlank = false;
          inputLine += c;
        }
      }
    }
    // give the web browser time to receive the data
    delay(1);
    // close the connection:
    client.stop();
    Serial.println(F("client disconnected"));
  }

  // check time if next effect is due and start it
  if(millis()-lasteffect > delaytonext) {
    lasteffect = millis();
    if (fire) fireEffect();
    else if (rainbow) rainbowCycle();
    else if (blinker) blinkerEffect();
    else if (sparks) sparksEffect();
    else if (white_sparks) white_sparksEffect();
    else if (knightrider) knightriderEffect();
    else {
      stripe_nextstep();
      delaytonext = delay_interval;
    }      
  }
}

// Reset stripe, all LED off and no effects
void reset() {
  for(int i=0; i<stripe_numPixels(); i++) {
    stripe_setPixelColor(i, 0);
  }
  stripe_setBrightness(255);
  stripe_show(1);
  fire = false;
  rainbow = false;
  blinker = false;
  sparks = false;
  white_sparks = false;
  knightrider = false;
}

// LED flicker fire effect
void fireEffect() {
  for(int x = 0; x <stripe_numPixels(); x++) {
    int flicker = random(0,55);
    int r1 = 226-flicker;
    int g1 = 121-flicker;
    int b1 = 35-flicker;
    if(g1<0) g1=0;
    if(r1<0) r1=0;
    if(b1<0) b1=0;
    stripe_setPixelColor(x,stripe_color(r1,g1, b1));
  }
  stripe_show(1);
  delaytonext = random(10,113);
}

// Slightly different, this makes the rainbow equally distributed throughout
void rainbowCycle() {
  uint16_t i;

  if (rainbowColor++>255) rainbowColor=0;
  for(i=0; i< stripe_numPixels(); i++) {
    stripe_setPixelColor(i, Wheel(((i * 256 / stripe_numPixels()) + rainbowColor) & 255));
  }
  stripe_show(1);
  delaytonext = delay_interval;  
}

void blinkerEffect() {
 for(int i=xfrom; i<=yto; i++) {
    stripe_setPixelColor(i, stripe_color(myredLevel,mygreenLevel,myblueLevel));
  }
  stripe_show(1);
 delay(myOn);
 for(int i=xfrom; i<= yto; i++) {
    stripe_setPixelColor(i, stripe_color(0,0,0));
  }
  stripe_show(1);
  delaytonext = myOff;
}

void sparksEffect() {
  uint16_t i = random(NUMPIXELS1+NUMPIXELS2);

  if (stripe_getPixelColor(i)==0) {
    stripe_setPixelColor(i,random(256*256*256));
  }

  for(i = 0; i < NUMPIXELS1+NUMPIXELS2; i++) {
    stripe_dimPixel(i);
  }

  stripe_show(1);
  delaytonext = delay_interval;
}

void white_sparksEffect() {
  uint16_t i = random(NUMPIXELS1+NUMPIXELS2);
  uint16_t rand = random(256);

  if (stripe_getPixelColor(i)==0) {
    stripe_setPixelColor(i,rand*256*256+rand*256+rand);
  }

  for(i = 0; i < NUMPIXELS1+NUMPIXELS2; i++) {
    stripe_dimPixel(i);
  }

  stripe_show(1);
  delaytonext = delay_interval;
}

void knightriderEffect() {
  uint16_t i;
  
  cur_step+=1;
  
  if(cur_step>=((NUMPIXELS1+NUMPIXELS2)*2)){
    cur_step=0;
  }
  

  if(cur_step<(NUMPIXELS1+NUMPIXELS2)){
    stripe_setPixelColor(cur_step, (256*256*256)-1);
    for(i=1;i<=32;i++){
      if((cur_step-i>-1)) {
        stripe_dimPixel(cur_step-i);
      }
      if((cur_step+i-1)<NUMPIXELS1+NUMPIXELS2) {
        stripe_dimPixel(cur_step+i-1);
      }
          
    }
  } else {
    stripe_setPixelColor((NUMPIXELS1+NUMPIXELS2)*2-cur_step-1, (256*256*256)-1);
    for(i=1;i<=32;i++){
      if(((NUMPIXELS1+NUMPIXELS2)*2-cur_step-1+i<NUMPIXELS1+NUMPIXELS2)) {
        stripe_dimPixel((NUMPIXELS1+NUMPIXELS2)*2-cur_step-1+i);
      }
      if(((NUMPIXELS1+NUMPIXELS2)*2-cur_step-1-i)>-1) {
        stripe_dimPixel((NUMPIXELS1+NUMPIXELS2)*2-cur_step-1-i);
      }
    }
  } 
  
  stripe_show(1);
  delaytonext = delay_interval;
}

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if(WheelPos < 85) {
    return stripe_color(255 - WheelPos * 3, 0, WheelPos * 3);
  }
  if(WheelPos < 170) {
    WheelPos -= 85;
    return stripe_color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
  WheelPos -= 170;
  return stripe_color(WheelPos * 3, 255 - WheelPos * 3, 0);
}

int colorVal(char c) {
  int i = (c>='0' && c<='9') ? (c-'0') : (c - 'A' + 10);
  return i*i + i*2;
}

void sendOkResponse(WiFiClient client) {
  client.println(F("HTTP/1.1 200 OK"));
  client.println(F("Content-Type: text/html"));
  client.println(F("Connection: close"));  // the connection will be closed after completion of the response
  client.println();
  // standard response
  client.print(F("OK,"));
  client.print(stripe_numPixels());
  client.print(F(","));
  int oncount=0;
  for(int i=0; i<stripe_numPixels(); i++) {
    if (stripe_getPixelColor(i) != 0) oncount++;
  }
  client.println(oncount);
}
