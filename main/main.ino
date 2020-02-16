/********************************************************
   This demo is only supported after RUI firmware version 3.0.0.13.X on RAK811
   Master Board Uart Receive buffer size at least 128 bytes.
 ********************************************************/

#include "RAK811.h"
#include "SoftwareSerial.h"
#define WORK_MODE LoRaWAN   //  LoRaWAN or LoRaP2P
#define JOIN_MODE OTAA    //  OTAA or ABP
#if JOIN_MODE == OTAA
String DevEui = "60C5A8FFFE001010";
String AppEui = "70B3D57ED0029873";
String AppKey = "34245CDAE5BE51C3343E23F02F2CD936";
#else JOIN_MODE == ABP
String NwkSKey = "E94D297FDAECC676F6103DFC24AA2FB0";
String AppSKey = "76BA2E11AAD92F866221243E7FB99528";
String DevAddr = "2604191B";
#endif
#define TXpin 11   // Set the virtual serial port pins
#define RXpin 10
#define ATSerial Serial
SoftwareSerial DebugSerial(RXpin, TXpin);   // Declare a virtual serial port

char buffer[] = "68656c6c6f20776f726c64"; //hello world
char fan_on[] = "66616e5f6f6e"; //fan_on
char fan_off[] = "66616e5f6f6666"; //fan_off
char fog_on[] = "666f675f6f6e"; //fog_on
char fog_off[] = "666f675f6f6666"; //fog_off

bool InitLoRaWAN(void);
RAK811 RAKLoRa(ATSerial, DebugSerial);


void setup() {
  pinMode(RXpin, INPUT);
  pinMode(TXpin, OUTPUT);
  DebugSerial.begin(115200);
  while (DebugSerial.read() >= 0) {}
  while (!DebugSerial);
  DebugSerial.println("StartUP");

  ATSerial.begin(115200); // Note: Please manually set the baud rate of the WisNode device to 115200.
  while (ATSerial.available())
  {
    ATSerial.read();
  }

  DebugSerial.println(RAKLoRa.rk_getVersion());  //get RAK811 firmware version
  delay(200);


}

void InitLoRaParams() {
  DebugSerial.println("Start init RAK811 parameters...");

  if (!RAKLoRa.rk_setSendinterval(0, 0)) //close auto join and send to LoRaWAN
  {
    DebugSerial.println("Close auto join and send to LoRaWAN failed, please reset module.");
    while (1);
  }

  if (!RAKLoRa.rk_setClass(2)) {
    DebugSerial.println("Error setting class, please reset module.");
    while (1);
  }

  if (!InitLoRaWAN())  //init LoRaWAN
  {
    DebugSerial.println("Init error, please reset module.");
    while (1);
  }

  DebugSerial.println("Start joining LoRaWAN...");
  while (!RAKLoRa.rk_joinLoRaNetwork(60)) //Joining LoRaNetwork timeout 60s
  {
    DebugSerial.println();
    DebugSerial.println("Rejoin again after 5s...");
    delay(5000);
  }

  if (!RAKLoRa.rk_isConfirm(0)) //set LoRa data send package type:0->unconfirm, 1->confirm
  {
    DebugSerial.println("LoRa data send package set error,please reset module.");
    while (1);
  }
}

bool InitLoRaWAN(void)
{

  InitLoRaParams();

  if (RAKLoRa.rk_setJoinMode(JOIN_MODE))
  {
    if (RAKLoRa.rk_setRegion(0)) //set region EU868
    {
      if (RAKLoRa.rk_initOTAA(DevEui, AppEui, AppKey))
      {
        DebugSerial.println("RAK811 init OK!");
        return true;
      }
    }
  }
  return false;
}

void send_data(char buffer[]) {
  if (RAKLoRa.rk_sendData(1, buffer))
  {
    for (unsigned long start = millis(); millis() - start < 90000L;)
    {
      String ret = RAKLoRa.rk_recvData();
      if (ret != NULL)
      {
        ret.trim();
        DebugSerial.println(ret);
      }
      if ((ret.indexOf("OK") > 0) || (ret.indexOf("ERROR") > 0))
      {
        RAKLoRa.rk_sleep(1);  //Set RAK811 enter sleep mode
        delay(10000);  //delay 10s
        RAKLoRa.rk_sleep(0);  //Wakeup RAK811 from sleep mode
        break;
      }
    }
  }
}

bool receive_data() {

  // at+recv=<status>,<port>[,<rssi>][,<snr>],<len>[,<data>]\r\n
  // STATUS_RECV_DATA == 0
  String r = RAKLoRa.rk_recvData();
  if (r.length()) {

    Serial.print("Response: ");
    Serial.println(r);

    if (r.startsWith("at+recv=0,")) {

      bool state = r.endsWith(",31");
      Serial.print("Turning relay ");
      Serial.println(state ? "ON" : "OFF");
      digitalWrite(RELAY_GPIO, !state);

      return true;

    }

  }

}

void loop() {
  DebugSerial.println("Start send data...");

  send_data(buffer);

  if (receive_data()) {
    DebugSerial.println("Received data");
  }

}
