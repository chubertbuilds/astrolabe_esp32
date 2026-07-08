<h1 align=center>
  Astrolabe
</h1>
<h3 align=center>
  An ESP32-S3 implementation of the historical scientific instrument
</h3>

## Licensing
The hardware and software designs in this repository are licensed separately:

* **Software:** The .ino sketch in the root folder, and all files in "src", are licensed under the [MIT License](LICENSE).
* **Hardware:** The CAD files in "hardware" are all licensed under the [CERN-OHL-S License](LICENSE-HW).

## Introduction
An [astrolabe](https://en.wikipedia.org/wiki/Astrolabe) is a historical scientific instrument which was used to tell the time, map the sky, and find directions—among, it is obligatory to say, many other applications.

This project provides the hardware and software files to build a modern, microcontroller-driven version of the astrolabe. Given a user-inputted date, time, and location, it maps the fixed stars and seven classical planets on a 4''x4'' LCD panel. A secondary 20x4 character LCD screen displays detailed information about the sky.
