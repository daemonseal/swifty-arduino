/********************************************************
   This demo is only supported after RUI firmware version 3.0.0.13.X on RAK811
   Master Board Uart Receive buffer size at least 128 bytes.
 ********************************************************/

#include "RAK811.h"
#include "SoftwareSerial.h"
#define WORK_MODE LoRaWAN //  LoRaWAN or LoRaP2P
#define JOIN_MODE OTAA    //  OTAA or ABP
#define APP_PORT        1
#define SEND_EVERY      60000   // Send message very 50 seconds
#define REQUIRE_ACK     1       // Require ACK
#define RETRIES         3       // Number of retries if message fails
#define RETRY_AFTER     5000    // Wait these many ms before retrying
#if JOIN_MODE == OTAA
String DevEui = "60C5A8FFFE001010";
String AppEui = "70B3D57ED0029873";
String AppKey = "34245CDAE5BE51C3343E23F02F2CD936";
#else
String NwkSKey = "E94D297FDAECC676F6103DFC24AA2FB0";
String AppSKey = "76BA2E11AAD92F866221243E7FB99528";
String DevAddr = "2604191B";
#endif
#define TXpin 11 // Set the virtual serial port pins
#define RXpin 10
#define ATSerial Serial
#define RELAY_GPIO 4
SoftwareSerial DebugSerial(RXpin, TXpin); // Declare a virtual serial port

char buffer[] = "68656c6c6f20776f726c64"; // hello world
#define fo "666f" // fan on
#define fx "6678" // fan off
#define ho "686f" // humidifier on
#define hx "6878" // humidifier off
#define cf "6366" // confirm

bool InitLoRaWAN(void);
void InitLoRaParams();
RAK811 RAKLoRa(ATSerial, DebugSerial);

void setup() {
  pinMode(RELAY_GPIO, OUTPUT);
  DebugSerial.begin(115200);
  while (DebugSerial.read() >= 0) {
  }
  while (!DebugSerial);
  DebugSerial.println("StartUP");

  ATSerial.begin(115200); // Note: Please manually set the baud rate of the
                          // WisNode device to 115200.
  while (ATSerial.available()) {
    ATSerial.read();
  }

  DebugSerial.println(RAKLoRa.rk_getVersion()); // get RAK811 firmware version
  delay(200);

  InitLoRaParams();
}

void InitLoRaParams() {
  DebugSerial.println("Start init RAK811 parameters...");

  if (!RAKLoRa.rk_setSendinterval(0, 0)) // close auto join and send to LoRaWAN
  {
    DebugSerial.println(
        "Close auto join and send to LoRaWAN failed, please reset module.");
    while (1)
      ;
  }

  if (!RAKLoRa.rk_setClass(2)) {
    DebugSerial.println("Error setting class, please reset module.");
    while (1)
      ;
  }

  if (!InitLoRaWAN()) // init LoRaWAN
  {
    DebugSerial.println("Init error, please reset module.");
    while (1)
      ;
  }

  DebugSerial.println("Start joining LoRaWAN...");
  while (!RAKLoRa.rk_joinLoRaNetwork(60)) // Joining LoRaNetwork timeout 60s
  {
    DebugSerial.println();
    DebugSerial.println("Rejoin again after 5s...");
    delay(5000);
  }

  if (!RAKLoRa.rk_isConfirm(
          0)) // set LoRa data send package type:0->unconfirm, 1->confirm
  {
    DebugSerial.println(
        "LoRa data send package set error,please reset module.");
    while (1)
      ;
  }
}

bool InitLoRaWAN(void) {

  if (RAKLoRa.rk_setJoinMode(JOIN_MODE)) {
    if (RAKLoRa.rk_setRegion(0)) // set region EU868
    {
      if (RAKLoRa.rk_initOTAA(DevEui, AppEui, AppKey)) {
        DebugSerial.println("RAK811 init OK!");
        return true;
      }
    }
  }
  return false;
}

void ttn_send() {

    DebugSerial.println("Sending message");

    // Send message
    unsigned char tries = 0;
    while (true) {

        // The payload has to be an HEX string
        char buffer[3];
        bool state = !digitalRead(RELAY_GPIO); // relay has inverse logic
        snprintf(buffer, sizeof(buffer), "%02X", state ? 0x31 : 0x30);

        // Sending test message
        if (RAKLoRa.rk_sendData(APP_PORT, buffer)) {
            DebugSerial.println("Message sent correctly!");
            return;
        }

        tries++;
        if (tries > RETRIES) break;

        DebugSerial.println("Error, trying again...");
        delay(RETRY_AFTER);

    }

    DebugSerial.println("Error sending message :(");

}

void ttn_receive() {

    // at+recv=<status>,<port>[,<rssi>][,<snr>],<len>[,<data>]\r\n
    // STATUS_RECV_DATA == 0
    String r = RAKLoRa.rk_recvData();
    if (r.length()) {

        DebugSerial.print("Response: ");
        DebugSerial.println(r);

        if (r.startsWith("at+recv=0,")) {

            bool state = r.endsWith(",31");
            DebugSerial.print("Turning relay ");
            DebugSerial.println(state ? "ON" : "OFF");
            digitalWrite(RELAY_GPIO, !state); // relay has inverse logic

        }

    }

}

void loop() {
    static unsigned long last = 0;
    if ((last == 0) || (millis() - last > SEND_EVERY)) {
        last = millis();
        ttn_send();
    }
    ttn_receive();
}
