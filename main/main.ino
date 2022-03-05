#include <SPI.h>
#include <Wire.h>
#include <WiFiClient.h>
#include <ESP8266WiFi.h>
#include <SoftwareSerial.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPClient.h>
#include <Adafruit_Fingerprint.h>  //https://github.com/adafruit/Adafruit-Fingerprint-Sensor-Library

#define Finger_Rx 15 //D8
#define Finger_Tx 13 //D7

//SoftwareSerial mySerial(Finger_Rx, Finger_Tx);
SoftwareSerial mySerial(Finger_Tx, Finger_Rx);
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);

const char *ssid = "SSID";  //ENTER YOUR WIFI SETTINGS
const char *password = "123";
String link = "http://localhost"; //computer IP or the server domain

WiFiClient wifiClient;

void setup() {
 
  Serial.begin(9600);
  connectToWiFi();
  finger.begin(57600);
  Serial.println("\n\nAdafruit finger detect test");
  if (finger.verifyPassword()) {
    Serial.println("Found fingerprint sensor!");
  } else {
    Serial.println("Did not find fingerprint sensor :(");
    while (1) { delay(1); }
  }
}

void loop() {
 
  //check if there's a connection to WiFi or not
  if(WiFi.status() != WL_CONNECTED){
    connectToWiFi();
  }
  Serial.println("Escolha uma opção entre 1 e 2");
  int input = readInputOption();
  switch (input) {
    case 1:
      Serial.println("Iniciando registro de novo fingerprint");
      registerNewFingerPrint();
      break;
    case 2:
      Serial.println("Iniciando login/logoff");
      loginLogoff();
      break;
    default:
      Serial.print("Opcao invalida: "); Serial.print(input); Serial.println(". Escolha entre 1 e 2");
      return;
  }
}

int readInputOption() {
  int option = 0;
  while (true) {
    while (!Serial.available());
    option = Serial.parseInt();
    if (option == 0) {continue;}
    if (option < 0 || option > 2) {
      Serial.print(option);
      Serial.println("Id escolhido é inválido. Precisa estar entre 1 e 2");
      continue;
    } else {
      Serial.print("Opção escolhida foi: "); Serial.println(option);
      break;
    }
  }
  return option;
}

bool registerNewFingerPrint() {
  Serial.println("Escolha o numero do ID desejado: ");
  int id_ = 0;
  while (true) {
    while (!Serial.available());
    id_ = Serial.parseInt();
    if (id_ == 0) {continue;}
    if (id_ < 0 || id_ > 127) {
      Serial.print(id_);
      Serial.println("Id escolhido é inválido. Precisa estar entre 1 e 127");
      continue;
    } else {
      Serial.print("Id Escolhido foi: "); Serial.println(id_);
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
  http.begin(wifiClient, endpoint);
  int statusCode = http.GET();
  String payload = "";
  if (statusCode == 200) {
    Serial.println("Requisicao realizada com sucesso!");
    payload = http.getString();
    Serial.println("Retorno: " + payload);
  } else {
     Serial.print("Algo deu errado na solicitacao. StatusCode: "); Serial.println(statusCode);
  }
  return payload == "found";
}

bool loginLogoff() {
  int fingerId = getStoredFingerPrintID();
  if (fingerId > 0) {
    return loginLogoffServer(fingerId);
  }
  Serial.println("FingerPrint não encontrado :/");
  return false;
}

int getStoredFingerPrintID() {
  Serial.println("Por favor, coloque o dedo no leitor");
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
  p = finger.image2Tz();
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image converted");
      break;
    case FINGERPRINT_IMAGEMESS:
      Serial.println("Image too messy");
      return -1;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      return -1;
    case FINGERPRINT_FEATUREFAIL:
      Serial.println("Could not find fingerprint features");
      return -1;
    case FINGERPRINT_INVALIDIMAGE:
      Serial.println("Could not find fingerprint features");
      return -1;
    default:
      Serial.println("Unknown error");
      return -1;
  }
  // OK converted!
  p = finger.fingerFastSearch();
  if (p == FINGERPRINT_OK) {
    Serial.println("Found a print match!");
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("Communication error");
    return -1;
  } else if (p == FINGERPRINT_NOTFOUND) {
    Serial.println("Did not find a match");
    return -1;
  } else {
    Serial.println("Unknown error");
    return -1;
  }   
  // found a match!
  Serial.print("Found ID #"); Serial.print(finger.fingerID); 
  //Serial.print(" with confidence of "); Serial.println(finger.confidence); 
 
  return finger.fingerID;
}

bool loginLogoffServer(int fingerId) {
  Serial.println("Iniciando comunicacao http");
  HTTPClient http;
  String endpoint = link + "/log";
  http.begin(wifiClient, endpoint);
  String body = "{\"id\": " + String(fingerId) + "}";
  http.header("Content-Type: application/json");
  int statusCode = http.POST(body);
  if (statusCode == 200) {
     Serial.println("Requisicao realizada com sucesso!");
  } else {
     Serial.print("Algo deu errado na solicitacao. StatusCode: "); Serial.println(statusCode);
  }
  String payload = http.getString();
  if (payload == "on") {
    Serial.println("Usuário estava deslogado e agora foi logado!");
    return true;
  } else {
    Serial.println("Usuário estava logado e agora foi deslogado!");
    return false;
  }
}

void deleteFingerprint(int fingerId) {
  int p = -1;
  p = finger.deleteModel(fingerId);
  if (p == FINGERPRINT_OK) {
    Serial.print("ID "); Serial.print(fingerId); Serial.println("Deletado com sucesso!");
    return;
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("Communication error");
    return;
  } else if (p == FINGERPRINT_BADLOCATION) {
    Serial.println("Could not delete in that location");
    return;
  } else if (p == FINGERPRINT_FLASHERR) {
    Serial.println("Error writing to flash");
    return;
  } else {
    Serial.print("Unknown error: 0x"); Serial.println(p, HEX);
    return;
  }
}

int registerFingerPrint(int fingerId) {
  int res = registerFingerPrintLocalMemory(fingerId);
  if (res == -1) {
    Serial.println("Algo deu errado ao registrar o id localmente :/");
    return -1;
  }
  bool created = registerFingerPrintServer(fingerId);
  if (!created) {
    Serial.println("Algo deu errado ao registrar o id no servidor :/");
    //Se algo deu errado com o servidor, delete da memoria local
    deleteFingerprint(fingerId);
    return -1;
  }
  Serial.println("Id registrado com sucesso!");
  return res;
}

bool registerFingerPrintServer(int fingerId) {
  Serial.println("Iniciando comunicacao http");
  HTTPClient http;
  String endpoint = link + "/createId";
  http.begin(wifiClient, endpoint);
  String body = "{\"id\": " + String(fingerId) + "}";
  http.header("Content-Type: application/json");
  int statusCode = http.POST(body);
  if (statusCode == 200) {
     Serial.println("Requisicao realizada com sucesso!");
  } else {
     Serial.print("Algo deu errado na solicitacao. StatusCode: "); Serial.println(statusCode);
  }
  String payload = http.getString();
  return payload == "ok";
}

int registerFingerPrintLocalMemory(int fingerId) {
  Serial.println("iniciando registro do dedo com id: " + String(fingerId));
  Serial.println("Por favor, insira o dedo");
  
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
  
  Serial.println("Por favor, remova o dedo");
  delay(2000);
  p = 0;
  while (p != FINGERPRINT_NOFINGER) {
    p = finger.getImage();
  }
  Serial.print("ID "); Serial.println(fingerId);
  p = -1;
  Serial.println("Por favor coloque o dedo novamente");
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
  Serial.println("Por favor remova o dedo");
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
    return fingerId;
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

void connectToWiFi(){
    WiFi.mode(WIFI_OFF); //Prevents reconnection issue (taking too long to connect)
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
