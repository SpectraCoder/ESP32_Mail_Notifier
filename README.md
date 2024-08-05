# ESP32 Mail Notifier
 Getting digital mail when you have analog mail!

A reed sensor is connected to a [LILYGO T8-C3 ESP32-C3 dev board](https://github.com/Xinyuan-LilyGO/T8-C3). When the reed sensor is opened (the postman opens the lid of the mailbox), it connects to WiFi, sends an email, and goes to deep sleep again.
 
For a full devlog about this project, [check my website](https://spectracoder.com/blog/post/getting-digital-mail-when-you-have-analog-mail).
 
![A working mail notifier.](https://github.com/SpectraCoder/ESP32_Mail_Notifier/blob/main/Images/MailboxNotifier01.jpg?raw=true)

## Usage

To use this in your own setup, make a copy of the [***config.example.h***](https://github.com/SpectraCoder/ESP32_Mail_Notifier/blob/main/config.example.h) file. Rename it to ***config.h***.
Change the contents of the file to your own needs.

## Case

I fitted this project inside of a small lunchbox of approximately 11 x 8 x 5cm.

![The luncbox I used to put it all in.](https://github.com/SpectraCoder/ESP32_Mail_Notifier/blob/main/Images/MailboxNotifier02.jpg?raw=true)
![The finished mailbox sensor.](https://github.com/SpectraCoder/ESP32_Mail_Notifier/blob/main/Images/MailboxNotifier03.jpg?raw=true)
