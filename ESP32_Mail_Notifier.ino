/// IMPORTANT! This file holds all credentials and settings.
/// Make a copy of config.example.h and rename it to config.h to make this work with your own setup.
#include "config.h"

#include "WiFi.h" /// From ESP32 library
#include "ESP_Mail_Client.h" /// By Mobizt https://github.com/mobizt/ESP-Mail-Client

/// Declare the global used SMTPSession object for SMTP transport
SMTPSession smtp;

/// Callback function to get the Email sending status
void smtpCallback(SMTP_Status status);

void setup()
{  
  Serial.begin(115200);
   
  pinMode(SENSOR_PIN, INPUT);
  pinMode(STATUS_LED_PIN, OUTPUT);
      
  /// Get the battery voltage and percentage values
  float batteryVoltage = getBatteryVoltage(BATTERY_STATUS_PIN);
  uint8_t batteryPercentage = getBatteryPercentage(batteryVoltage);
  
  /// Enable the status led dimly when turning on or waking up.
  analogWrite(STATUS_LED_PIN, 10);

  /// I had to add some delays in the code during development to be able to get it to print something before going into deep sleep mode again.
  delay(1000); 
  Serial.println("Hello");

  /// When booting for the first time, this prevents sending an email.
  if(IsWokenUpBySensorOrTimer())
  {
    int tries = 0;
    int maxTries = 10;

    while(digitalRead(SENSOR_PIN) == LOW)
    {
        /// When the sensor is stuck open, it tries to check for a closed state for 10 times.
        /// If it is still not closed, it sets a timer to wake up at a later moment and try it again.
        if(tries > maxTries)
        {
          Serial.println("Door stuck open!");          
          Serial.printf("Going to sleep for %d seconds. \n", (TIMER_SLEEP_MICROSECS/1000000));
          esp_sleep_enable_timer_wakeup(TIMER_SLEEP_MICROSECS);

          StartDeepSleep();
        }
        
        Serial.printf("Waiting for sensor to close %d/%d \n", tries, maxTries);
        delay(1000);

        tries++;
    }
    
    Serial.println("-----------------| SENDING EMAIL |-----------------");
    ConnectWifi();

    /// In the body of the email that gets sent when there is mail, 
    /// we can show some WiFi signal information, als well as battery information.

    int rssi = WiFi.RSSI();
    int signalStrength = map(rssi, -100, -50, 0, 100);
    signalStrength = constrain(signalStrength, 0, 100);    

    String body = R"(
     Battery voltage: )" + String(batteryVoltage) + R"(v
     Battery percentage: )" + String(batteryPercentage) + R"(%
     WiFi Signal Strength: )" + String(signalStrength) + R"(% (RSSI )" + String(rssi) + R"())";

    SendEmail(MAIL_SUBJECT, body);    
    Serial.println("-----------------| EMAIL SENT |-----------------");
  }
  
  /// Wake up the ESP32 when SENSOR_PIN is pulled low.

  /// For ESP32 Dev Module use this:
  //esp_sleep_enable_ext0_wakeup(SENSOR_PIN, 0); //1 = High, 0 = Low

  /// For ESP32-C3 Dev Module use this:
  esp_deep_sleep_enable_gpio_wakeup(BIT(SENSOR_PIN), ESP_GPIO_WAKEUP_GPIO_LOW);

  StartDeepSleep();
}


void StartDeepSleep()
{    
  Serial.println("Going to sleep now");
  delay(1000);
  //digitalWrite(STATUS_LED_PIN, LOW); /// Disable the status led when going into deep sleep mode
  analogWrite(STATUS_LED_PIN, 0);
  esp_deep_sleep_start();
}


bool IsWokenUpBySensorOrTimer()
{  
  esp_sleep_wakeup_cause_t wakeup_reason;

  wakeup_reason = esp_sleep_get_wakeup_cause();

  switch(wakeup_reason)
  {
    case ESP_SLEEP_WAKEUP_EXT0 : 
      Serial.println("Wakeup caused by external signal using RTC_IO"); 
      return false; /// <--- Set this to true if you're not using an ESP32C3
      break;

    case ESP_SLEEP_WAKEUP_GPIO : 
      Serial.println("Wakeup caused by external signal using GPIO"); 
      return true; /// <--- Set this to false if you're not using an ESP32C3
      break;
      
    case ESP_SLEEP_WAKEUP_EXT1 : 
      Serial.println("Wakeup caused by external signal using RTC_CNTL"); 
      return false; 
      break;

    case ESP_SLEEP_WAKEUP_TIMER : 
      Serial.println("Wakeup caused by timer"); 
      return true; 
      break;

    case ESP_SLEEP_WAKEUP_TOUCHPAD :
      Serial.println("Wakeup caused by touchpad"); 
      return false; 
      break;

    case ESP_SLEEP_WAKEUP_ULP :
      Serial.println("Wakeup caused by ULP program"); 
      return false;
      break;

    default : 
      Serial.printf("Wakeup was not caused by deep sleep: %d\n",wakeup_reason);
      return false; 
      break;
  }
}

void ConnectWifi()
{
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.println("Connecting to Wi-Fi");

  if (WiFi.waitForConnectResult() == WL_CONNECTED)
  {
    Serial.print("Connected with IP: ");
    Serial.println(WiFi.localIP());
  } 
  else 
  {
    Serial.println("Not connected");
  }  
}

void SendEmail(String emailSubject, String emailText)
{
    /*  Set the network reconnection option */
  MailClient.networkReconnect(true);

  /** Enable the debug via Serial port
   * 0 for no debugging
   * 1 for basic level debugging
   *
   * Debug port can be changed via ESP_MAIL_DEFAULT_DEBUG_PORT in ESP_Mail_FS.h
   */
  smtp.debug(1);

  /* Set the callback function to get the sending results */
  smtp.callback(smtpCallback);

  /* Declare the Session_Config for user defined session credentials */
  Session_Config config;

  /* Set the session config */
  config.server.host_name = F(SMTP_HOST);
  config.server.port = SMTP_PORT;
  config.login.email = F(AUTHOR_EMAIL);
  config.login.password = F(AUTHOR_PASSWORD);
  config.login.user_domain = F("");

  /*
  Set the NTP config time
  For times east of the Prime Meridian use 0-12
  For times west of the Prime Meridian add 12 to the offset.
  Ex. American/Denver GMT would be -6. 6 + 12 = 18
  See https://en.wikipedia.org/wiki/Time_zone for a list of the GMT/UTC timezone offsets
  */
  //config.time.ntp_server = F("pool.ntp.org,time.nist.gov");
  //config.time.gmt_offset = 1;
  //config.time.day_light_offset = 1;
  config.time.ntp_server = F("ntppool1.time.nl");
  config.time.gmt_offset = 1;
  config.time.day_light_offset = 1;
  config.time.timezone_env_string = "CET-1CEST,M3.5.0,M10.5.0/3";

  /* Declare the message class */
  SMTP_Message message;

  /* Set the message headers */
  message.sender.name = F(AUTHOR_NAME);
  message.sender.email = F(AUTHOR_EMAIL);
  message.subject = emailSubject;
  message.addRecipient(F(RECIPIENT_NAME), (RECIPIENT_EMAIL));
    
  /*Send HTML message*/
  /*String htmlMsg = "<div style=\"color:#2f4468;\"><h1>Hello World!</h1><p>- Sent from ESP board</p></div>";
  message.html.content = htmlMsg.c_str();
  message.html.content = htmlMsg.c_str();
  message.text.charSet = "us-ascii";
  message.html.transfer_encoding = Content_Transfer_Encoding::enc_7bit;*/

   
  //Send raw text message
  //String textMsg = "Er is post!";
  message.text.content = emailText.c_str();
  message.text.charSet = "us-ascii";
  message.text.transfer_encoding = Content_Transfer_Encoding::enc_7bit;
  
  message.priority = esp_mail_smtp_priority::esp_mail_smtp_priority_normal;
  message.response.notify = esp_mail_smtp_notify_success | esp_mail_smtp_notify_failure | esp_mail_smtp_notify_delay;


  /* Connect to the server */
  if (!smtp.connect(&config)){
    ESP_MAIL_PRINTF("Connection error, Status Code: %d, Error Code: %d, Reason: %s", smtp.statusCode(), smtp.errorCode(), smtp.errorReason().c_str());
    return;
  }

  if (!smtp.isLoggedIn()){
    Serial.println("\nNot yet logged in.");
  }
  else{
    if (smtp.isAuthenticated())
      Serial.println("\nSuccessfully logged in.");
    else
      Serial.println("\nConnected with no Auth.");
  }

  /* Start sending Email and close the session */
  if (!MailClient.sendMail(&smtp, &message))
  {
      ESP_MAIL_PRINTF("Error, Status Code: %d, Error Code: %d, Reason: %s", smtp.statusCode(), smtp.errorCode(), smtp.errorReason().c_str());
  }
}

/* Callback function to get the Email sending status */
void smtpCallback(SMTP_Status status){
  /* Print the current status */
  Serial.println(status.info());

  /* Print the sending result */
  if (status.success()){
    // ESP_MAIL_PRINTF used in the examples is for format printing via debug Serial port
    // that works for all supported Arduino platform SDKs e.g. AVR, SAMD, ESP32 and ESP8266.
    // In ESP8266 and ESP32, you can use Serial.printf directly.

    Serial.println("----------------");
    ESP_MAIL_PRINTF("Message sent success: %d\n", status.completedCount());
    ESP_MAIL_PRINTF("Message sent failed: %d\n", status.failedCount());
    Serial.println("----------------\n");

    for (size_t i = 0; i < smtp.sendingResult.size(); i++)
    {
      /* Get the result item */
      SMTP_Result result = smtp.sendingResult.getItem(i);

      // In case, ESP32, ESP8266 and SAMD device, the timestamp get from result.timestamp should be valid if
      // your device time was synched with NTP server.
      // Other devices may show invalid timestamp as the device time was not set i.e. it will show Jan 1, 1970.
      // You can call smtp.setSystemTime(xxx) to set device time manually. Where xxx is timestamp (seconds since Jan 1, 1970)
      
      ESP_MAIL_PRINTF("Message No: %d\n", i + 1);
      ESP_MAIL_PRINTF("Status: %s\n", result.completed ? "success" : "failed");
      ESP_MAIL_PRINTF("Date/Time: %s\n", MailClient.Time.getDateTimeString(result.timestamp, "%B %d, %Y %H:%M:%S").c_str());
      ESP_MAIL_PRINTF("Recipient: %s\n", result.recipients.c_str());
      ESP_MAIL_PRINTF("Subject: %s\n", result.subject.c_str());
    }
    Serial.println("----------------\n");

    // You need to clear sending result as the memory usage will grow up.
    smtp.sendingResult.clear();
  }
}

void loop()
{  
  /// Mandatory empty loop function.
}

/// https://www.youtube.com/watch?v=qKUrXwkr3cc
/// https://github.com/G6EJD/LiPo_Battery_Capacity_Estimator/blob/master/ReadBatteryCapacity_LIPO.ino
/* An improved battery estimation function 
   This software, the ideas and concepts is Copyright (c) David Bird 2019 and beyond.
   All rights to this software are reserved.
   It is prohibited to redistribute or reproduce of any part or all of the software contents in any form other than the following:
   1. You may print or download to a local hard disk extracts for your personal and non-commercial use only.
   2. You may copy the content to individual third parties for their personal use, but only if you acknowledge
      the author David Bird as the source of the material.
   3. You may not, except with my express written permission, distribute or commercially exploit the content.
   4. You may not transmit it or store it in any other website or other form of electronic retrieval system for commercial purposes.
   5. You MUST include all of this copyright and permission notice ('as annotated') and this shall be included in all copies
      or substantial portions of the software and where the software use is visible to an end-user.
   THE SOFTWARE IS PROVIDED "AS IS" FOR PRIVATE USE ONLY, IT IS NOT FOR COMMERCIAL USE IN WHOLE OR PART OR CONCEPT.
   FOR PERSONAL USE IT IS SUPPLIED WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
   WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
   IN NO EVENT SHALL THE AUTHOR OR COPYRIGHT HOLDER BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
   AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
   OTHER DEALINGS IN THE SOFTWARE.
*/
// String readBattery()
// {
//   uint8_t percentage = 100;
//   float voltage = analogRead(GPIO_NUM_0) / 4096.0 * 7.23;      // LOLIN D32 (no voltage divider need already fitted to board.or NODEMCU ESP32 with 100K+100K voltage divider
//   //float voltage = analogRead(39) / 4096.0 * 7.23;    // NODEMCU ESP32 with 100K+100K voltage divider added
//   //float voltage = analogRead(A0) / 4096.0 * 4.24;    // Wemos / Lolin D1 Mini 100K series resistor added
//   //float voltage = analogRead(A0) / 4096.0 * 5.00;    // Ardunio UNO, no voltage divider required
//   Serial.println("Voltage = " + String(voltage));
//   percentage = 2808.3808 * pow(voltage, 4) - 43560.9157 * pow(voltage, 3) + 252848.5888 * pow(voltage, 2) - 650767.4615 * voltage + 626532.5703;
//   if (voltage > 4.19) percentage = 100;
//   else if (voltage <= 3.50) percentage = 0;
//   return String(percentage)+"%";
// }

float getBatteryVoltage(uint8_t voltagePin)
{ 
  float voltage = analogRead(voltagePin) / 4096.0 * 6.0; /// 6.0 is just a guess to get a estimate reading of the battery level
  return voltage;
}

uint8_t getBatteryPercentage(float voltage)
{ 
  uint8_t percentage = 100;
  percentage = 2808.3808 * pow(voltage, 4) - 43560.9157 * pow(voltage, 3) + 252848.5888 * pow(voltage, 2) - 650767.4615 * voltage + 626532.5703;
  
  if (voltage > 4.19)
  {
      percentage = 100;
  } 
  else if (voltage <= 3.50)
  {
    percentage = 0;
  };

  return percentage;
}

