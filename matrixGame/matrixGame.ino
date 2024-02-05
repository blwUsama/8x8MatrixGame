#include "LedControl.h"
#include <LiquidCrystal.h>
#include <EEPROM.h>
#include <string.h>

const byte dinPin = 12; // pin 12 is connected to the MAX7219 pin 1
const byte clockPin = 11; // pin 11 is connected to the CLK pin 13
const byte loadPin = 10; // pin 10 is connected to LOAD pin 12
const int xJoystickPin = A0;
const int yJoystickPin = A1;
const int goalLedPin = 3;
const int buttonPin = 2;
const int buzzerPin = 6;
const byte rs = 13;
const byte en = 8;
const byte d4 = 7;
const byte d5 = A5;
const byte d6 = 5;
const byte d7 = 4;
const int lcdBrightnessPin = 9;

LiquidCrystal lcd(rs, en, d4, d5, d6, d7);
LedControl lc = LedControl(dinPin, clockPin, loadPin, 1); //DIN, CLK, LOAD, No. DRIVER

const int noOfMainMenuOptions = 3; //1 Start game, 2 Settings, 3 About
const char* mainMenuOptions[noOfMainMenuOptions + 1] =
{
  "",
  "Start game",
  "Settings",
  "About",
};
const int noOfSettingsMenuOptions = 3; // 1 LCD brightness, 2 Matrix brightness, 3 Back
const char* settingsMenuOptions[noOfSettingsMenuOptions + 1] =
{
  "",
  "LCD brightness",
  "8x8 brightness",
  "Back",
};
int currentOption = 1;
bool mainMenuAccessed = true;
bool isChoosingMenuOption = true;
bool isChoosingLcdBrightness = false;
bool isChoosingMatrixBrightness = false;
bool isPrintingAboutText = false;
int lcdBrightness = EEPROM.read(0);
byte matrixBrightness = EEPROM.read(1);

byte bombChar[] =
{
  B11000,
  B01100,
  B00100,
  B01110,
  B11111,
  B11111,
  B11111,
  B01110
};

byte rightArrow[] =
{
  B00000,
  B00100,
  B00110,
  B11111,
  B11111,
  B00110,
  B00100,
  B00000
};

bool startGame = false;

int xJoystickValue;
int yJoystickValue;
int minThreshold = 300;
int maxThreshold = 750;
const int playerMoveRate = 600; //milliseconds
bool joystickIsMoving = false;
bool moveAvailable = true;
int playerXCoordinate = 0;
int playerYCoordinate = 0;

volatile unsigned long lastInterruptTime = 0;
const unsigned long debounceDelay = 200;
volatile bool buttonPressed = false;
int buttonPressCounter = 0;

const int playerBlinkRate = 300; //milliseconds
bool playerBlinkState = true;
bool goalBlinkState = true;
const int dangerBlinkRate = 100;


int timeElapsed; //since game start
int score = 1000;
int bombsLeft = 3;
int level = 1;
int goalXCoordinate = 5;
int goalYCoordinate = 8;
const int maximumDistance = 21; //the maximum distance in a straight line in our map is the diagonal
// considering the map is a square the diagonal is side * rad(2) = 21.21


const int totalRows = 16;
const int totalColumns = 16;

int currentMap[totalRows][totalColumns] =
{
  // 0  1  2  3  4  5  6  7  8  9  10 11 12 13 14 15
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, // 0
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0}, // 1
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0}, // 2
    {0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 1, 0}, // 3
    {0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 1, 0}, // 4
    {1, 1, 1, 1, 1, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 0}, // 5
    {0, 0, 1, 1, 0, 1, 1, 0, 0, 1, 1, 1, 0, 1, 1, 0}, // 6
    {0, 0, 1, 0, 1, 0, 1, 0, 0, 0, 1, 0, 0, 1, 1, 0}, // 7
    {0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 1, 0}, // 8
    {1, 1, 1, 1, 0, 0, 0, 1, 0, 0, 1, 0, 0, 1, 1, 0}, // 9
    {1, 0, 0, 1, 1, 1, 1, 1, 0, 1, 1, 0, 0, 1, 1, 0}, // 10
    {1, 0, 0, 0, 0, 0, 1, 0, 1, 1, 1, 1, 1, 1, 1, 0}, // 11
    {1, 0, 0, 0, 0, 0, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0}, // 12
    {1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 1, 1, 0}, // 13
    {1, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0}, // 14
    {1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, // 15
};

int level2Map[totalRows][totalColumns] = 
{
  // 0  1  2  3  4  5  6  7  8  9  10 11 12 13 14 15
    {1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1}, // 0
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1}, // 1
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1}, // 2
    {1, 0, 0, 0, 0, 1, 1, 0, 1, 1, 0, 0, 0, 0, 0, 1}, // 3
    {1, 1, 1, 1, 0, 1, 1, 0, 1, 1, 0, 0, 1, 1, 1, 1}, // 4
    {1, 1, 1, 1, 0, 1, 1, 0, 1, 1, 0, 0, 1, 1, 1, 1}, // 5
    {1, 0, 1, 1, 0, 1, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1}, // 6
    {1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1}, // 7
    {1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1}, // 8
    {1, 0, 1, 1, 0, 1, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1}, // 9
    {1, 1, 1, 1, 0, 1, 1, 0, 1, 1, 0, 0, 1, 1, 1, 1}, // 10
    {1, 1, 1, 1, 0, 1, 1, 0, 1, 1, 0, 0, 1, 1, 1, 1}, // 11
    {1, 0, 0, 0, 0, 1, 1, 0, 1, 1, 0, 0, 0, 0, 0, 1}, // 12
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1}, // 13
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1}, // 14
    {1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1}, // 15
};

const int fieldOfViewRows = 8;
const int fieldOfViewColumns = 8;
int fieldOfViewXCoordinate = 0; // the fieldOfView point represents the upper-left corner of an 8x8 square
int fieldOfViewYCoordinate = 0; // if i know this point i know which points of the 16 x 16 matrix the
// square intersects as well
// the field of view adjusts whenever the player gets 1 cell away from
// the square's margin


void setup() {
  Serial.begin(9600);
  pinMode(lcdBrightnessPin, OUTPUT);
  pinMode(buttonPin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(buttonPin), handleButtonPress, FALLING);

  // the zero refers to the MAX7219 number, it is zero for 1 chip
  lc.shutdown(0, false); // turn off power saving, enables display
  lc.setIntensity(0, matrixBrightness); // sets brightness (0~15 possible values)
  lc.clearDisplay(0);// clear screen
  for (int row = 0; row < fieldOfViewRows; row++)
  {
    for (int col = 0; col < fieldOfViewColumns; col++)
    {
     lc.setLed(0, row, col, true); // turns on LED at col, row
    }
  }

  randomSeed(analogRead(A0));

  lcd.begin(16, 2);
  analogWrite(lcdBrightnessPin, lcdBrightness * 50);
  lcd.createChar(1, bombChar);
  lcd.createChar(2, rightArrow);
  lcd.clear();
  lcd.setCursor(1, 0);
  lcd.write(1);
  lcd.print("Bomb Dungeon");
  lcd.write(1);
  delay(2000);
  lcd.clear();
  printMenu(currentOption, mainMenuAccessed);
}
void loop()
{
  if(startGame)
  {
    playSong();
    printInGameMenu();
    drawMap();
    calculateDistance();
    listenMovement();
    updateFieldOfView();
    displayPlayer();
    displayGoal();
    checkEndGame();
//    Serial.println(xJoystickValue);
  }
  else
  {
    listenMovement();
  }
}

void printMenu(int option, bool mainMenu)
{
  if (mainMenu)
  {
    switch (option)
    {
      case 1:
        {
          lcd.clear();
          lcd.write(2); // arrow character
          lcd.setCursor(2, 0);
          lcd.print(mainMenuOptions[option]); // -> Start game
          lcd.setCursor(2, 1);
          lcd.print(mainMenuOptions[option + 1]); // Settings
          break;
        }
      case 2:
        {
          lcd.clear();
          lcd.write(2); // arrow character
          lcd.setCursor(2, 0);
          lcd.print(mainMenuOptions[option]); // -> Settings
          lcd.setCursor(2, 1);
          lcd.print(mainMenuOptions[option + 1]); // About
          break;
        }
      case 3:
        {
          lcd.clear();
          lcd.setCursor(2, 0);
          lcd.print(mainMenuOptions[option - 1]); // Settings
          lcd.setCursor(0, 1);
          lcd.write(2); // arrow character
          lcd.setCursor(2, 1);
          lcd.print(mainMenuOptions[option]); // -> About
          break;
        }
    }
  }
  else //if it's the settings submenu
  {
    switch (option)
    {
      case 1:
        {
          lcd.clear();
          lcd.write(2); // arrow character
          lcd.setCursor(2, 0);
          lcd.print(settingsMenuOptions[option]); // -> LCD brightness
          lcd.setCursor(2, 1);
          lcd.print(settingsMenuOptions[option + 1]); // 8x8 brightness
          break;
        }
      case 2:
        {
          lcd.clear();
          lcd.write(2); // arrow character
          lcd.setCursor(2, 0);
          lcd.print(settingsMenuOptions[option]); // -> 8x8 brightness
          lcd.setCursor(2, 1);
          lcd.print(settingsMenuOptions[option + 1]); // back
          break;
        }
      case 3:
        {
          lcd.clear();
          lcd.setCursor(2, 0);
          lcd.print(settingsMenuOptions[option - 1]); // 8x8 brightness
          lcd.setCursor(0, 1);
          lcd.write(2); // arrow character
          lcd.setCursor(2, 1);
          lcd.print(settingsMenuOptions[option]); // -> back
          break;
        }
    }
  }
}

void chooseMenuOption(int option, bool mainMenu)
{
  if (mainMenu)
  {
    switch (option)
    {
      case 1: // start game option
        {
          isChoosingMenuOption = false;
          startGame = true;
          Serial.print("game started");
          break;
        }
      case 2: // settings option
        {
          mainMenuAccessed = false;
          currentOption = 1;
          printMenu(currentOption, mainMenuAccessed);
          break;
        }
      case 3: // about option
        {
          isChoosingMenuOption = false;
          mainMenuAccessed = false;
          isPrintingAboutText = true;
          const char* aboutMessage = "Bomb Dungeon by Usama. Github:blwUsama";
          lcd.clear();
          printAboutText(aboutMessage);
          break;
        }
    }
  }
  else // settings submenu options
  {
    switch (option)
    {
      case 1: // LCD brightness
        {
          isChoosingMenuOption = false;
          isChoosingLcdBrightness = true;
          lcd.clear();
          lcd.print("from 1 to 5:");
          lcd.setCursor(0, 1);
          lcd.print(lcdBrightness);
          lcd.setCursor(0,1);
          lcd.blink();
          analogWrite(lcdBrightnessPin, 50 * lcdBrightness); // * 50 to map it from 1-5 to 50-250
          break;
        }
      case 2: // Matrix brightness
        {
          isChoosingMenuOption = false;
          isChoosingMatrixBrightness = true;
          lcd.clear();
          lcd.print("from 1 to 5:");
          lcd.setCursor(0, 1);
          lcd.print(matrixBrightness);
          lcd.setCursor(0,1);
          lcd.blink();
          lc.setIntensity(0, matrixBrightness * 3); // * 3 to map it from 1-5 to 3-15
          break;
        }
      case 3: // Back
        {
          mainMenuAccessed = true;
          currentOption = 1;
          printMenu(currentOption, mainMenuAccessed);
          break;
        }
    }
  }
}

void printAboutText(char* string)
{
  lcd.clear();
  lcd.setCursor(1, 0);
  lcd.write(1);
  lcd.print("Bomb Dungeon");
  lcd.write(1);

  lcd.setCursor(0, 1);
  lcd.print("Github:blwUsama");
}

void printInGameMenu()
{
  const static unsigned long timeGameStarted = millis(); //in milliseconds
  timeElapsed = (millis() - timeGameStarted) / 1000; //in seconds
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Score:");
  lcd.print(score - timeElapsed);
  lcd.setCursor(0, 1);
  lcd.print("bombs:");
  for(int i = 0; i < bombsLeft; i++)
  {
    lcd.write(1);
  }
}

void listenMovement()
{
  static long unsigned lastTime = 0;
  if (millis() - lastTime > playerMoveRate)
  {
    moveAvailable = true;
    lastTime = millis();
  }
  if (moveAvailable)
  {
    xJoystickValue = analogRead(xJoystickPin);
    yJoystickValue = analogRead(yJoystickPin);
    if (xJoystickValue > maxThreshold)  //do right
    {
      lc.setLed(0, playerYCoordinate, playerXCoordinate, false);
      if (playerXCoordinate < totalColumns - 1) // makes sure X coordinate doesn't overflow to the right(always less than 16)
      {
        if (currentMap[playerYCoordinate][playerXCoordinate + 1] == 0)
        {
          playerXCoordinate++;
          moveAvailable = false;
        }

      }
    }
    if (xJoystickValue < minThreshold)  // do left
    {
      lc.setLed(0, playerYCoordinate, playerXCoordinate, false);
      if (playerXCoordinate > 0) // makes sure X coordinate doesn't overflow to the left (always greater than 0)
      {
        if (currentMap[playerYCoordinate][playerXCoordinate - 1] == 0)
        {
          playerXCoordinate--;
          moveAvailable = false;
        }
      }
    }
    if (yJoystickValue > maxThreshold)  // do up
    {
      if(startGame)
      {
        lc.setLed(0, playerYCoordinate, playerXCoordinate, false);
        if (playerYCoordinate > 0) // makes sure Y coordinate doesn't overflow to the top (always greater than 0)
        {
          if (currentMap[playerYCoordinate - 1][playerXCoordinate] == 0)
          {
            playerYCoordinate--;
            moveAvailable = false;
          }
  
        }
      }
      else //in menu
      {
        if(isChoosingLcdBrightness)
        {
          if(lcdBrightness < 5)
          {
            lcdBrightness++;
            moveAvailable = false;
            chooseMenuOption(currentOption, mainMenuAccessed); 
            //we call this function again to update the prompt
          }
        }
        if(isChoosingMatrixBrightness)
        {
          if(matrixBrightness < 5)
          {
            matrixBrightness++;
            lc.setIntensity(0, matrixBrightness * 3); // * 3 to map it from 1-5 to 3-15
            moveAvailable = false;
            chooseMenuOption(currentOption, mainMenuAccessed); 
            //we call this function again to update the prompt
          }
        }
        if(isChoosingMenuOption)
        {
          if (currentOption > 1)
          {
            currentOption--;
            printMenu(currentOption, mainMenuAccessed);
            moveAvailable = false;
            //we call this function again to update the prompt
          }
        }
      }
    }
    if (yJoystickValue < minThreshold)  //do down
    {
      if(startGame)
      {
        lc.setLed(0, playerYCoordinate, playerXCoordinate, false);
        if (playerYCoordinate < totalRows - 1) // makes sure Y coordinate doesn't overflow to the bottom (always less than 16)
        {
          if (currentMap[playerYCoordinate + 1][playerXCoordinate] == 0) // collision check
          {
            playerYCoordinate++;
            moveAvailable = false;
          }
        }
      }
      else //in menu
      {
        if(isChoosingLcdBrightness)
        {
          if(lcdBrightness > 1)
          {
            lcdBrightness--;
            moveAvailable = false;
            chooseMenuOption(currentOption, mainMenuAccessed); 
            //we call this function again to update the prompt
          }
        }
        if(isChoosingMatrixBrightness)
        {
          if(matrixBrightness > 1)
          {
            matrixBrightness--;
            lc.setIntensity(0, matrixBrightness * 3); // * 3 to map it from 1-5 to 3-15
            moveAvailable = false;
            chooseMenuOption(currentOption, mainMenuAccessed); 
            //we call this function again to update the prompt
          }
        }
        if(isChoosingMenuOption)
        {
          if (currentOption < 3)
          {
            currentOption++;
            printMenu(currentOption, mainMenuAccessed);
            moveAvailable = false;
          }        
        }
      }
    }
  }
}

void calculateDistance()
{
  int distance;
  int mappedValue;
  distance = pow((playerXCoordinate - goalXCoordinate), 2) + pow((playerYCoordinate - goalYCoordinate), 2);
  distance = sqrt(distance);
    Serial.println(distance);
  if(distance > 6)
  {
    mappedValue = 0;
  }
  else if(distance <= 6)
  {
    Serial.println("|  getting close");
    mappedValue = map(distance, 0, 6, 255, 0);
  }
  analogWrite(goalLedPin, mappedValue);
}

void updateFieldOfView()
{
  const int rightMargin = fieldOfViewXCoordinate + fieldOfViewColumns - 1; // represents the X coordinate of the right margin of the field of view square
  const int leftMargin = fieldOfViewXCoordinate;  // represents the X coordinate of the left margin of the field of view square
  const int upperMargin = fieldOfViewYCoordinate; // represents the Y coordinate of the upper margin of the field of view square
  const int lowerMargin = fieldOfViewYCoordinate + fieldOfViewRows - 1;    // represents the Y coordinate of the lower margin of the field of view square

  if (lowerMargin - playerYCoordinate <= 1)   //pan down
  {
    if (fieldOfViewYCoordinate < totalRows - fieldOfViewRows) // make sure field of view doesn't overflow from map
    {
      fieldOfViewYCoordinate++;
    }
  }

  if (playerYCoordinate - upperMargin <= 1) //pan up
  {
    if (fieldOfViewYCoordinate > 0)
    {
      fieldOfViewYCoordinate--;
    }
  }

  if (rightMargin - playerXCoordinate <= 1) //pan right
  {
    if (fieldOfViewXCoordinate < totalColumns - fieldOfViewColumns)
    {
      fieldOfViewXCoordinate++;
    }
  }

  if (playerXCoordinate - leftMargin <= 1) // pan left
  {
    if (fieldOfViewXCoordinate > 0)
    {
      fieldOfViewXCoordinate--;
    }
  }
  //  Serial.println(fieldOfViewYCoordinate);

}

void displayPlayer()
{
  static long unsigned lastTime = 0;
  if (millis() - lastTime > playerBlinkRate)
  {
    playerBlinkState = !playerBlinkState;
    lastTime = millis();
  }
  lc.setLed(0, playerYCoordinate - fieldOfViewYCoordinate, playerXCoordinate - fieldOfViewXCoordinate, playerBlinkState);
  //  Serial.println(playerBlinkState);
}

void displayGoal()
{
  static long unsigned lastTime = 0;
  if(millis() - lastTime > playerBlinkRate)
  {
    goalBlinkState = !goalBlinkState;
    lastTime = millis();
  }
  if (goalXCoordinate >= fieldOfViewXCoordinate && goalXCoordinate <= fieldOfViewXCoordinate + fieldOfViewColumns)
  {
    if (goalYCoordinate >= fieldOfViewYCoordinate && goalYCoordinate <= fieldOfViewYCoordinate + fieldOfViewRows)
    {
      lc.setLed(0, goalYCoordinate - fieldOfViewYCoordinate, goalXCoordinate - fieldOfViewXCoordinate, goalBlinkState);

    }
  }
}

void drawMap()
{
  for (int i = 0; i < fieldOfViewColumns; i++)
  {
    for (int j = 0; j < fieldOfViewRows; j++)
    {
      if (i == playerYCoordinate - fieldOfViewYCoordinate && j == playerXCoordinate - fieldOfViewXCoordinate)
      {
        continue;
      }
      lc.setLed(0, i, j, currentMap[i + fieldOfViewYCoordinate][j + fieldOfViewXCoordinate]);
    }
  }
}

void handleButtonPress()
{
  //  Serial.println("handler function called");
  static unsigned long interruptTime = 0;
  interruptTime = micros();
  if (interruptTime - lastInterruptTime > debounceDelay * 1000)
  {
    if(startGame)
    {
      placeBomb(playerXCoordinate, playerYCoordinate);
    }
    else if(isChoosingMenuOption)
    {
      chooseMenuOption(currentOption, mainMenuAccessed);
    }
    else if(isPrintingAboutText)
    {
      isPrintingAboutText = false;
      isChoosingMenuOption = true;
      mainMenuAccessed = true;
      printMenu(currentOption, mainMenuAccessed);
      
    }
    else //if is choosing lcd or matrix brightness
    {
      printMenu(currentOption, mainMenuAccessed);
      isChoosingLcdBrightness = false;
      isChoosingMatrixBrightness = false;
      isChoosingMenuOption = true; 
      EEPROM.update(0, lcdBrightness);
      EEPROM.update(1, matrixBrightness);
      lcd.noBlink();
    }
  }
  lastInterruptTime = interruptTime;
}

void placeBomb(const int xCoordinate, const int yCoordinate)
{
  if(bombsLeft > 0)
  {
    Serial.println("bomb has been placed");
    const int bombedCells = 5; //the bomb can blow up a maximum of 5 positions at once
    const int bombedPositions[bombedCells][2] =  //the 2 comes from the X and Y coordinate of each position
    {
      {xCoordinate, yCoordinate},
      {xCoordinate + 1, yCoordinate},
      {xCoordinate - 1, yCoordinate},
      {xCoordinate, yCoordinate + 1},
      {xCoordinate, yCoordinate - 1},
    };
  
    unsigned long lastBlinkTime = millis();
    for (int i = 0; i < bombedCells; i++)
    {
      if(bombedPositions[i][0] >= 0 && bombedPositions[i][0] <= 15 && bombedPositions[i][1] >= 0 && bombedPositions[i][1] <= 15)
      {
        lc.setLed(0, bombedPositions[i][1] - fieldOfViewYCoordinate, bombedPositions[i][0] - fieldOfViewXCoordinate, 1);
        currentMap[bombedPositions[i][1]][bombedPositions[i][0]] = 0;
      }
    }
    bombsLeft--;
  }
  
}

void checkEndGame()
{
  if (playerXCoordinate == goalXCoordinate && playerYCoordinate == goalYCoordinate)
  {
    for (int row = 0; row < fieldOfViewRows; row++)
    {
      for (int col = 0; col < fieldOfViewColumns; col++)
      {
        lc.setLed(0, row, col, true); // turns on LED at col, row
        delay(25);
        playSong();
      }
    }

    for (int row = 0; row < fieldOfViewRows; row++)
    {
      for (int col = 0; col < fieldOfViewColumns; col++)
      {
        lc.setLed(0, row, col, false); // turns on LED at col, row
        delay(25);
        playSong();
      }
    }
    if(level == 1)
    {
      for(int i = 0; i < totalRows; i++)
      {
        for(int j = 0; j < totalColumns; j++)
        {
          currentMap[i][j] = level2Map[i][j];
        }
      }
      fieldOfViewXCoordinate = 4;
      fieldOfViewYCoordinate = 4;
      goalXCoordinate = 1;
      goalYCoordinate = 6;
      playerXCoordinate = 7;
      playerYCoordinate = 7;
      bombsLeft = 3;  
      level++;  
    }
    else if(level == 2)
    {
      startGame = false;
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("You Win!");
      lcd.setCursor(0, 1);
      lcd.print("Score:");
      lcd.print(score - timeElapsed);
      
    }
  }  
}

void playSong()
{
  int melody[] = {440, 523, 554, 440, 523, 554, 440, 523, 554, 349, 392, 440, 349, 392, 440, 392, 554, 523};
  int noteDurations[] = {4, 4, 4, 8, 4, 4, 8, 4, 4, 4, 4, 4, 8, 4, 4, 8, 4, 4};
  static unsigned long lastPlayTime = 0;
  static int noteDelay = 0;
  static int notePlayed = 0;
  calculateDistance();

  if(millis() - lastPlayTime > noteDelay)
  {
    int noteDuration = 1000 / noteDurations[notePlayed];
    tone(buzzerPin, melody[notePlayed], noteDuration);
    noteDelay = noteDuration * 1.30;
    lastPlayTime = millis();
    notePlayed++;
    if(notePlayed == 18)
      notePlayed = 0;
  }
}
