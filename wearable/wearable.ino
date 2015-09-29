#include <SoftwareSerial.h>
#define SSID "IDA_LABS_2"  //name of wireless access point to connect to
#define PASS "IDA@labs"  //wifi password
#define DST_IP "213.186.33.19" // IP of api.pushingbox.com maybe it changes, so take DST_HOST
#define DST_HOST "api.pushingbox.com"

SoftwareSerial debug(11, 10); // RX, TX - For listening in to the board

#define LED 13

String sMessage_1 = "v69B25A081CB8C63"; //put your DEVID from pushingbox here

void setup()  //initialise device & connect to access point in setup
{
  pinMode(LED, OUTPUT);

  //softwarereset();
  reset();

  Serial.begin(9600);    // hardware debug connects to esp8266 module
  debug.begin(9600); // usb debug connects to to pc
  delay(4000);    //wait for usb debug enumeration on 'debug' & device startup
  debug.println("KIDS CODE");
}

void loop()
{
  static boolean wifi_connected = false; //store state of wifi connection
  static boolean isHome = false; //state if child is home
  static int count = 0; //store disconnect count

  if (connectWiFi()) //check if wifi has been connected
  {
    wifi_connected = true;  //yes
    if (!wifi_connected) hang("wifi not connected");  //these seem ok - never had a problem
    delay(250);
    if (!cipmux0()) hang("cipmux0 failed");
    delay(250);
    if (!cipmode0()) hang("cipmode0 failed");
    delay(250);
    //if the child has left the house and reached home
    //this is to prevent spamming the mom everytime wifi gets connected
    //executes only once. every time the child reaches home
    if (!isHome)
    {
      //flag to prevent sending next loop
      isHome = true;
      debug.println("Mother notified");
      //send notification to KIN
      sendToPushingBox(sMessage_1);
      delay(1000);
    }
  }
  else
  {
    //count how many wifi disconnections to make sure it's not a fluke
    //this is to confirm child is not home
    count++;
    if(count > 25)
    {
      //reset the flag so Arduino can send again once the child reaches home again
      isHome = false;
      count = 0;
    }
  }

}
//------------------------------------------------------------------------------------
void sendToPushingBox(String devid)
{
  String cmd = "AT+CIPSTART=\"TCP\",\"";
  cmd += DST_HOST;
  cmd += "\",80";

  Serial.println(cmd);  //send command to device

  delay(2000);  //wait a little while for 'Linked' response - this makes a difference

  cmd =  "GET /pushingbox?devid=";
  cmd += devid;
  cmd += " HTTP/1.1\r\n";  //construct http GET request
  cmd += "Host: api.pushingbox.com\r\n\r\n";
  Serial.print("AT+CIPSEND=");
  Serial.println(cmd.length());  //esp8266 needs to know message length of incoming message - .length provides this

  if (Serial.find(">"))   //prompt offered by esp8266
  {
    Serial.println(cmd);  //this is our http GET request
  }
  else
  {
    Serial.println("AT+CIPCLOSE");  //doesn't seem to work here?
    debug.println("No '>' prompt received after AT+CPISEND");
  }

  Serial.println("AT+CIPCLOSE");
}

boolean connectWiFi()
{
  String cmd = "AT+CWJAP=\""; //form eg: AT+CWJAP="dynamode","55555555555555555555555555"
  cmd += SSID;
  cmd += "\",\"";
  cmd += PASS;
  cmd += "\"";
  Serial.println(cmd);
  delay(5000); //give it time - my access point can be very slow sometimes
  if (Serial.find("OK")) //healthy response
  {
    debug.println("Connected to WiFi...");
    return true;
  }
  else
  {
    debug.println("Not connected to WiFi.");
    return false;
  }
}
//--------------------------------------------------------------------------------
//ditch this in favour of hardware reset. Done
boolean softwarereset()
{
  Serial.println("AT+RST");
  if (Serial.find("ready"))
  {
    return true;
  }
  else
  {
    return false;
  }
}
//--------------------------------------------------------------------------------
void reset()
{
  Serial.println("AT+RST");
  digitalWrite(LED, HIGH);
  delay(100);
  digitalWrite(LED, LOW);
}
//------------------------------------------------------------------------------
boolean cwmode3()
// Odd one. CWMODE=3 means configure the device as access point & station. This function can't fail?

{
  Serial.println("AT+CWMODE=3");
  if (Serial.find("no change"))  //only works if CWMODE was 3 previously
  {
    return true;
  }
  else
  {
    return false;
  }
}
//----------------------------------------------------------------------------------
boolean cipmux0()
{
  Serial.println("AT+CIPMUX=0");
  if (Serial.find("OK"))
  {
    return true;
  }
  else
  {
    return false;
  }
}
//-----------------------------------------------------------------------
boolean cipmode0()
{
  Serial.println("AT+CIPMODE=0");
  if (Serial.find("OK"))
  {
    return true;
  }
  else
  {
    return false;
  }
}
//------------------------------------------------------------------------
void hang(String error_String)    //for debugging
{
  debug.print("Halted...   ");
  debug.println(error_String);
  while (1)
  {
    digitalWrite(LED, HIGH);
    delay(100);
    digitalWrite(LED, LOW);
    delay(100);
  }
}
//----------------------------------------------------------------------------
void hangreset (String error_String)    //for debugging
{
  debug.print(error_String);
  debug.println(" - resetting");
  reset();
}


