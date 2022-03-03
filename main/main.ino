#include <SPI.h>
#include <Wire.h>
#include <WiFiClient.h> 
#include <ESP8266WiFi.h>
#include <SoftwareSerial.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPClient.h>
#include <Adafruit_Fingerprint.h>  //https://github.com/adafruit/Adafruit-Fingerprint-Sensor-Library
//************************************************************************
//Fingerprint scanner Pins
#define Finger_Rx 14 //D5
#define Finger_Tx 12 //D6
// Declaration for SSD1306 display connected using software I2C
//************************************************************************
SoftwareSerial mySerial(Finger_Rx, Finger_Tx);
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);
//************************************************************************
/* Set these to your desired credentials. */
const char *ssid = "SSID";  //ENTER YOUR WIFI SETTINGS
const char *password = "PASSWORD";
//************************************************************************
//String postData ; // post array that will be send to the website
String link = "http://something:5000"; //computer IP or the server domain
//int FingerID = 0;     // The Fingerprint ID from the scanner 
//uint8_t id;

void setup() {
 
  Serial.begin(9600);
  //---------------------------------------------
  connectToWiFi();
  //---------------------------------------------
  
  // set the data rate for the sensor serial port
  finger.begin(57600);
  Serial.println("\n\nAdafruit finger detect test");
 
  if (finger.verifyPassword()) {
    Serial.println("Found fingerprint sensor!");
  } else {
    Serial.println("Did not find fingerprint sensor :(");
    while (1) { delay(1); }
  }
  //---------------------------------------------
  
//  finger.getTemplateCount();
//  Serial.print("Sensor contains "); Serial.print(finger.templateCount); Serial.println(" templates");
//  Serial.println("Waiting for valid finger...");
  
  //------------*test the connection*------------
  
  //SendFingerprintID( FingerID );
  
}
//************************************************************************
void loop() {
 
  //check if there's a connection to WiFi or not
  if(WiFi.status() != WL_CONNECTED){
    connectToWiFi();
  }

  char input = readInputOption();
  switch (input) {
    case '1':
      //registrar fingerprint
      registerNewFingerPrint();
      break;
    case '2':
      // logar/loggout
      break;
    default:
      Serial.println("Opcao invalida");
      return;
  }
//  
//  //---------------------------------------------
//  //If there no fingerprint has been scanned return -1 or -2 if there an error or 0 if there nothing, The ID start form 1 to 127
//  FingerID = getFingerprintID();  // Get the Fingerprint ID from the Scanner
//  delay(50);            //don't need to run this at full speed.
// //---------------------------------------------
//  ChecktoAddID();
//  //---------------------------------------------
//  ChecktoDeleteID();
//  //---------------------------------------------
}

char readInputOption() {
  if (Serial.available() > 0) {
    char rx_byte = Serial.read();
    // verificar se é um numero
    if ((rx_byte >= '0') && (rx_byte <= '9')) {
      Serial.print("Opção escolhida: ");
      Serial.println(rx_byte);
      return rx_byte; 
    } else {
      Serial.print("Opção invalida. Escolha entre 1 e 2.");
      return 0;
    }
  }
}

bool registerNewFingerPrint() {
  Serial.println("Escolha o numero do ID desejado: ");
  int id_ = 0;
  while (true) {
    while (!Serial.available());
    id_ = Serial.parseInt();
    if (id_ <= 0 || id_ > 127) {
      Serial.println("Id escolhido é inválido. Precisa estar entre 1 e 127");
      continue;
    } else {
      Serial.print("Id Escolhido foi: "); Serial.print(id_);
      break;
    }
  }
  bool exists = checkIfIdExists(id_);
  if (exists) {
    Serial.println("Id já registrado, por favor escolha outro");
    return false;
  }
  int result = registerFingerPrint(id_);
  if (result == -1) {
    return false;
  }
  return true;
}

bool checkIfIdExists(int fingerId) {
  Serial.println("Iniciando comunicacao http");
  HTTPClient http;
  String endpoint = link + "/checkExists?id_user=" + fingerId;
  http.begin(endpoint);
  int statusCode = http.GET();
  if (statusCode == 200) {
    Serial.println("Requisicao realizada com sucesso!");
  } else {
     Serial.print("Algo deu errado na solicitacao. StatusCode: "); Serial.println(statusCode);
  }
  String payload = http.getString();
  return payload == "found";
}

//************send the fingerprint ID to the website*************
//void SendFingerprintID( int finger ){
//  
//  HTTPClient http;    //Declare object of class HTTPClient
//  //Post Data
//  postData = "FingerID=" + String(finger); // Add the Fingerprint ID to the Post array in order to send it
//  // Post methode
// 
//  http.begin(link); //initiate HTTP request, put your Website URL or Your Computer IP 
//  http.addHeader("Content-Type", "application/x-www-form-urlencoded");    //Specify content-type header
//  
//  int httpCode = http.POST(postData);   //Send the request
//  String payload = http.getString();    //Get the response payload
//  
//  Serial.println(httpCode);   //Print HTTP return code
//  Serial.println(payload);    //Print request response payload
//  Serial.println(postData);   //Post Data
//  Serial.println(finger);     //Print fingerprint ID
// 
//  if (payload.substring(0, 5) == "login") {
//    Serial.println("login");
//    String user_name = payload.substring(5);
//    Serial.println(user_name);
//  }
//  else if (payload.substring(0, 6) == "logout") {
//    Serial.println("logout");
//    String user_name = payload.substring(6);
//    Serial.println(user_name);
//  }
//  delay(1000);
//  
//  postData = "";
//  http.end();  //Close connection
//}
////********************Get the Fingerprint ID******************
//int getFingerprintID() {
//  
//  uint8_t p = finger.getImage();
//  switch (p) {
//    case FINGERPRINT_OK:
//      Serial.println("Image taken");
//      break;
//    case FINGERPRINT_NOFINGER:
//      Serial.println("No finger detected");
//      return 0;
//    case FINGERPRINT_PACKETRECIEVEERR:
//      Serial.println("Communication error");
//      return -2;
//    case FINGERPRINT_IMAGEFAIL:
//      Serial.println("Imaging error");
//      return -2;
//    default:
//      Serial.println("Unknown error");
//      return -2;
//  }
//  // OK success!
//  p = finger.image2Tz();
//  switch (p) {
//    case FINGERPRINT_OK:
//      Serial.println("Image converted");
//      break;
//    case FINGERPRINT_IMAGEMESS:
//      Serial.println("Image too messy");
//      return -1;
//    case FINGERPRINT_PACKETRECIEVEERR:
//      Serial.println("Communication error");
//      return -2;
//    case FINGERPRINT_FEATUREFAIL:
//      Serial.println("Could not find fingerprint features");
//      return -2;
//    case FINGERPRINT_INVALIDIMAGE:
//      Serial.println("Could not find fingerprint features");
//      return -2;
//    default:
//      Serial.println("Unknown error");
//      return -2;
//  }
//  // OK converted!
//  p = finger.fingerFastSearch();
//  if (p == FINGERPRINT_OK) {
//    Serial.println("Found a print match!");
//  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
//    Serial.println("Communication error");
//    return -2;
//  } else if (p == FINGERPRINT_NOTFOUND) {
//    Serial.println("Did not find a match");
//    return -1;
//  } else {
//    Serial.println("Unknown error");
//    return -2;
//  }   
//  // found a match!
//  Serial.print("Found ID #"); Serial.print(finger.fingerID); 
//  //Serial.print(" with confidence of "); Serial.println(finger.confidence); 
// 
//  return finger.fingerID;
//}
//******************Check if there a Fingerprint ID to delete******************
//void ChecktoDeleteID(){
// 
//  HTTPClient http;    //Declare object of class HTTPClient
//  //Post Data
//  postData = "DeleteID=check"; // Add the Fingerprint ID to the Post array in order to send it
//  // Post methode
// 
//  http.begin(link); //initiate HTTP request, put your Website URL or Your Computer IP 
//  http.addHeader("Content-Type", "application/x-www-form-urlencoded");    //Specify content-type header
//  
//  int httpCode = http.POST(postData);   //Send the request
//  String payload = http.getString();    //Get the response payload
// 
//  if (payload.substring(0, 6) == "del-id") {
//    String del_id = payload.substring(6);
//    Serial.println(del_id);
//    deleteFingerprint( del_id.toInt() );
//  }
//  
//  http.end();  //Close connection
//}
////******************Delete Finpgerprint ID*****************
//uint8_t deleteFingerprint( int id) {
//  uint8_t p = -1;
//  
//  p = finger.deleteModel(id);
// 
//  if (p == FINGERPRINT_OK) {
//    Serial.println("Deleted!");
//  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
//    Serial.println("Communication error");
//    return p;
//  } else if (p == FINGERPRINT_BADLOCATION) {
//    Serial.println("Could not delete in that location");
//    return p;
//  } else if (p == FINGERPRINT_FLASHERR) {
//    Serial.println("Error writing to flash");
//    return p;
//  } else {
//    Serial.print("Unknown error: 0x"); Serial.println(p, HEX);
//    return p;
//  }   
//}
//******************Check if there a Fingerprint ID to add******************
//void ChecktoAddID(){
// 
//  HTTPClient http;    //Declare object of class HTTPClient
//  //Post Data
//  postData = "Get_Fingerid=get_id"; // Add the Fingerprint ID to the Post array in order to send it
//  // Post methode
// 
//  http.begin(link); //initiate HTTP request, put your Website URL or Your Computer IP 
//  http.addHeader("Content-Type", "application/x-www-form-urlencoded");    //Specify content-type header
//  
//  int httpCode = http.POST(postData);   //Send the request
//  String payload = http.getString();    //Get the response payload
// 
//  if (payload.substring(0, 6) == "add-id") {
//    String add_id = payload.substring(6);
//    Serial.println(add_id);
//    id = add_id.toInt();
//    getFingerprintEnroll();
//  }
//  http.end();  //Close connection
//}

int registerFingerPrint(int fingerId) {
  int res = registerFingerPrintLocalMemory(fingerId);
  if (res == -1) {
    Serial.println("Algo deu errado ao registrar o id localmente :/");
    return -1;
  }
  bool created = registerFingerPrintServer(fingerId);
  if (!created) {
    Serial.println("Algo deu errado ao registrar o id no servidor :/");
    return -1;
  }
  Serial.println("Id registrado com sucesso!");
  return res;
}

bool registerFingerPrintServer(int fingerId) {
  Serial.println("Iniciando comunicacao http");
  HTTPClient http;
  String endpoint = link + "/createId";
  http.begin(endpoint);
  String body = "{\"id:\": " fingerId "}";
  int statusCode = http.POST(body);
  if (statusCode == 200) {
    Serial.println("Requisicao realizada com sucesso!");
  } else {
     Serial.print("Algo deu errado na solicitacao. StatusCode: "); Serial.println(statusCode);
  }
  String payload = http.getString();
  return payload == "ok";
}

//******************Register Finger Print*****************
int registerFingerPrintLocalMemory(int fingerId) {
 
  int p = -1;
  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
    switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image taken");
      break;
    case FINGERPRINT_NOFINGER:
      Serial.println(".");
      break;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("packet reciever");
      break;
    case FINGERPRINT_IMAGEFAIL:
      Serial.println("Imaging error");
      break;
    default:
      Serial.println("Unknown error");
      break;
    }
  }
 
  // OK success!
 
  p = finger.image2Tz(1);
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image converted");
      break;
    case FINGERPRINT_IMAGEMESS:
      Serial.println("Image too messy");
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      return p;
    case FINGERPRINT_FEATUREFAIL:
      Serial.println("Could not find fingerprint features");
      return p;
    case FINGERPRINT_INVALIDIMAGE:
      Serial.println("Could not find fingerprint features");
      return p;
    default:
      Serial.println("Unknown error");
      return p;
  }
  
  Serial.println("Remove finger");
  delay(2000);
  p = 0;
  while (p != FINGERPRINT_NOFINGER) {
    p = finger.getImage();
  }
  Serial.print("ID "); Serial.println(fingerId);
  p = -1;
  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
    switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image taken");
      break;
    case FINGERPRINT_NOFINGER:
      Serial.println(".");
      break;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      break;
    case FINGERPRINT_IMAGEFAIL:
      Serial.println("Imaging error");
      break;
    default:
      Serial.println("Unknown error");
      break;
    }
  }
 
  // OK success!
 
  p = finger.image2Tz(2);
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image converted");
      break;
    case FINGERPRINT_IMAGEMESS:
      Serial.println("Image too messy");
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      return p;
    case FINGERPRINT_FEATUREFAIL:
      Serial.println("Could not find fingerprint features");
      return p;
    case FINGERPRINT_INVALIDIMAGE:
      Serial.println("Could not find fingerprint features");
      return p;
    default:
      Serial.println("Unknown error");
      return p;
  }
  
  // OK converted!
  Serial.print("Creating model for #");  Serial.println(fingerId);
  
  p = finger.createModel();
  if (p == FINGERPRINT_OK) {
    Serial.println("Prints matched!");
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("Communication error");
    return p;
  } else if (p == FINGERPRINT_ENROLLMISMATCH) {
    Serial.println("Fingerprints did not match");
    return p;
  } else {
    Serial.println("Unknown error");
    return p;
  }   
  
  Serial.print("ID "); Serial.println(fingerId);
  p = finger.storeModel(fingerId);
  if (p == FINGERPRINT_OK) {
    Serial.println("Stored!");
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("Communication error");
    return p;
  } else if (p == FINGERPRINT_BADLOCATION) {
    Serial.println("Could not store in that location");
    return p;
  } else if (p == FINGERPRINT_FLASHERR) {
    Serial.println("Error writing to flash");
    return p;
  } else {
    Serial.println("Unknown error");
    return p;
  }
}

//
////******************Check if there a Fingerprint ID to add******************
//void confirmAdding(){
//  Serial.println("confirming Adding...");
//  HTTPClient http;    //Declare object of class HTTPClient
//  //Post Data
//  postData = "confirm_id=" + String(id); // Add the Fingerprint ID to the Post array in order to send it
//  // Post methode
// 
//  http.begin(link); //initiate HTTP request, put your Website URL or Your Computer IP 
//  http.addHeader("Content-Type", "application/x-www-form-urlencoded");    //Specify content-type header
//  
//  int httpCode = http.POST(postData);   //Send the request
//  String payload = http.getString();    //Get the response payload
//  delay(1000);
//  Serial.println(payload);
//  
//  http.end();  //Close connection
//  Serial.println("confirmation complete");
//}
//********************connect to the WiFi******************
void connectToWiFi(){
    WiFi.mode(WIFI_OFF);        //Prevents reconnection issue (taking too long to connect)
    delay(1000);
    WiFi.mode(WIFI_STA);
    Serial.print("Connecting to ");
    Serial.println(ssid);
    WiFi.begin(ssid, password);   
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
    }
    Serial.println("");
    Serial.println("Connected");
    
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());  //IP address assigned to your ESP
}
