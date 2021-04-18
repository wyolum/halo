#include <rtcBOB.h>


void setup(){
  Serial.begin(115200);
  String t = __TIME__;
  int hh = t.substring(0, 2).toInt();
  int mm = t.substring(3, 5).toInt();
  int ss = t.substring(6, 8).toInt();

  
  Serial.print("date: ");
  Serial.println(__DATE__);
  Serial.print("Setting time to : ");
  Serial.println(t);

  setRTC(2021, 1, 1, hh, mm, ss);


}
void loop(){
}
