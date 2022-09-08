// send every 4h 24 pictures of each of the last 10 min intervals.
// previous default: every 6h, half-hourly.

#include "esp_camera.h"
#include "SPI.h"
#include "driver/rtc_io.h"
#include "ESP32_MailClient.h"
#include <FS.h>
#include <SPIFFS.h>
#include <WiFi.h>
#include "time.h"

const char* ssid = "YOUR WIFI NAME";
const char* password = "YOUR WIFI PASSWORD";

#define sender_email            "YOURACTUALEMAILADDRESS@gmail.com"
#define sender_email_password   "YOURGMAILAPPPASSWORD"
#define SMTP_Server             "smtp.gmail.com"
#define SMTP_Server_Port        465
#define email_subject           "ESP32-CAM Image Capture"
#define email_recipient         "YOURACTUALEMAILADDRESS@gmail.com"


#define CAMERA_MODEL_AI_THINKER

#if defined(CAMERA_MODEL_AI_THINKER)
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
#else
  #error "Camera model not selected"
#endif

SMTPData smtpData;
int loopcounter = 0;
int imagecounter = 0;

#define IMAGE_PATH "/imageXA.jpg"

void copyFile(String ORIGINALFILE, String COPYCREATED) {
  char bytebuffer[64];

  Serial.print("COPY TO ");
  Serial.print(COPYCREATED);


  if (SPIFFS.exists(COPYCREATED) == true) {
    SPIFFS.remove(COPYCREATED);
  }

  File fileorig = SPIFFS.open(ORIGINALFILE, FILE_READ);
  if (!fileorig) {
    Serial.println("error: cannot open original file for reading");
    SPIFFS.end(); ESP.restart();
  }

  File filecopy = SPIFFS.open(COPYCREATED, FILE_WRITE);
  if (!filecopy) {
    Serial.println("error: cannot open copy file for writing");
    SPIFFS.end(); ESP.restart();
  }
   
  while (fileorig.available() > 0) {
    byte byt = fileorig.readBytes(bytebuffer, 64);
    filecopy.write((byte *)bytebuffer, byt);
    // Serial.print(".");
  }
  

  fileorig.close();
  filecopy.close();
  Serial.println(" ... COPYING DONE.");
}

void setup() {
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); //disable brownout detector
  
  delay(2000);
  SPIFFS.begin(); SPIFFS.format(); SPIFFS.end();
  delay(1000);
  Serial.begin(115200);
  Serial.println("Formatted file system.");
  Serial.flush();
  delay(1000);
    
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi...");
  int wifiwaiter = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    wifiwaiter++;
    if (wifiwaiter > 1200) { // tried to get Wifi for 10 minutes
      SPIFFS.end(); ESP.restart();
    }
  }
  Serial.println();
  
  if (!SPIFFS.begin(true)) {
    Serial.println("An Error has occurred while mounting SPIFFS");
    SPIFFS.end(); ESP.restart();
  }
  else {
    delay(500);
    Serial.println("SPIFFS mounted successfully");
  }
  Serial.print("IP Address: http://");
  Serial.println(WiFi.localIP());
   
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
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;
  
  if(psramFound()){
    /*
    FRAMESIZE_96X96,    // 96x96
    FRAMESIZE_QQVGA,    // 160x120
    FRAMESIZE_QCIF,     // 176x144
    FRAMESIZE_HQVGA,    // 240x176
    FRAMESIZE_240X240,  // 240x240
    FRAMESIZE_QVGA,     // 320x240
    FRAMESIZE_CIF,      // 400x296
    FRAMESIZE_HVGA,     // 480x320
    FRAMESIZE_VGA,      // 640x480
    FRAMESIZE_SVGA,     // 800x600
    FRAMESIZE_XGA,      // 1024x768
    FRAMESIZE_HD,       // 1280x720
    FRAMESIZE_SXGA,     // 1280x1024
    FRAMESIZE_UXGA,     // 1600x1200
    // 3MP Sensors
    FRAMESIZE_FHD,      // 1920x1080
    FRAMESIZE_P_HD,     //  720x1280
    FRAMESIZE_P_3MP,    //  864x1536
    FRAMESIZE_QXGA,     // 2048x1536
    // 5MP Sensors
    FRAMESIZE_QHD,      // 2560x1440
    FRAMESIZE_WQXGA,    // 2560x1600
    FRAMESIZE_P_FHD,    // 1080x1920
    FRAMESIZE_QSXGA,    // 2560x1920
    */
    
    config.frame_size = FRAMESIZE_SVGA; // was: FRAMESIZE_SXGA;
    config.jpeg_quality = 12; // 10 or 12
    config.fb_count = 2;
  } else {
    config.frame_size = FRAMESIZE_VGA; // or SVGA
    config.jpeg_quality = 10; // 10 or 12
    config.fb_count = 1;
  }

  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    SPIFFS.end(); ESP.restart();
    return;
  }

  configTime(0, 0, "time.google.com"); // time.google.com or pool.ntp.org

}

void loop() {

    delay(60000); // 60000 = 1 min

    loopcounter++;
    if (loopcounter == 10) { // every ten minutes --> 3h periods, 8 mails/24h
        loopcounter = 0;
        captureSave_image();

        if (imagecounter == 0) {;
          copyFile(IMAGE_PATH, "/imageXR.jpg");
        } else if (imagecounter == 1) {
          copyFile(IMAGE_PATH, "/imageXQ.jpg");
        } else if (imagecounter == 2) {
          copyFile(IMAGE_PATH, "/imageXP.jpg");
        } else if (imagecounter == 3) {
          copyFile(IMAGE_PATH, "/imageXO.jpg");
        } else if (imagecounter == 4) {
          copyFile(IMAGE_PATH, "/imageXN.jpg");
        } else if (imagecounter == 5) {
          copyFile(IMAGE_PATH, "/imageXM.jpg");
        } else if (imagecounter == 6) {
          copyFile(IMAGE_PATH, "/imageXL.jpg");
        } else if (imagecounter == 7) {
          copyFile(IMAGE_PATH, "/imageXK.jpg");
        } else if (imagecounter == 8) {
          copyFile(IMAGE_PATH, "/imageXJ.jpg");
        } else if (imagecounter == 9) {
          copyFile(IMAGE_PATH, "/imageXI.jpg");
        } else if (imagecounter == 10) {
          copyFile(IMAGE_PATH, "/imageXH.jpg");
        } else if (imagecounter == 11) {
          copyFile(IMAGE_PATH, "/imageXG.jpg");
        } else if (imagecounter == 12) {
          copyFile(IMAGE_PATH, "/imageXF.jpg");
        } else if (imagecounter == 13) {
          copyFile(IMAGE_PATH, "/imageXE.jpg");
        } else if (imagecounter == 14) {
          copyFile(IMAGE_PATH, "/imageXD.jpg");
        } else if (imagecounter == 15) {
          copyFile(IMAGE_PATH, "/imageXC.jpg");
        } else if (imagecounter == 16) {
          copyFile(IMAGE_PATH, "/imageXB.jpg");
        } else if (imagecounter == 17) {
          sendImage();
          imagecounter = -1; // to become 0 immedialy below
          SPIFFS.end(); ESP.restart(); // optional
        }
        imagecounter++;
        Serial.print((long) SPIFFS.usedBytes()); Serial.print(" ex "); Serial.println((long) SPIFFS.totalBytes());
    } 

}

// Check if photo capture was successful
bool check_photo( fs::FS &fs ) {
  File f_pic = fs.open( IMAGE_PATH );
  unsigned int pic_sz = f_pic.size();
  return ( pic_sz > 100 );
}

// Capture Photo and Save it to SPIFFS
void captureSave_image( void ) {
  camera_fb_t * fb = NULL; 
  bool ok = 0;

  do {
    Serial.println("ESP32-CAM capturing photo...");

    fb = esp_camera_fb_get();
    if (!fb) {
      Serial.println("Failed");
      return;
    }
    Serial.printf("Picture file name: %s\n", IMAGE_PATH);
    File file = SPIFFS.open(IMAGE_PATH, FILE_WRITE);
    if (!file) {
      Serial.println("Failed to open file in writing mode");
    }
    else {
      file.write(fb->buf, fb->len); 
      Serial.print("The picture has been saved in ");
      Serial.print(IMAGE_PATH);
      Serial.print(" - Size: ");
      Serial.print(file.size());
      Serial.println(" bytes");
    }
   
    file.close();
    esp_camera_fb_return(fb);

    ok = check_photo(SPIFFS);
  } while ( !ok );
}

void sendImage( void ) {
  smtpData.setLogin(SMTP_Server, SMTP_Server_Port, sender_email, sender_email_password);
  smtpData.setSender("ESP32-CAM", sender_email);
  smtpData.setPriority("High");
  smtpData.setSubject(email_subject);
  smtpData.setMessage("ESP32-CAM Periodic Captured Image.", false);
  smtpData.addRecipient(email_recipient);
  smtpData.addAttachFile(IMAGE_PATH, "image/jpg");

  if (imagecounter > 0) {
    smtpData.addAttachFile("/imageXR.jpg", "image/jpg");
  }

  if (imagecounter > 1) {
    smtpData.addAttachFile("/imageXQ.jpg", "image/jpg");
  }

  if (imagecounter > 2) {
    smtpData.addAttachFile("/imageXP.jpg", "image/jpg");
  }

  if (imagecounter > 3) {
    smtpData.addAttachFile("/imageXO.jpg", "image/jpg");
  }

  if (imagecounter > 4) {
    smtpData.addAttachFile("/imageXN.jpg", "image/jpg");
  }

  if (imagecounter > 5) {
    smtpData.addAttachFile("/imageXM.jpg", "image/jpg");
  }

  if (imagecounter > 6) {
    smtpData.addAttachFile("/imageXL.jpg", "image/jpg");
  }

  if (imagecounter > 7) {
    smtpData.addAttachFile("/imageXK.jpg", "image/jpg");
  }

  if (imagecounter > 8) {
    smtpData.addAttachFile("/imageXJ.jpg", "image/jpg");
  }

  if (imagecounter > 9) {
    smtpData.addAttachFile("/imageXI.jpg", "image/jpg");
  }

  if (imagecounter > 10) {
    smtpData.addAttachFile("/imageXH.jpg", "image/jpg");
  }

  if (imagecounter > 11) {
    smtpData.addAttachFile("/imageXG.jpg", "image/jpg");
  }

  if (imagecounter > 12) {
    smtpData.addAttachFile("/imageXF.jpg", "image/jpg");
  }

  if (imagecounter > 13) {
    smtpData.addAttachFile("/imageXE.jpg", "image/jpg");
  }

  if (imagecounter > 14) {
    smtpData.addAttachFile("/imageXD.jpg", "image/jpg");
  }

  if (imagecounter > 15) {
    smtpData.addAttachFile("/imageXC.jpg", "image/jpg");
  }

  if (imagecounter > 16) {
    smtpData.addAttachFile("/imageXB.jpg", "image/jpg");
  }


  smtpData.setFileStorageType(MailClientStorageType::SPIFFS);
  smtpData.setSendCallback(sendCallback);
  
  if (!MailClient.sendMail(smtpData)) {
    Serial.println("Error sending Email, " + MailClient.smtpErrorReason());
    SPIFFS.end(); ESP.restart();
  }

  smtpData.empty();
}

void sendCallback(SendStatus msg) {
  Serial.println(msg.info());
}
