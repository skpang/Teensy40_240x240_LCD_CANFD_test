/*
 * Demo sketch for use with:
 * http://skpang.co.uk/catalog/teensy-40-can-fd-board-with-240x240-ips-lcd-and-usd-holder-p-1584.html
 * 
 * Nominal baudrate 500kbps
 * CAN FD data baudrate 2000kbps
 * 
 * www.skpang.co.uk Jan 2020
 * 
 * 
 * */

 
#include <FlexCAN_T4.h>
FlexCAN_T4FD<CAN3, RX_SIZE_256, TX_SIZE_16> FD;

//#include <Adafruit_GFX.h>    // Core graphics library
#include <SPI.h>
#include <ST7735_t3.h> // Hardware-specific library
#include <ST7789_t3.h> // Hardware-specific library
#include <ST7735_t3_font_Arial.h>
#include <ST7735_t3_font_ArialBold.h>

#define TFT_RST    9   // chip reset
#define TFT_DC     8   // tells the display if you're sending data (D) or commands (C)   --> WR pin on TFT
#define TFT_MOSI   11  // Data out    (SPI standard)
#define TFT_SCLK   13  // Clock out   (SPI standard)
#define TFT_CS     10  // chip select (SPI standard)
#define SD_CS     7


ST7789_t3 tft = ST7789_t3(TFT_CS, TFT_DC, TFT_MOSI, TFT_SCLK, TFT_RST);
IntervalTimer timer;
uint8_t d=0;
bool stopfd = 0;

void setup(void) {
  
  Serial.begin(115200); delay(1000);
  Serial.println("Teensy 4.0 CAN FD test. www.skpang.co.uk Jan 2020");
 
  FD.begin();

  CANFD_timings_t config;
  config.clock = CLK_24MHz;
  config.baudrate =   500000;
  config.baudrateFD = 2000000;
  config.propdelay = 190;
  config.bus_length = 1;
  config.sample = 75;
  FD.setRegions(64);
  FD.setBaudRate(config);
  FD.onReceive(canSniff);

  FD.setMBFilter(ACCEPT_ALL);
  FD.setMBFilter(MB13, 0x1);
  FD.setMBFilter(MB12, 0x1, 0x3);
  FD.setMBFilterRange(MB8, 0x1, 0x04);
  FD.enableMBInterrupt(MB8);
  FD.enableMBInterrupt(MB12);
  FD.enableMBInterrupt(MB13);
  FD.enhanceFilter(MB8);
  FD.enhanceFilter(MB10);
  FD.distribute();
  FD.mailboxStatus();
  
  tft.init(240, 240);
  tft.setRotation(2);
  tft.fillScreen(ST77XX_BLUE);

  tft.setCursor(5,5);
  tft.setFont(Arial_32);
  tft.println("Teensy  4.0");

  tft.setCursor(5,100);
  tft.setFont(Arial_32_Bold); 
  tft.println("CAN FD");
 
  tft.setFont(Arial_16);
  tft.setCursor(5,215);
  tft.println("www.skpang.co.uk");   

  timer.begin(sendframe, 500000); // Send frame every 500ms
}

void sendframe()
{
  CANFD_message_t msg;
  msg.len = 64;
  msg.id = 0x321;
  msg.seq = 1;
  msg.buf[0] = d; msg.buf[1] = 2; msg.buf[2] = 3; msg.buf[3] = 4;
  msg.buf[4] = 5; msg.buf[5] = 6; msg.buf[6] = 9; msg.buf[7] = 9;
  FD.write( msg);
}

void loop() {
  
  FD.events(); /* needed for sequential frame transmit and callback queues */
  CANFD_message_t msg;

  if(FD.readMB(msg)){   /* check if we received a CAN frame */
      Serial.print("MB: "); Serial.print(msg.mb);
      Serial.print("  OVERRUN: "); Serial.print(msg.flags.overrun);
      Serial.print("  ID: 0x"); Serial.print(msg.id, HEX );
      Serial.print("  EXT: "); Serial.print(msg.flags.extended );
      Serial.print("  LEN: "); Serial.print(msg.len);
      Serial.print("  BRS: "); Serial.print(msg.brs);
      Serial.print(" DATA: ");
      for ( uint8_t i = 0; i <msg.len ; i++ ) {
        Serial.print(msg.buf[i]); Serial.print(" ");
      }
      Serial.print("  TS: "); Serial.println(msg.timestamp);
    
  }
}

void canSniff(const CANFD_message_t &msg) {
  if ( stopfd ) return;
  Serial.print("ISR - MB "); Serial.print(msg.mb);
  Serial.print("  OVERRUN: "); Serial.print(msg.flags.overrun);
  Serial.print("  LEN: "); Serial.print(msg.len);
  Serial.print(" EXT: "); Serial.print(msg.flags.extended);
  Serial.print(" TS: "); Serial.print(msg.timestamp);
  Serial.print(" ID: "); Serial.print(msg.id, HEX);
  Serial.print(" Buffer: ");
  for ( uint8_t i = 0; i < msg.len; i++ ) {
    Serial.print(msg.buf[i], HEX); Serial.print(" ");
  } Serial.println();
}
