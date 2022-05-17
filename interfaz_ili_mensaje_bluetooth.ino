/*
 * Yaxcal Tec
 * Diseño de interfaz con una
 * ILI9341 de Adafruit
 * ESP32 WROOM
 * Arduino placa --> FireBeetle-ESP32
 * Mayo 2021
 */
#include <Adafruit_GFX.h>         // Core graphics library
#include <Adafruit_ILI9341.h>      // Hardware-specific library
#include <SdFat.h>                // SD card & FAT filesystem library
#include <Adafruit_SPIFlash.h>    // SPI / QSPI flash library
#include <Adafruit_ImageReader.h> // Image-reading functions
#include <stdint.h>
#include "SPI.h"                  //Libreria puerto SPI
#include "TouchScreen.h"          //Libreria para elemento touch
#define USE_SD_CARD
#include "BluetoothSerial.h"
BluetoothSerial SerialBT;
/*
 * Definicion de pines de conexion
 * #23MOSI, #19MISO, #18CLK
 */
#define TFT_RST -1
#define TFT_CS   22
#define TFT_DC   21
#define SD_CS   17
#define YP 15   //Pin en el breakout Y+ debe ser Analogico
#define XM 4   //Pin en el breakout X- debe ser analogico
#define YM 0    //Pin en el breakout Y- debe ser digital
#define XP 2    //Pin en el breakout X+ debe ser digital
#define cc 10    //puerto de calentador
#define vv 9    //puerto de ventilador
/*
 * Se conecta el módulo SPI del chip
 * PIN-CHIP------|-----HX8357
 * 18--SPI_CLK--->-----CLK
 * 19--SPI_MISO-->-----MISO
 * 23--SPI_MOSI-->-----MOSI
 */
//Proceso de configuracion SD
#if defined(USE_SD_CARD)
  SdFat                SD;         // SD card filesystem
  Adafruit_ImageReader reader(SD); // Image-reader object, pass in SD filesys
#else
  // SPI or QSPI flash filesystem (i.e. CIRCUITPY drive)
  #if defined(__SAMD51__) || defined(NRF52840_XXAA)
    Adafruit_FlashTransport_QSPI flashTransport(PIN_QSPI_SCK, PIN_QSPI_CS,
      PIN_QSPI_IO0, PIN_QSPI_IO1, PIN_QSPI_IO2, PIN_QSPI_IO3);
  #else
    #if (SPI_INTERFACES_COUNT == 1)
      Adafruit_FlashTransport_SPI flashTransport(SS, &SPI);
    #else
      Adafruit_FlashTransport_SPI flashTransport(SS1, &SPI1);
    #endif
  #endif
  Adafruit_SPIFlash    flash(&flashTransport);
  FatFileSystem        filesys;
  Adafruit_ImageReader reader(filesys); // Image-reader, pass in flash filesys
#endif

//Se define pÃ­nes para el core del LCD y se define variable para imagen
Adafruit_ILI9341        tft    = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_RST); //Se agrega RST
Adafruit_Image         img;        // An image loaded into RAM

//Se definen pines para el uso del TScreen. Ademas se pasa el valor resistivo
//entre X+ y X-
TouchScreen ts = TouchScreen(XP, YP, XM, YM, 30);
//*******************************************|-->pressureThreshhold
/*
 * banderas globales
 */
int xpos = 39;
int programa = 0;
int tiempo = 0;
String txt1,txt2;
int bandera = 1;
int color_select=0;
String message1 = "";
bool men = false;
/*
 * son accesadas por las funciones
 * de operacion en funcion loop
 */
void letrero(String, String);
void inicio(void);
void secado(void);
void selec(int);
void pantalla_inicial(void);
void paro_emergencia(void);
/*
función de configuración principal
*/
void setup() {
  Serial.begin(9600);
  SerialBT.begin("ESP32pumitas");
  analogReadResolution(10);
  SPI.setFrequency(80000000);
  pinMode(cc,OUTPUT);
  pinMode(vv,OUTPUT);
  ImageReturnCode stat;
  tft.begin();

#if defined(USE_SD_CARD)
  // SD card is pretty straightforward, a single call...
  if(!SD.begin(SD_CS, SD_SCK_MHZ(25))) { // ESP32 requires 25 MHz limit
   // Serial.println(F("SD begin() failed"));
    for(;;); // Fatal error, do not continue
  }
#else
  // SPI or QSPI flash requires two steps, one to access the bare flash
  // memory itself, then the second to access the filesystem within...
  if(!flash.begin()) {

    for(;;);
  }
  if(!filesys.begin(&flash)) {

    for(;;);
  }
#endif

  tft.fillScreen(ILI9341_BLACK);
  txt1 = "---------*BUAP*----------";
  txt2 = "-Jonathan Torres Serrano-";
  letrero1(txt1,txt2);
  delay(4000);
  /*
   * Permanece por 4 segundos el logo de solar
   */
  tft.fillScreen(ILI9341_BLACK);
  stat = reader.drawBMP("/chungo.bmp", tft, 0, 145);

  pantalla_inicial();

  txt1 = "--*Bienvenido*--";
  txt2 = "----|-----|-----";
  letrero1(txt1,txt2);

}

void loop() {
   TSPoint p = ts.getPoint();
      
  if (p.z > ts.pressureThreshhold) {
    //Boton Start
    if(p.x>273 && p.x<324 && p.y>651 && p.y<687)
    {
      txt1 = "Inicia proceso de";
      txt2 = "esterilizacion";
      bandera = 0;
      letrero1(txt1,txt2);
      
    }
    

  delay(100);
  }

  if (SerialBT.available()) {
    
    char incomingChar1 = SerialBT.read();
    
    if (incomingChar1!= '\n'){
      message1 += String(incomingChar1);
      //Serial.println(message1);
    }
    else{
      men=true;
      
    }
  }
  if (men){
    Serial.println(message1);
   txt1 = "Mensaje recibido:";
      txt2 = message1;
      bandera = 0;
      letrero(txt1,txt2);
      delay(100);
    men = false;
    message1="";
  }
}

void letrero(String txt1, String txt2){
  tft.fillRect(40,190,180,60,0xFC00);
  tft.fillRect(45,195,180,60,0xFFFF);
  tft.setCursor(85,210); tft.setTextColor(0x001F); tft.setTextSize(1);
  tft.println(txt1);
  tft.setCursor(95,240); tft.setTextColor(0x001F); tft.setTextSize(1);
  tft.println(txt2);
}

void letrero1(String txt1, String txt2){
  tft.fillRect(20,60,215,70,0x03EF);
  tft.fillRect(25,75,210,70,0xFFFF);
  tft.setCursor(65,90); tft.setTextColor(0x001F); tft.setTextSize(1);
  tft.println(txt1);
  tft.setCursor(75,120); tft.setTextColor(0x001F); tft.setTextSize(1);
  tft.println(txt2);
} 
 void pantalla_inicial(void){
  tft.fillRect(0,165,240,145,0x45D5);
 
 }
