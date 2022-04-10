#include <Arduino.h>
#include <ArduinoJson.h>
#include <EspMQTTClient.h>
#include <SoftwareSerial.h>
#include <ESP8266WiFi.h>

// Update these with values suitable for your network.
const char* ssid = "YOURSSID";
const char* password = "YOURSSIDPASSWORD";
const char* mqtt_server = "YOURMQTTSERVER";

#define MQTT_USER "" //can be blank
#define MQTT_PASSWORD "" //can be blank
#define MQTT_SERIAL_PUBLISH_CH "/spanet/serialdata/tx"
#define MQTT_SERIAL_RECEIVER_CH "/spanet/serialdata/rx"
#define D5 (14) //goes to rx on spanet pin5
#define D6 (12) //goes to tx on spanet pin6
#define BAUD_RATE 38400

unsigned long startMillis;
unsigned long currentMillis;
const unsigned long period = 10000;  //milliseconds for refresh interval of SPA

int Pump1State;
int Pump2State;
int Pump3State;
int Pump4State;
int Pump5State;
int BlowerMode;
int BlowerSpeed;
int LightsONOFF;
int LightsMode;
int LightsBrightness;
int LightsEffectSpeed;
int LightsColour;
float SetTemperature;
float CurrentTemperature;
String cmd;
IPAddress ip;

SoftwareSerial swSer;

ESP8266WebServer server(80); //WEBServer on port 80

String header;
unsigned long currentTime = millis();  // Current time
unsigned long previousTime = 0;  // Previous time
const long timeoutTime = 2000;  // Define timeout time in milliseconds (example: 2000ms = 2s)

EspMQTTClient client(
  ssid,
  password,
  mqtt_server,
  MQTT_USER,
  MQTT_PASSWORD"",
  "SpaNetClient"
);

String MQTTBaseTopic = "/spanet/status";

String returndata(String passdata, int location) {
  int lastcomma = 0;
  for (int i = 0; i < location; i++) {
      int comma = passdata.indexOf(",", lastcomma+1);
      lastcomma=comma;
      } 
  return passdata.substring(lastcomma+1, passdata.indexOf(",", lastcomma+1));
}

void convertpayload(String StringData) { //Publish each line to MQTT from RF Return
   String R2 = StringData.substring(StringData.indexOf("R2,"), StringData.indexOf("R3,")-5);
   String R3 = StringData.substring(StringData.indexOf("R3,"), StringData.indexOf("R4,")-5);
   String R4 = StringData.substring(StringData.indexOf("R4,"), StringData.indexOf("R5,")-5);
   String R5 = StringData.substring(StringData.indexOf("R5,"), StringData.indexOf("R6,")-5);
   String R6 = StringData.substring(StringData.indexOf("R6,"), StringData.indexOf("R7,")-5);
   String R7 = StringData.substring(StringData.indexOf("R7,"), StringData.indexOf("R9,")-5);
   String R9 = StringData.substring(StringData.indexOf("R9,"), StringData.indexOf("RA,")-5);
   String RA = StringData.substring(StringData.indexOf("RA,"), StringData.indexOf("RB,")-5);
   String RB = StringData.substring(StringData.indexOf("RB,"), StringData.indexOf("RC,")-5);
   String RC = StringData.substring(StringData.indexOf("RC,"), StringData.indexOf("RE,")-5);
   String RE = StringData.substring(StringData.indexOf("RE,"), StringData.indexOf("RG,")-5);
   String RG = StringData.substring(StringData.indexOf("RG,"), StringData.indexOf("RG,")+51); 

   StaticJsonDocument<250> jsonBuffer;
   jsonBuffer["OpMODE"] = returndata(R4, 1).toInt();
   jsonBuffer["Sleeping"] = returndata(R5, 10).toInt();
   jsonBuffer["UVOzone"] = returndata(R5, 11).toInt();
   jsonBuffer["Heating"] = returndata(R5, 12).toInt();
   jsonBuffer["Auto"] = returndata(R5, 13).toInt();
   jsonBuffer["Sanitise"] = returndata(R5, 16).toInt();
   jsonBuffer["AutoSanitise"] = returndata(R7, 1).toInt();
   jsonBuffer["HeatPumpMode"] = returndata(R7, 26).toInt();
   char buffer[250];
   serializeJson(jsonBuffer, buffer);
   Serial.println(buffer);
   client.publish(MQTTBaseTopic+"/Operations", buffer);

   StaticJsonDocument<250> jsonBuffer2;
   LightsONOFF = returndata(R5, 14).toInt();
   LightsBrightness = returndata(R6, 2).toInt();
   LightsColour = returndata(R6, 3).toInt();
   LightsMode = returndata(R6, 4).toInt();
   LightsEffectSpeed = returndata(R6, 5).toInt();
   jsonBuffer2["Lights"] = LightsONOFF;
   jsonBuffer2["LightsBrightness"] = LightsBrightness;
   jsonBuffer2["LightsColour"] = LightsColour;
   jsonBuffer2["LightsMode"] = LightsMode;
   jsonBuffer2["LightsSpeed"] = LightsEffectSpeed;
   char buffer2[250];
   serializeJson(jsonBuffer2, buffer2);
   Serial.println(buffer2);
   client.publish(MQTTBaseTopic+"/Lights", buffer2);

   StaticJsonDocument<250> jsonBuffer3;   
   SetTemperature = returndata(R6, 8).toFloat()/10;
   CurrentTemperature = returndata(R5, 15).toFloat()/10;
   jsonBuffer3["WaterTemp"] = CurrentTemperature;
   jsonBuffer3["SetTemp"] = SetTemperature;
   char buffer3[250];
   serializeJson(jsonBuffer3, buffer3);
   Serial.println(buffer3);
   client.publish(MQTTBaseTopic+"/Temperature", buffer3);
   
   StaticJsonDocument<250> jsonBuffer4;  
   Pump1State = returndata(R5, 18).toInt();
   Pump2State = returndata(R5, 19).toInt();
   Pump3State = returndata(R5, 20).toInt();
   Pump4State = returndata(R5, 21).toInt();
   Pump5State = returndata(R5, 22).toInt();
   BlowerSpeed = returndata(R6, 1).toInt();
   BlowerMode = returndata(RC, 10).toInt();
   jsonBuffer4["Pump1State"] = Pump1State;
   jsonBuffer4["Pump2State"] = Pump2State;
   jsonBuffer4["Pump3State"] = Pump3State;
   jsonBuffer4["Pump4State"] = Pump4State;
   jsonBuffer4["Pump5State"] = Pump5State;
   jsonBuffer4["BlowerSpeed"] = BlowerSpeed;
   jsonBuffer4["BlowerMode"] = BlowerMode;
   char buffer4[250];
   serializeJson(jsonBuffer4, buffer4);
   Serial.println(buffer4);
   client.publish(MQTTBaseTopic+"/Pumps", buffer4);
   
   StaticJsonDocument<250> jsonBuffer5;  
   jsonBuffer5["FiltrationHour"] = returndata(R6, 6).toInt();
   jsonBuffer5["FiltrationCycles"] = returndata(R6, 7).toInt();
   char buffer5[250];
   serializeJson(jsonBuffer5, buffer5);
   Serial.println(buffer5);
   client.publish(MQTTBaseTopic+"/Filtration", buffer5);
}


void handleRoot() {
 String cmd = server.arg("command");
 Serial.println("Command Request");
 Serial.println(cmd);
 swSer.write(cmd.c_str());
 swSer.write("\n");     //  Send Command to SPA
 Serial.println("Client Request");
 String webString = "<!DOCTYPE html><html>";    //Build WEB Page
 webString +="<head><meta http-equiv=\"refresh\" content=\"10\">";
 // CSS to style the refresh buttons
 webString +="<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}";
 webString +=".button { background-color: #195B6A; border: none; color: white; padding: 12px 30px;";
 webString +="text-decoration: none; font-size: 18px; margin: 2px; cursor: pointer;}";
 webString +=".button2 {background-color: #77878A;}</style></head>";
 webString += "<body><h2>SPANET Controller</h2>";
 webString += "<h3>Spapool Temp " + String(CurrentTemperature) + "</h3>";
 webString += "<h3>Spapool Set Temp " + String(SetTemperature) + "</h3>";
 webString += "<h3>Spapool Blower Mode ";
   if (BlowerMode == 0) {webString+="Variable";} else if (BlowerMode ==1) {webString+="Ramp";} else if (BlowerMode ==2) {webString+="Off";}
 webString+="</h3>";
 webString += "<h3>Spapool Blower Speed " + String(BlowerSpeed) + "</h3>";
 webString += "<h3>Spapool lights ON/OFF ";
   if (LightsONOFF == 0) {webString+="OFF";} else if (LightsONOFF == 1) {webString+="ON";}
 webString +="</h3>";
 webString +="<h3>Spapool Lights Mode ";
   if (LightsMode == 0) {webString+="White";} else if (LightsMode ==1) {webString+="Colour";} else if (LightsMode ==2) {webString+="Fade";} else if (LightsMode ==3) {webString+="Step";} else if (LightsMode ==4) {webString+="Party";}
 webString +="</h3>";
 webString += "<h3>Spapool Lights Brightness " + String(LightsBrightness) + "</h3>";
 webString += "<h3>Spapool Lights Effect Speed " + String(LightsEffectSpeed) + "</h3>";
 webString += "<h3>Spapool Lights Colour " + String(LightsColour) + "</h3>";

 webString +="<p><center><table><tr><th>Item</th><th>Status</th><th>Control<th></tr>";
 webString += "<tr><td><h3>Spapool Pump 1 </td><td>";
 if (Pump1State == 0) {webString+="OFF";} else if (Pump1State==1) {webString+="ON";}
 webString +="</td><td><form action=\"/\" method=\"post\"><button type=\"submit\" name=\"command\" value=\"S22:1\" class=\"btn-link\">Pump 1 ON</button>";
 webString +="<button type=\"submit\" name=\"command\" value=\"S22:0\" class=\"btn-link\">Pump 1 OFF</button>";
 webString +="<button type=\"submit\" name=\"command\" value=\"S22:4\" class=\"btn-link\">Pump 1 AUTO</button></form>";
 webString += "</td></h3></tr>";

 webString += "<tr><td><h3>Spapool Pump 2 </td><td>";
 if (Pump2State == 0) {webString+="OFF";} else if (Pump2State==1) {webString+="ON";} else if (Pump2State==4) {webString+="AUTO";}
 webString +="</td><td><form action=\"/\" method=\"post\"><button type=\"submit\" name=\"command\" value=\"S23:1\" class=\"btn-link\">Pump 2 ON</button>";
 webString +="<button type=\"submit\" name=\"command\" value=\"S23:0\" class=\"btn-link\">Pump 2 OFF</button>";
 webString +="<button type=\"submit\" name=\"command\" value=\"S23:4\" class=\"btn-link\">Pump 2 AUTO</button></form>";
 webString += "</td></tr></h3>";
 webString += "<tr><td><h3>Spapool Pump 3 </td><td>";
 if (Pump3State == 0) {webString+="OFF";} else if (Pump3State ==1) {webString+="ON";}
 webString +="</td><td><form action=\"/\" method=\"post\"><button type=\"submit\" name=\"command\" value=\"S24:1\" class=\"btn-link\">Pump 3 ON</button>";
 webString +="<button type=\"submit\" name=\"command\" value=\"S24:0\" class=\"btn-link\">Pump 3 OFF</button></form>";
 webString += "</td></tr></h3>";

 webString += "<tr><td><h3>Spapool Pump 4 </td><td>";
 if (Pump4State == 0) {webString+="OFF";} else if (Pump4State ==1) {webString+="ON";}
 webString +="</td><td><form action=\"/\" method=\"post\"><button type=\"submit\" name=\"command\" value=\"S25:1\" class=\"btn-link\">Pump 4 ON</button>";
 webString +="<button type=\"submit\" name=\"command\" value=\"S25:0\" class=\"btn-link\">Pump 4 OFF</button></form>";
 webString += "</td></tr></h3>";

 webString += "<tr><td><h3>Spapool Pump 5 </td><td>";
 if (Pump5State == 0) {webString+="OFF";} else if (Pump5State ==1) {webString+="ON";}
 webString +="</td><td><form action=\"/\" method=\"post\"><button type=\"submit\" name=\"command\" value=\"S26:1\" class=\"btn-link\">Pump 5 ON</button>";
 webString +="<button type=\"submit\" name=\"command\" value=\"S26:0\" class=\"btn-link\">Pump 5 OFF</button></form>";
 webString += "</td></tr></h3>";

 webString += "<tr><td><h3>Spapool Lights ON/OFF </td><td>";
 if (LightsONOFF == 0) {webString+="OFF";} else if (LightsONOFF ==1) {webString+="ON";}
 webString +="</td><td><form action=\"/\" method=\"post\"><button type=\"submit\" name=\"command\" value=\"W14\" class=\"btn-link\">Toggle</button>";
 webString +="<button type=\"submit\" name=\"command\" value=\"S11\" class=\"btn-link\">Lights OFF</button></form>";
 webString += "</td></tr></h3>";

 webString += "<tr><td><h3>Spapool Blower Mode </td><td>";
 if (BlowerMode == 0) {webString+="Variable";} else if (BlowerMode==1) {webString+="Ramp";} else if (BlowerMode==2) {webString+="OFF";}
 webString +="</td><td><form action=\"/\" method=\"post\"><button type=\"submit\" name=\"command\" value=\"S28:1\" class=\"btn-link\">Blower Variable</button>";
 webString +="<button type=\"submit\" name=\"command\" value=\"S28:1\" class=\"btn-link\">Blower Ramp</button>";
 webString +="<button type=\"submit\" name=\"command\" value=\"S28:2\" class=\"btn-link\">Blower OFF</button></form>";
 webString += "</td></tr></h3>";
 
 webString += "<tr><td><h3>Spapool Blower Speed </td><td>" + String(BlowerSpeed);
 webString +="</td><td><form action=\"/\" method=\"post\"><button type=\"submit\" name=\"command\" value=\"S13:1\" class=\"btn-link\">1</button>";
 webString +="<button type=\"submit\" name=\"command\" value=\"S13:2\" class=\"btn-link\">2</button><button type=\"submit\" name=\"command\" value=\"S13:3\" class=\"btn-link\">3</button>";
 webString +="<button type=\"submit\" name=\"command\" value=\"S13:4\" class=\"btn-link\">4</button><button type=\"submit\" name=\"command\" value=\"S13:5\" class=\"btn-link\">5</button></form>";
 webString += "</td></tr></h3>";
 webString += "</table></centre></p>";

 webString += "<form action=\"/\" method=\"post\">Send RAW Command: <input type=\"text\" name=\"command\">";
 webString += "<input type=\"submit\" value=\"Submit\">";
 webString += "</form>"; 
 
 ip = WiFi.localIP();
 webString += "<br><a href=\"http://" + ip.toString() + "\"><button class=\"button button2\">Refresh</button></a>";

 webString += "<br><p> To send RAW commands to the SPANET controller send in this format. HTTP://";
 webString += ip.toString();
 webString += "/?command=XXX:X </p>";
 webString += "<style>table, th, td {border: 1px solid black;}</style>";
 webString += "<p><center><table><tr><td>Function</td><td>Command</td><td>Detail</td></tr>";
 webString += "<tr><td>Control Pump1</td><td>S22:n</td><td>0 - Off, 1 - On, 4 - Auto</td></tr>";
 webString += "<tr><td>Control Pump2</td><td>S23:n</td><td>0 - Off, 1 - On, 4 - Auto</td></tr>";
 webString += "<tr><td>Control Pump3</td><td>S24:n</td><td>0 - Off, 1 - On</td></tr>";
 webString += "<tr><td>Control Pump4</td><td>S25:n</td><td>0 - Off, 1 - On</td></tr>";
 webString += "<tr><td>Control Pump5</td><td>S26:n</td><td>0 - Off, 1 - On</td></tr>";
 webString += "<tr><td>Control Blower</td><td>S28:n</td><td>0 - Variable, 1 - Ramp, 2 - Off</td></tr>";
 webString += "<tr><td>Variable Speed of Blower</td><td>S13:n</td><td>Speed 1 - 5</td></tr>";
 webString += "<tr><td>Toggle State of Lights</td><td>W14</td><td>No Options</td></tr>";
 webString += "<tr><td>Turn Lights Off</td><td>S11</td><td>No Options</td></tr>";
 webString += "<tr><td>Lights Mode</td><td>S07:n</td><td>0 - White, 1 - Colour, 2 - Fade, 3 - Step, 4 - Party</td></tr>";
 webString += "<tr><td>Lights Brightness</td><td>S08:n</td><td>Brightness 1 - 5</td></tr>";
 webString += "<tr><td>Lights Effect Speed</td><td>S09:n</td><td>Speed 1 - 5</td></tr>";
 webString += "<tr><td>Lights Colour</td><td>S10:n</td><td>Colour 0-30</td></tr>";
 webString += "<tr><td>Set Spa Temperature</td><td>W40:nnn</td><td> temperature in celsius * 10 between 5.0-41.0c (ie: for 35.6c, nnn = 356)</td></tr>";
 webString += "</table></center></p>";

 webString += "<br>For a full list of commands see <a href=\"https://github.com/BlaT2512/spanet-api/blob/main/spanet.md\">https://github.com/BlaT2512/spanet-api/blob/main/spanet.md</a>";

 webString += "</body></html>";           
 server.send(200, "text/html", webString); //Send web page
}


void setup() {
  Serial.begin(115200);
  Serial.println("Loading Serial");
  swSer.begin(BAUD_RATE, SWSERIAL_8N1, D5, D6, false, 95, 11);  //Start Software Serial
  Serial.println("\nSoftware serial test started");
  Serial.println("\nWeb Server Starting");
  server.on("/", handleRoot);
  server.begin();  //start WebServer
  startMillis = millis();  //initial start time
  }


void onConnectionEstablished() {
  client.subscribe(MQTT_SERIAL_RECEIVER_CH, [] (const String &payload)  {
    Serial.println(payload);
    swSer.write(payload.c_str());   //Send MQTT RAW Commands
    swSer.write("\n");
  });
  Serial.println("Connected to MQTT Server");
  Serial.print("Local IP: ");
  Serial.print(WiFi.localIP());
}


void loop() {
  client.loop();  //MQTT Client Process
  server.handleClient();  //WEBServer Process
  currentMillis = millis();
  if (currentMillis - startMillis >= period)
  {
    swSer.write("RF\n"); //Refresh SPA Data every 10 seconds
    startMillis = currentMillis;
  }
  
  if (swSer.available() >0) {   //  Get data from SPA
    String inString = swSer.readString();
    Serial.println(inString);
    if (inString.startsWith("RF:"))
    {convertpayload(inString);
    }
    else
    inString.trim();
    inString = "{\"spanetdata\":{\"Response\": \""+ inString +"\"}}";
    {client.publish(MQTT_SERIAL_PUBLISH_CH, inString);}
  }

}
