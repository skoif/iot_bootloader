#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <EEPROM.h>
#include <ESP8266Ping.h>
ESP8266WebServer server(80);
SocketIOClient client;
/*  PINS  */
#define led_green 12
#define led_red 15
#define led_blue 13
#define led_indicator 2
#define h_button 4
/*  BOOTLOADER VARS  */
const char* bl_wifi_ap_ssid = "ESP_MODULE";
const char* bl_wifi_ap_pass = "phygital";
String bl_wifi_ssid;
String bl_wifi_pass;
String bl_wifi_serv;
String bl_meshes;
bool bl_afterstart_state = true;
/*  BOOTLOADER FUNCS  */
bool testWifi(void) {
  int c = 0;
  while ( c < 20 ) {
    if (WiFi.status() == WL_CONNECTED) { return true; } 
    delay(500);
    Serial.print(".");    
    c++;
  }
  return false;
}
void bootloader(){
  pinMode(h_button, INPUT);
  WiFi.disconnect(true);
  delay(3000);
  EEPROM.begin(512);
  Serial.begin(115200);
  Serial.println("[BOOTLOADER] Welcome");
  if(!digitalRead(h_button)){
    Serial.print("[BOOTLOADER] clearing EEPROM");
    for (int i = 0; i < 512; ++i) { EEPROM.write(i, 0); Serial.print("."); }
    Serial.println("Done");
    EEPROM.commit();
    EEPROM.end();
    delay(500);
    ESP.eraseConfig();
    delay(500);
    ESP.reset();
  }
  Serial.print("[BOOTLOADER] Reading EEPROM...");
  for (int i = 0; i < 32; ++i){
    bl_wifi_ssid += char(EEPROM.read(i));
  }
  for (int i = 32; i < 96; ++i){
    bl_wifi_pass += char(EEPROM.read(i));
  }
  for (int i = 96; i < 112; ++i){
    bl_wifi_serv += char(EEPROM.read(i));
  }
  Serial.println("done");
  Serial.print("[BOOTLOADER] SSID: ");
  Serial.print(bl_wifi_ssid);
  Serial.print("  PASS: ");
  Serial.print(bl_wifi_pass);
  Serial.print("  SERV: ");
  Serial.println(bl_wifi_serv);
  if(bl_wifi_ssid == ""){
    bl_afterstart_state = false;
    Serial.println("[BOOTLOADER] NO WIFI SSID! Starting AP mode");
    WiFi.mode(WIFI_AP);
    WiFi.disconnect();
    delay(1000);
    Serial.print("[BOOTLOADER] Scanning networks....");
    int n = WiFi.scanNetworks();
    Serial.println("done");
    Serial.print("[BOOTLOADER] ");
    Serial.print(n);
    Serial.println(" networks found");
    if (n == 0){
      bl_meshes = "No networks";
    }else{
      bl_meshes = "<ol>";
      for (int i = 0; i < n; ++i){
        bl_meshes += "<li>";
        bl_meshes += WiFi.SSID(i);
        Serial.print(WiFi.SSID(i));
        Serial.print(" | ");
        bl_meshes += " (";
        bl_meshes += WiFi.RSSI(i);
        Serial.print(WiFi.RSSI(i));
        Serial.print(" | ");
        bl_meshes += ")";
        bl_meshes += (WiFi.encryptionType(i) == ENC_TYPE_NONE)?" ":"*";
        Serial.println((WiFi.encryptionType(i) == ENC_TYPE_NONE)?" ":"*");
        bl_meshes += "</li>";
      }
    }
    Serial.println("[BOOTLOADER] Setting up WiFi AP");
    WiFi.softAP(bl_wifi_ap_ssid, bl_wifi_ap_pass);
    Serial.println("[BOOTLOADER] AP Setup: ok");
    Serial.print("[BOOTLOADER] AP SSID: ");
    Serial.println(bl_wifi_ap_ssid);
    Serial.print("[BOOTLOADER] AP PASS: ");
    Serial.println(bl_wifi_ap_pass);
    Serial.print("[BOOTLOADER] AP IP: ");
    Serial.println(WiFi.softAPIP());
    Serial.print("[BOOTLOADER] Configuring web...");
    server.on("/", []() {
      String content = "<!DOCTYPE HTML>\r\n<html>";
      content += "<p>";
      content += bl_meshes;
      content += "</p><form method='get' action='setting'><label>SSID: </label><input name='ssid' length=32><br><label>PASS: </label><input name='pass' length=64><br><label>SERVER IP: </label><input name='server' length=64><br><input type='submit'></form>";
      content += "</html>";
      server.send(200, "text/html", content);
    });
    server.on("/setting", []() {
      String qssid = server.arg("ssid");
      String qpass = server.arg("pass");
      String qserv = server.arg("server");
      if (qssid.length() > 0 && qpass.length() > 0 && qserv.length() > 0) {
        Serial.println("[BOOTLOADER] GOT SETTINGS");
        Serial.print("[BOOTLOADER] clearing EEPROM");
        for (int i = 0; i < 512; ++i) { EEPROM.write(i, 0); Serial.print("."); }
        Serial.println("Done");
        Serial.print("[BOOTLOADER] Flashing EEPROM");
        for (int i = 0; i < qssid.length(); ++i){
          EEPROM.write(i, qssid[i]);
          Serial.print(".");
        }
        Serial.print("|");
        for (int i = 0; i < qpass.length(); ++i){
          EEPROM.write(32+i, qpass[i]);
          Serial.print("*");
        }
        Serial.print("|");
        for (int i = 0; i < qserv.length(); ++i){
          EEPROM.write(96+i, qserv[i]);
          Serial.print(".");
        }
        EEPROM.commit();
        Serial.println("Done");
      }
      server.send(200, "text/html", "ok");
      WiFi.disconnect(true);
      delay(3000);
      Serial.println("[BOOTLOADER] restarting...");
      EEPROM.end();
      delay(500);
      ESP.eraseConfig();
      delay(500);
      ESP.reset();
    });
    server.begin();
    Serial.println("Done");
  }else{
    WiFi.mode(WIFI_STA);
    Serial.print("[BOOTLOADER] Connecting to WiFi");
    WiFi.begin(bl_wifi_ssid.c_str(), bl_wifi_pass.c_str());
    if(!testWifi()){
      Serial.println("Timeout");
      Serial.print("[BOOTLOADER] clearing EEPROM");
      for (int i = 0; i < 512; ++i) { EEPROM.write(i, 0); Serial.print("."); }
      EEPROM.commit();
      Serial.println("Done");
      WiFi.disconnect(true);
      delay(3000);
      Serial.println("[BOOTLOADER] restarting...");
      EEPROM.end();
      delay(500);
      ESP.eraseConfig();
      delay(500);
      ESP.reset();
    }else{
      Serial.println("OK");
      Serial.print("[BOOTLOADER] Local IP:");
      Serial.println(WiFi.localIP());
      if(Ping.ping(bl_wifi_serv.c_str())){
        Serial.println("[BOOTLOADER] Ping: OK");
      }else{
        Serial.println("[BOOTLOADER] Ping: ERROR");
        Serial.print("[BOOTLOADER] clearing EEPROM");
        for (int i = 0; i < 512; ++i) { EEPROM.write(i, 0); Serial.print("."); }
        EEPROM.commit();
        Serial.println("Done");
        WiFi.disconnect(true);
        delay(3000);
        Serial.println("[BOOTLOADER] restarting...");
        EEPROM.end();
        delay(500);
        ESP.eraseConfig();
        delay(500);
        ESP.reset();
      }
    }
  }
  Serial.println("[BOOTLOADER] Boot: OK");
  Serial.print("[BOOTLOADER] CPU Cycles since start: ");
  Serial.println(ESP.getCycleCount());
  Serial.print("[BOOTLOADER] Free Memory: ");
  Serial.println(ESP.getFreeHeap());
  Serial.print("[BOOTLOADER] Chip ID: ");
  Serial.println(ESP.getChipId());
  Serial.print("[BOOTLOADER] Flash Chip ID: ");
  Serial.println(ESP.getFlashChipId());
  Serial.print("[BOOTLOADER] Flash Chip Size: ");
  Serial.println(ESP.getFlashChipSize());
  Serial.print("[BOOTLOADER] All jobs done in ");
  Serial.print(millis());
  Serial.println(" milliseconds");
}

void setup(){
  bootloader();
  if(bl_afterstart_state){
    /* Fired if normal start */
  }else{
    /* Fired if AP mode */
  }
}

void loop(){
  if(bl_afterstart_state){
    /* Fired if normal start */
  }else{
    server.handleClient();
    /* Fired if AP mode */
  }
}
