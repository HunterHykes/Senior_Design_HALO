# Senior_Design_HALO

This is the repository for the Hard Hat Ambient Liability Observer (H.A.L.O.) I assisted in developing for my senior design project at the University of Akron from Spring 2020 - Sprint 2021.

## What the H.A.L.O. Does
This product is a hard hat attachment which utilizes six time-of-flight (ToF) sensors to detect hazards within a 1.5m radius from the wearer's head with a 150° field of view. Dependent on the distance between the user and the hazard, some RGB LEDs placed near the brim of the hard hat will turn on and change hue from yellow to red as objects approach. This enhances the awareness of the user of potential hazards.
In the event that the user hits their head either from a collision from walking or perhaps from slipping and falling, an on-board accelerometer will detect the impact severity. If this impact is determined to pose a concussion threat to the user, the accelerometer data is logged to a .csv file on a micro-SD card, which can then be used to assess health-related concerns.
