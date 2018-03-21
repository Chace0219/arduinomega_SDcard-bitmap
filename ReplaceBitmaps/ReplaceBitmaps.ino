#include <SPI.h>
#include <SD.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <TimerOne.h>


// change this to match your SD shield or module;
//     Arduino Ethernet shield: pin 4
//     Adafruit SD shields and modules: pin 10
//     Sparkfun SD shield: pin 8
const int chipSelect = 4;

#define OLED_MOSI 20
#define OLED_CLK 21
#define OLED_DC 5
#define OLED_CS 7 // from 4 to 7
#define OLED_RESET 19
Adafruit_SSD1306 display(OLED_MOSI, OLED_CLK, OLED_DC, OLED_RESET, OLED_CS);

#define NUMFLAKES 10
#define XPOS 0
#define YPOS 1
#define DELTAY 2

#define LOGO16_GLCD_HEIGHT 16
#define LOGO16_GLCD_WIDTH 16

// temporary display array variable
static uint8_t tempFrameMem[1024];

// 
int x = 0;
int loopCount = -1;
int arrayNumber = 0;

// 
uint8_t nFrameIndex = 0;

const uint8_t BUTTON = 22; // D22


int buttonState;             // the current reading from the input pin
int lastButtonState = LOW;   // the previous reading from the input pin

unsigned long lastDebounceTime = 0;  // the last time the output pin was toggled
unsigned long debounceDelay = 3; //yeah? debounce time setting    // the debounce time; increase if the output flickers

volatile static uint8_t nDirSel = 0;

String directoryPath = "";

File bmpDir;

String path1st = "/bin/0";
String path2nd = "/bin/1";
String path3rd = "/bin/2";
String path4th = "/bin/3";

bool bDirExist = false;

void setup()
{
  // set the baud rate of serial port as 9600
  Serial.begin(9600);

  delay(2000);

  /* SSD1306 LCD initialize */
  // Initializes the interface to the LCD screen
  display.begin(SSD1306_SWITCHCAPVCC);

  // Clears the LCD screen and positions the cursor in the upper-left corner  
  display.clearDisplay();

  // This will invert the colours of the screen
  display.invertDisplay(true);


  // Rotation 270 degree
  display.setRotation(3);

  Serial.print("Initializing SD card...");
  pinMode(SS, OUTPUT);

  if (!SD.begin(chipSelect)) {
    Serial.println("initialization failed!");
    return;
  }
  Serial.println("initialization done.");


  //
  directoryPath = path1st;
  String path = directoryPath;
  bmpDir = SD.open(directoryPath);

  if (!bmpDir)
  {
    bDirExist = false;
    bmpDir.close();
    Serial.print("I can open directory, ");
    Serial.println(directoryPath);
  }
  else
    bDirExist = true;

  nFrameIndex = 0;
  // Set pin 22 to be an input device, in current situation, I don't know that functonality in more detail.
  
  
  pinMode(BUTTON, INPUT_PULLUP);

  Timer1.initialize(10000); // set a timer of length 1 millis
  Timer1.attachInterrupt( timerIsr ); // attach the service routine here
  
}

volatile uint8_t previousButton = false;
volatile bool bInterrupt = false;
void timerIsr()
{
  Timer1.detachInterrupt();
  //Timer1.stop();
  uint8_t currState = digitalRead(BUTTON);
  if(previousButton != currState)
  {
    previousButton = currState;
    if(currState == LOW)
    {
      bInterrupt = true;
    }
  }
  //Timer1.restart();
  Timer1.attachInterrupt( timerIsr ); // attach the service routine here

}


bool getBinaryData(uint8_t *dstArr, uint16_t nCount, String fileName)//Get BIN data from file in SD card.  This takes in a pointer to a 8 bit integer, a 16 bit integer and a string.
{
  File myFile;                                                                                          //Declare a new file called myFile
  bool bResult = false;                                                                                 //declare a bool called bResult and set it to false

  uint16_t nMax = nCount;                                                                               //Set the number of elements in each file to nMax
  
  //Open file in SD card.
  myFile = SD.open(fileName);                                                                           //Open the file using the file nama.  Assign the file to myFile

  if (!myFile)                                                                                          //If myFile doesn't exist or is empty or can't be opened...
  {
    Serial.print("File open error, ");                                                                  //Output an error message
    Serial.println(fileName);                                                                           //Output the name of the file causing the error
    return false;                                                                                       //End this method
  }

  //Read buffer from file and store data to buffer
  myFile.read(dstArr, 1024);
  myFile.close();                                                                                       //Close the file
  return true;                                                                                          //Return true
}
void loop()
{

  if(bInterrupt)
  {
    bInterrupt = false;
    Serial.println("You have clicked Button!");
    bmpDir.close();

    nDirSel++;
    if (nDirSel > 3)
      nDirSel = 0;

    switch (nDirSel)
    {
      case 0:
        directoryPath = path1st;
      break;

      case 1:
        directoryPath = path2nd;
        break;

      case 2:
        directoryPath = path3rd;
        break;

      case 3:
        directoryPath = path4th;
        break;
    }

    String Path = directoryPath;
    bmpDir = SD.open(Path);
    if (!bmpDir)
    {
      bDirExist = false;
      bmpDir.close();
      Serial.print("I can open directory, ");
      Serial.println(directoryPath);
    }
    else
    {
      Serial.print("Current Working directory is ");
      Serial.println(directoryPath);
      bDirExist = true;
    }
    nFrameIndex = 0;  
    
  }
  
  if (bDirExist)
  {
    File entry = bmpDir.openNextFile();
    if (!entry) 
    {
      bmpDir.close();
      bmpDir = SD.open(directoryPath);
      entry = bmpDir.openNextFile();
      nFrameIndex = 0;
    }

    
    
    String fileName = entry.name();
    Serial.println(fileName);

    if (!entry.isDirectory())
    {
      String strBinFile = (directoryPath + "/") + fileName;
      uint32_t nTime = millis();

      // As you know, in this line it takes 300ms, block
      if (getBinaryData(tempFrameMem, 1024, strBinFile))
      {
        Serial.println(strBinFile);

        Serial.print("Loading time is ");
        Serial.print(millis() - nTime);
        Serial.println("ms."); 
        nTime = millis();
        
        display.clearDisplay();

        // And in this line also, block is happened. (70ms)
        display.drawBitmap(0, 0, tempFrameMem, display.width(), display.height(), WHITE);


        // So, if we monitor button in loop part, maybe, 
        // It looks button works by 500ms holding press, right?
        // So, we have to monitor button in interrupt part,
        // There are two solutions, one is timer interrupt, one is external change interrupt
        // I am testing about external interrupt
        // Output current frame index at the left bottom of LCD
        display.setTextSize(1);
        display.setTextColor(BLACK);
        display.setCursor(0, 110);
        if (nFrameIndex < 100)
          display.print(0);
        if (nFrameIndex < 10)
          display.print(0);
        display.print(nFrameIndex);

        Serial.print("Displaying time is ");
        Serial.print(millis() - nTime);
        Serial.println("ms.");
        //

        nFrameIndex++;
        // Print current frame index via serial port

        // Represent modified screen status
        display.display();
      }
    }
    entry.close();
  }
}
