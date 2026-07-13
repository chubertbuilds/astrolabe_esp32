<h1 align=center>
  Astrolabe
</h1>
<h3 align=center>
  An ESP32-S3 implementation of the historical scientific instrument
</h3>

## Licensing
The hardware and software designs in this repository are licensed separately:

* **Software:** The .ino sketch in the root folder and all files in "src" are licensed under the [MIT License](LICENSE).
* **Hardware:** The CAD files in "hardware" are all licensed under the [CERN-OHL-S License](LICENSE-HW).

## Introduction
An [astrolabe](https://en.wikipedia.org/wiki/Astrolabe) is a historical scientific instrument which was used to tell the time, map the sky, and find directions—among, it is obligatory to say, many other applications.

This project provides the hardware and software files to build a modern, microcontroller-driven version of the astrolabe. Given a user-inputted date, time, and location, it maps the fixed stars and seven classical planets on a 4''x4'' LCD panel. A secondary 20x4 character LCD screen displays detailed information about the sky.

## Components

I used the following components in this build:

| Component  | Quantity
| --- | --- |
|[Qualia ESP32-S3 Dev Board](https://www.adafruit.com/product/5800)| 1
|[4'' 720 x 720 Display](https://www.adafruit.com/product/5793)| 1
|[i2c LCD Backpack](https://www.adafruit.com/product/292)| 1
|[20x4 Character Display](https://www.adafruit.com/product/198)| 1
|[Rotary Encoder Breakout w/ Encoder](https://www.adafruit.com/product/5880)| 1
|[PCF8574 I2C Breakout](https://www.adafruit.com/product/5545)| 1
|[16mm RGB Momentary Button](https://www.adafruit.com/product/3350)| 2
|[200mm JST SH 4-Pin Cables](https://www.adafruit.com/product/4401)| 3
|[40-pin FPC Extension](https://www.adafruit.com/product/2098)| 1
|3-pin JST PH Cable male connector w/ pigtails | 1 |
|Pair of 2-pin JST male-female connectors w/ pigtails | 1 |
| Pair of 6-pin JST male-female connectors w/ pigtails | 2|
| [10-turn 1K ohm Potentiometer](https://www.amazon.com/Taiss-Precision-Potentiometer-Adjustable-3590S-2-103L/dp/B07D7YH9N2/) | 1 |
| 110 ohm trimmer resistor | 1 |
| [#4 1/4'' Thread-Forming Phillips Screws](https://www.mcmaster.com/90417A116/) | 50
| [5/16'' Ø Delrin Balls](https://www.mcmaster.com/products/9614k57/) | 12
| [0.06'' Ø 1/8'' Thick Neodymium Magnets](https://www.mcmaster.com/catalog/5862K11) | 16
| [4-40 1/4'' Set Screws](https://www.mcmaster.com/products/92311a106/) | 4

Where possible, I'm linking to the exact component I purchased. Substitutions can be made as needed.

## 3D Printing

The twelve 3D-printed parts used in this project are all listed in the [hardware](hardware) folder. All of these parts must be printed once, except for Astrolabe_BearingCage, which must be printed twice.

I printed all these parts with a Bambu P1S printer, using a combination of matte black and silk gold PLA filament. Given that my setup will likely differ substantially from yours, I haven't provided .3mf files (also, to tell the truth, I didn't do a great job of record-keeping with my print files).

Some 3D printing pointers:

- Some parts need to be printed with their counterbores upside down. For these parts, there are features on the counterbores which force the slicer to use bridges, eliminating the need for supports.
- While some parts can be printed in orientations that eliminate supports, those orientations may not be ideal. For example, the encoder knob can be printed with the knob face on the print bed; this orientation doesn't need supports, but it makes the knob less shiny.
- When printing with silk gold PLA, I recommend lowering speeds to <50 mm/s for the outer walls and the top layer.
- When printing with supports, I recommend using a PLA-incompatible material, such as PETG or the proprietary Bambu "Support for PLA/PETG," for the support interface.

Finally, several parts interface with tight clearance fits. Depending on your setup, you may need to modify dimensions to get a good fit. If you need to do so, or you want to make any other change to the design, you can access .STEP files [here.](hardware/step/)


## Build Instructions

### Tools
This build uses the following tools and consumables:

- A #1 phillips head screwdriver
- A 0.05'' Allen wrench
- A soldering station
- Epoxy glue 
- Nylon tape
- A 4-40 tap

### Soldering
Before beginning assembly, all the soldering work should be complete. The soldering work is comprised of the following tasks:

  1. Solder the character display backpack to the character display. Note: the screw terminal on the backpack needs to be removed, as it doesn't fit in the build. This can be done via a skillful, dextrous desoldering job, or you can use a pair of pliers to rip the screw terminal off the PCB. Guess which path I took.
  2. Solder the 3-pin JST PH cable to the prongs of the potentiometer. On the Qualia's 3-pin JST connector, the left pin is signal, the middle pin is power, and the right pin is ground. Connect signal to potentiometer pin 2, power to potentiometer pin 1, and ground to potentiometer pin 3.
  3. Solder one half of the 2-pin JST connector pair to the Qualia board. Solder one wire to MOSI and the other to MISO. We are repurposing these extra GPIO pins for interrupt signals.
  4. Solder the other half of the 2-pin connector pair to the interrupt pins of the encoder breakout and the GPIO expander. Connect MOSI to the encoder interrupt pin and MISO to the expander interrupt pin.
  5. For both buttons, solder one half of a 6-pin JST connector to the button. Each button comes with six prongs: an anode, two button contacts, and three LED cathodes.
  6. Solder the other half of the two 6-pin connectors to the GPIO expander. For the button-expander connections, solder the anode to power, the three LED cathodes to GPIO pins, one of the button contacts to ground, and the other button contact to a GPIO pin. All eight of the GPIO expander's pins should be occupied. The existing code uses the following pin assignments: 

      ~~~
      #define RIGHT_BLUE 0 
      #define RIGHT_GREEN 1 
      #define RIGHT_RED 2 
      #define RIGHT_BUTTON 3 
      #define LEFT_BLUE 4 
      #define LEFT_GREEN 5 
      #define LEFT_RED 6 
      #define LEFT_BUTTON 7 
      ~~~

### Bearing Cage Magnet Insertion

Before assembly, we also need to insert and glue magnets into the two bearing cage halves.

It's important to keep all the magnetic poles aligned—one of the cage halves must have all its magnets mounted with the north pole facing out, and one of them must have all the magnets with the south pole facing out.

To cleanly insert and glue the magnets:

1. Create a reservoir of epoxy.
2. Use a flathead screwdriver to pick up the magnet. The side that will face out should be contacting the tool.
3. Dip the other end of the magnet in the epoxy reservoir.
4. Use the screwdriver to press the magnet into the cage bore. 
5. Once the magnet has been fully inserted, slide the screwdriver off to break contact with the magnet. This breaks contact without pulling the magnet back out of the bore.
6. Let the cage sit for your epoxy's cure time.

### Tapping the Axle

The potentiometer axle uses four set screws to couple it to the potentiometer shaft. To insert these set screws into the axle, the four through holes in the axle must be tapped with a 4-40 tap.

To be sure, it is absolutely not best practice to directly tap a 3D-printed part, given that the resulting threads are quite weak. That said, they are strong enough for our application.

### Assembly

After printing, soldering, gluing, and tapping, it's time to build!

1. Gather all the 3D-printed parts and components on a large, clear workspace.
![alt text](docs\20260702_174930_001.jpg)
2. Assemble the bearing. 
    - Arrange the inner bearing inside the outer bearing eccentrically, such that the outside of the inner bearing is making contact with the inside of the outer bearing at one point.
    - This will create a large gap on the opposite side of the contact point, through which the nylon balls can be inserted. Insert all twelve.
    ![alt text](docs\20260702_175514.jpg)
    - As the balls are inserted, arrange them in a roughly even spacing. This will cause the inner and outer bearing to become concentric.
    - Take one half of the bearing cage and place it under the balls. The balls will not be inside their pockets, so the cage will not be fully seated.
    - Using a long tool such as a screwdriver, push the balls along the track until each ball is over a pocket. Once this happens, the assembly will fall into place over the bearing cage.
    ![alt text](docs\20260702_175757.jpg)
    - Place the other half of the bearing cage over the assembly; the magnets will lock it into place.
    ![alt text](docs\20260702_175843.jpg)
    - Spin the bearing around like a fidget toy.
3. Assemble the display enclosure.
    - Insert the display inside the display mount, facing out.
    ![alt text](docs\20260702_175927.jpg)
    - Fit the display base on the other side of the display mount, sandwiching the display between the two parts.
    ![alt text](docs\20260702_180007.jpg)
    - Fasten the display mount and base together with four screws.
4. Insert the potentiometer into the housing base.
5. Capture the potentiometer with the potentiometer cap and three screws.
![alt text](docs\20260702_180230.jpg)
6. Connect one JST SH 4-pin cable (hereafter referred to as an I2C cable) to the I2C port on the Qualia dev board.
7. Connect the potentiometer connector to the 3-pin JST port on the Qualia dev board.
8. Fasten the Qualia to the four bosses on the bottom of the housing base.
![alt text](docs\20260702_181347.jpg)
9. Insert one end of the 40-pin FPC extension cable into the FPC extension board, and the other end into the Qualia. The metal pins **must be facing downward, as shown in the photograph.** It is possible to insert the cable backwards and engage the locking mechanism, which will invert the connections and damage your display.
![alt text](docs\20260702_181632.jpg)
10. Fold and tape the extension cable as shown in the photograph. This prevents the cable from interfering with the bearing's rotary action.
![alt text](docs\20260702_182457.jpg)
11. Connect the FPC cable to the display.
![alt text](docs\20260702_182615.jpg)
12. Fasten the display enclosure to the five inner bosses.
![alt text](docs\20260702_182945.jpg)
13. Lower the bearing over the display enclosure and fasten it to the seven outer bosses.
![alt text](docs\20260702_183757.jpg)
14. Slide the potentiometer axle over the potentiometer shaft and tighten it with the hex key.
![alt text](docs\20260702_183823.jpg)
15. Connect the GPIO expander to the Qualia dev board with the I2C cable.
![alt text](docs\20260702_184624.jpg)
16. Use the included nuts to fasten the two buttons and the encoder to the housing lid.
![alt text](docs\20260702_184706.jpg)
17. Connect the GPIO expander and the encoder breakout with an I2C cable.
![alt text](docs\20260702_184834.jpg)
18. Fasten the character LCD to the lid with three screws.
19. Connect the encoder breakout and the character LCD with an I2C cable.
![alt text](docs\20260702_185358.jpg)
20. Fasten the GPIO expander to the two remaining bosses on the bottom of the housing base.
21. Connect the two 6-pin JST connectors.
![alt text](docs\20260702_190156.jpg)
22. With everything connected, fit the housing lid over the housing base. It snaps into place.
![alt text](docs\20260702_190351.jpg)
23. At this point, the build is basically complete—congratulations! To finalize the build, the firmware needs to be uploaded. Follow the instructions in [firmware setup](#firmware-setup).
24. Once the astrolabe gets its firmware flashed, you will see the star map displayed on the main LCD panel. Insert the rete into the slots on the inner bearing, and then rotate the bearing until the rete is aligned with the star map. 
![alt text](docs\20260702_191308.jpg)
25. Press the secondary potentiometer gear over the hex boss, coupling the movement of the rete to animation on the display.
![alt text](docs\20260702_191859.jpg)

## Firmware Setup

We'll use the Arduino IDE to flash the firmware onto the astrolabe.

The Qualia dev board uses an ESP32-S3, and the Qualia itself is a supported board in the Arduino IDE. To start, we need to set up the Arduino IDE with the Qualia dev board. Follow [Adafruit's instructions](https://learn.adafruit.com/adafruit-qualia-esp32-s3-for-rgb666-displays/arduino-ide-setup) to do so.

You'll also need to download several Arduino libraries to use this firmware; these libraries are mostly used to drive the various peripheral chips in this project. These external libraries are:

1. Adafruit XCA9554
2. Adafruit LiquidCrystal
3. Adafruit PCF8574
4. Adafruit Seesaw Library
5. CircularBuffer

Once these libraries are all installed, you should be able to compile the main .ino sketch file and flash it onto your device. I use the following settings, taken from [jfseaman's Qualia examples repo](https://github.com/jfseaman/Qualia_ESP32S3_RGB666):

| Arduino IDE Setting                  | Value                                |
| ------------------------------------ | ------------------------------------ |
| Board                                | **Adafruit Qualia ESP32-S3 RGB666**  |
| Port                                 | Your port                            |
| USB CDC On Boot                      | Enable                               |
| CPU Frequency                        | 240MHZ(WiFi)                         |
| Core Debug Level                     | None                                 |
| USB DFU On Boot                      | Disable                              |
| Erase All Flash Before Sketch Upload | Disable                              |
| Events Run On                        | Core1                                |
| Flash Mode                           | QIO 80MHZ                            |
| Flash Size                           | **16MB(128Mb)**                      |
| Arduino Runs On                      | Core1                                |
| USB Firmware MSC On Boot             | Disable                              |
| Partition Scheme                     | **Default(6.25MB APP/3.43MB FATFS)** |
| PSRAM                                | **OPI PSRAM**                        |
| Upload Mode                          | **UART0/Hardware CDC**               |
| Upload Speed                         | 921600                               |
| USB Mode                             | **CDC and JTAG**                     |


## User Settings

The .ino sketch contains the following lines of code:

~~~
bool DST = false; //insert your own default DST setting
int UTC_offset = 0; //insert your own default timezone

...

float latitude = 0.1; //insert your own default latitude
float longitude = 0.0; //insert your own default longitude
~~~

Modifying these values will modify your astrolabe's default settings. 









