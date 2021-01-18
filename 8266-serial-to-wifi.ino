/*
 ESP 8266 NodeMCU 1.0
*/
#include <ESP8266WiFi.h>

#define LED_BUILTIN 2
#define MAX_SRV_CLIENTS 1
#define TCP_PORT (23)
WiFiServer tcpServer(TCP_PORT);
WiFiClient tcpServerClients[MAX_SRV_CLIENTS];

IPAddress ip(10, 0, 0, 2);
IPAddress gateway(10, 0, 0, 1);
IPAddress subnet(255, 255, 255, 0);

const char ssid[] = "wifissid";
const char password[] = "wifipassword";

#ifndef min
#define min(x,y)  ((x)<(y)?(x):(y))
#endif

void setup() {
 pinMode(LED_BUILTIN, OUTPUT);
 digitalWrite(LED_BUILTIN, LOW); // Turn the LED on

 Serial.begin(115200);
 Serial.swap();

 WiFi.setAutoReconnect(true);
 WiFi.mode(WIFI_STA);
 WiFi.config(ip, gateway, subnet);
 delay(100);
 WiFi.begin(ssid, password);

 while (WiFi.status() != WL_CONNECTED) {
   digitalWrite(LED_BUILTIN, HIGH);
   delay(200);
   digitalWrite(LED_BUILTIN, LOW);
   delay(200);
 }
 tcpServer.begin();
 tcpServer.setNoDelay(true);
}


void loop() {
 uint8_t i;
 uint8_t buf[1024];
 int bytesAvail, bytesIn;
 //check if there are any new clients
 if (tcpServer.hasClient()) {
   for (i = 0; i < MAX_SRV_CLIENTS; i++) {
     //find free/disconnected spot
     if (!tcpServerClients[i] || !tcpServerClients[i].connected()) {
       if (tcpServerClients[i]) tcpServerClients[i].stop();
       tcpServerClients[i] = tcpServer.available();
       Serial.print(i);
       continue;
     }
   }
   //no free/disconnected spot so reject
   WiFiClient tcpServerClient = tcpServer.available();
   tcpServerClient.stop();
 }
 //check clients for data
 for (i = 0; i < MAX_SRV_CLIENTS; i++) {
   if (tcpServerClients[i] && tcpServerClients[i].connected()) {
     //get data from the telnet client and push it to the UART
     while ((bytesAvail = tcpServerClients[i].available()) > 0) {
       bytesIn = tcpServerClients[i].readBytes(buf, min(sizeof(buf), bytesAvail));
       if (bytesIn > 0) {
         Serial.write(buf, bytesIn);
       }
     }
   }
 }
 //check UART for data
 while ((bytesAvail = Serial.available()) > 0) {
   bytesIn = Serial.readBytes(buf, min(sizeof(buf), bytesAvail));
   if (bytesIn > 0) {
     //push UART data to all connected telnet clients
     for (i = 0; i < MAX_SRV_CLIENTS; i++) {
       if (tcpServerClients[i] && tcpServerClients[i].connected()) {
         tcpServerClients[i].write((uint8_t*)buf, bytesIn);
       }
     }
   }
 }
}
