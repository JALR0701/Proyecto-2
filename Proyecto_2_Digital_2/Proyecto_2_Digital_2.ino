//***************************************************************************************************************************************
/* Librería para el uso de la pantalla ILI9341 en modo 8 bits
 * Basado en el código de martinayotte - https://www.stm32duino.com/viewtopic.php?t=637
 * Adaptación, migración y creación de nuevas funciones: Pablo Mazariegos y José Morales
 * Con ayuda de: José Guerra
 * IE3027: Electrónica Digital 2 - 2019
 */
//***************************************************************************************************************************************
#include <stdint.h>
#include <stdbool.h>
#include <TM4C123GH6PM.h>
#include <SPI.h>
#include <SD.h>
#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/debug.h"
#include "driverlib/gpio.h"
#include "driverlib/interrupt.h"
#include "driverlib/rom_map.h"
#include "driverlib/rom.h"
#include "driverlib/sysctl.h"
#include "driverlib/timer.h"

#include "bitmaps.h"
#include "font.h"
#include "lcd_registers.h"

#define LCD_RST PD_0
#define LCD_CS PD_1
#define LCD_RS PD_2
#define LCD_WR PD_3
#define LCD_RD PE_1
int DPINS[] = {PB_0, PB_1, PB_2, PB_3, PB_4, PB_5, PB_6, PB_7};  
//***************************************************************************************************************************************
// Functions Prototypes
//***************************************************************************************************************************************
void LCD_Init(void);
void LCD_CMD(uint8_t cmd);
void LCD_DATA(uint8_t data);
void SetWindows(unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2);
void LCD_Clear(unsigned int c);
void H_line(unsigned int x, unsigned int y, unsigned int l, unsigned int c);
void V_line(unsigned int x, unsigned int y, unsigned int l, unsigned int c);
void Rect(unsigned int x, unsigned int y, unsigned int w, unsigned int h, unsigned int c);
void FillRect(unsigned int x, unsigned int y, unsigned int w, unsigned int h, unsigned int c);
void LCD_Print(String text, int x, int y, int fontSize, int color, int background);

void LCD_Bitmap(unsigned int x, unsigned int y, unsigned int width, unsigned int height, unsigned char bitmap[]);
void LCD_Sprite(int x, int y, int width, int height, unsigned char bitmap[],int columns, int index, char flip, char offset);

void Game_Menu(void);
void characterMenu(void);
void musicMenu(void);
void gamePlay(int song, int character1, int character2);

//***************************************************************************************************************************************
// Variables
//***************************************************************************************************************************************

int modeSelect = 1, characterSelect = 1, songSelect = 1, characterSelect2 = 1;
int started = 0, chooseCharacter = 0, chooseSong = 0, game = 0;
String text;
int button;
int escritor = 0, reseteo = 0;
int PinBuzzer = PC_4;
int rebote = 0;
int up = 56, left = 52, down = 50, right = 54, up2 = 119, left2 = 97, down2 = 115, right2 = 100;
int player1 = 0;
int thisNote = 0;

unsigned long  previousMillis = 0;

int melody[] = { 
698, 587, 440, 587, 698, 587, 440, 587, 698, 523, 440, 523, 698, 523, 440, 523, 659, 554, 440, 554, 659, 554, 440, 554, 659, 554, 440, 554, 659, 554, 440, 554, 587
};
int noteDurations[] = { 
176, 176, 176, 176, 176, 176, 176, 176, 176, 176, 176, 176, 176, 176, 176, 176, 176, 176, 176, 176, 176, 176, 176, 176, 176, 176, 176, 176, 176, 176, 176, 176, 1056
};

int melody1[] = {330, 392, 294, 262, 294, 330, 392, 294, 330, 392, 587, 523, 392, 349, 330, 294, 330, 392, 294, 262, 294, 330, 392, 294, 330, 392, 587, 523, 392, 349, 330, 349, 330, 262, 349, 330, 294, 330, 294, 220, 392, 349, 330, 349, 330, 262, 349, 523};
int noteDurations1[] = {1111, 556, 1111, 278, 278, 1111, 556, 1667, 1111, 556, 1111, 556, 1111, 278, 278, 1667, 1111, 556, 1111, 278, 278, 1111, 556, 1667, 1111, 556, 1111, 556, 1111, 278, 278, 278, 278, 1111, 1111, 278, 278, 278, 278, 1111, 1111, 278, 278, 278, 278, 556, 556, 1667};

int melody2[] = {440, 523, 659, 880, 988, 659, 523, 988, 1047, 659, 523, 1047, 740, 587, 440, 740, 659, 523, 440, 523, 0, 659, 523, 440, 392, 440, 440, 0, 0, 440, 698, 659, 440, 440, 523, 659, 988, 659, 523, 988, 1047, 659, 523, 1047, 740, 587, 440, 740, 659, 523, 440, 523, 0, 659, 523, 440, 392, 440, 440, 0, 0, 0, 440, 494, 523, 659, 784, 523, 494, 587, 784, 494, 494, 523, 440, 440, 523, 659, 880, 440, 494, 523, 392, 262, 523, 784, 587, 392, 784, 784, 740, 587, 587, 0, 0, 440, 494, 523, 659, 784, 523, 494, 587, 784, 494, 494, 523, 440, 440, 523, 659, 880, 440, 494, 523, 659, 784, 1047, 587, 740, 880, 1175, 659, 659, 659, 0, 0};
int noteDurations2[] = {405, 405, 405, 405, 405, 405, 405, 405, 405, 405, 405, 405, 405, 405, 405, 405, 405, 405, 405, 810, 270, 405, 405, 405, 405, 405, 405, 405, 810, 405, 405, 405, 405, 405, 405, 405, 405, 405, 405, 405, 405, 405, 405, 405, 405, 405, 405, 405, 405, 405, 405, 810, 270, 405, 405, 405, 405, 405, 405, 405, 810, 405, 405, 405, 405, 405, 405, 405, 405, 405, 405, 405, 203, 203, 405, 405, 405, 405, 405, 405, 405, 405, 405, 405, 405, 405, 405, 405, 405, 203, 203, 405, 810, 270, 405, 405, 405, 405, 405, 405, 405, 405, 405, 405, 405, 203, 203, 405, 405, 405, 405, 405, 405, 405, 405, 405, 405, 405, 405, 405, 405, 405, 405, 405, 405, 1620, 810, 810};

int melody3[] = {784, 698, 622, 587, 466, 392, 523, 587, 622, 698, 587, 0, 1047, 784, 622, 587, 523, 523, 587, 622, 523, 466, 523, 392, 1047, 784, 622, 587, 523, 523, 587, 622, 698, 587, 466, 523, 392, 523, 587, 622, 698, 784, 831, 1047, 1047, 784, 622, 587, 523, 523, 587, 622, 523, 466, 523, 392, 1047, 784, 622, 698, 784, 523, 587, 698, 587, 466, 523, 349, 349, 415, 523, 622, 587, 523, 392, 349, 349, 415, 523, 622, 698, 784, 831, 784, 698, 622, 698, 784, 523, 784, 698, 698, 698, 622, 587, 622, 698, 622, 698, 784, 622, 349, 349, 415, 523, 622, 587, 523, 392, 349, 349, 415, 523, 622, 698, 784, 831, 784, 698, 831, 933, 1047, 784, 622, 523, 587, 587, 587, 698, 587, 466, 392, 466, 523, 466, 622, 622, 622, 622, 622, 698, 784, 784, 784, 698, 622, 587, 587, 587, 587, 587, 622, 587, 622, 587, 523, 523, 523, 523, 523, 587, 622, 622, 622, 587, 523, 466, 466, 466, 466, 523, 587, 523, 466, 415, 415, 415, 415, 466, 523, 523, 523, 466, 415, 392, 392, 392, 622, 698, 622, 466, 622, 622, 622, 622, 698, 740, 740, 740, 831, 740, 698, 698, 698, 698, 740, 698, 784, 698, 622, 622, 622, 622, 622, 698, 784, 784, 784, 698, 622, 587, 587, 587, 587, 587, 622, 587, 622, 587, 523, 523, 523, 523, 523, 587, 622, 622, 622, 587, 523, 466, 466, 466, 466, 523, 587, 523, 466, 415, 415, 415, 415, 466, 523, 523, 523, 587, 698, 784, 784, 784, 784, 933, 784, 784, 698, 622, 587, 698, 784, 587, 698, 784, 587, 698, 587, 698, 784, 1047, 988, 523, 587, 622, 698, 0};
int noteDurations3[] = {308, 154, 154, 154, 154, 308, 154, 154, 154, 154, 308, 308, 616, 616, 308, 308, 616, 308, 308, 308, 308, 308, 308, 616, 616, 616, 308, 308, 308, 154, 154, 308, 308, 308, 308, 308, 308, 154, 77, 77, 77, 77, 77, 77, 616, 616, 308, 308, 616, 308, 308, 308, 308, 308, 308, 616, 616, 616, 308, 308, 308, 308, 308, 308, 308, 308, 1232, 308, 308, 308, 308, 308, 308, 308, 308, 308, 308, 308, 308, 308, 308, 616, 462, 154, 308, 154, 154, 308, 308, 616, 231, 77, 154, 154, 308, 154, 154, 308, 308, 308, 308, 308, 308, 308, 308, 308, 308, 308, 308, 308, 308, 308, 308, 308, 308, 616, 462, 154, 308, 154, 154, 308, 308, 308, 308, 231, 77, 154, 154, 154, 154, 154, 154, 924, 308, 154, 77, 77, 154, 77, 77, 154, 77, 77, 154, 154, 154, 77, 77, 154, 77, 77, 462, 77, 77, 154, 77, 77, 154, 77, 77, 154, 77, 77, 154, 154, 154, 77, 77, 154, 154, 462, 77, 77, 154, 77, 77, 154, 154, 154, 77, 77, 154, 154, 154, 77, 77, 154, 154, 308, 308, 154, 77, 77, 231, 77, 154, 77, 77, 154, 154, 154, 77, 77, 154, 154, 462, 77, 77, 154, 77, 77, 154, 77, 77, 154, 77, 77, 154, 154, 154, 77, 77, 154, 77, 77, 462, 77, 77, 154, 77, 77, 154, 77, 77, 154, 77, 77, 154, 154, 154, 77, 77, 154, 154, 462, 77, 77, 154, 77, 77, 154, 154, 154, 77, 77, 154, 154, 154, 77, 77, 154, 154, 154, 154, 154, 154, 154, 154, 154, 154, 154, 154, 154, 154, 154, 154, 154, 154, 308, 77, 77, 77, 77, 0};


//***************************************************************************************************************************************
// Inicialización
//***************************************************************************************************************************************

void setup() {
  pinMode(PF_4, INPUT);
  pinMode(PD_7, INPUT);
  pinMode(PD_6, INPUT);
  pinMode(PC_7, INPUT);
  pinMode(PA_6, INPUT);
  pinMode(PE_3, INPUT);
  pinMode(PA_7, INPUT);
  pinMode(PF_1, INPUT);
  SysCtlClockSet(SYSCTL_SYSDIV_2_5|SYSCTL_USE_PLL|SYSCTL_OSC_MAIN|SYSCTL_XTAL_16MHZ);
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo only
  }
  SPI.setModule(0);
  Serial.print("Initializing SD card..."); //Inicialización de la SD
  pinMode(10, OUTPUT);
  if (!SD.begin(4)) {
    Serial.println("initialization failed!");
    return;
  }
  Serial.println("initialization done.");
  
  GPIOPadConfigSet(GPIO_PORTB_BASE, 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7, GPIO_STRENGTH_8MA, GPIO_PIN_TYPE_STD_WPU); 
  LCD_Init();
  Game_Menu();
  if (started == 1){
    Serial.println("started");
  }

//LCD_Sprite(int x, int y, int width, int height, unsigned char bitmap[],int columns, int index, char flip, char offset);
//LCD_Bitmap(unsigned int x, unsigned int y, unsigned int width, unsigned int height, unsigned char bitmap[]);

  
}
//***************************************************************************************************************************************
// Loop Infinito
//***************************************************************************************************************************************
void loop() {

  while(started){
    //button = Serial.read();
    if (modeSelect == 1){
      text = "1 Player";
      LCD_Print(text, 8, 140, 2, 0x1007, 0xffff);
      text = "2 Player";
      LCD_Print(text, 185, 140, 2, 0xffff, 0x1007);
    }else if (modeSelect == 2){
      text = "1 Player";
      LCD_Print(text, 8, 140, 2, 0xffff, 0x1007);
      text = "2 Player";
      LCD_Print(text, 185, 140, 2, 0x1007, 0xffff);
    }
    if (rebote == 0){
      if (digitalRead(PD_6) == 1){
        modeSelect = 1;
        tone(PinBuzzer, 440, 100 * .7);
        rebote = 1;
      }else if (digitalRead(PD_7) == 1){
        tone(PinBuzzer, 440, 100 * .7);
        modeSelect = 2;
        rebote = 1;
      }else if (digitalRead(PF_4) == 1){
        tone(PinBuzzer, 500, 170 * .7);
        delay(170);    
        noTone(PinBuzzer);
        tone(PinBuzzer, 440, 170 * .7);
        delay(170);    
        noTone(PinBuzzer);
        tone(PinBuzzer, 500, 170 * .7);
        delay(170);    
        noTone(PinBuzzer);
        tone(PinBuzzer, 440, 170 * .7);
        delay(170);    
        noTone(PinBuzzer);
        Serial.println("");
        characterMenu();
        chooseCharacter = 1;
        player1 = 0;
        started = 0;
        characterSelect = 1;
        characterSelect2 = 1;
        rebote = 1;
      }
    }else if (rebote == 1){
      if (digitalRead(PD_6) == 0 and digitalRead(PD_7) == 0 and digitalRead(PC_7) == 0 and digitalRead(PF_4) == 0){
        rebote = 0;
      }
    }
  }
  while (chooseCharacter){
    if (escritor == 0){
      Serial.println("");
      Serial.print("Modo de juego seleccionado: ");
      Serial.print(modeSelect);
      Serial.println(" Player");
      escritor = 1;
    }
    if (modeSelect == 1){
      characterSelect2 = 0;
      //button = Serial.read();
      if (characterSelect == 1){
        text = "David";
        LCD_Print(text, 18, 135, 1, 0x00, 0xffff);
        text = "Alberto";
        LCD_Print(text, 88, 135, 1, 0xffff, 0x00);
        text = "Emilio";
        LCD_Print(text, 175, 135, 1, 0xffff, 0x00);
        text = "Aina";
        LCD_Print(text, 264, 135, 1, 0xffff, 0x00);
      }else if (characterSelect == 2){
        text = "David";
        LCD_Print(text, 18, 135, 1, 0xffff, 0x00);
        text = "Alberto";
        LCD_Print(text, 88, 135, 1, 0x00, 0xffff);
        text = "Emilio";
        LCD_Print(text, 175, 135, 1, 0xffff, 0x00);
        text = "Aina";
        LCD_Print(text, 264, 135, 1, 0xffff, 0x00);
      }else if (characterSelect == 3){
        text = "David";
        LCD_Print(text, 18, 135, 1, 0xffff, 0x00);
        text = "Alberto";
        LCD_Print(text, 88, 135, 1, 0xffff, 0x00);
        text = "Emilio";
        LCD_Print(text, 175, 135, 1, 0x00, 0xffff);
        text = "Aina";
        LCD_Print(text, 264, 135, 1, 0xffff, 0x00);
      }else if (characterSelect == 4){
        text = "David";
        LCD_Print(text, 18, 135, 1, 0xffff, 0x00);
        text = "Alberto";
        LCD_Print(text, 88, 135, 1, 0xffff, 0x00);
        text = "Emilio";
        LCD_Print(text, 175, 135, 1, 0xffff, 0x00);
        text = "Aina";
        LCD_Print(text, 264, 135, 1, 0x00, 0xffff);
      }
      if (rebote == 0){
        if (digitalRead(PC_7) == 1){
          rebote = 1;
          tone(PinBuzzer, 440, 170 * .7);
          delay(170);    
          noTone(PinBuzzer);
          tone(PinBuzzer, 500, 170 * .7);
          delay(170);    
          noTone(PinBuzzer);
          tone(PinBuzzer, 440, 170 * .7);
          delay(170);    
          noTone(PinBuzzer);
          tone(PinBuzzer, 500, 170 * .7);
          chooseCharacter = 0;
          escritor = 0;
          Game_Menu();
          Serial.println("Menu principal");
        }else if (digitalRead(PD_6) == 1){
          rebote = 1;
          tone(PinBuzzer, 440, 100 * .7);
          characterSelect --;
          if (characterSelect < 1){
            characterSelect = 4;
          }
        }else if (digitalRead(PD_7) == 1){
          rebote = 1;
          tone(PinBuzzer, 440, 100 * .7);
          characterSelect ++;
          if (characterSelect > 4){
            characterSelect = 1;
          }
        }else if (digitalRead(PF_4) == 1){
          rebote = 1;
          tone(PinBuzzer, 500, 170 * .7);
          delay(170);    
          noTone(PinBuzzer);
          tone(PinBuzzer, 440, 170 * .7);
          delay(170);    
          noTone(PinBuzzer);
          tone(PinBuzzer, 500, 170 * .7);
          delay(170);    
          noTone(PinBuzzer);
          tone(PinBuzzer, 440, 170 * .7);
          delay(170);    
          noTone(PinBuzzer);
          chooseCharacter = 0;
          chooseSong = 1;
          musicMenu();
          Serial.print("Personaje escogido: ");
          Serial.println(characterSelect);
        }
      }else if (rebote == 1){
        if (digitalRead(PD_6) == 0 and digitalRead(PD_7) == 0 and digitalRead(PC_7) == 0 and digitalRead(PF_4) == 0){
          rebote = 0;
        }
      }
    }else if (modeSelect == 2){
      //button = Serial.read();
      if(player1 == 0){
        text = "Player 1";
        LCD_Print(text, 80, 170, 2, 0xffff, 0x00);
        if (characterSelect == 1){
          text = "David";
          LCD_Print(text, 18, 135, 1, 0x00, 0xffff);
          text = "Alberto";
          LCD_Print(text, 88, 135, 1, 0xffff, 0x00);
          text = "Emilio";
          LCD_Print(text, 175, 135, 1, 0xffff, 0x00);
          text = "Aina";
          LCD_Print(text, 264, 135, 1, 0xffff, 0x00);
        }else if (characterSelect == 2){
          text = "David";
          LCD_Print(text, 18, 135, 1, 0xffff, 0x00);
          text = "Alberto";
          LCD_Print(text, 88, 135, 1, 0x00, 0xffff);
          text = "Emilio";
          LCD_Print(text, 175, 135, 1, 0xffff, 0x00);
          text = "Aina";
          LCD_Print(text, 264, 135, 1, 0xffff, 0x00);
        }else if (characterSelect == 3){
          text = "David";
          LCD_Print(text, 18, 135, 1, 0xffff, 0x00);
          text = "Alberto";
          LCD_Print(text, 88, 135, 1, 0xffff, 0x00);
          text = "Emilio";
          LCD_Print(text, 175, 135, 1, 0x00, 0xffff);
          text = "Aina";
          LCD_Print(text, 264, 135, 1, 0xffff, 0x00);
        }else if (characterSelect == 4){
          text = "David";
          LCD_Print(text, 18, 135, 1, 0xffff, 0x00);
          text = "Alberto";
          LCD_Print(text, 88, 135, 1, 0xffff, 0x00);
          text = "Emilio";
          LCD_Print(text, 175, 135, 1, 0xffff, 0x00);
          text = "Aina";
          LCD_Print(text, 264, 135, 1, 0x00, 0xffff);
        }
        if (rebote == 0){
          if (digitalRead(PC_7) == 1){
            rebote = 1;
            tone(PinBuzzer, 440, 170 * .7);
            delay(170);    
            noTone(PinBuzzer);
            tone(PinBuzzer, 500, 170 * .7);
            delay(170);    
            noTone(PinBuzzer);
            tone(PinBuzzer, 440, 170 * .7);
            delay(170);    
            noTone(PinBuzzer);
            tone(PinBuzzer, 500, 170 * .7);
            chooseCharacter = 0;
            escritor = 0;
            Game_Menu();
            Serial.println("Menu principal");
          }else if (digitalRead(PD_6) == 1){
            rebote = 1;
            tone(PinBuzzer, 440, 100 * .7);
            characterSelect --;
            if (characterSelect < 1){
              characterSelect = 4;
            }
          }else if (digitalRead(PD_7) == 1){
            rebote = 1;
            tone(PinBuzzer, 440, 100 * .7);
            characterSelect ++;
            if (characterSelect > 4){
              characterSelect = 1;
            }
          }else if (digitalRead(PF_4) == 1){
            rebote = 1;
            tone(PinBuzzer, 500, 170 * .7);
            delay(170);    
            noTone(PinBuzzer);
            tone(PinBuzzer, 440, 170 * .7);
            delay(170);    
            noTone(PinBuzzer);
            tone(PinBuzzer, 500, 170 * .7);
            delay(170);    
            noTone(PinBuzzer);
            tone(PinBuzzer, 440, 170 * .7);
            delay(170);    
            noTone(PinBuzzer);
  //          chooseCharacter = 0;
  //          chooseSong = 1;
  //          musicMenu();
            player1 = 1;
            Serial.print("1Personaje escogido: ");
            Serial.println(characterSelect);
          }
        }else if (rebote == 1){
          if (digitalRead(PD_6) == 0 and digitalRead(PD_7) == 0 and digitalRead(PC_7) == 0 and digitalRead(PF_4) == 0){
            rebote = 0;
          }
        }
      }else if (player1 == 1){
        text = "Player 2";
        LCD_Print(text, 80, 170, 2, 0xffff, 0x00);
        if (characterSelect2 == 1){
          text = "David";
          LCD_Print(text, 18, 135, 1, 0x00, 0xffff);
          text = "Alberto";
          LCD_Print(text, 88, 135, 1, 0xffff, 0x00);
          text = "Emilio";
          LCD_Print(text, 175, 135, 1, 0xffff, 0x00);
          text = "Aina";
          LCD_Print(text, 264, 135, 1, 0xffff, 0x00);
        }else if (characterSelect2 == 2){
          text = "David";
          LCD_Print(text, 18, 135, 1, 0xffff, 0x00);
          text = "Alberto";
          LCD_Print(text, 88, 135, 1, 0x00, 0xffff);
          text = "Emilio";
          LCD_Print(text, 175, 135, 1, 0xffff, 0x00);
          text = "Aina";
          LCD_Print(text, 264, 135, 1, 0xffff, 0x00);
        }else if (characterSelect2 == 3){
          text = "David";
          LCD_Print(text, 18, 135, 1, 0xffff, 0x00);
          text = "Alberto";
          LCD_Print(text, 88, 135, 1, 0xffff, 0x00);
          text = "Emilio";
          LCD_Print(text, 175, 135, 1, 0x00, 0xffff);
          text = "Aina";
          LCD_Print(text, 264, 135, 1, 0xffff, 0x00);
        }else if (characterSelect2 == 4){
          text = "David";
          LCD_Print(text, 18, 135, 1, 0xffff, 0x00);
          text = "Alberto";
          LCD_Print(text, 88, 135, 1, 0xffff, 0x00);
          text = "Emilio";
          LCD_Print(text, 175, 135, 1, 0xffff, 0x00);
          text = "Aina";
          LCD_Print(text, 264, 135, 1, 0x00, 0xffff);
        }
        if (rebote == 0){
          if (PC_7 == 1){
            rebote = 1;
            tone(PinBuzzer, 440, 170 * .7);
            delay(170);    
            noTone(PinBuzzer);
            tone(PinBuzzer, 500, 170 * .7);
            delay(170);    
            noTone(PinBuzzer);
            tone(PinBuzzer, 440, 170 * .7);
            delay(170);    
            noTone(PinBuzzer);
            tone(PinBuzzer, 500, 170 * .7);
            chooseCharacter = 0;
            escritor = 0;
            Game_Menu();
            Serial.println("Menu principal");
          }else if (PE_3 == 1){
            rebote = 1;
            tone(PinBuzzer, 440, 100 * .7);
            characterSelect2 --;
            if (characterSelect2 < 1){
              characterSelect2 = 4;
            }
          }else if (PF_1 == 1){
            rebote = 1;
            tone(PinBuzzer, 440, 100 * .7);
            characterSelect2 ++;
            if (characterSelect2 > 4){
              characterSelect2 = 1;
            }
          }else if (PA_7 == 1){
            rebote = 1;
            tone(PinBuzzer, 500, 170 * .7);
            delay(170);    
            noTone(PinBuzzer);
            tone(PinBuzzer, 440, 170 * .7);
            delay(170);    
            noTone(PinBuzzer);
            tone(PinBuzzer, 500, 170 * .7);
            delay(170);    
            noTone(PinBuzzer);
            tone(PinBuzzer, 440, 170 * .7);
            delay(170);    
            noTone(PinBuzzer);
            Serial.print("2Personaje escogido: ");
            Serial.println(characterSelect2);
            chooseCharacter = 0;
            chooseSong = 1;
            musicMenu();
          }
        }else if (rebote == 1){
          if (digitalRead(PD_6) == 0 and digitalRead(PD_7) == 0 and digitalRead(PC_7) == 0 and digitalRead(PF_4) == 0){
            rebote = 0;
          }
        }
      }
    }
  }
  while (chooseSong == 1){
    //button = Serial.read();
    if (songSelect == 1){
      text = "The Legend of Zelda - Zelda's Lullaby";
      LCD_Print(text, 8, 105, 1, 0xffff, 0x00);
    }else if (songSelect == 2){
      text = "Led Zepellin - StairWay To Heaven";
      LCD_Print(text, 30, 105, 1, 0xffff, 0x00);
    }else if (songSelect == 3){
      text = "Kirby - Gourmet Race";
      LCD_Print(text, 80, 105, 1, 0xffff, 0x00);
    }
    if (rebote == 0){
      if (digitalRead(PD_7) == 1){
        rebote = 1;
        tone(PinBuzzer, 440, 100 * .7);
        songSelect ++;
        FillRect(0, 104, 320, 16, 0x00);
        if(songSelect > 3){
          songSelect = 1;
        }
      }else if (digitalRead(PD_6) == 1){
        rebote = 1;
        tone(PinBuzzer, 440, 100 * .7);
        songSelect --;
        FillRect(0, 104, 320, 16, 0x00);
        if(songSelect < 1){
          songSelect = 3;
        }
      }else if (digitalRead(PC_7) == 1){
          rebote = 1;
          tone(PinBuzzer, 440, 170 * .7);
          delay(170);    
          noTone(PinBuzzer);
          tone(PinBuzzer, 500, 170 * .7);
          delay(170);    
          noTone(PinBuzzer);
          tone(PinBuzzer, 440, 170 * .7);
          delay(170);    
          noTone(PinBuzzer);
          tone(PinBuzzer, 500, 170 * .7);
          chooseCharacter = 1;
          escritor = 0;
          chooseSong = 0;
          characterMenu();
          Serial.println("Menu Personaje");
      }else if (digitalRead(PF_4) == 1){
          rebote = 1;
          tone(PinBuzzer, 500, 170 * .7);
          delay(170);    
          noTone(PinBuzzer);
          tone(PinBuzzer, 440, 170 * .7);
          delay(170);    
          noTone(PinBuzzer);
          tone(PinBuzzer, 500, 170 * .7);
          delay(170);    
          noTone(PinBuzzer);
          tone(PinBuzzer, 440, 170 * .7);
          delay(170);    
          noTone(PinBuzzer);
          chooseSong = 0;
          game = 1;
          thisNote = 0;
          gamePlay(songSelect, characterSelect, characterSelect2);
          Serial.print("Cancion: ");
          Serial.println(songSelect);
        }
    }else if (rebote == 1){
      if (digitalRead(PD_6) == 0 and digitalRead(PD_7) == 0 and digitalRead(PC_7) == 0 and digitalRead(PF_4) == 0){
        rebote = 0;
      }
    }
  }
  while (game == 1){
    if(songSelect == 1){
      GamePlaySong1();
    }
    if(songSelect == 2){
      GamePlaySong2();
    }
    if(songSelect == 3){
      GamePlaySong3();
    }

  }
}

void GamePlaySong1(void){
  int P1SC = 0;
  int P2SC = 0;
    int thisNote = 0;
    int x = 84;
    while( x <202){
        delay(10);
        LCD_Bitmap(112, x, 16, 16, arrow_up);
        LCD_Bitmap(138, x, 16, 16, arrow_left);
        LCD_Bitmap(167, x, 16, 16, arrow_right);
        LCD_Bitmap(193, x, 16, 16, arrow_down);
        x++;
    }
   x = 85;
   int s = 1600/noteDurations1[0];
   int frand = 1;
    while(thisNote <= sizeof(melody1)){
      if(thisNote < sizeof(melody1)-1){
          if(x<=202){
            delay(10);
            switch (frand) {
              case 1:
                LCD_Bitmap(112, x, 16, 16, arrow_up);// statements
                LCD_Bitmap(112, x-16, 16, 16, bArrow);
                break;
              case 2:
                LCD_Bitmap(138, x, 16, 16, arrow_left);// statements
                LCD_Bitmap(138, x-16, 16, 16, bArrow);
                break;
              case 3:
                LCD_Bitmap(167, x, 16, 16, arrow_right);// statements
                LCD_Bitmap(167, x-16, 16, 16, bArrow);
                break;
              case 4:
                LCD_Bitmap(193, x, 16, 16, arrow_down);// statements
                LCD_Bitmap(193, x-16, 16, 16, bArrow);
                break;
              default:
                LCD_Bitmap(112, x, 16, 16, arrow_up);// statements
                LCD_Bitmap(112, x-16, 16, 16, bArrow);
                break;
            }
            
            x = x + s;
            }
      }
      unsigned long currentMillis = millis();
        if(thisNote != (sizeof(melody1)/sizeof(int))){
          if ((currentMillis - previousMillis) < noteDurations1[thisNote]){
            if (melody1[thisNote] == 0) {
                noTone(PinBuzzer);
              } else {
                tone(PinBuzzer, melody1[thisNote]);
              }
          }else {
            previousMillis = currentMillis;
            noTone(PinBuzzer);
            thisNote++;
            switch(frand){
              case 1:
                if(digitalRead(PC_6) == HIGH){
                  P1SC = P1SC + 20;
                } else{
                  P1SC--;
                }
                if(digitalRead(PA_6) == HIGH){
                  P2SC = P2SC + 20;
                } else{
                  P2SC--;
                }
                break;
              case 2:
                if(digitalRead(PD_6) == HIGH){
                  P1SC = P1SC + 20;
                } else{
                  P1SC--;
                }
                if(digitalRead(PE_3) == HIGH){
                  P2SC = P2SC + 20;
                } else{
                  P2SC--;
                }
                break;
              case 3:
                if(digitalRead(PD_7) == HIGH){
                  P1SC = P1SC + 20;
                } else{
                  P1SC--;
                }
                if(digitalRead(PA_7) == HIGH){
                  P2SC = P2SC + 20;
                } else{
                  P2SC--;
                }
                break;
              case 4:
                if(digitalRead(PF_4) == HIGH){
                  P1SC = P1SC + 20;
                } else{
                  P1SC--;
                }
                if(digitalRead(PF_1) == HIGH){
                  P2SC = P2SC + 20;
                } else{
                  P2SC--;
                }
                break;
            }
            if(P1SC < 0){
              P1SC = 0;
            }
            Serial.println(P1SC);
            x = 85;
            LCD_Bitmap(112, 186, 16, 16, bArrow);
            LCD_Bitmap(138, 186, 16, 16, bArrow);
            LCD_Bitmap(167, 186, 16, 16, bArrow);
            LCD_Bitmap(193, 186, 16, 16, bArrow);
            LCD_Bitmap(112, 170, 16, 16, bArrow);
            LCD_Bitmap(138, 170, 16, 16, bArrow);
            LCD_Bitmap(167, 170, 16, 16, bArrow);
            LCD_Bitmap(193, 170, 16, 16, bArrow);
            LCD_Bitmap(112, 202, 16, 16, arrow_up);
            LCD_Bitmap(138, 202, 16, 16, arrow_left);
            LCD_Bitmap(167, 202, 16, 16, arrow_right);
            LCD_Bitmap(193, 202, 16, 16, arrow_down);
            s = 1600/noteDurations1[thisNote];
            if (s <= 2){
              s++;
            }
            Serial.println(s);
            frand = random(1,5);
          }
        }else{
          x = 84;
          while( x <=202){
              delay(10);
              LCD_Bitmap(112, x, 16, 16, arrow_up);
              LCD_Bitmap(138, x, 16, 16, arrow_left);
              LCD_Bitmap(167, x, 16, 16, arrow_right);
              LCD_Bitmap(193, x, 16, 16, arrow_down);
              x++;
          }
          break;
        }
    }
    LCD_Print("DONE!", 120, 100, 2, 0xffff, 0x00);
    LCD_Print("P1", 40, 50, 2, 0xffff, 0x00);
    LCD_Print("SCORE:", 5, 100, 2, 0xffff, 0x00);
    LCD_Print(String(P1SC), 30, 130, 2, 0xffff, 0x00);
    if (modeSelect == 2){
      LCD_Print("P2", 250, 50, 2, 0xffff, 0x00);
      LCD_Print("SCORE:", 215, 100, 2, 0xffff, 0x00);
      LCD_Print(String(P2SC), 240, 130, 2, 0xffff, 0x00);
    }
    tone(PinBuzzer, 0);
    noTone(PinBuzzer);
    delay(2000);
    game = 0;
}

void GamePlaySong2(void){
  int P2SC = 0;
  int P1SC = 0;
    int thisNote = 0;
    int x = 84;
    while( x <202){
        delay(10);
        LCD_Bitmap(112, x, 16, 16, arrow_up);
        LCD_Bitmap(138, x, 16, 16, arrow_left);
        LCD_Bitmap(167, x, 16, 16, arrow_right);
        LCD_Bitmap(193, x, 16, 16, arrow_down);
        x++;
    }
   x = 90;
   int s = 1600/noteDurations2[0];
   int frand = 1;
    while(thisNote <= sizeof(melody2)){
      if(thisNote < sizeof(melody2)-1){
          if(x<=202){
            delay(10);
            switch (frand) {
              case 1:
                LCD_Bitmap(112, x, 16, 16, arrow_up);// statements
                LCD_Bitmap(112, x-16, 16, 16, bArrow);
                break;
              case 2:
                LCD_Bitmap(138, x, 16, 16, arrow_left);// statements
                LCD_Bitmap(138, x-16, 16, 16, bArrow);
                break;
              case 3:
                LCD_Bitmap(167, x, 16, 16, arrow_right);// statements
                LCD_Bitmap(167, x-16, 16, 16, bArrow);
                break;
              case 4:
                LCD_Bitmap(193, x, 16, 16, arrow_down);// statements
                LCD_Bitmap(193, x-16, 16, 16, bArrow);
                break;
              default:
                LCD_Bitmap(112, x, 16, 16, arrow_up);// statements
                LCD_Bitmap(112, x-16, 16, 16, bArrow);
                break;
            }
            
            x = x + s;
            }
      }
      unsigned long currentMillis = millis();
        if(thisNote != (sizeof(melody2)/sizeof(int))){
          if ((currentMillis - previousMillis) < noteDurations2[thisNote]){
            if (melody1[thisNote] == 0) {
                noTone(PinBuzzer);
              } else {
                tone(PinBuzzer, melody2[thisNote]);
              }
          }else {
            previousMillis = currentMillis;
            noTone(PinBuzzer);
            thisNote++;
            switch(frand){
              case 1:
                if(digitalRead(PC_6) == HIGH){
                  P1SC = P1SC + 20;
                } else{
                  P1SC--;
                }
                if(digitalRead(PA_6) == HIGH){
                  P2SC = P2SC + 20;
                } else{
                  P2SC--;
                }
                break;
              case 2:
                if(digitalRead(PD_6) == HIGH){
                  P1SC = P1SC + 20;
                } else{
                  P1SC--;
                }
                if(digitalRead(PE_3) == HIGH){
                  P2SC = P2SC + 20;
                } else{
                  P2SC--;
                }
                break;
              case 3:
                if(digitalRead(PD_7) == HIGH){
                  P1SC = P1SC + 20;
                } else{
                  P1SC--;
                }
                if(digitalRead(PA_7) == HIGH){
                  P2SC = P2SC + 20;
                } else{
                  P2SC--;
                }
                break;
              case 4:
                if(digitalRead(PF_4) == HIGH){
                  P1SC = P1SC + 20;
                } else{
                  P1SC--;
                }
                if(digitalRead(PF_1) == HIGH){
                  P2SC = P2SC + 20;
                } else{
                  P2SC--;
                }
                break;
            }
            if(P1SC < 0){
              P1SC = 0;
            }
            Serial.println(P1SC);
            x = 90;
            LCD_Bitmap(112, 186, 16, 16, bArrow);
            LCD_Bitmap(138, 186, 16, 16, bArrow);
            LCD_Bitmap(167, 186, 16, 16, bArrow);
            LCD_Bitmap(193, 186, 16, 16, bArrow);
            LCD_Bitmap(112, 202, 16, 16, arrow_up);
            LCD_Bitmap(138, 202, 16, 16, arrow_left);
            LCD_Bitmap(167, 202, 16, 16, arrow_right);
            LCD_Bitmap(193, 202, 16, 16, arrow_down);
            s = 1600/noteDurations2[thisNote];
            s++;
            frand = random(1,5);
          }
        }else{
          x = 84;
          while( x <=202){
              delay(10);
              LCD_Bitmap(112, x, 16, 16, arrow_up);
              LCD_Bitmap(138, x, 16, 16, arrow_left);
              LCD_Bitmap(167, x, 16, 16, arrow_right);
              LCD_Bitmap(193, x, 16, 16, arrow_down);
              x++;
          }
          break;
        }
    }
    LCD_Print("DONE!", 120, 100, 2, 0xffff, 0x00);
    LCD_Print("P1", 40, 50, 2, 0xffff, 0x00);
    LCD_Print("SCORE:", 5, 100, 2, 0xffff, 0x00);
    LCD_Print(String(P1SC), 30, 130, 2, 0xffff, 0x00);
    if (modeSelect == 2){
      LCD_Print("P2", 250, 50, 2, 0xffff, 0x00);
      LCD_Print("SCORE:", 215, 100, 2, 0xffff, 0x00);
      LCD_Print(String(P2SC), 240, 130, 2, 0xffff, 0x00);
    }
    tone(PinBuzzer, 0);
    noTone(PinBuzzer);
    delay(2000);
    game = 0;
}

void GamePlaySong3(void){
  int P2SC = 0;
  int P1SC = 0;
    int thisNote = 0;
    int x = 84;
    while( x <202){
        delay(10);
        LCD_Bitmap(112, x, 16, 16, arrow_up);
        LCD_Bitmap(138, x, 16, 16, arrow_left);
        LCD_Bitmap(167, x, 16, 16, arrow_right);
        LCD_Bitmap(193, x, 16, 16, arrow_down);
        x++;
    }
   x = 90;
   int s = 1600/noteDurations3[0];
   int frand = 1;
    while(thisNote <= sizeof(melody3)){
      if(thisNote < sizeof(melody3)-1){
          if(x<=205){
            delay(10);
            switch (frand) {
              case 1:
                LCD_Bitmap(112, x, 16, 16, arrow_up);// statements
                LCD_Bitmap(112, x-16, 16, 16, bArrow);
                break;
              case 2:
                LCD_Bitmap(138, x, 16, 16, arrow_left);// statements
                LCD_Bitmap(138, x-16, 16, 16, bArrow);
                break;
              case 3:
                LCD_Bitmap(167, x, 16, 16, arrow_right);// statements
                LCD_Bitmap(167, x-16, 16, 16, bArrow);
                break;
              case 4:
                LCD_Bitmap(193, x, 16, 16, arrow_down);// statements
                LCD_Bitmap(193, x-16, 16, 16, bArrow);
                break;
              default:
                LCD_Bitmap(112, x, 16, 16, arrow_up);// statements
                LCD_Bitmap(112, x-16, 16, 16, bArrow);
                break;
            }
            
            x = x + s;
            }
      }
      unsigned long currentMillis = millis();
        if(thisNote != (sizeof(melody3)/sizeof(int))){
          if ((currentMillis - previousMillis) < noteDurations3[thisNote]){
            if (melody3[thisNote] == 0) {
                noTone(PinBuzzer);
              } else {
                tone(PinBuzzer, melody3[thisNote]);
              }
          }else {
            previousMillis = currentMillis;
            noTone(PinBuzzer);
            thisNote++;
            switch(frand){
              case 1:
                if(digitalRead(PC_6) == HIGH){
                  P1SC = P1SC + 20;
                } else{
                  P1SC--;
                }
                if(digitalRead(PA_6) == HIGH){
                  P2SC = P2SC + 20;
                } else{
                  P2SC--;
                }
                break;
              case 2:
                if(digitalRead(PD_6) == HIGH){
                  P1SC = P1SC + 20;
                } else{
                  P1SC--;
                }
                if(digitalRead(PE_3) == HIGH){
                  P2SC = P2SC + 20;
                } else{
                  P2SC--;
                }
                break;
              case 3:
                if(digitalRead(PD_7) == HIGH){
                  P1SC = P1SC + 20;
                } else{
                  P1SC--;
                }
                if(digitalRead(PA_7) == HIGH){
                  P2SC = P2SC + 20;
                } else{
                  P2SC--;
                }
                break;
              case 4:
                if(digitalRead(PF_4) == HIGH){
                  P1SC = P1SC + 20;
                } else{
                  P1SC--;
                }
                if(digitalRead(PF_1) == HIGH){
                  P2SC = P2SC + 20;
                } else{
                  P2SC--;
                }
                break;
            }
            if(P1SC < 0){
              P1SC = 0;
            }
            x = 90;
            LCD_Bitmap(112, 186, 16, 16, bArrow);
            LCD_Bitmap(138, 186, 16, 16, bArrow);
            LCD_Bitmap(167, 186, 16, 16, bArrow);
            LCD_Bitmap(193, 186, 16, 16, bArrow);
            LCD_Bitmap(112, 202, 16, 16, arrow_up);
            LCD_Bitmap(138, 202, 16, 16, arrow_left);
            LCD_Bitmap(167, 202, 16, 16, arrow_right);
            LCD_Bitmap(193, 202, 16, 16, arrow_down);
            s = 1600/noteDurations3[thisNote];
            frand = random(1,5);
          }
        }else{
          x = 84;
          while( x <=202){
              delay(10);
              LCD_Bitmap(112, x, 16, 16, arrow_up);
              LCD_Bitmap(138, x, 16, 16, arrow_left);
              LCD_Bitmap(167, x, 16, 16, arrow_right);
              LCD_Bitmap(193, x, 16, 16, arrow_down);
              x++;
          }
          break;
        }
    }
    LCD_Print("DONE!", 120, 100, 2, 0xffff, 0x00);
    LCD_Print("P1", 40, 50, 2, 0xffff, 0x00);
    LCD_Print("SCORE:", 5, 100, 2, 0xffff, 0x00);
    LCD_Print(String(P1SC), 30, 130, 2, 0xffff, 0x00);
    if (modeSelect == 2){
      LCD_Print("P2", 250, 50, 2, 0xffff, 0x00);
      LCD_Print("SCORE:", 215, 100, 2, 0xffff, 0x00);
      LCD_Print(String(P2SC), 240, 130, 2, 0xffff, 0x00);
    }
    tone(PinBuzzer, 0);
    noTone(PinBuzzer);
    delay(2000);
    game = 0;
}

//***************************************************************************************************************************************
// Función para inicializar LCD
//***************************************************************************************************************************************
void LCD_Init(void) {
  pinMode(LCD_RST, OUTPUT);
  pinMode(LCD_CS, OUTPUT);
  pinMode(LCD_RS, OUTPUT);
  pinMode(LCD_WR, OUTPUT);
  pinMode(LCD_RD, OUTPUT);
  for (uint8_t i = 0; i < 8; i++){
    pinMode(DPINS[i], OUTPUT);
  }
  //****************************************
  // Secuencia de Inicialización
  //****************************************
  digitalWrite(LCD_CS, HIGH);
  digitalWrite(LCD_RS, HIGH);
  digitalWrite(LCD_WR, HIGH);
  digitalWrite(LCD_RD, HIGH);
  digitalWrite(LCD_RST, HIGH);
  delay(5);
  digitalWrite(LCD_RST, LOW);
  delay(20);
  digitalWrite(LCD_RST, HIGH);
  delay(150);
  digitalWrite(LCD_CS, LOW);
  //****************************************
  LCD_CMD(0xE9);  // SETPANELRELATED
  LCD_DATA(0x20);
  //****************************************
  LCD_CMD(0x11); // Exit Sleep SLEEP OUT (SLPOUT)
  delay(100);
  //****************************************
  LCD_CMD(0xD1);    // (SETVCOM)
  LCD_DATA(0x00);
  LCD_DATA(0x71);
  LCD_DATA(0x19);
  //****************************************
  LCD_CMD(0xD0);   // (SETPOWER) 
  LCD_DATA(0x07);
  LCD_DATA(0x01);
  LCD_DATA(0x08);
  //****************************************
  LCD_CMD(0x36);  // (MEMORYACCESS)
  LCD_DATA(0x40|0x80|0x20|0x08); // LCD_DATA(0x19);
  //****************************************
  LCD_CMD(0x3A); // Set_pixel_format (PIXELFORMAT)
  LCD_DATA(0x05); // color setings, 05h - 16bit pixel, 11h - 3bit pixel
  //****************************************
  LCD_CMD(0xC1);    // (POWERCONTROL2)
  LCD_DATA(0x10);
  LCD_DATA(0x10);
  LCD_DATA(0x02);
  LCD_DATA(0x02);
  //****************************************
  LCD_CMD(0xC0); // Set Default Gamma (POWERCONTROL1)
  LCD_DATA(0x00);
  LCD_DATA(0x35);
  LCD_DATA(0x00);
  LCD_DATA(0x00);
  LCD_DATA(0x01);
  LCD_DATA(0x02);
  //****************************************
  LCD_CMD(0xC5); // Set Frame Rate (VCOMCONTROL1)
  LCD_DATA(0x04); // 72Hz
  //****************************************
  LCD_CMD(0xD2); // Power Settings  (SETPWRNORMAL)
  LCD_DATA(0x01);
  LCD_DATA(0x44);
  //****************************************
  LCD_CMD(0xC8); //Set Gamma  (GAMMASET)
  LCD_DATA(0x04);
  LCD_DATA(0x67);
  LCD_DATA(0x35);
  LCD_DATA(0x04);
  LCD_DATA(0x08);
  LCD_DATA(0x06);
  LCD_DATA(0x24);
  LCD_DATA(0x01);
  LCD_DATA(0x37);
  LCD_DATA(0x40);
  LCD_DATA(0x03);
  LCD_DATA(0x10);
  LCD_DATA(0x08);
  LCD_DATA(0x80);
  LCD_DATA(0x00);
  //****************************************
  LCD_CMD(0x2A); // Set_column_address 320px (CASET)
  LCD_DATA(0x00);
  LCD_DATA(0x00);
  LCD_DATA(0x01);
  LCD_DATA(0x3F);
  //****************************************
  LCD_CMD(0x2B); // Set_page_address 480px (PASET)
  LCD_DATA(0x00);
  LCD_DATA(0x00);
  LCD_DATA(0x01);
  LCD_DATA(0xE0);
//  LCD_DATA(0x8F);
  LCD_CMD(0x29); //display on 
  LCD_CMD(0x2C); //display on

  LCD_CMD(ILI9341_INVOFF); //Invert Off
  delay(120);
  LCD_CMD(ILI9341_SLPOUT);    //Exit Sleep
  delay(120);
  LCD_CMD(ILI9341_DISPON);    //Display on
  digitalWrite(LCD_CS, HIGH);
}
//***************************************************************************************************************************************
// Función para enviar comandos a la LCD - parámetro (comando)
//***************************************************************************************************************************************
void LCD_CMD(uint8_t cmd) {
  digitalWrite(LCD_RS, LOW);
  digitalWrite(LCD_WR, LOW);
  GPIO_PORTB_DATA_R = cmd;
  digitalWrite(LCD_WR, HIGH);
}
//***************************************************************************************************************************************
// Función para enviar datos a la LCD - parámetro (dato)
//***************************************************************************************************************************************
void LCD_DATA(uint8_t data) {
  digitalWrite(LCD_RS, HIGH);
  digitalWrite(LCD_WR, LOW);
  GPIO_PORTB_DATA_R = data;
  digitalWrite(LCD_WR, HIGH);
}
//***************************************************************************************************************************************
// Función para definir rango de direcciones de memoria con las cuales se trabajara (se define una ventana)
//***************************************************************************************************************************************
void SetWindows(unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2) {
  LCD_CMD(0x2a); // Set_column_address 4 parameters
  LCD_DATA(x1 >> 8);
  LCD_DATA(x1);   
  LCD_DATA(x2 >> 8);
  LCD_DATA(x2);   
  LCD_CMD(0x2b); // Set_page_address 4 parameters
  LCD_DATA(y1 >> 8);
  LCD_DATA(y1);   
  LCD_DATA(y2 >> 8);
  LCD_DATA(y2);   
  LCD_CMD(0x2c); // Write_memory_start
}
//***************************************************************************************************************************************
// Función para borrar la pantalla - parámetros (color)
//***************************************************************************************************************************************
void LCD_Clear(unsigned int c){  
  unsigned int x, y;
  LCD_CMD(0x02c); // write_memory_start
  digitalWrite(LCD_RS, HIGH);
  digitalWrite(LCD_CS, LOW);   
  SetWindows(0, 0, 319, 239); // 479, 319);
  for (x = 0; x < 320; x++)
    for (y = 0; y < 240; y++) {
      LCD_DATA(c >> 8); 
      LCD_DATA(c); 
    }
  digitalWrite(LCD_CS, HIGH);
} 
//***************************************************************************************************************************************
// Función para dibujar una línea horizontal - parámetros ( coordenada x, cordenada y, longitud, color)
//*************************************************************************************************************************************** 
void H_line(unsigned int x, unsigned int y, unsigned int l, unsigned int c) {  
  unsigned int i, j;
  LCD_CMD(0x02c); //write_memory_start
  digitalWrite(LCD_RS, HIGH);
  digitalWrite(LCD_CS, LOW);
  l = l + x;
  SetWindows(x, y, l, y);
  j = l;// * 2;
  for (i = 0; i < l; i++) {
      LCD_DATA(c >> 8); 
      LCD_DATA(c); 
  }
  digitalWrite(LCD_CS, HIGH);
}
//***************************************************************************************************************************************
// Función para dibujar una línea vertical - parámetros ( coordenada x, cordenada y, longitud, color)
//*************************************************************************************************************************************** 
void V_line(unsigned int x, unsigned int y, unsigned int l, unsigned int c) {  
  unsigned int i,j;
  LCD_CMD(0x02c); //write_memory_start
  digitalWrite(LCD_RS, HIGH);
  digitalWrite(LCD_CS, LOW);
  l = l + y;
  SetWindows(x, y, x, l);
  j = l; //* 2;
  for (i = 1; i <= j; i++) {
    LCD_DATA(c >> 8); 
    LCD_DATA(c);
  }
  digitalWrite(LCD_CS, HIGH);  
}
//***************************************************************************************************************************************
// Función para dibujar un rectángulo - parámetros ( coordenada x, cordenada y, ancho, alto, color)
//***************************************************************************************************************************************
void Rect(unsigned int x, unsigned int y, unsigned int w, unsigned int h, unsigned int c) {
  H_line(x  , y  , w, c);
  H_line(x  , y+h, w, c);
  V_line(x  , y  , h, c);
  V_line(x+w, y  , h, c);
}
//***************************************************************************************************************************************
// Función para dibujar un rectángulo relleno - parámetros ( coordenada x, cordenada y, ancho, alto, color)
//***************************************************************************************************************************************
void FillRect(unsigned int x, unsigned int y, unsigned int w, unsigned int h, unsigned int c) {
  unsigned int i;
  for (i = 0; i < h; i++) {
    H_line(x  , y  , w, c);
    H_line(x  , y+i, w, c);
  }
}
//***************************************************************************************************************************************
// Función para dibujar texto - parámetros ( texto, coordenada x, cordenada y, color, background) 
//***************************************************************************************************************************************
void LCD_Print(String text, int x, int y, int fontSize, int color, int background) {
  int fontXSize ;
  int fontYSize ;
  
  if(fontSize == 1){
    fontXSize = fontXSizeSmal ;
    fontYSize = fontYSizeSmal ;
  }
  if(fontSize == 2){
    fontXSize = fontXSizeBig ;
    fontYSize = fontYSizeBig ;
  }
  
  char charInput ;
  int cLength = text.length();
  //Serial.println(cLength,DEC);
  int charDec ;
  int c ;
  int charHex ;
  char char_array[cLength+1];
  text.toCharArray(char_array, cLength+1) ;
  for (int i = 0; i < cLength ; i++) {
    charInput = char_array[i];
    //Serial.println(char_array[i]);
    charDec = int(charInput);
    digitalWrite(LCD_CS, LOW);
    SetWindows(x + (i * fontXSize), y, x + (i * fontXSize) + fontXSize - 1, y + fontYSize );
    long charHex1 ;
    for ( int n = 0 ; n < fontYSize ; n++ ) {
      if (fontSize == 1){
        charHex1 = pgm_read_word_near(smallFont + ((charDec - 32) * fontYSize) + n);
      }
      if (fontSize == 2){
        charHex1 = pgm_read_word_near(bigFont + ((charDec - 32) * fontYSize) + n);
      }
      for (int t = 1; t < fontXSize + 1 ; t++) {
        if (( charHex1 & (1 << (fontXSize - t))) > 0 ) {
          c = color ;
        } else {
          c = background ;
        }
        LCD_DATA(c >> 8);
        LCD_DATA(c);
      }
    }
    digitalWrite(LCD_CS, HIGH);
  }
}
//***************************************************************************************************************************************
// Función para dibujar una imagen a partir de un arreglo de colores (Bitmap) Formato (Color 16bit R 5bits G 6bits B 5bits)
//***************************************************************************************************************************************
void LCD_Bitmap(unsigned int x, unsigned int y, unsigned int width, unsigned int height, unsigned char bitmap[]){  
  LCD_CMD(0x02c); // write_memory_start
  digitalWrite(LCD_RS, HIGH);
  digitalWrite(LCD_CS, LOW); 
  
  unsigned int x2, y2;
  x2 = x+width;
  y2 = y+height;
  SetWindows(x, y, x2-1, y2-1);
  unsigned int k = 0;
  unsigned int i, j;

  for (int i = 0; i < width; i++) {
    for (int j = 0; j < height; j++) {
      LCD_DATA(bitmap[k]);
      LCD_DATA(bitmap[k+1]);
      //LCD_DATA(bitmap[k]);    
      k = k + 2;
     } 
  }
  digitalWrite(LCD_CS, HIGH);
}
//***************************************************************************************************************************************
// Función para dibujar una imagen sprite - los parámetros columns = número de imagenes en el sprite, index = cual desplegar, flip = darle vuelta
//***************************************************************************************************************************************
void LCD_Sprite(int x, int y, int width, int height, unsigned char bitmap[],int columns, int index, char flip, char offset){
  LCD_CMD(0x02c); // write_memory_start
  digitalWrite(LCD_RS, HIGH);
  digitalWrite(LCD_CS, LOW); 

  unsigned int x2, y2;
  x2 =   x+width;
  y2=    y+height;
  SetWindows(x, y, x2-1, y2-1);
  int k = 0;
  int ancho = ((width*columns));
  if(flip){
  for (int j = 0; j < height; j++){
      k = (j*(ancho) + index*width -1 - offset)*2;
      k = k+width*2;
     for (int i = 0; i < width; i++){
      LCD_DATA(bitmap[k]);
      LCD_DATA(bitmap[k+1]);
      k = k - 2;
     } 
  }
  }else{
     for (int j = 0; j < height; j++){
      k = (j*(ancho) + index*width + 1 + offset)*2;
     for (int i = 0; i < width; i++){
      LCD_DATA(bitmap[k]);
      LCD_DATA(bitmap[k+1]);
      k = k + 2;
     } 
  }
    
    
    }
  digitalWrite(LCD_CS, HIGH);
}

void Game_Menu(void){
  File myFile;
  String palabra;
  char caracter;
  char numero [5];
  int bits = 0, posx = 0, posy = 0, val = 0, color = 0;

//  int melody[] = { 
//  698, 587, 440, 587, 698, 587, 440, 587, 698, 523, 440, 523, 698, 523, 440, 523, 659, 554, 440, 554, 659, 554, 440, 554, 659, 554, 440, 554, 659, 554, 440, 554, 587 
//  };
//  int noteDurations[] = { 
//  176, 176, 176, 176, 176, 176, 176, 176, 176, 176, 176, 176, 176, 176, 176, 176, 176, 176, 176, 176, 176, 176, 176, 176, 176, 176, 176, 176, 176, 176, 176, 176, 1056
//  };  

  LCD_Clear(0x00);
  String text1 = "Loading...";
  LCD_Print(text1, 85, 120, 2, 0xffff, 0x00);
  
//***************************************************************************************************************************************
// Imprimir fondo de pantalla en la LCD
//***************************************************************************************************************************************

  myFile = SD.open("FondoMod.txt");
    if (myFile) {
      Serial.println("FondoMod.txt:");
  
      // read from the file until there's nothing else in it:
      while (myFile.available()) {
        caracter = char(myFile.read());
        //Serial.print(caracter);
        if (caracter == ','){
          palabra.toCharArray(numero, 5);
          val = strtol(numero, NULL, 16);
          fondo[bits] = (val);
          palabra = "";
          bits++;
        }else if (caracter == ' '){
          
        }else {
          palabra.concat(caracter);
        }
        if (bits == 640){
          //Serial.println();
//          for (int x = 0; x <= 640; x++){
//            Serial.print(fondo[x]);
//          }
          LCD_Bitmap(0, posy, 320, 1, fondo);
          bits = 0;
          posy += 1;
        }
      }
      // close the file:
      myFile.close();
    } else {
      // if the file didn't open, print an error:
      Serial.println("error opening FondoMod.txt");
    }
    Serial.println("Fondo de pantalla cargado exitosamente");

//***************************************************************************************************************************************
// Fondo de pantalla cargado exitosamente
//***************************************************************************************************************************************
    

    for (int thisNote = 0; thisNote < sizeof(melody) / sizeof(int); thisNote++){    
      text1 = "8-BIT";
      LCD_Print(text1, 120, 30, 2, random(0xffff), 0x1007);
      text1 = "SONGS";
      LCD_Print(text1, 120, 55, 2, random(0xffff), 0x1007);
      tone(PinBuzzer, melody[thisNote], noteDurations[thisNote] * .7);    
      delay(noteDurations[thisNote]);    
      noTone(PinBuzzer);
    }

    text1 = "8-BIT";
    LCD_Print(text1, 120, 30, 2, 0xffff, 0x1007);
    text1 = "SONGS";
    LCD_Print(text1, 120, 55, 2, 0xffff, 0x1007);

    FillRect(0, 222, 320, 18, 0x00);
    LCD_Bitmap(0, 223, 16, 16, arrow_left);
    LCD_Bitmap(24, 223, 16, 16, arrow_right);
    text1 = "Select";
    LCD_Print(text1, 48, 224, 1, 0xffff, 0x00);
    LCD_Bitmap(120, 223, 16, 16, arrow_down);
    text1 = "Next";
    LCD_Print(text1, 144, 224, 1, 0xffff, 0x00);

    text1 = "1 Player";
    LCD_Print(text1, 8, 140, 2, 0xffff, 0x1007);
    text1 = "2 Player";
    LCD_Print(text1, 185, 140, 2, 0xffff, 0x1007);
    started = 1;
}

void characterMenu (void){
  File myFile;
  String palabra;
  char caracter;
  char numero [5];
  int bits = 0, posx = 0, posy = 66, val = 0, color = 0;
  String text2;

    LCD_Clear(0x00);
    text2 = "CHOOSE YOUR";
    LCD_Print(text2, 70, 10, 2, 0xffff, 0x00);
    text2 = "CHARACTER";
    LCD_Print(text2, 85, 25, 2, 0xffff, 0x00);
    
    myFile = SD.open("David.txt");
    if (myFile) {
      Serial.println("David.txt:");
  
      // read from the file until there's nothing else in it:
      while (myFile.available()) {
        caracter = char(myFile.read());
        if (caracter == ','){
          palabra.toCharArray(numero, 5);
          val = strtol(numero, NULL, 16);
          character[bits] = (val);
          palabra = "";
          bits++;
        }else if (caracter == ' '){
          
        }else {
          palabra.concat(caracter);
        }
        if (bits == 128){
          LCD_Bitmap(8, posy, 64, 1, character);
          bits = 0;
          posy += 1;
        }
      }
      // close the file:
      myFile.close();
      text2 = "David";
      LCD_Print(text2, 18, 135, 1, 0xffff, 0x00);
    } else {
      // if the file didn't open, print an error:
      Serial.println("error opening David.txt");
    }
    posy = 66;
    bits = 0;
    Serial.println("David cargado exitosamente");

    myFile = SD.open("Alberto.txt");
    if (myFile) {
      Serial.println("Alberto.txt:");
  
      // read from the file until there's nothing else in it:
      while (myFile.available()) {
        caracter = char(myFile.read());
        if (caracter == ','){
          palabra.toCharArray(numero, 5);
          val = strtol(numero, NULL, 16);
          character[bits] = (val);
          palabra = "";
          bits++;
        }else if (caracter == ' '){
          
        }else {
          palabra.concat(caracter);
        }
        if (bits == 128){
          LCD_Bitmap(88, posy, 64, 1, character);
          bits = 0;
          posy += 1;
        }
      }
      // close the file:
      myFile.close();
      text2 = "Alberto";
      LCD_Print(text2, 88, 135, 1, 0xffff, 0x00);
    } else {
      // if the file didn't open, print an error:
      Serial.println("error opening Alberto.txt");
    }
    posy = 66;
    bits = 0;
    Serial.println("Alberto cargado exitosamente");

    myFile = SD.open("Emilio.txt");
    if (myFile) {
      Serial.println("Emilio.txt:");
  
      // read from the file until there's nothing else in it:
      while (myFile.available()) {
        caracter = char(myFile.read());
        if (caracter == ','){
          palabra.toCharArray(numero, 5);
          val = strtol(numero, NULL, 16);
          character[bits] = (val);
          palabra = "";
          bits++;
        }else if (caracter == ' '){
          
        }else {
          palabra.concat(caracter);
        }
        if (bits == 128){
          LCD_Bitmap(168, posy, 64, 1, character);
          bits = 0;
          posy += 1;
        }
      }
      // close the file:
      myFile.close();
      text2 = "Emilio";
      LCD_Print(text2, 175, 135, 1, 0xffff, 0x00);
    } else {
      // if the file didn't open, print an error:
      Serial.println("error opening Emilio.txt");
    }
    posy = 66;
    bits = 0;
    Serial.println("Emilio cargado exitosamente");

    myFile = SD.open("Aina.txt");
    if (myFile) {
      Serial.println("Aina.txt:");
  
      // read from the file until there's nothing else in it:
      while (myFile.available()) {
        caracter = char(myFile.read());
        if (caracter == ','){
          palabra.toCharArray(numero, 5);
          val = strtol(numero, NULL, 16);
          character[bits] = (val);
          palabra = "";
          bits++;
        }else if (caracter == ' '){
          
        }else {
          palabra.concat(caracter);
        }
        if (bits == 128){
          LCD_Bitmap(248, posy, 64, 1, character);
          bits = 0;
          posy += 1;
        }
      }
      // close the file:
      myFile.close();
      text2 = "Aina";
      LCD_Print(text2, 264, 135, 1, 0xffff, 0x00);
    } else {
      // if the file didn't open, print an error:
      Serial.println("error opening Aina.txt");
    }
    Serial.println("Aina cargado exitosamente");

    LCD_Bitmap(0, 223, 16, 16, arrow_left);
    LCD_Bitmap(24, 223, 16, 16, arrow_right);
    text2 = "Select";
    LCD_Print(text2, 48, 224, 1, 0xffff, 0x00);
    LCD_Bitmap(120, 223, 16, 16, arrow_down);
    text2 = "Next";
    LCD_Print(text2, 144, 224, 1, 0xffff, 0x00);
    LCD_Bitmap(200, 223, 16, 16, arrow_up);
    text2 = "Return";
    LCD_Print(text2, 224, 224, 1, 0xffff, 0x00);
}

void musicMenu (void){
  File myFile;
  String palabra;
  char caracter;
  char numero [5];
  int bits = 0, posx = 0, posy = 0, val = 0, color = 0; 

  LCD_Clear(0x00);
  String text3 = "Loading...";
  LCD_Print(text3, 85, 120, 2, 0xffff, 0x00);
  
//***************************************************************************************************************************************
// Imprimir fondo de pantalla en la LCD
//***************************************************************************************************************************************

  myFile = SD.open("MUSICA~1.txt");
    if (myFile) {
      Serial.println("MUSICA~1.txt:");
  
      // read from the file until there's nothing else in it:
      while (myFile.available()) {
        caracter = char(myFile.read());
        //Serial.print(caracter);
        if (caracter == ','){
          palabra.toCharArray(numero, 5);
          val = strtol(numero, NULL, 16);
          fondo[bits] = (val);
          palabra = "";
          bits++;
        }else if (caracter == ' '){
          
        }else {
          palabra.concat(caracter);
        }
        if (bits == 640){
          //Serial.println();
//          for (int x = 0; x <= 640; x++){
//            Serial.print(fondo[x]);
//          }
          LCD_Bitmap(0, posy, 320, 1, fondo);
          bits = 0;
          posy += 1;
        }
      }
      // close the file:
      myFile.close();
    } else {
      // if the file didn't open, print an error:
      Serial.println("error opening MUSICA~1.txt");
    }
    Serial.println("Fondo de musica cargado exitosamente");

//***************************************************************************************************************************************
// Fondo de pantalla cargado exitosamente
//***************************************************************************************************************************************

    FillRect(0, 104, 320, 16, 0x00);

    FillRect(0, 222, 320, 18, 0x00);
    LCD_Bitmap(0, 223, 16, 16, arrow_left);
    LCD_Bitmap(24, 223, 16, 16, arrow_right);
    text3 = "Select";
    LCD_Print(text3, 48, 224, 1, 0xffff, 0x00);
    LCD_Bitmap(120, 223, 16, 16, arrow_down);
    text3 = "Next";
    LCD_Print(text3, 144, 224, 1, 0xffff, 0x00);
    LCD_Bitmap(200, 223, 16, 16, arrow_up);
    text3 = "Return";
    LCD_Print(text3, 224, 224, 1, 0xffff, 0x00);
}

void gamePlay(int song, int character1, int character2){
  File myFile;
  String palabra;
  char caracter;
  char numero [5];
  int bits = 0, posx = 0, posy = 0, val = 0, color = 0; 

  LCD_Clear(0x00);
  String text4 = "Loading...";
  LCD_Print(text4, 85, 120, 2, 0xffff, 0x00);
  
//***************************************************************************************************************************************
// Imprimir fondo de pantalla en la LCD
//***************************************************************************************************************************************
  if (song == 1){
    myFile = SD.open("Zelda.txt");
    if (myFile) {
      Serial.println("Zelda.txt:");
  
      // read from the file until there's nothing else in it:
      while (myFile.available()) {
        caracter = char(myFile.read());
        //Serial.print(caracter);
        if (caracter == ','){
          palabra.toCharArray(numero, 5);
          val = strtol(numero, NULL, 16);
          fondo[bits] = (val);
          palabra = "";
          bits++;
        }else if (caracter == ' '){
          
        }else {
          palabra.concat(caracter);
        }
        if (bits == 640){
          //Serial.println();
//          for (int x = 0; x <= 640; x++){
//            Serial.print(fondo[x]);
//          }
          LCD_Bitmap(0, posy, 320, 1, fondo);
          bits = 0;
          posy += 1;
        }
      }
      // close the file:
      myFile.close();
    } else {
      // if the file didn't open, print an error:
      Serial.println("error opening Zelda.txt");
    }
    Serial.println("Fondo de cancion cargado exitosamente");
    
  }else if (song == 2){
    myFile = SD.open("LedZ.txt");
    if (myFile) {
      Serial.println("LedZ.txt:");
  
      // read from the file until there's nothing else in it:
      while (myFile.available()) {
        caracter = char(myFile.read());
        //Serial.print(caracter);
        if (caracter == ','){
          palabra.toCharArray(numero, 5);
          val = strtol(numero, NULL, 16);
          fondo[bits] = (val);
          palabra = "";
          bits++;
        }else if (caracter == ' '){
          
        }else {
          palabra.concat(caracter);
        }
        if (bits == 640){
          //Serial.println();
//          for (int x = 0; x <= 640; x++){
//            Serial.print(fondo[x]);
//          }
          LCD_Bitmap(0, posy, 320, 1, fondo);
          bits = 0;
          posy += 1;
        }
      }
      // close the file:
      myFile.close();
    } else {
      // if the file didn't open, print an error:
      Serial.println("error opening LedZ.txt");
    }
    Serial.println("Fondo de cancion cargado exitosamente");
    
  }else if (song == 3){
    myFile = SD.open("Kirby.txt");
    if (myFile) {
      Serial.println("Kirby.txt:");
  
      // read from the file until there's nothing else in it:
      while (myFile.available()) {
        caracter = char(myFile.read());
        //Serial.print(caracter);
        if (caracter == ','){
          palabra.toCharArray(numero, 5);
          val = strtol(numero, NULL, 16);
          fondo[bits] = (val);
          palabra = "";
          bits++;
        }else if (caracter == ' '){
          
        }else {
          palabra.concat(caracter);
        }
        if (bits == 640){
          //Serial.println();
//          for (int x = 0; x <= 640; x++){
//            Serial.print(fondo[x]);
//          }
          LCD_Bitmap(0, posy, 320, 1, fondo);
          bits = 0;
          posy += 1;
        }
      }
      // close the file:
      myFile.close();
    } else {
      // if the file didn't open, print an error:
      Serial.println("error opening Kirby.txt");
    }
    Serial.println("Fondo de cancion cargado exitosamente");
    
  }

//***************************************************************************************************************************************
// Fondo de pantalla cargado exitosamente
//***************************************************************************************************************************************

//***************************************************************************************************************************************
// Cargando personajes
//***************************************************************************************************************************************

//***************************************************************************************************************************************
// Personajes cargados exitosamente
//***************************************************************************************************************************************
  if (modeSelect == 1){
    FillRect(108, 64, 104, 160, 0x00);
    
  }else if (modeSelect == 2){
    FillRect(32, 64, 104, 160, 0x00);
    FillRect(184, 64, 104, 160, 0x00);
    
  }
}
