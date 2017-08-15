// Parameter 1 = number of pixels in strip
// Parameter 2 = Arduino pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
Adafruit_NeoPixel strip1 = Adafruit_NeoPixel(NUMPIXELS1, LEDPIN1, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel strip2 = Adafruit_NeoPixel(NUMPIXELS2, LEDPIN2, NEO_GRB + NEO_KHZ800);
uint32_t startpixels[NUMPIXELS1+NUMPIXELS2];
uint32_t finalpixels[NUMPIXELS1+NUMPIXELS2];
int currentstep;
int numsteps;

// Initialize all pixels to 'off'
void stripe_setup() {
  strip1.begin();
  strip2.begin();
  for(int i=0; i<NUMPIXELS1+NUMPIXELS2; i++) {
    startpixels[i] = 0;
    finalpixels[i] = 0;
  }
  stripe_show(1);
}

void stripe_show(int steps) {
  numsteps = steps;
  if(numsteps<1) numsteps = 1; // there can be less than one step to new color
  currentstep = 0;
  stripe_nextstep(); // call first step immediately (1 step = show immediately)
}

void stripe_nextstep() {
  if(currentstep>=numsteps) return; // already finished
  // calculate next step
  currentstep++;
  for(int i=0; i<NUMPIXELS1+NUMPIXELS2; i++) {
    uint32_t newcolor = stripe_interpolateColor(startpixels[i],finalpixels[i],currentstep,numsteps);
    stripe_setPixelColorDirect(i, newcolor);
  }
  strip1.show();
  strip2.show();
}

uint32_t stripe_interpolateColor(uint32_t startcolor, uint32_t finalcolor, int step, int num)
{
  int32_t startred = Red(startcolor);
  int32_t finalred = Red(finalcolor);
  int32_t newred = (finalred-startred)*step/num + startred;
  int32_t startgreen = Green(startcolor);
  int32_t finalgreen = Green(finalcolor);
  int32_t newgreen = (finalgreen-startgreen)*step/num + startgreen;
  int32_t startblue = Blue(startcolor);
  int32_t finalblue = Blue(finalcolor);
  int32_t newblue = (finalblue-startblue)*step/num + startblue;
  return stripe_color((uint8_t)newred,(uint8_t)newgreen,(uint8_t)newblue);
}

// stripe_setPixelColor writes new color as final color after transition
void stripe_setPixelColor(uint16_t pixel, uint32_t color) {
  // save current state as starting point for transition
  if(currentstep>0) {
    for(int i=0; i<NUMPIXELS1+NUMPIXELS2; i++) {
      startpixels[i] = stripe_getPixelColorDirect(i);
    }
    currentstep = 0;
  }
  // set pixel color as final color in transition
  finalpixels[pixel] = color;
}

// stripe_getPixelColor returns the final color of the transition - the calling function does have to deal with halfway colors
uint32_t stripe_getPixelColor(uint16_t pixel) {
  return finalpixels[pixel];
}

void stripe_setPixelColorDirect(uint16_t pixel, uint32_t color) {
  if(pixel < NUMPIXELS1) {
    strip1.setPixelColor(NUMPIXELS1-1-pixel, color);
  } else {
    strip2.setPixelColor(pixel-NUMPIXELS1, color);
  }
}

void stripe_dimPixel(uint16_t pixel) {
  stripe_setPixelColor(pixel, DimColor(stripe_getPixelColor(pixel)));
}

uint32_t stripe_getPixelColorDirect(uint16_t pixel) {
  if(pixel < NUMPIXELS1) {
    return strip1.getPixelColor(NUMPIXELS1-1-pixel);
  } else {
    return strip2.getPixelColor(pixel-NUMPIXELS1);
  }
}

void stripe_setBrightness(uint8_t b) {
  strip1.setBrightness(b);
  strip2.setBrightness(b);
}

uint16_t stripe_numPixels() {
  return NUMPIXELS1+NUMPIXELS2;
}

uint32_t stripe_color(uint8_t r, uint8_t g, uint8_t b) {
  return ((uint32_t)r << 16) | ((uint32_t)g <<  8) | b;
}

uint8_t Red(uint32_t color)
{
  return (color >> 16) & 0xFF;
}

// Returns the Green component of a 32-bit color
uint8_t Green(uint32_t color)
{
  return (color >> 8) & 0xFF;
}

// Returns the Blue component of a 32-bit color
uint8_t Blue(uint32_t color)
{
  return color & 0xFF;
}

uint32_t DimColor(uint32_t color)
{
  uint32_t dimColor = strip1.Color(Red(color) >> 1, Green(color) >> 1, Blue(color) >> 1);
  return dimColor;
}
