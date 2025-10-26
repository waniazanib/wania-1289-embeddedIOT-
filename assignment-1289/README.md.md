##### Assignment - Embedded IoT System (Fall 2025)

##### 

##### Name   : Wania Zanib  

##### Reg No.: 1289  

##### Board  : ESP32 Devkit c V4  



---



###### 

###### Task A 



**link**: https://wokwi.com/projects/445865576113924097



**Pin Map**



Component	ESP32 Pin	Description

Mode Button	GPIO 13	        Switches between LED modes

Reset Button	GPIO 12	        Resets back to mode 0

LED 1	        GPIO 18	        LED output

LED 2	        GPIO 19	        LED output

LED 3	        GPIO 23         LED output

OLED SDA	GPIO 21	        I²C Data Line

OLED SCL	GPIO 22	        I²C Clock Line

OLED VCC	3.3V	        Power

OLED GND	GND	        Ground





**Description**



Mode 0 – OFF

Mode 1 – Alternate Blink

Mode 2 – ON

Mode 3 – PWM Fade



Button 2 allow resetting to default.



###### Task B 



**link** : https://wokwi.com/projects/445872733591798785



**Pin Map**



Component	ESP32 Pin	Description

Button	        GPIO 13	        Input (with internal pull-up)

LED	        GPIO 18	        Output LED

Buzzer	        GPIO 27	        Output (plays tone on long press)

OLED SDA	GPIO 21	        I²C Data line

OLED SCL	GPIO 22	        I²C Clock line

OLED VCC	3.3V	        Power

OLED GND	GND	        Ground



**Description**



\- Short press toggles LED ON/OFF  

\- Long press activates buzzer  

\- OLED displays the last event (LED ON, LED OFF, or Long Press)

