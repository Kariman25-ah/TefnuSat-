#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"
#include "esp_camera.h"
#include "SD_MMC.h"
#include "time.h"

//-------------𓏏𓍋𓈖𓅱 TefnuSat CAM 𓏏𓍋𓈖𓅱------------------//

 //𓏏𓍋𓈖𓅱WIFI Network

   const char* ssid = "TefnuSat";
   const char* password = "tefnuSat3";

 // 𓏏𓍋𓈖𓅱 Server
   String serverName = "satellitecamvercel2.vercel.app";
   String serverPath = "/upload";

   const int serverPort = 443; 

   WiFiClientSecure client;
 // 𓏏𓍋𓈖𓅱 Init CAM Pins

   #define PWDN_GPIO_NUM     32
   #define RESET_GPIO_NUM    -1
   #define XCLK_GPIO_NUM      0
   #define SIOD_GPIO_NUM     26
   #define SIOC_GPIO_NUM     27
   #define Y9_GPIO_NUM       35
   #define Y8_GPIO_NUM       34
   #define Y7_GPIO_NUM       39
   #define Y6_GPIO_NUM       36
   #define Y5_GPIO_NUM       21
   #define Y4_GPIO_NUM       19
   #define Y3_GPIO_NUM       18
   #define Y2_GPIO_NUM        5
   #define VSYNC_GPIO_NUM    25
   #define HREF_GPIO_NUM     23
   #define PCLK_GPIO_NUM     22

 // 𓏏𓍋𓈖𓅱 Send photos
   String sendPhoto();
   const int timerInterval = 20000;  
   unsigned long previousMillis = 0;   // last time image was sent

void setup() {

  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);
  Serial.begin(115200);

 // 𓏏𓍋𓈖𓅱 Connect with WIFI
   WiFi.mode(WIFI_STA);
   Serial.println();
   Serial.print("Connecting to ");
   Serial.println(ssid);
   WiFi.begin(ssid, password);  

   while (WiFi.status() != WL_CONNECTED) {
    Serial.print("..."); 
    delay(500);
   }

 // 𓏏𓍋𓈖𓅱 Syn Time with CAM

   configTime(0, 0, "pool.ntp.org", "time.nist.gov");
   Serial.println("Syncing time...");
   struct tm timeinfo;
   while (!getLocalTime(&timeinfo)) {
    Serial.print(".");
    delay(500);
   }

   Serial.println("\nTime ready!");

   Serial.println();
   Serial.print("ESP32-CAM IP Address: ");
   Serial.println(WiFi.localIP());

 // 𓏏𓍋𓈖𓅱 CAM Pins with processor

   camera_config_t config;
   config.ledc_channel = LEDC_CHANNEL_0;
   config.ledc_timer = LEDC_TIMER_0;
   config.pin_d0 = Y2_GPIO_NUM;
   config.pin_d1 = Y3_GPIO_NUM;
   config.pin_d2 = Y4_GPIO_NUM;
   config.pin_d3 = Y5_GPIO_NUM;
   config.pin_d4 = Y6_GPIO_NUM;
   config.pin_d5 = Y7_GPIO_NUM;
   config.pin_d6 = Y8_GPIO_NUM;
   config.pin_d7 = Y9_GPIO_NUM;
   config.pin_xclk = XCLK_GPIO_NUM;
   config.pin_pclk = PCLK_GPIO_NUM;
   config.pin_vsync = VSYNC_GPIO_NUM;
   config.pin_href = HREF_GPIO_NUM;
   config.pin_sccb_sda = SIOD_GPIO_NUM;
   config.pin_sccb_scl = SIOC_GPIO_NUM;
   config.pin_pwdn = PWDN_GPIO_NUM;
   config.pin_reset = RESET_GPIO_NUM;
   config.xclk_freq_hz = 20000000;
   config.pixel_format = PIXFORMAT_JPEG; 

   if(psramFound()){
    config.frame_size = FRAMESIZE_SVGA; 
    config.jpeg_quality = 10;  
    config.fb_count = 2;
   } 
   else {
    config.frame_size = FRAMESIZE_CIF;
    config.jpeg_quality = 12;  //0-63 lower number means higher quality
    config.fb_count = 1;
  }
  
  // 𓏏𓍋𓈖𓅱 camera init
    esp_err_t err = esp_camera_init(&config); 
   if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    delay(1000);
    ESP.restart();
  }

  // 𓏏𓍋𓈖𓅱 SD Card init
   if (!SD_MMC.begin()) {
    Serial.println("SD Card Mount Failed - trying 1-bit mode...");
    if (!SD_MMC.begin("/sdcard", true)) {
      Serial.println("SD Card Mount Failed in 1-bit mode too");
    } else {
      Serial.println("SD Card initialized in 1-bit mode");
    }
   } else {
    uint8_t cardType = SD_MMC.cardType();
    if (cardType == CARD_NONE) {
      Serial.println("No SD card attached");
    } else {
      Serial.println("SD Card initialized successfully");
      Serial.printf("SD Card Size: %lluMB\n", SD_MMC.cardSize() / (1024 * 1024));
    }
   }

   sendPhoto(); 
   previousMillis = millis();
}

void loop() {
  
  // 𓏏𓍋𓈖𓅱 Always check WiFi first

    if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi lost! Reconnecting...");
    WiFi.begin(ssid, password);
    int tries = 0;
    while (WiFi.status() != WL_CONNECTED && tries < 20) {
      delay(500);
      Serial.print(".");
      tries++;
    }
  }

   unsigned long currentMillis = millis();
   if (currentMillis - previousMillis >= timerInterval) {
    sendPhoto();
    previousMillis = currentMillis;
  }
}
 // 𓏏𓍋𓈖𓅱 Store the photo
  String sendPhoto() {
  String getAll;
  String getBody;

  camera_fb_t * fb = NULL; 
  fb = esp_camera_fb_get();
  int retries = 0;
  while (!fb && retries < 3) {
    Serial.println("Camera capture failed, retrying...");
    delay(1000);
    fb = esp_camera_fb_get();
    retries++;
  }
  if (!fb) {
    Serial.println("Camera totally failed, restarting...");
    delay(3000);
    ESP.restart();
  }

  // 𓏏𓍋𓈖𓅱 Save to SD card 

    struct tm timeinfo;
   char timeStr[30];
   if (getLocalTime(&timeinfo)) {
    strftime(timeStr, sizeof(timeStr), "%Y-%m-%d_%H-%M-%S", &timeinfo);
   } else {
    sprintf(timeStr, "%lu", millis());
   }
   String path = "/photo_" + String(timeStr) + ".jpg";
   fs::FS &fs = SD_MMC;
   File file = fs.open(path.c_str(), FILE_WRITE);
   if (!file) {
     Serial.println("Failed to open file for writing on SD card");
   }
   else {
     file.write(fb->buf, fb->len);
     file.close();
     Serial.println("Photo saved to SD: " + path);
   }

  // 𓏏𓍋𓈖𓅱 Communication between server and CAM

   Serial.println("Connecting to server: " + serverName);
  
   client.setInsecure(); 
   if (client.connect(serverName.c_str(), serverPort)) {
    Serial.println("Connection successful!"); 

  // 𓏏𓍋𓈖𓅱 DATA

    String head = "--RandomNerdTutorials\r\nContent-Disposition: form-data; name=\"imageFile\"; filename=\"esp32-cam.jpg\"\r\nContent-Type: image/jpeg\r\n\r\n";
    String tail = "\r\n--RandomNerdTutorials--\r\n"; 

    uint32_t imageLen = fb->len; 
    uint32_t extraLen = head.length() + tail.length();
    uint32_t totalLen = imageLen + extraLen;

  // 𓏏𓍋𓈖𓅱 Communication with HTTP 
    client.println("POST " + serverPath + " HTTP/1.1");
    client.println("Host: " + serverName);
    client.println("Content-Length: " + String(totalLen));
    client.println("Content-Type: multipart/form-data; boundary=RandomNerdTutorials");
    client.println();
    client.print(head);
  
  // 𓏏𓍋𓈖𓅱 Send the photo DATA

     uint8_t *fbBuf = fb->buf;
     size_t fbLen = fb->len;
     for (size_t n=0; n<fbLen; n=n+1024) {
      if (n+1024 < fbLen) {
        client.write(fbBuf, 1024);
        fbBuf += 1024;
      }
      else if (fbLen%1024>0) {
        size_t remainder = fbLen%1024;
        client.write(fbBuf, remainder);
      }
     }   
     client.print(tail);
    
 //𓏏𓍋𓈖𓅱 Get a respond from server  

    esp_camera_fb_return(fb);
    
    int timoutTimer = 10000;
    long startTimer = millis();
    boolean state = false;
    
    while ((startTimer + timoutTimer) > millis()) {
      Serial.print(".");
      delay(100);      
      while (client.available()) {
        char c = client.read();
        if (c == '\n') {
          if (getAll.length()==0) { state=true; }
          getAll = "";
        }
        else if (c != '\r') { getAll += String(c); }
        if (state==true) { getBody += String(c); }
        startTimer = millis();
      }
      if (getBody.length()>0) { break; }
    }
    Serial.println();
    client.stop();
    Serial.println(getBody);
  }
  // 𓏏𓍋𓈖𓅱 If the communication failed
    else {
    esp_camera_fb_return(fb);
    getBody = "Connection to " + serverName +  " failed.";
    Serial.println(getBody);
   }
   return getBody;
}
//𓏏𓍋𓈖𓅱 THE END 𓏏𓍋𓈖𓅱//
