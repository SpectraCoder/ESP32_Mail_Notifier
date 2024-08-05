/// Make a copy of this file, and rename it to "config.h" to make it work with the EPS32_Mail_Notifier.ino script.
/// Change the variables in ths file to your needs. 
#define WIFI_SSID "The name of your WiFi network"
#define WIFI_PASSWORD "The password of your WiFi network"
#define SENSOR_PIN GPIO_NUM_1 /// The sensor pin that will wake the ESP32 from deep sleep
#define STATUS_LED_PIN GPIO_NUM_3 /// The built in green LED of the Lilygo T8-C3 is connected to GPIO_NUM_3
#define BATTERY_STATUS_PIN GPIO_NUM_0
#define TIMER_SLEEP_MICROSECS 1 * 60 * 1000000  /// When on timer interrupt how long to sleep in minutes * seconds * microseconds

/// The smtp host name e.g. smtp.gmail.com for GMail or smtp.office365.com for Outlook or smtp.mail.yahoo.com
#define SMTP_HOST "smtp.example.com"
#define SMTP_PORT 999

/// The sign in credentials
#define AUTHOR_NAME "Name"
#define AUTHOR_EMAIL "email@example.com"
#define AUTHOR_PASSWORD "password"

#define MAIL_SUBJECT "You've got mail!"

/// Recipient's email
#define RECIPIENT_NAME "Name"
#define RECIPIENT_EMAIL "emal@example.com"