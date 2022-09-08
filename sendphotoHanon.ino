/**
 * Camera motion detection + JPEG capture + Mail transmission
 * BASED ON:
 * https://eloquentarduino.com/projects/esp32-arduino-motion-detection/
 * and
 * https://raw.githubusercontent.com/RuiSantosdotme/ESP32-CAM-eBook/master/Code/Module_3/Send_Photos_Email/Send_Photos_Email.ino
 * as well as
 * https://www.esp8266.com/viewtopic.php?f=13&t=22170
 * https://www.esp8266.com/viewtopic.php?p=51048
 * Board: AI Thinker ESP32-CAM CPU Speed: 160MHz
 */

#define MSGSUBJ "ESP32 CAM image"

// replace with your own values
#define WIFI_SSID "YOUR WIFI NAME"
#define WIFI_PASSWORD "YOUR WIFI PASSWORD"

#define sender_email            "YOURACTUALEMAILADDRESS@gmail.com"
#define sender_email_password   "YOURGMAILAPPPASSWORD"
#define SMTP_Server             "smtp.gmail.com"
#define SMTP_Server_Port        465
#define email_subject1          "Regular ESP32-CAM Images"
#define email_subject2          "Motion-Activated ESP32-CAM Images"
#define email_recipient         "YOURACTUALEMAILADDRESS@gmail.com"

#include <WiFi.h>
#include <eloquent.h> /* GET THIS */
#include <eloquent/vision/image/gray/custom.h>
#include <eloquent/vision/motion/naive.h>
#include <eloquent/fs/spiffs.h>
#include <eloquent/networking/wifi.h>


#include "esp_camera.h"
#include "SPI.h"
#include "driver/rtc_io.h"
#include "ESP32_MailClient.h"
#include <FS.h>
#include <SPIFFS.h>

// uncomment based on your camera and resolution

//#include "eloquent/vision/camera/ov767x/gray/vga.h"
//#include "eloquent/vision/camera/ov767x/gray/qvga.h"
//#include "eloquent/vision/camera/ov767x/gray/qqvga.h"
//#include "eloquent/vision/camera/esp32/aithinker/gray/vga.h"

#include "eloquent/vision/camera/esp32/aithinker/gray/vga.h"

//#include "eloquent/vision/camera/esp32/aithinker/gray/qvga.h"
//#include "eloquent/vision/camera/esp32/aithinker/gray/qqvga.h"
//#include "eloquent/vision/camera/esp32/wrover/gray/vga.h"
//#include "eloquent/vision/camera/esp32/wrover/gray/qvga.h"
//#include "eloquent/vision/camera/esp32/wrover/gray/qqvga.h"
//#include "eloquent/vision/camera/esp32/eye/gray/vga.h"
//#include "eloquent/vision/camera/esp32/eye/gray/qvga.h"
//#include "eloquent/vision/camera/esp32/eye/gray/qqvga.h"
//#include "eloquent/vision/camera/esp32/m5/gray/vga.h"
//#include "eloquent/vision/camera/esp32/m5/gray/qvga.h"
//#include "eloquent/vision/camera/esp32/m5/gray/qqvga.h"
//#include "eloquent/vision/camera/esp32/m5wide/gray/vga.h"
//#include "eloquent/vision/camera/esp32/m5wide/gray/qvga.h"
//#include "eloquent/vision/camera/esp32/m5wide/gray/qqvga.h"


#define THUMB_WIDTH 32
#define THUMB_HEIGHT 24

SMTPData smtpData;
long loopcounter = 0;
int imagecounter = 0;
int subjselector = 1;

// allocate memory to store the thumbnail into a new image
Eloquent::Vision::Image::Gray::CustomWithBuffer<THUMB_WIDTH, THUMB_HEIGHT> thumbnail;
Eloquent::Vision::Motion::Naive<THUMB_WIDTH, THUMB_HEIGHT> detector;

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

void sendCallback(SendStatus msg) {
  Serial.println(msg.info());
}

void sendImage( void ) {

  smtpData.setLogin(SMTP_Server, SMTP_Server_Port, sender_email, sender_email_password);
  smtpData.setSender("ESP32-CAM", sender_email);
  smtpData.setPriority("High");
  if (subjselector == 1) {
    smtpData.setSubject(email_subject1);
  } else {
    smtpData.setSubject(email_subject2);
  }
  smtpData.setMessage(MSGSUBJ, false);
  smtpData.addRecipient(email_recipient);
  smtpData.addAttachFile("/imageA.jpg", "image/jpg");

  if (imagecounter > 0) {
    smtpData.addAttachFile("/imageP.jpg", "image/jpg");
  }

  if (imagecounter > 1) {
    smtpData.addAttachFile("/imageO.jpg", "image/jpg");
  }

  if (imagecounter > 2) {
    smtpData.addAttachFile("/imageN.jpg", "image/jpg");
  }

  if (imagecounter > 3) {
    smtpData.addAttachFile("/imageM.jpg", "image/jpg");
  }

  if (imagecounter > 4) {
    smtpData.addAttachFile("/imageL.jpg", "image/jpg");
  }

  if (imagecounter > 5) {
    smtpData.addAttachFile("/imageK.jpg", "image/jpg");
  }

  if (imagecounter > 6) {
    smtpData.addAttachFile("/imageJ.jpg", "image/jpg");
  }

  if (imagecounter > 7) {
    smtpData.addAttachFile("/imageI.jpg", "image/jpg");
  }

  if (imagecounter > 8) {
    smtpData.addAttachFile("/imageH.jpg", "image/jpg");
  }

  if (imagecounter > 9) {
    smtpData.addAttachFile("/imageG.jpg", "image/jpg");
  }

  if (imagecounter > 10) {
    smtpData.addAttachFile("/imageF.jpg", "image/jpg");
  }

  if (imagecounter > 11) {
    smtpData.addAttachFile("/imageE.jpg", "image/jpg");
  }

  if (imagecounter > 12) {
    smtpData.addAttachFile("/imageD.jpg", "image/jpg");
  }

  if (imagecounter > 13) {
    smtpData.addAttachFile("/imageC.jpg", "image/jpg");
  }

  if (imagecounter > 14) {
    smtpData.addAttachFile("/imageB.jpg", "image/jpg");
  }
  
  smtpData.setFileStorageType(MailClientStorageType::SPIFFS);
  smtpData.setSendCallback(sendCallback);
  
  if (!MailClient.sendMail(smtpData)) {
    Serial.println("Error sending Email, " + MailClient.smtpErrorReason());
    SPIFFS.end(); ESP.restart();
  }

  smtpData.empty();
}

void imgCap() {
        auto fileA = filesystem.writeBinary("/imageA.jpg");
        auto jpegA = camera.image.jpeg().bestQuality();

        if (jpegA.writeTo(fileA)) {
            Serial.println("Jpeg written");
        }
        else {
            Serial.print("Jpeg encoding error: ");
            Serial.println(jpegA.getErrorMessage());
            SPIFFS.end(); ESP.restart();
        }
        // fileA.close();
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
    filesystem.begin(true); // SPIFFS.end();

    // turn on high freq for fast streaming speed
    camera.setHighFreq();

    if (!camera.begin()) {
        Serial.println("Cannot activate camera");
        SPIFFS.end(); ESP.restart();
    }

    if (!wifi.connectTo(WIFI_SSID, WIFI_PASSWORD)) {
        Serial.println("Cannot connect to WiFi");
        SPIFFS.end(); ESP.restart();
    }


    detector.throttle(10);
    detector.setIntensityChangeThreshold(15);
    detector.setPixelChangesThreshold(0.1);
}


void loop() {
    // Serial.println((int) SPIFFS.totalBytes()); // 896321
  
    if (!camera.capture()) {
        Serial.println(camera.getErrorMessage());
        delay(250);
        SPIFFS.end(); ESP.restart(); // optional
        return;
    }

    // perform motion detection on resized image for fast detection
    // but keep original image for capture at full resolution
    camera.image.resizeTo<THUMB_WIDTH, THUMB_HEIGHT>(thumbnail);
    detector.update(thumbnail);

    // if motion is detected, save original image as jpeg
    if (detector.isMotionDetected()) {
        Serial.print("Motion detected: ");
        Serial.println(imagecounter);

        imgCap();
        
        delay(500);

        if (imagecounter == 0) {;
          copyFile("/imageA.jpg", "/imageP.jpg");
        } else if (imagecounter == 1) {
          copyFile("/imageA.jpg", "/imageO.jpg");
        } else if (imagecounter == 2) {
          copyFile("/imageA.jpg", "/imageN.jpg");
        } else if (imagecounter == 3) {
          copyFile("/imageA.jpg", "/imageM.jpg");
        } else if (imagecounter == 4) {
          copyFile("/imageA.jpg", "/imageL.jpg");
        } else if (imagecounter == 5) {
          copyFile("/imageA.jpg", "/imageK.jpg");
        } else if (imagecounter == 6) {
          copyFile("/imageA.jpg", "/imageJ.jpg");
        } else if (imagecounter == 7) {
          copyFile("/imageA.jpg", "/imageI.jpg");
        } else if (imagecounter == 8) {
          copyFile("/imageA.jpg", "/imageH.jpg");
        } else if (imagecounter == 9) {
          copyFile("/imageA.jpg", "/imageG.jpg");
        } else if (imagecounter == 10) {
          copyFile("/imageA.jpg", "/imageF.jpg");
        } else if (imagecounter == 11) {
          copyFile("/imageA.jpg", "/imageE.jpg");
        } else if (imagecounter == 12) {
          copyFile("/imageA.jpg", "/imageD.jpg");
        } else if (imagecounter == 13) {
          copyFile("/imageA.jpg", "/imageC.jpg");
        } else if (imagecounter == 14) {
          copyFile("/imageA.jpg", "/imageB.jpg");
        } else if (imagecounter == 15) {
          subjselector = 2;
          sendImage();
          SPIFFS.format(); SPIFFS.end(); ESP.restart(); // optional
          imagecounter = -1; // to become 0 immedialy below
          subjselector = 1;
        }
        imagecounter++;
        Serial.print((long) SPIFFS.usedBytes()); Serial.print(" ex "); Serial.println((long) SPIFFS.totalBytes());
    } 

      // delay(250);

      // restart every 6 or 12 hours anyway:
      loopcounter++;
      if (loopcounter == 86400) {
        loopcounter = 0;

        /* take an image just like that */
        imgCap();
        sendImage();
        SPIFFS.format(); SPIFFS.end(); ESP.restart(); // optional
        imagecounter = 0;
        Serial.println("Restarting...");
        // delay(2000);
        SPIFFS.end(); ESP.restart(); // restart every 6 hours, if no motion was detected.
      }

    // release memory
    camera.free();
}
