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
char buffer[] = "72616B776972656C657373";

bool InitLoRaWAN(void);
RAK811 RAKLoRa(ATSerial, DebugSerial);


void setup() {
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

  DebugSerial.println("Start init RAK811 parameters...");
  if (!RAKLoRa.rk_setSendinterval(0, 30)) //close auto join and send to LoRaWAN
  {
    DebugSerial.println("Close auto join and send to LoRaWAN failed, please reset module.");
    while (1);
  }
//
//  if (!InitLoRaWAN())  //init LoRaWAN
//  {
//    DebugSerial.println("Init error,please reset module.");
//    while (1);
//  }
//
//  DebugSerial.println("Start joining LoRaWAN...");
//  while (!RAKLoRa.rk_joinLoRaNetwork(60)) //Joining LoRaNetwork timeout 60s
//  {
//    DebugSerial.println();
//    DebugSerial.println("Rejoin again after 5s...");
//    delay(5000);
//  }
//
//  if (!RAKLoRa.rk_isConfirm(0)) //set LoRa data send package type:0->unconfirm, 1->confirm
//  {
//    DebugSerial.println("LoRa data send package set error,please reset module.");
//    while (1);
//  }
//}
//
//bool InitLoRaWAN(void)
//{
//  if (RAKLoRa.rk_setJoinMode(JOIN_MODE))
//  {
//    if (RAKLoRa.rk_setRegion(0)) //set region EU868
//    {
//      if (RAKLoRa.rk_initOTAA(DevEui, AppEui, AppKey))
//      {
//        DebugSerial.println("RAK811 init OK!");
//        return true;
//      }
//    }
//  }
//  return false;
}

void loop() {
  DebugSerial.println("Start send data...");
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
