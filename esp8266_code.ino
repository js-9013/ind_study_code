#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>

const char* host = "awsEndpoint.aws.com"; //AWS endpoint
const int httpsPort = 443;

// Use web browser to view and copy
// SHA1 fingerprint of the certificate
const char* fingerprint = "CF 05 98 89 CA FF 8E D8 5E 5C E0 C2 E4 F7 E6 C3 C7 50 DD 5C"; //meaningless fingerprint

#define MESSAGE_SIZE 4
#define LED_BUILTIN 2
const char* ssid = "wifi";
const char* password = "wifipassword";
WiFiClient client;
//uint16_t port = 8888;
String serial_data;

void sendRequest() {
  // Use WiFiClientSecure class to create TLS connection
  WiFiClientSecure client;
  Serial.print("connecting to ");
  Serial.println(host);
  if (!client.connect(host, httpsPort)) {
    Serial.println("connection failed");
    return;
  }

  if (client.verify(fingerprint, host)) {
    Serial.println("certificate matches");
  } else {
    Serial.println("certificate doesn't match");
  }

  String url = "/<stage>/lambdaFunctionName";
  String transmit = String("GET ") + url + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" +
               "Connection: close\r\n\r\n";
  Serial.print("requesting URL: ");
  Serial.println(url);

  client.print(transmit);

  /*Serial.println("request sent");
  while (client.connected()) {
    String line = client.readStringUntil('\n');
    if (line == "\r") {
      Serial.println("headers received");
      break;
    }
  }
  String line = client.readStringUntil('\n');
  if (line.startsWith("{\"state\":\"success\"")) {
    Serial.println("esp8266/Arduino CI successfull!");
  } else {
    Serial.println("esp8266/Arduino CI has failed");
  }
  Serial.println("reply was:");
  Serial.println("==========");
  Serial.println(line);
  Serial.println("==========");
  Serial.println("closing connection");*/
}

void flushRxBuffer() {
  while(Serial.available())
    Serial.read();
}

void setup() {
  Serial.begin(9600);
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.println();
  Serial.print("connecting to ");
  Serial.println(ssid);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

uint8_t data;

void loop(){  
  while (Serial.available() == 0){
  }

  while (Serial.available() > 0) {
    data = Serial.read();
   
    if (data == 'g')
    {
      digitalWrite(LED_BUILTIN, LOW);
      sendRequest();
      digitalWrite(LED_BUILTIN, HIGH);
      /*Serial.flush();
      flushRxBuffer();*/
    }
  }
  
 /* if (client.connected()) {
      serial_data = String(Serial.read());
      client.print(serial_data);
  }
  else{
    client.connect(server, port);*/
 
}


