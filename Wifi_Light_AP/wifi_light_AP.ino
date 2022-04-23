#include <ESP8266WiFi.h>          
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>
#include <FS.h>

#define Pin 16
const byte DNS_PORT = 53;
IPAddress apIP(192, 168, 1, 1);
DNSServer dnsServer;        
ESP8266WebServer esp8266_server(80);// 建立网络服务器对象，该对象用于响应HTTP请求。监听端口（80）



void setup() {
  Serial.begin(9600);
  pinMode(Pin, OUTPUT);
  digitalWrite(Pin, LOW);
  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
  WiFi.softAP("嘉然今天吃什么");
  
  Serial.print("IP address:\t");            
  Serial.println(WiFi.localIP());
  dnsServer.setTTL(300);
  dnsServer.setErrorReplyCode(DNSReplyCode::ServerFailure);
  dnsServer.start(DNS_PORT, "www.jiaran.com", apIP);
  
  


  
  if(SPIFFS.begin()){                       // 启动闪存文件系统
    Serial.println("SPIFFS Started.");
  } else {
    Serial.println("SPIFFS Failed to Start.");
  }
  esp8266_server.on("/setLight", handleLightD0);
  esp8266_server.on("/readLight", readLightState); 
  esp8266_server.onNotFound(handleUserRequest); // 处理其它网络请求
  //服务器启动
  esp8266_server.begin();                           // 启动网站服务

}
 
void loop() {
  dnsServer.processNextRequest();
  esp8266_server.handleClient();
}



void handleLightD0() {
 String lightState;
 
 String Light_State = esp8266_server.arg("LightState"); 
 
 if(Light_State == "1"){
  digitalWrite(Pin,HIGH); //点亮
  lightState = "ON"; 
 } 
 if(Light_State == "0"){
  digitalWrite(Pin,LOW); //熄灭
  lightState = "OFF"; 
 }
 esp8266_server.send(200, "text/plain", lightState); 
}

void readLightState(){
  String State;
 int lightState = digitalRead(Pin);
 if(lightState == HIGH){
  State = "ON";
 }
 if(lightState == LOW){
  State = "OFF";
 }
 esp8266_server.send(200, "text/plain", State);
}

void handleUserRequest() {         
     
  // 获取用户请求资源(Request Resource）
  String reqResource = esp8266_server.uri();
  Serial.print("reqResource: ");
  Serial.println(reqResource);
  
  // 通过handleFileRead函数处处理用户请求资源
  bool fileReadOK = handleFileRead(reqResource);

  // 如果在SPIFFS无法找到用户访问的资源，则回复404 (Not Found)
  if (!fileReadOK){                                                 
    esp8266_server.send(404, "text/plain", "404 Not Found"); 
  }
}

bool handleFileRead(String resource) {            //处理浏览器HTTP访问

  if (resource.endsWith("/")) {                   // 如果访问地址以"/"为结尾
    resource = "/index.html";                     // 则将访问地址修改为/index.html便于SPIFFS访问
  } 
  
  String contentType = getContentType(resource);  // 获取文件类型
  
  if (SPIFFS.exists(resource)) {                     // 如果访问的文件可以在SPIFFS中找到
    File file = SPIFFS.open(resource, "r"); // 则尝试打开该文件
    esp8266_server.streamFile(file, contentType);// 并且将该文件返回给浏览器
    file.close();                                // 并且关闭文件
    return true;                                 // 返回true
  }
  return false;                                  // 如果文件未找到，则返回false
}

String getContentType(String filename){
  if(filename.endsWith(".htm")) return "text/html";
  else if(filename.endsWith(".html")) return "text/html";
  else if(filename.endsWith(".css")) return "text/css";
  else if(filename.endsWith(".js")) return "application/javascript";
  else if(filename.endsWith(".png")) return "image/png";
  else if(filename.endsWith(".gif")) return "image/gif";
  else if(filename.endsWith(".jpg")) return "image/jpeg";
  else if(filename.endsWith(".ico")) return "image/x-icon";
  else if(filename.endsWith(".xml")) return "text/xml";
  else if(filename.endsWith(".pdf")) return "application/x-pdf";
  else if(filename.endsWith(".zip")) return "application/x-zip";
  else if(filename.endsWith(".gz")) return "application/x-gzip";
  return "text/plain";
}
