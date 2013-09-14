/********* Ed Dawson's OLED-Accelerometer-Compass Project: 
Based on the Adafruit FLORA Arduino compatible:
(see http://www.adafruit.com/products/659 )
Wiring: Uses Pin 10 for OLED_RESET ("Rst" pinout on FLORA board) 
instead of Pin 4 as per the Adafruit default code (for Arduino Uno). 
Takes readings from the Adafruit FLORA accelerometer LSM303 
(see http://www.adafruit.com/products/1247 ) 
and displays them on the Adafruit 128-64 pixel OLED display 
(see http://www.adafruit.com/products/326 ). 
(this code is based on from the Adafruit example code for these components.)
*********/

/*********************************************************************
This is an example for our Monochrome OLEDs based on SSD1306 drivers

  Pick one up today in the adafruit shop!
  ------> http://www.adafruit.com/category/63_98

This example is for a 128x64 size display using I2C to communicate
3 pins are required to interface (2 I2C and one reset)

Adafruit invests time and resources providing this open source code, 
please support Adafruit and open-source hardware by purchasing 
products from Adafruit!

Written by Limor Fried/Ladyada  for Adafruit Industries.  
BSD license, check license.txt for more information
All text above, and the splash screen must be included in any redistribution
*********************************************************************/

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_LSM303.h>

/* Setting up the OLED display and defining constants */
#define OLED_RESET 10
Adafruit_SSD1306 display(OLED_RESET);

#define NUMFLAKES 10
#define XPOS 0
#define YPOS 1
#define DELTAY 2

#define LOGO16_GLCD_HEIGHT 16 
#define LOGO16_GLCD_WIDTH  16 
static unsigned char PROGMEM logo16_glcd_bmp[] =
{ B00000000, B11000000,
  B00000001, B11000000,
  B00000001, B11000000,
  B00000011, B11100000,
  B11110011, B11100000,
  B11111110, B11111000,
  B01111110, B11111111,
  B00110011, B10011111,
  B00011111, B11111100,
  B00001101, B01110000,
  B00011011, B10100000,
  B00111111, B11100000,
  B00111111, B11110000,
  B01111100, B11110000,
  B01110000, B01110000,
  B00000000, B00110000 };

#if (SSD1306_LCDHEIGHT != 64)
#error("Height incorrect, please fix Adafruit_SSD1306.h!");
#endif

/* Ed initialises variables */

int motionCurrent = 0;
int motionPrevious = 0;
int motionSize = 0;
int motionScore = 0;
int displayMode = 1; /* Can be 0(textmode) or 1(animated line graph)*/
int graphSpaceY = (display.height()-display.height()/4); /* The uppermost border of the graph space */
int displayQuarter = (display.height()/4); /* One quarter of the screen height (the graph area)*/
int graphOnePercent = (displayQuarter/100);/* One percent of the graph area height */
int accelScale = 0; /* Holds the scale of the most recent vertical G-Force reading */
int plotScale = 0; /* Holds value of the scale of the current plot as percentage */
int graphPoint1X = 0; int graphPoint1Y = 0; /* Plot positions for points on the animated graph */
int graphPoint2X = 0; int graphPoint2Y = 0;
int graphPoint3X = 0; int graphPoint3Y = 0;
int graphPoint4X = 0; int graphPoint4Y = 0;
int graphPoint5X = 0; int graphPoint5Y = 0;

/* Main Program begins */

void setup()   {                
  Serial.begin(9600);

  // by default, we'll generate the high voltage from the 3.3v line internally! (neat!)
  display.begin(SSD1306_SWITCHCAPVCC, 0x3D);  // initialize with the I2C addr 0x3D (for the 128x64)
  // init done

  /* Setting up the Accelerometer and Magnetometer */
  // Try to initialise accelerometer and warn if we couldn't detect the chip
  Adafruit_LSM303 lsm;
  if (!lsm.begin())
  {
    Serial.println("Oops ... unable to initialize the LSM303. Check wiring!");
    while (1);
  }
  
  // Bootup Screen (smiley face)
  display.clearDisplay();
  display.drawPixel(64, 32, WHITE);
  display.drawPixel(68, 32, WHITE);
  display.drawPixel(66, 36, WHITE);
  display.drawPixel(64, 39, WHITE);  
  display.drawPixel(64, 40, WHITE);  
  display.drawPixel(65, 40, WHITE);  
  display.drawPixel(66, 40, WHITE);  
  display.drawPixel(67, 40, WHITE);  
  display.drawPixel(68, 40, WHITE);
  display.drawPixel(68, 39, WHITE);  
  display.display();
  delay(2000);
 
  // clean up test graphics
  delay(1000); 
  display.clearDisplay();
  
while (1) { //loop indefinitely if no error
// Read from the accelerometer 
  lsm.read();

motionCurrent = ((int)lsm.accelData.z); /* Capture current accelerometer Z-plane (vertical) reading */

// Reset text cursor  
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0,0);

//Begin displaying sensor readings on screen
  /* display.println("   G-Force Monitor");
  display.println(" "); */
    
    //Determining scale of the difference between previous and current Z-plane (vertical) measurements
  if (motionPrevious > motionCurrent)
   { motionSize = (motionPrevious-motionCurrent);
   }

  if (motionCurrent > motionPrevious)
   { motionSize = (motionCurrent-motionPrevious);
   }
  
  /* display.print("Motion: "); */ // Show whether the unit is moving or not.
   
  if (motionSize > 100)
   { display.print("Moving"); 
     motionScore++; 
   }

  if (motionSize < 100)
   { display.print("Stationary");  
   }

  /* display.println(" "); //line spacing */

  /*  display.print("Change: "); */ display.print(motionSize); display.print(" mG"); 
  display.println(" "); //line spacing

  display.print("Score: "); display.print(motionScore); // Print the motion score 
  /* display.print(" Z: "); display.print(motionCurrent); display.print(" mG"); */
  /* display.print(" px: "); display.print(graphPoint1X); display.print(" py: "); display.print(graphPoint1Y); */
  /* display.print(" plotscale: "); display.print(plotScale);  */

  /*  display.println(" "); //line spacing
  display.println(" "); //line spacing */
  
  if (displayMode == 0)
   { display.print("Z: "); display.print((int)lsm.accelData.z); display.print(" mG "); 
     /* display.print("X: "); display.print((int)lsm.accelData.x); display.print(" mG ");
     display.print("Y: "); display.print((int)lsm.accelData.y); display.print(" mG"); */
   }

  if (displayMode == 1) /* Calculating and adding to the graph points*/
  { 
  accelScale = (motionCurrent/10); /* Storing the acceleration reading as a scaled value from 0-100 (100=1G) */
  if (motionSize > 100){ /* Checking if we should add a point to the graph.*/
       graphPoint1X = 0;
       graphPoint1Y = 0;
       /*plotScale = (accelScale * graphOnePercent); */ /* Plotting how percentage value for how big the plot should be*/
       plotScale = (motionCurrent/64);
       graphPoint1Y = ((int)display.height()-plotScale); /* Setting actual plot coordinates in pixels */
       graphPoint1X = (display.width()-1); 
      }
  }

  if (displayMode == 1) /* Drawing the graph lines*/
   { if (graphPoint1X > 0) 
        { display.drawLine(graphPoint1X, graphPoint1Y, graphPoint1X, display.height()-1, WHITE);
          display.drawLine(graphPoint1X, graphPoint1Y, display.width(), display.height()-1, WHITE);
        }
   }

  if (displayMode == 1) /* Animating the graph points & resetting them once off screen*/
  {  if (graphPoint1X == 1){
         graphPoint1X = 0;
         graphPoint1Y = 0;
     }
     if (graphPoint1X != 0){graphPoint1X--;}
  }

  // Cache previous Y plane (vertical) reading
  motionPrevious = motionCurrent;
  
  //display.print("X:");  display.print((int)lsm.magData.x); display.print(" "); //Magnetic compass X plane (bearing, when level)
  //display.print("Y:"); display.print((int)lsm.magData.y); display.print(" "); //Magnetic compass Y plane
  //display.print("Z:"); display.println((int)lsm.magData.z); display.print(" "); //Magnetic compass Z plane
  display.display();
  delay(1000);
  display.clearDisplay();
  
} // end while

} // end program main void 



void loop() {
  
}


void testdrawbitmap(const uint8_t *bitmap, uint8_t w, uint8_t h) {
  uint8_t icons[NUMFLAKES][3];
  srandom(666);     // whatever seed
 
  // initialize
  for (uint8_t f=0; f< NUMFLAKES; f++) {
    icons[f][XPOS] = random() % display.width();
    icons[f][YPOS] = 0;
    icons[f][DELTAY] = random() % 5 + 1;
    
    Serial.print("x: ");
    Serial.print(icons[f][XPOS], DEC);
    Serial.print(" y: ");
    Serial.print(icons[f][YPOS], DEC);
    Serial.print(" dy: ");
    Serial.println(icons[f][DELTAY], DEC);
  }

  while (1) {
    // draw each icon
    for (uint8_t f=0; f< NUMFLAKES; f++) {
      display.drawBitmap(icons[f][XPOS], icons[f][YPOS], logo16_glcd_bmp, w, h, WHITE);
    }
    display.display();
    delay(200);
    
    // then erase it + move it
    for (uint8_t f=0; f< NUMFLAKES; f++) {
      display.drawBitmap(icons[f][XPOS], icons[f][YPOS],  logo16_glcd_bmp, w, h, BLACK);
      // move it
      icons[f][YPOS] += icons[f][DELTAY];
      // if its gone, reinit
      if (icons[f][YPOS] > display.height()) {
	icons[f][XPOS] = random() % display.width();
	icons[f][YPOS] = 0;
	icons[f][DELTAY] = random() % 5 + 1;
      }
    }
   }
}


void testdrawchar(void) {
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0,0);

  for (uint8_t i=0; i < 168; i++) {
    if (i == '\n') continue;
    display.write(i);
    if ((i > 0) && (i % 21 == 0))
      display.println();
  }    
  display.display();
}

void testdrawcircle(void) {
  for (int16_t i=0; i<display.height(); i+=2) {
    display.drawCircle(display.width()/2, display.height()/2, i, WHITE);
    display.display();
  }
}

void testfillrect(void) {
  uint8_t color = 1;
  for (int16_t i=0; i<display.height()/2; i+=3) {
    // alternate colors
    display.fillRect(i, i, display.width()-i*2, display.height()-i*2, color%2);
    display.display();
    color++;
  }
}

void testdrawtriangle(void) {
  for (int16_t i=0; i<min(display.width(),display.height())/2; i+=5) {
    display.drawTriangle(display.width()/2, display.height()/2-i,
                     display.width()/2-i, display.height()/2+i,
                     display.width()/2+i, display.height()/2+i, WHITE);
    display.display();
  }
}

void testfilltriangle(void) {
  uint8_t color = WHITE;
  for (int16_t i=min(display.width(),display.height())/2; i>0; i-=5) {
    display.fillTriangle(display.width()/2, display.height()/2-i,
                     display.width()/2-i, display.height()/2+i,
                     display.width()/2+i, display.height()/2+i, WHITE);
    if (color == WHITE) color = BLACK;
    else color = WHITE;
    display.display();
  }
}

void testdrawroundrect(void) {
  for (int16_t i=0; i<display.height()/2-2; i+=2) {
    display.drawRoundRect(i, i, display.width()-2*i, display.height()-2*i, display.height()/4, WHITE);
    display.display();
  }
}

void testfillroundrect(void) {
  uint8_t color = WHITE;
  for (int16_t i=0; i<display.height()/2-2; i+=2) {
    display.fillRoundRect(i, i, display.width()-2*i, display.height()-2*i, display.height()/4, color);
    if (color == WHITE) color = BLACK;
    else color = WHITE;
    display.display();
  }
}
   
void testdrawrect(void) {
  for (int16_t i=0; i<display.height()/2; i+=2) {
    display.drawRect(i, i, display.width()-2*i, display.height()-2*i, WHITE);
    display.display();
  }
}

void testdrawline() {  
  for (int16_t i=0; i<display.width(); i+=4) {
    display.drawLine(0, 0, i, display.height()-1, WHITE);
    display.display();
  }
  for (int16_t i=0; i<display.height(); i+=4) {
    display.drawLine(0, 0, display.width()-1, i, WHITE);
    display.display();
  }
  delay(250);
  
  display.clearDisplay();
  for (int16_t i=0; i<display.width(); i+=4) {
    display.drawLine(0, display.height()-1, i, 0, WHITE);
    display.display();
  }
  for (int16_t i=display.height()-1; i>=0; i-=4) {
    display.drawLine(0, display.height()-1, display.width()-1, i, WHITE);
    display.display();
  }
  delay(250);
  
  display.clearDisplay();
  for (int16_t i=display.width()-1; i>=0; i-=4) {
    display.drawLine(display.width()-1, display.height()-1, i, 0, WHITE);
    display.display();
  }
  for (int16_t i=display.height()-1; i>=0; i-=4) {
    display.drawLine(display.width()-1, display.height()-1, 0, i, WHITE);
    display.display();
  }
  delay(250);

  display.clearDisplay();
  for (int16_t i=0; i<display.height(); i+=4) {
    display.drawLine(display.width()-1, 0, 0, i, WHITE);
    display.display();
  }
  for (int16_t i=0; i<display.width(); i+=4) {
    display.drawLine(display.width()-1, 0, i, display.height()-1, WHITE); 
    display.display();
  }
  delay(250);
}

void testscrolltext(void) {
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(10,0);
  display.clearDisplay();
  display.println("scroll");
  display.display();
 
  display.startscrollright(0x00, 0x0F);
  delay(2000);
  display.stopscroll();
  delay(1000);
  display.startscrollleft(0x00, 0x0F);
  delay(2000);
  display.stopscroll();
  delay(1000);    
  display.startscrolldiagright(0x00, 0x07);
  delay(2000);
  display.startscrolldiagleft(0x00, 0x07);
  delay(2000);
  display.stopscroll();
}
