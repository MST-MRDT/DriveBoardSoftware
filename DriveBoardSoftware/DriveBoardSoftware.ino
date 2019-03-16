/*
 * DriveBoard Software Rev 2 2013
 * Used with ODrive Motor Controllers
 * Writes Serial to 3 motor drives
 * 
 * Andrew Van Horn
 */

#include "RoveComm.h"
#include "RovesODrive.h"
#include "DriveBoardSoftware.h"

RoveCommEthernetUdp RoveComm;

void checkButtons()
{
    for(int i = 0; i<NUMMOTORS; i++)
    {
      if(digitalRead(motor[i].button_pin))
      {
        motor[i].speed = (digitalRead(DIRECTION_SWITCH_PIN)?1:-1)*BUTTONPRESS_SPEED;
      }
      else if(watchdog_triggered)
      {
        motor[i].speed = 0;
      }
    }
}

void writeSpeeds()
{
  for(int i = 0; i<NUMDRIVES; i++)
  {
    for(int j = 0; j<2; j++)
    {
      Drive[i].motor[j].setRamp(ramp_rate);
      Drive[i].motor[j].writeConfig();
      Drive[i].motor[j].setSpeed(motor[i+j].speed);
    }
  }
}

void getSpeeds()
{
  for(int i = 0; i<NUMDRIVES; i++)
  {
    for(int j = 0; j<2; j++)
    {
      Serial.print(2*i+j);
      Serial.print(":");
      int speed = Drive[i].motor[j].getSpeed();
    }
  }
}

void parseRoveComm()
{
  rovecomm_packet packet = RoveComm.read();
  switch(packet.data_id)
  {
    case RC_DRIVEBOARD_DRIVELEFTRIGHT_DATAID:
      motor[LF].speed = packet.data[0];
      motor[LM].speed = packet.data[0];
      motor[LR].speed = packet.data[0];
      motor[RF].speed = packet.data[1];
      motor[RM].speed = packet.data[1];
      motor[RR].speed = packet.data[1];
      break;
    case RC_DRIVEBOARD_SPEEDRAMPVALUEs_DATAID:
      ramp_rate = packet.data[0];
      break;
  }
}

void setup()
{
  Serial.begin(115200);

  pinMode(DIRECTION_SWITCH_PIN, INPUT);

  for(int i = 0; i<NUMMOTORS; i++)
  {
    pinMode(motor[i].button_pin, INPUT);
  }
                                
  pinMode(WATCHDOG_IND_PIN    , OUTPUT);
  pinMode(LSPEED_IND_PIN      , OUTPUT);
  pinMode(RSPEED_IND_PIN      , OUTPUT);

  Serial.println("Setting up drive");

  Serial4.begin(115200);

  for(int i = 0; i<NUMDRIVES; i++)
  {
    Drive[i].begin();
    delay(10);
    for(int j = 0; j<2; j++)
    {
      Drive[i].motor[j].setPolePairs(4);
      delay(10);
      Drive[i].motor[j].setKV(480);
      delay(10);
      Drive[i].motor[j].setControlMode(CTRL_MODE_SENSORLESS_VELOCITY_CONTROL);
      delay(10);
      Serial.print("-");
      Serial.println(i+j);
    }
  }
  RoveComm.begin(RC_DRIVEBOARD_FOURTHOCTET);
    
}

void loop()
{
  static int i = 0;
  i++;
  delay(100);
  Serial.println(i);

  checkButtons();

  parseRoveComm();

  if(watchdog_triggered)
  {
    digitalWrite(WATCHDOG_IND_PIN, HIGH);
  }

  writeSpeeds();

  getSpeeds();
  

}