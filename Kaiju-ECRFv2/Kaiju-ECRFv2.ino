#include "ELECHOUSE_CC1101_SRC_DRV.h"
#include <WiFiClient.h> 
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
//#include <AsyncElegantOTA.h>
#define DEST_FS_USES_SD
//#include <ESP32-targz.h>
#include <SPIFFSEditor.h>
#include <EEPROM.h>
#include "SPIFFS.h"
#include "SPI.h"
#include <WiFiAP.h>
#include "FS.h"
#include "SD.h"
#include <HTTPClient.h>
#include <ArduinoJson.h>

#define eepromsize 4096
#define samplesize 2000

#define SD_SCLK 18
#define SD_MISO 19
#define SD_MOSI 23
#define SD_SS   22

SPIClass sdspi(VSPI);

#if defined(ESP8266)
    #define RECEIVE_ATTR ICACHE_RAM_ATTR
#elif defined(ESP32)
    #define RECEIVE_ATTR IRAM_ATTR
#else
    #define RECEIVE_ATTR
#endif

// Config SSID, password and channel
const char* ssid = "testing";  // Enter your SSID here
const char* password = "123456789";  //Enter your Password here
const int wifi_channel = 12; //Enter your preferred Wi-Fi Channel

// HTML and CSS style
//const String MENU = "<body><p>Evil Crow RF v1.0</p><div id=\"header\"><body><nav id='menu'><input type='checkbox' id='responsive-menu' onclick='updatemenu()'><label></label><ul><li><a href='/'>Home</a></li><li><a class='dropdown-arrow'>Config</a><ul class='sub-menus'><li><a href='/txconfig'>RAW TX Config</a></li><li><a href='/txbinary'>Binary TX Config</a></li><li><a href='/rxconfig'>RAW RX Config</a></li><li><a href='/btnconfig'>Button TX Config</a></li></ul></li><li><a class='dropdown-arrow'>RX Log</a><ul class='sub-menus'><li><a href='/viewlog'>RX Logs</a></li><li><a href='/delete'>Delete Logs</a></li><li><a href='/downloadlog'>Download Logs</a></li><li><a href='/cleanspiffs'>Clean SPIFFS</a></li></ul></li><li><a class='dropdown-arrow'>URH Protocol</a><ul class='sub-menus'><li><a href='/txprotocol'>TX Protocol</a></li><li><a href='/listxmlfiles'>List Protocol</a></li><li><a href='/uploadxmlfiles'>Upload Protocol</a></li><li><a href='/cleanspiffs'>Clean SPIFFS</a></li></ul></li><li><a href='/jammer'>Simple Jammer</a></li><li><a href='/update'>OTA Update</a></li></ul></nav><br></div>";
const String HTML_CSS_STYLING = "<html><head><meta charset=\"utf-8\"><title>Evil Crow RF</title><link rel=\"stylesheet\" href=\"style.css\"><script src=\"lib.js\"></script></head>";
const String HTML_ANALYSIS = "<body><body><nav id='menu'><input type='checkbox' id='responsive-menu' onclick='updatemenu()'><label></label><ul><li><a href='/'>Home</a></li><li><a class='dropdown-arrow'>Config</a><ul class='sub-menus'><li><a href='/txconfig'>RAW TX Config</a></li><li><a href='/txbinary'>Binary TX Config</a></li><li><a href='/rxconfig'>RX Config</a></li><li><a href='/btnconfig'>Button TX Config</a></li><li><a href='/jammer'>Simple Jammer</a></li></ul></li><li><a class='dropdown-arrow'>RX Log</a><ul class='sub-menus'><li><a href='/viewlog'>RX Logs</a></li><li><a href='/delete'>Delete Logs</a></li><li><a href='/downloadlog'>Download Logs</a></li><li><a href='/cleanspiffs'>Clean SPIFFS</a></li></ul></li><li><a class='dropdown-arrow'>URH Protocol</a><ul class='sub-menus'><li><a href='/txprotocol'>TX Protocol</a></li><li><a href='/listxmlfiles'>List Protocol</a></li><li><a href='/uploadxmlfiles'>Upload Protocol</a></li></ul></li><li><a class='dropdown-arrow'>Tesla Charge</a><ul class='sub-menus'><li><a href='/txtesla'>TX</a></li><li><a href='/btnconfigtesla'>Button TX Config</a></li></ul></li><li><a class='dropdown-arrow'>Kaiju</a><ul class='sub-menus'><li><a href='/kaijulogin'>Login</a></li><li><a href='/kaijuanalyzer'>Analyzer</a></li></ul></li><li><a class='dropdown-arrow'>ECRF Config</a><ul class='sub-menus'><li><a href='/wificonfig'>WiFi Config</a></li><li><a href='/update'>OTA Firmware</a></li><li><a href='/updatesd'>OTA SD Files</a></li></ul></li></ul></nav>";

//Pushbutton Pins
int push1 = 34;
int push2 = 35;

int led = 32;
static unsigned long Blinktime = 0;

int error_toleranz = 200;

int RXPin = 26;
int RXPin0 = 4;
int TXPin0 = 2;
int Gdo0 = 25;
const int minsample = 30;
unsigned long sample[samplesize];
unsigned long samplesmooth[samplesize];
int samplecount;
static unsigned long lastTime = 0;
String transmit = "";
long data_to_send[2000];
long data_button1[2000];
long data_button2[2000];
long data_button3[2000];
long transmit_push[2000];
String tmp_module;
String tmp_frequency;
String tmp_xmlname;
String tmp_codelen;
String tmp_setrxbw;
String tmp_mod;
int mod;
String tmp_deviation;
float deviation;
String tmp_datarate;
String tmp_powerjammer;
int power_jammer;
int datarate;
float frequency;
float setrxbw;
String raw_rx = "0";
String jammer_tx = "0";
const bool formatOnFail = true;
String webString;
String bindata;
int samplepulse;
String tmp_samplepulse;
String tmp_transmissions;
int counter=0;
int pos = 0;
int transmissions;
int pushbutton1 = 0;
int pushbutton2 = 0;
byte jammer[11] = {0xff,0xff,};
String bindataprotocol;
String bindata_protocol;

// Wi-Fi config storage
int storage_status;
String tmp_config1;
String tmp_config2;
String config_wifi;
String ssid_new;
String password_new;
String tmp_channel_new;
String tmp_mode_new;
int channel_new;
int mode_new;

// Kaiju
String email;
String otp;
String token;
char json[255];
char json2[2000];
String id_task;
String new_idtask;
String buttonrolling_tmp;
String numrolling_tmp;
int buttonrolling;
int numrolling;
String token2;
String new_token;
String api_email = "https://rolling.pandwarf.com/api/v1/auth/email/";
String api_otp = "https://rolling.pandwarf.com/api/v1/auth/token/";
String api_analyzer = "https://rolling.pandwarf.com/api/v1/analyze/detailed";
String api_status_task = "https://rolling.pandwarf.com/api/v1/task/";
String api_remote = "https://rolling.pandwarf.com/api/v1/remote/";
String api_addrollingcode = "https://rolling.pandwarf.com/api/v1/add/rolling-code";
String api_addrollingcode_task = "https://rolling.pandwarf.com/api/v1/task/";

const char* root_ca= \
"-----BEGIN CERTIFICATE-----\n" \
"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\n" \
"-----END CERTIFICATE-----\n";

// File
File logs;
File file;

AsyncWebServer controlserver(80);

void readConfigWiFi(fs::FS &fs, String path){
  File file = fs.open(path);
  
  if(!file || file.isDirectory()){
    storage_status = 0;
    return;
  }
  
  while(file.available()){
    config_wifi = file.readString();
    int file_len = config_wifi.length()+1;
    int index_config = config_wifi.indexOf('\n');
    tmp_config1 = config_wifi.substring(0, index_config-1);
    
    tmp_config2 = config_wifi.substring(index_config+1, file_len-3);
    storage_status = 1;
  }
  file.close();
}

void writeConfigWiFi(fs::FS &fs, const char * path, String message){
    File file = fs.open(path, FILE_APPEND);
    
    if(!file){
      return;
    }
    
    if(file.println(message)){
    } else {
    }
    file.close();
}

// handles uploads
void handleUpload(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final) {
  String logmessage = "Client:" + request->client()->remoteIP().toString() + " " + request->url();

  if (!index) {
    logmessage = "Upload Start: " + String(filename);
    request->_tempFile = SD.open("/URH/" + filename, "w");
  }

  if (len) {
    request->_tempFile.write(data, len);
    logmessage = "Writing file: " + String(filename) + " index=" + String(index) + " len=" + String(len);
  }

  if (final) {
    logmessage = "Upload Complete: " + String(filename) + ",size: " + String(index + len);
    request->_tempFile.close();
    request->redirect("/");
  }
}

void listDir(fs::FS &fs, const char * dirname, uint8_t levels){
  deleteFile(SD, "/dir.txt");

  File root = fs.open(dirname);
  if(!root){
    return;
  }
  if(!root.isDirectory()){
    return;
  }

  File file = root.openNextFile();
  while(file){
    if(file.isDirectory()){
      appendFile(SD, "/dir.txt","  DIR : ", file.name());
      if(levels){
        listDir(fs, file.name(), levels -1);
      }
    } else {
      appendFile(SD, "/dir.txt","", "<br>");
      appendFile(SD, "/dir.txt","", file.name());
      appendFile(SD, "/dir.txt","  SIZE: ", "");
      appendFileLong(SD, "/dir.txt",file.size());
    }
    file = root.openNextFile();
  }
}

void appendFile(fs::FS &fs, const char * path, const char * message, String messagestring){

  logs = fs.open(path, FILE_APPEND);
  if(!logs){
    //Serial.println("Failed to open file for appending");
    return;
  }
  if(logs.print(message)|logs.print(messagestring)){
    //Serial.println("Message appended");
  } else {
    //Serial.println("Append failed");
  }
  logs.close();
}

void appendFileLong(fs::FS &fs, const char * path, unsigned long messagechar){
  //Serial.printf("Appending to file: %s\n", path);

  logs = fs.open(path, FILE_APPEND);
  if(!logs){
    //Serial.println("Failed to open file for appending");
    return;
  }
  if(logs.print(messagechar)){
    //Serial.println("Message appended");
  } else {
    //Serial.println("Append failed");
  }
  logs.close();
}

void deleteFile(fs::FS &fs, const char * path){
  //Serial.printf("Deleting file: %s\n", path);
  if(fs.remove(path)){
    //Serial.println("File deleted");
  } else {
    //Serial.println("Delete failed");
  }
}

void readFile(fs::FS &fs, String path){
  //Serial.printf("Reading file: %s\n", path);

  File file = fs.open(path);
  if(!file){
    //Serial.println("Failed to open file for reading");
    return;
  }

  //Serial.print("Read from file: ");
  while(file.available()){
    bindataprotocol = file.readString();
    //Serial.println("");
    //Serial.println(bindataprotocol);
  }
  file.close();
}


void removeDir(fs::FS &fs, const char * dirname){

  File root = fs.open(dirname);
  if(!root){
    //Serial.println("Failed to open directory");
    return;
  }
  if(!root.isDirectory()){
    //Serial.println("Not a directory");
    return;
  }

  File file = root.openNextFile();
  while(file){
    //Serial.print(file.name());
    //Serial.println("");
    
    if(file.isDirectory()){
      deleteFile(SD, file.name());
    } else {
      deleteFile(SD, file.name());
    }
    file = root.openNextFile();
  }
}

bool checkReceived(void){
  
  delay(1);
  if (samplecount >= minsample && micros()-lastTime >100000){
    detachInterrupt(RXPin0);
    detachInterrupt(RXPin);
    return 1;
  }else{
    return 0;
  }
}

void printReceived(){
  
  Serial.print("Count=");
  Serial.println(samplecount);
  appendFile(SD, "/logs.txt", NULL, "<br>\n");
  appendFile(SD, "/logs.txt", NULL, "Count=");
  appendFileLong(SD, "/logs.txt", samplecount);
  appendFile(SD, "/logs.txt", NULL, "<br>");
  
  for (int i = 1; i<samplecount; i++){
    Serial.print(sample[i]);
    Serial.print(",");
    appendFileLong(SD, "/logs.txt", sample[i]);
    appendFile(SD, "/logs.txt", NULL, ",");  
  }
  Serial.println();
  Serial.println();
  appendFile(SD, "/logs.txt", "<br>\n", "<br>\n");
  appendFile(SD, "/logs.txt", "\n", "\n");
}

void RECEIVE_ATTR receiver() {
  const long time = micros();
  const unsigned int duration = time - lastTime;

  if (duration > 100000){
    samplecount = 0;
  }

  if (duration >= 100){
    sample[samplecount++] = duration;
  }

  if (samplecount>=samplesize){
    detachInterrupt(RXPin0);
    detachInterrupt(RXPin);
    checkReceived();
  }

  if (mod == 0 && tmp_module == "2") {
    if (samplecount == 1 and digitalRead(RXPin) != HIGH){
      samplecount = 0;
    }
  }
  
  lastTime = time;
}

void enableReceive(){
  pinMode(RXPin0,INPUT);
  RXPin0 = digitalPinToInterrupt(RXPin0);
  ELECHOUSE_cc1101.SetRx();
  samplecount = 0;
  attachInterrupt(RXPin0, receiver, CHANGE);
  pinMode(RXPin,INPUT);
  RXPin = digitalPinToInterrupt(RXPin);
  ELECHOUSE_cc1101.SetRx();
  samplecount = 0;
  attachInterrupt(RXPin, receiver, CHANGE);
}

void setup() {

  Serial.begin(38400);

  SPIFFS.begin(formatOnFail);
 
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  
  delay(2000);
  
  sdspi.begin(18, 19, 23, 22);
  SD.begin(22, sdspi);
  pinMode(push1, INPUT);
  pinMode(push2, INPUT);
  HTTPClient http;

  controlserver.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(SD, "/HTML/index.html", "text/html");
  });

  controlserver.on("/rxconfig", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(SD, "/HTML/rxconfig.html", "text/html");
  });

  controlserver.on("/txbinary", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(SD, "/HTML/txbinary.html", "text/html");
  });

  controlserver.on("/kaijugenerator", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(SD, "/HTML/kaijugenerator.html", "text/html");
  });

  controlserver.on("/kaijulogin", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(SD, "/HTML/kaijulogin.html", "text/html");
  });

  controlserver.on("/kaijuanalyzer", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(SD, "/HTML/kaijuanalyzer.html", "text/html");
  });

  controlserver.on("/remoteinfo", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(SD, "/HTML/remote.html", "text/html");
  });

  controlserver.on("/remoterolling", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(SD, "/HTML/rollingcodes.html", "text/html");
  });
  
  controlserver.on("/settxbinary", HTTP_POST, [](AsyncWebServerRequest *request){
    raw_rx = "0";
    tmp_module = request->arg("module");
    tmp_frequency = request->arg("frequency");
    bindata = request->arg("binarydata");
    tmp_deviation = request->arg("deviation");
    tmp_mod = request->arg("mod");
    tmp_samplepulse = request->arg("samplepulse");
    tmp_transmissions = request->arg("transmissions");

    if (request->hasArg("configmodule")) {
      int counter=0;
      int pos = 0;
      frequency = tmp_frequency.toFloat();
      deviation = tmp_deviation.toFloat();
      mod = tmp_mod.toInt();
      samplepulse = tmp_samplepulse.toInt();
      transmissions = tmp_transmissions.toInt();

      for (int i=0; i<1000; i++){
        data_to_send[i]=0;
      }

      bindata.replace(" ","");
      bindata.replace("\n","");
      bindata.replace("Pause:","");
      int count_binconvert=0;
      String lastbit_convert="1";
      Serial.println("");
      Serial.println(bindata);

      for (int i = 0; i<bindata.length()+1; i++){
        if (lastbit_convert != bindata.substring(i, i+1)){
          if (lastbit_convert == "1"){
            lastbit_convert="0";
          }else if (lastbit_convert == "0"){
            lastbit_convert="1";
          }
          count_binconvert++;
        }
    
        if (bindata.substring(i, i+1)=="["){
          data_to_send[count_binconvert]= bindata.substring(i+1,bindata.indexOf("]",i)).toInt();
          lastbit_convert="0";
          i+= bindata.substring(i,bindata.indexOf("]",i)).length();
        }else{
          data_to_send[count_binconvert]+=samplepulse;
        }
      }

      for (int i = 0; i<count_binconvert; i++){
        Serial.print(data_to_send[i]);
        Serial.print(",");
      }

      if (tmp_module == "1") {
        pinMode(2,OUTPUT);
        ELECHOUSE_cc1101.setModul(0);
        ELECHOUSE_cc1101.Init();
        ELECHOUSE_cc1101.setModulation(mod);
        ELECHOUSE_cc1101.setMHZ(frequency);
        ELECHOUSE_cc1101.setDeviation(deviation);
        //delay(400);
        ELECHOUSE_cc1101.SetTx();

        delay(1000);

        for (int r = 0; r<transmissions; r++) {
          for (int i = 0; i<count_binconvert; i+=2){
            digitalWrite(2,HIGH);
            delayMicroseconds(data_to_send[i]);
            digitalWrite(2,LOW);
            delayMicroseconds(data_to_send[i+1]);
          }
          delay(2000); //Set this for the delay between retransmissions    
        }
      }

      else if (tmp_module == "2") {
        pinMode(25,OUTPUT);
        ELECHOUSE_cc1101.setModul(1);
        ELECHOUSE_cc1101.Init();
        ELECHOUSE_cc1101.setModulation(mod);
        ELECHOUSE_cc1101.setMHZ(frequency);
        ELECHOUSE_cc1101.setDeviation(deviation);
        //delay(400);
        ELECHOUSE_cc1101.SetTx();  

        delay(1000);

        for (int r = 0; r<transmissions; r++) {
          for (int i = 0; i<count_binconvert; i+=2){
            digitalWrite(25,HIGH);
            delayMicroseconds(data_to_send[i]);
            digitalWrite(25,LOW);
            delayMicroseconds(data_to_send[i+1]);
          }
          delay(2000); //Set this for the delay between retransmissions    
        }
      }
      request->send(200, "text/html", HTML_CSS_STYLING + "<script>alert(\"Signal has been transmitted\")</script>");
      ELECHOUSE_cc1101.setSidle();
      //sdspi.end();
      //sdspi.begin(18, 19, 23, 22);
      //SD.begin(22, sdspi);
    }
  });

  controlserver.on("/setrx", HTTP_POST, [](AsyncWebServerRequest *request){
    tmp_module = request->arg("module");
    //Serial.print("Module: ");
    //Serial.println(tmp_module);
    tmp_frequency = request->arg("frequency");
    tmp_setrxbw = request->arg("setrxbw");
    tmp_mod = request->arg("mod");
    tmp_deviation = request->arg("deviation");
    tmp_datarate = request->arg("datarate");
    if (request->hasArg("configmodule")) {
      frequency = tmp_frequency.toFloat();
      setrxbw = tmp_setrxbw.toFloat();
      mod = tmp_mod.toInt();
      //Serial.print("Modulation: ");
      //Serial.println(mod);
      deviation = tmp_deviation.toFloat();
      datarate = tmp_datarate.toInt();

      if (tmp_module == "1") {
        ELECHOUSE_cc1101.setModul(0);
        ELECHOUSE_cc1101.Init();
      }

      else if (tmp_module == "2") {
        ELECHOUSE_cc1101.setModul(1);
        ELECHOUSE_cc1101.Init();

        if(mod == 2) {
          ELECHOUSE_cc1101.setDcFilterOff(0);
        }

        if(mod == 0) {
          ELECHOUSE_cc1101.setDcFilterOff(1);
        }
        //Serial.println("Module 2");
      }

      ELECHOUSE_cc1101.setSyncMode(0);        // Combined sync-word qualifier mode. 0 = No preamble/sync. 1 = 16 sync word bits detected. 2 = 16/16 sync word bits detected. 3 = 30/32 sync word bits detected. 4 = No preamble/sync, carrier-sense above threshold. 5 = 15/16 + carrier-sense above threshold. 6 = 16/16 + carrier-sense above threshold. 7 = 30/32 + carrier-sense above threshold.
      ELECHOUSE_cc1101.setPktFormat(3);       // Format of RX and TX data. 0 = Normal mode, use FIFOs for RX and TX. 1 = Synchronous serial mode, Data in on GDO0 and data out on either of the GDOx pins. 2 = Random TX mode; sends random data using PN9 generator. Used for test. Works as normal mode, setting 0 (00), in RX. 3 = Asynchronous serial mode, Data in on GDO0 and data out on either of the GDOx pins.

      ELECHOUSE_cc1101.setModulation(mod);      // set modulation mode. 0 = 2-FSK, 1 = GFSK, 2 = ASK/OOK, 3 = 4-FSK, 4 = MSK.
      ELECHOUSE_cc1101.setRxBW(setrxbw);
      ELECHOUSE_cc1101.setMHZ(frequency);
      ELECHOUSE_cc1101.setDeviation(deviation);   // Set the Frequency deviation in kHz. Value from 1.58 to 380.85. Default is 47.60 kHz.
      ELECHOUSE_cc1101.setDRate(datarate);           // Set the Data Rate in kBaud. Value from 0.02 to 1621.83. Default is 99.97 kBaud!

      enableReceive();
      raw_rx = "1";
      //sdspi.end();
      //sdspi.begin(18, 19, 23, 22);
      //SD.begin(22, sdspi);
      request->send(200, "text/html", HTML_CSS_STYLING + "<script>alert(\"RX Config OK\")</script>");
    }
  });

  controlserver.on("/viewlog", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SD, "/logs.txt", "text/html");
  });

  controlserver.on("/cleanspiffs", HTTP_GET, [](AsyncWebServerRequest *request){
    SPIFFS.remove("/");
    request->send(200, "text/html", HTML_CSS_STYLING+ "<body onload=\"JavaScript:AutoRedirect()\">"
    "<br><h2>SPIFFS cleared!<br>You will be redirected in 5 seconds.</h2></body>" );
  });

  controlserver.on("/downloadlog", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SD, "/logs.txt", String(), true);
  });

  controlserver.on("/delete", HTTP_GET, [](AsyncWebServerRequest *request){
    deleteFile(SD, "/logs.txt");
    request->send(200, "text/html", HTML_CSS_STYLING+ "<body onload=\"JavaScript:AutoRedirect()\">"
    "<br><h2>File cleared!<br>You will be redirected in 5 seconds.</h2></body>" );
    webString="";
    appendFile(SD, "/logs.txt","Viewlog:\n", "<br>\n");
  });

  controlserver.on("/style.css", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(SD, "/HTML/style.css", "text/css");
  });

  controlserver.on("/lib.js", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(SD, "/HTML/javascript.js", "text/javascript");
  });

  controlserver.on("/setaddrolling", HTTP_POST, [](AsyncWebServerRequest *request) {    
    new_idtask = request->arg("taskid");
    buttonrolling_tmp = request->arg("buttonrolling");
    numrolling_tmp = request->arg("numrolling");
    deleteFile(SD, "/HTML/rollingcodes.html");
    Serial.print("Task ID: ");
    Serial.println(new_idtask);
    Serial.print("buttonrolling: ");
    Serial.println(buttonrolling_tmp);
    Serial.print("numrolling: ");
    Serial.println(numrolling_tmp);
    Serial.println(token2);
    String response;
    String tmp_idtask;
    
    if (request->hasArg("configmodule")) {
      HTTPClient http;
      http.begin(api_addrollingcode, root_ca);
      http.addHeader("Content-Type", "application/json");
      http.addHeader("Authorization", token2);

      String senddata = "{\"rollingCodeAddition\":{\"remoteInfo\":\"" + new_idtask + "\",\"button\":" + buttonrolling_tmp + ",\"numCodesRequested\":" + numrolling_tmp + "}}";
      int httpCode = http.POST(senddata);
      
      if (httpCode > 0) { //Check for the returning code
        response = http.getString();
        Serial.println(response);
        response.toCharArray(json, response.length()+1);
        StaticJsonDocument<200> doc;
        DeserializationError error = deserializeJson(doc, json);
        String id_task_rolling = doc["id"];
        //Serial.print("New ID Rolling: ");
        //Serial.println(id_task_rolling);
        tmp_idtask = api_addrollingcode_task+id_task_rolling;
        
        //request->send(200, "text/html", HTML_CSS_STYLING + "<script>alert(\"Add Rolling Code OK\")</script>");
      } else {
        Serial.println("Error on HTTP request");
        request->send(200, "text/html", HTML_CSS_STYLING + "<script>alert(\"Error on HTTP request\")</script>");
      }
      //Serial.print("URL Task: ");
      //Serial.println(tmp_idtask);
      http.begin(tmp_idtask, root_ca);
      http.addHeader("Content-Type", "application/json");
      http.addHeader("Authorization", token2);
      httpCode = http.GET();
      
      bool progress_loop = true;
      while(progress_loop == true) {
        http.addHeader("Content-Type", "application/json");
        http.addHeader("Authorization", token2);
        httpCode = http.GET();  
        if (httpCode > 0) {
          response = http.getString();
          response.toCharArray(json, response.length()+1);
          StaticJsonDocument<200> doc;
          DeserializationError error = deserializeJson(doc, json);
          int progress = doc["progress"];
          if (progress == 100){
            progress_loop = false;
          }
            delay(1);
          }
        }
      http.begin(api_remote+new_idtask, root_ca);
      http.addHeader("Content-Type", "application/json");
      http.addHeader("Authorization", token2);
      httpCode = http.GET();

      if (httpCode > 0) {
        response = http.getString();
        Serial.println(response);
        response.toCharArray(json, response.length()+1);
        DynamicJsonDocument doc(10000);
        deserializeJson(doc, json);
        String remoterollingcodes = doc["remoteData"]["rollingCodes"];
        //Serial.println("");
        //Serial.print("Rolling Codes: ");
        //Serial.print(remoterollingcodes);

        writeConfigWiFi(SD, "/HTML/rollingcodes.html", "<html><head><meta charset=\"utf-8\"><title>Evil Crow RF</title><link rel=\"stylesheet\" href=\"style.css\"><script src=\"lib.js\"></script></head>");    
        writeConfigWiFi(SD, "/HTML/rollingcodes.html", "<body><body><nav id='menu'><input type='checkbox' id='responsive-menu' onclick='updatemenu()'><label></label><ul><li><a href='/'>Home</a></li><li><a class='dropdown-arrow'>Config</a><ul class='sub-menus'><li><li><a href='/txbinary'>Binary TX Config</a></li><li><a href='/rxconfig'>RX Config</a></li></ul></li><li><a class='dropdown-arrow'>RX Log</a><ul class='sub-menus'><li><a href='/viewlog'>RX Logs</a></li><li><a href='/delete'>Delete Logs</a></li><li><a href='/downloadlog'>Download Logs</a></li><li><a href='/cleanspiffs'>Clean SPIFFS</a></li></ul></li><li><a class='dropdown-arrow'>Kaiju</a><ul class='sub-menus'><li><a href='/kaijulogin'>Login</a></li><li><a href='/kaijuanalyzer'>Analyzer</a></li><li><a href='/kaijugenerator'>Generator</a></li></ul></li></nav>");
        writeConfigWiFi(SD, "/HTML/rollingcodes.html", "<br><hr><p>Information: </p></h>");
        writeConfigWiFi(SD, "/HTML/rollingcodes.html", "<table><tr><td>Rolling Codes: ");
        writeConfigWiFi(SD, "/HTML/rollingcodes.html", "<br><br>");
        writeConfigWiFi(SD, "/HTML/rollingcodes.html", remoterollingcodes);
        writeConfigWiFi(SD, "/HTML/rollingcodes.html", "</td></table><br><br></body></html>");
      }

      request->send(200, "text/html", HTML_CSS_STYLING + "<script>alert(\"Go to /remoterolling\")</script>");
      
      Serial.println("Complete");  
      http.end();
    }
  });

  controlserver.on("/setsendotp", HTTP_POST, [](AsyncWebServerRequest *request) {
    raw_rx = "0";
    email = request->arg("email");
    otp = request->arg("otpcode");
    String response;
    Serial.println(otp);
    if (request->hasArg("configmodule")) {
      if(otp == "") {
        HTTPClient http;
        http.begin(api_email, root_ca);
        http.addHeader("Content-Type", "application/x-www-form-urlencoded");

        String sendotp = "email= "+email;
        int httpCode = http.POST(sendotp);

        if (httpCode > 0) { //Check for the returning code
          response = http.getString();
          Serial.println(response);
          request->send(200, "text/html", HTML_CSS_STYLING + "<script>alert(\"A login token has been sent to your email\")</script>");
        } else {
          Serial.println("Error on HTTP request");
          request->send(200, "text/html", HTML_CSS_STYLING + "<script>alert(\"Error on HTTP request\")</script>");
        }
        http.end();
      } else {
        HTTPClient http;
        http.begin(api_otp, root_ca);
        http.addHeader("Content-Type", "application/x-www-form-urlencoded");

        String sendotp = "email= "+email+"&token= "+otp;
        Serial.println(sendotp);
        int httpCode = http.POST(sendotp);

        if (httpCode > 0) { //Check for the returning code
          response = http.getString();
          response.toCharArray(json, response.length()+1);
          StaticJsonDocument<200> doc;
          DeserializationError error = deserializeJson(doc, json);
          String json_token = doc["token"];
          token = json_token;
          Serial.print("Token: ");
          Serial.println(token);
          
          request->send(200, "text/html", HTML_CSS_STYLING + "<script>alert(\"Login OK\")</script>");
        } else {
          Serial.println("Error on HTTP request");
          request->send(200, "text/html", HTML_CSS_STYLING + "<script>alert(\"Error on HTTP request\")</script>");
        }
          http.end();
      }      
    }
  });

  controlserver.on("/setkaijuanalyzer", HTTP_POST, [](AsyncWebServerRequest *request) {
    String binary_kaiju = request->arg("binary");
    String response;
    String new_token = "Token "+token;
    token2 = new_token;
    deleteFile(SD, "/HTML/remote.html");

    if (request->hasArg("configmodule")) {
      HTTPClient http;
      http.begin(api_analyzer, root_ca);
      http.addHeader("Content-Type", "application/json");
      http.addHeader("Authorization", "Token "+token);

      String sendbinary = "{\"rawBitStream\":\""+binary_kaiju+"\"}";
      int httpCode = http.POST(sendbinary);

      if (httpCode > 0) {
        response = http.getString();
        Serial.println(response);
        response.toCharArray(json, response.length()+1);
        StaticJsonDocument<200> doc;
        DeserializationError error = deserializeJson(doc, json);
        String id_task = doc["id"];

        http.end();
        http.begin(api_status_task+id_task, root_ca);
        
        bool progress_loop = true;
        while(progress_loop == true) {
          http.addHeader("Content-Type", "application/x-www-form-urlencoded");
          http.addHeader("Authorization", new_token);
          httpCode = http.GET();  
          if (httpCode > 0) {
            response = http.getString();
            response.toCharArray(json, response.length()+1);
            StaticJsonDocument<200> doc;
            DeserializationError error = deserializeJson(doc, json);
            int progress = doc["progress"];
            if (progress == 100){
              progress_loop = false;
            }
            delay(1);
          }
        }
        http.begin(api_remote+id_task, root_ca);
        http.addHeader("Content-Type", "application/x-www-form-urlencoded");
        http.addHeader("Authorization", new_token);
        httpCode = http.GET();

        if (httpCode > 0) {
          response = http.getString();
          Serial.println(response);
          response.toCharArray(json, response.length()+1);
          StaticJsonDocument<2000> doc;
          DeserializationError error = deserializeJson(doc, json);
          String json_frequency = doc["generatedTxConfig"]["frequency"];
          String json_datarate = doc["generatedTxConfig"]["dataRate"];
          String json_modulation = doc["generatedTxConfig"]["modulationName"];
          String json_deviation = doc["generatedTxConfig"]["deviation"];
          String json_manufacturerCodeId = doc["remoteData"]["manufacturerCodeId"];
          String json_type = doc["remoteData"]["type"];
          String json_brand = doc["remoteData"]["brand"];
          String json_model = doc["remoteData"]["model"];
          String json_serialNumberHex = doc["remoteData"]["serialNumberHex"];
          String json_syncCounter = doc["remoteData"]["syncCounter"];
          String json_fixedPartHex = doc["remoteData"]["fixedPartHex"];
          String json_cipherTextHex = doc["remoteData"]["cipherTextHex"];
          String json_encoder = doc["remoteData"]["encoder"];
          String json_cipher = doc["remoteData"]["cipher"];
          String json_imageUrl = doc["remoteData"]["imageUrl"];
          
          writeConfigWiFi(SD, "/HTML/remote.html", "<html><head><meta charset=\"utf-8\"><title>Evil Crow RF</title><link rel=\"stylesheet\" href=\"style.css\"><script src=\"lib.js\"></script></head>");    
          writeConfigWiFi(SD, "/HTML/remote.html", "<body><body><nav id='menu'><input type='checkbox' id='responsive-menu' onclick='updatemenu()'><label></label><ul><li><a href='/'>Home</a></li><li><a class='dropdown-arrow'>Config</a><ul class='sub-menus'><li><li><a href='/txbinary'>Binary TX Config</a></li><li><a href='/rxconfig'>RX Config</a></li></ul></li><li><a class='dropdown-arrow'>RX Log</a><ul class='sub-menus'><li><a href='/viewlog'>RX Logs</a></li><li><a href='/delete'>Delete Logs</a></li><li><a href='/downloadlog'>Download Logs</a></li><li><a href='/cleanspiffs'>Clean SPIFFS</a></li></ul></li><li><a class='dropdown-arrow'>Kaiju</a><ul class='sub-menus'><li><a href='/kaijulogin'>Login</a></li><li><a href='/kaijuanalyzer'>Analyzer</a></li><li><a href='/kaijugenerator'>Generator</a></li></ul></li></nav>");
          writeConfigWiFi(SD, "/HTML/remote.html", "<br><hr><p>Information: </p></h>");
          writeConfigWiFi(SD, "/HTML/remote.html", "<table><tr><td>Frequency: ");
          writeConfigWiFi(SD, "/HTML/remote.html", json_frequency);
          writeConfigWiFi(SD, "/HTML/remote.html", "</td></tr><tr><td>Datarate: ");
          writeConfigWiFi(SD, "/HTML/remote.html", json_datarate);
          writeConfigWiFi(SD, "/HTML/remote.html", "</td></tr><tr><td>Modulation: ");
          writeConfigWiFi(SD, "/HTML/remote.html", json_modulation);
          writeConfigWiFi(SD, "/HTML/remote.html", "</td></tr><tr><td>Deviation: ");
          writeConfigWiFi(SD, "/HTML/remote.html", json_deviation);
          writeConfigWiFi(SD, "/HTML/remote.html", "</td></tr><tr><td>Manufacturer: ");
          writeConfigWiFi(SD, "/HTML/remote.html", json_manufacturerCodeId);
          writeConfigWiFi(SD, "/HTML/remote.html", "</td></tr><tr><td>Type: ");
          writeConfigWiFi(SD, "/HTML/remote.html", json_type);
          writeConfigWiFi(SD, "/HTML/remote.html", "</td></tr><tr><td>Brand: ");
          writeConfigWiFi(SD, "/HTML/remote.html", json_brand);
          writeConfigWiFi(SD, "/HTML/remote.html", "</td></tr><tr><td>Model: ");
          writeConfigWiFi(SD, "/HTML/remote.html", json_model);
          writeConfigWiFi(SD, "/HTML/remote.html", "</td></tr><tr><td>Serial Number: ");
          writeConfigWiFi(SD, "/HTML/remote.html", json_serialNumberHex);
          writeConfigWiFi(SD, "/HTML/remote.html", "</td></tr><tr><td>Sync Counter: ");
          writeConfigWiFi(SD, "/HTML/remote.html", json_syncCounter);
          writeConfigWiFi(SD, "/HTML/remote.html", "</td></tr><tr><td>Fixed Part: ");
          writeConfigWiFi(SD, "/HTML/remote.html", json_fixedPartHex);
          writeConfigWiFi(SD, "/HTML/remote.html", "</td></tr><tr><td>Cipher Text: ");
          writeConfigWiFi(SD, "/HTML/remote.html", json_cipherTextHex);
          writeConfigWiFi(SD, "/HTML/remote.html", "</td></tr><tr><td>Encoder: ");
          writeConfigWiFi(SD, "/HTML/remote.html", json_encoder);
          writeConfigWiFi(SD, "/HTML/remote.html", "</td></tr><tr><td>Cipher: ");
          writeConfigWiFi(SD, "/HTML/remote.html", json_cipher);
          writeConfigWiFi(SD, "/HTML/remote.html", "</td></tr><tr><td>Task ID: ");
          writeConfigWiFi(SD, "/HTML/remote.html", id_task);
          
          writeConfigWiFi(SD, "/HTML/remote.html", "</td></table><br><br><IMG SRC=");
          writeConfigWiFi(SD, "/HTML/remote.html", json_imageUrl);
          
          writeConfigWiFi(SD, "/HTML/remote.html", ">");
          writeConfigWiFi(SD, "/HTML/remote.html", "<br><br></body></html>");
        }

        request->send(200, "text/html", HTML_CSS_STYLING + "<script>alert(\"Go to /remoteinfo\")</script>");
        
      } else {;
        request->send(200, "text/html", HTML_CSS_STYLING + "<script>alert(\"Error on HTTP request\")</script>");
      }
    }         
  });

  //AsyncElegantOTA.begin(&controlserver);
  controlserver.begin();

  ELECHOUSE_cc1101.addSpiPin(14, 12, 13, 5, 0);
  ELECHOUSE_cc1101.addSpiPin(14, 12, 13, 27, 1);
  appendFile(SD, "/logs.txt","Viewlog:\n", "<br>\n");
}

void signalanalyse(){
  #define signalstorage 10

  int signalanz=0;
  int timingdelay[signalstorage];
  float pulse[signalstorage];
  long signaltimings[signalstorage*2];
  int signaltimingscount[signalstorage];
  long signaltimingssum[signalstorage];
  long signalsum=0;

  for (int i = 0; i<signalstorage; i++){
    signaltimings[i*2] = 100000;
    signaltimings[i*2+1] = 0;
    signaltimingscount[i] = 0;
    signaltimingssum[i] = 0;
  }
  for (int i = 1; i<samplecount; i++){
    signalsum+=sample[i];
  }

  for (int p = 0; p<signalstorage; p++){

  for (int i = 1; i<samplecount; i++){
    if (p==0){
      if (sample[i]<signaltimings[p*2]){
        signaltimings[p*2]=sample[i];
      }
    }else{
      if (sample[i]<signaltimings[p*2] && sample[i]>signaltimings[p*2-1]){
        signaltimings[p*2]=sample[i];
      }
    }
  }

  for (int i = 1; i<samplecount; i++){
    if (sample[i]<signaltimings[p*2]+error_toleranz && sample[i]>signaltimings[p*2+1]){
      signaltimings[p*2+1]=sample[i];
    }
  }

  for (int i = 1; i<samplecount; i++){
    if (sample[i]>=signaltimings[p*2] && sample[i]<=signaltimings[p*2+1]){
      signaltimingscount[p]++;
      signaltimingssum[p]+=sample[i];
    }
  }
  }
  
  int firstsample = signaltimings[0];
  
  signalanz=signalstorage;
  for (int i = 0; i<signalstorage; i++){
    if (signaltimingscount[i] == 0){
      signalanz=i;
      i=signalstorage;
    }
  }

  for (int s=1; s<signalanz; s++){
  for (int i=0; i<signalanz-s; i++){
    if (signaltimingscount[i] < signaltimingscount[i+1]){
      int temp1 = signaltimings[i*2];
      int temp2 = signaltimings[i*2+1];
      int temp3 = signaltimingssum[i];
      int temp4 = signaltimingscount[i];
      signaltimings[i*2] = signaltimings[(i+1)*2];
      signaltimings[i*2+1] = signaltimings[(i+1)*2+1];
      signaltimingssum[i] = signaltimingssum[i+1];
      signaltimingscount[i] = signaltimingscount[i+1];
      signaltimings[(i+1)*2] = temp1;
      signaltimings[(i+1)*2+1] = temp2;
      signaltimingssum[i+1] = temp3;
      signaltimingscount[i+1] = temp4;
    }
  }
  }

  for (int i=0; i<signalanz; i++){
    timingdelay[i] = signaltimingssum[i]/signaltimingscount[i];
  }

  if (firstsample == sample[1] and firstsample < timingdelay[0]){
    sample[1] = timingdelay[0];
  }


  bool lastbin=0;
  for (int i=1; i<samplecount; i++){
    float r = (float)sample[i]/timingdelay[0];
    int calculate = r;
    r = r-calculate;
    r*=10;
    if (r>=5){calculate+=1;}
    if (calculate>0){
      if (lastbin==0){
        lastbin=1;
      }else{
      lastbin=0;
    }
      if (lastbin==0 && calculate>8){
        Serial.print(" [Pause: ");
        Serial.print(sample[i]);
        Serial.println(" samples]");
        appendFile(SD, "/logs.txt",NULL, " [Pause: ");
        appendFileLong(SD, "/logs.txt", sample[i]);
        appendFile(SD, "/logs.txt"," samples]", "\n");
      }else{
        for (int b=0; b<calculate; b++){
          Serial.print(lastbin);
          appendFileLong(SD, "/logs.txt", lastbin);
        }
      }
    }
  }
  Serial.println();
  Serial.print("Samples/Symbol: ");
  Serial.println(timingdelay[0]);
  Serial.println();
  appendFile(SD, "/logs.txt","<br>\n", "<br>\n");
  appendFile(SD, "/logs.txt",NULL, "Samples/Symbol: ");
  appendFileLong(SD, "/logs.txt", timingdelay[0]);
  appendFile(SD, "/logs.txt",NULL, "<br>\n");

  int smoothcount=0;
  for (int i=1; i<samplecount; i++){
    float r = (float)sample[i]/timingdelay[0];
    int calculate = r;
    r = r-calculate;
    r*=10;
    if (r>=5){calculate+=1;}
    if (calculate>0){
      samplesmooth[smoothcount] = calculate*timingdelay[0];
      smoothcount++;
    }
  }
  Serial.println("Rawdata corrected:");
  Serial.print("Count=");
  Serial.println(smoothcount+1);
  appendFile(SD, "/logs.txt",NULL, "Count=");
  appendFileLong(SD, "/logs.txt", smoothcount+1);
  appendFile(SD, "/logs.txt","\n", "<br>\n");
  appendFile(SD, "/logs.txt",NULL, "Rawdata corrected:\n");
  for (int i=0; i<smoothcount; i++){
    Serial.print(samplesmooth[i]);
    Serial.print(",");
    transmit_push[i] = samplesmooth[i];
    appendFileLong(SD, "/logs.txt", samplesmooth[i]);
    appendFile(SD, "/logs.txt", NULL, ",");  
  }
  Serial.println();
  Serial.println();
  appendFile(SD, "/logs.txt", NULL, "<br>\n");
  appendFile(SD, "/logs.txt", "-------------------------------------------------------\n", "<br>");
  return;
}

void loop() {

  if(raw_rx == "1") {
    if(checkReceived()){
      printReceived();
      signalanalyse();
      enableReceive();
      delay(500);
    }
  }
}
