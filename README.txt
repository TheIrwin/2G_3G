Project:     The Irwin
Description: StingRay / IMSI Catcher detector
Objective:   To detect rogue cell phone towers for less than $100 USD.
Model:       2G/3G (Adafruit FONA Feather 32u4)
Parts:       Adafruit Feather 32u4 FONA
               https://www.adafruit.com/product/3027
             Lithium Ion Battery (3.7v 1200mAh)
               https://www.adafruit.com/product/258
             SMA to uFL/u.FL/IPX/IPEX RF Adapter Cable
               https://www.adafruit.com/product/851
             Mini GSM/Cellular Quad-Band Antenna (2dBi SMA Plug)
               https://www.adafruit.com/product/1859
===============================================================================
-- Introduction --
In the last few years advanced software defined radios have become available to
the general public. Several projects now exist that allow you to deploy your
own cell phone network or impersonate a legitimate cell phone network. In the
old days it use to be that only law enforcement agencies had this capability; 
but now if you go to hacker conventions, like Blackhat or Defcon, you are 
almost certainly going to be around a hostile cell network controlled by a 
hacker.

These devices are generally referred to as a "StingRay" or "IMSI Catcher".
They are used to track movement, intercept communication, and makes targeting
the user's mobile device with exploits or vulnerabilities easier for the 
attacker.

This project's aim is to help individuals detect when a active IMSI catcher is
running close enough to affect their cell phone, and alert the user to the 
presence of such a device.  The goal was to make this for less than $100 USD.


-- Geographic and Environment considerations --
Different results have been seen between day and night. At sunrise and sunset
you may get electromagnetic interference in the atmosphere that can cause 
towers that are not near you to appears for a few seconds. This can trigger
your device to alert you when in White/Black list mode.

Temperature can effect the quality of the signal broadcast from the radio on 
the tower.  It can also have an effect on the FONA device itself. Over heating
in HOT climates (Defcon in Las Vegas) can happen which can cause strange
affects on your results.

If you live in a city with a dense population, you may have many towers which
will fill up your Whitelist quickly. This is what the "MaxBuffer" option and
"WhitelistLite" array is for.  With this enabled you can double the number
of towers on your White/Black list.


-- Usage differences with or without SIM card --
Results from the FONA can differ depending on whether or not a SIM card is used.
You do not have to use a SIM card, but you might not see everything without it.
If you have a specific network you want to watch, then use a SIM card from them.
It is best to play around with it to determine if you need a SIM card or not.

Example: The FONA will default to T-Mobile in the USA without a SIM card. So if
you have a T-Mobile cell phone you could use this without a SIM card to detect
rogue T-Mobile towers.


-- No promises --
This device and software have been tested in North America, South America,
Europe, and Southeast Asia.  The code compiles without warnings or errors.
So far it seems to work well. I believe I've detected only 1 rogue cell phone 
tower while using this.  The tower in question seemed to moved over 12 miles
away from it's original location, on the other side of a 1,600 ft hill. When
I drove over to where the tower should have been, it was also there. So one
tower was magically in two places at the same time.  Not normal behavior.

That being said, each country and network provider have different setups
and it is impossible to know if this will work everywhere. I welcome feedback.
Also, I am not perfect and it is possible that bugs exist in the code.

It is important to lookup the results of your scans to determine if a tower is
close to you.  https://cellidfinder.com/ is a good site to help you see where 
towers are located at.  Accuracy can be a bit off for some towers, so use your
best judgement or go on a walk with your equipment and figure it out.

* DO NOT DEPEND ON THIS TO BE A PERFECT SOLUTION TO DETECT MALICIOUS ACTIVITY! *


-- Limitations --
The hardware has only 1024 bytes of usable RAM.  This is a HUGE limitation, but
I tried to work around it by providing a "WhitelistLite" array that does not
track the MCC or MNC when you need to track more towers.  This should double the
amount of towers you have in your Whitelist if you need more.

The hardware does not have a RealTime Clock, so no date/time to record. It might 
be possible to acquire date/time from cell network. I need to research this more
before I say for certain. Even with this information, memory is VERY limited and
it may not be wise to store this on the Arduino and instead use python to record
the results on a PC or RaspberryPI.

===============================================================================

-- Buying, Building, Installation --
Buy the required hardware from https://www.adafruit.com. URL's are above.
Assemble the hardware. It should be straight forward to do.
Download and install Arduino. Built and tested with version 1.8.5.
Download and install the Adafruit AVR Boards version 1.4.13.
 * Arduino Menu -> Preferences...
   * Add "https://adafruit.github.io/arduino-board-index/package_adafruit_index.json"
     to the "Additional Boards Manager URLs" field.
   * Click "OK"
 * Tools Menu -> Board: -> Boards Manager...
   * Search for and Install "Adafruit AVR Boards" version 1.4.11.
   * Click "OK"
Download and install the Adafruit FONA Library version 1.3.3.
Download and install the SoftwareSerial Library version 1.0.
 * Sketch Menu -> Include Library -> Manage Libraries
   * Search for and Install "Adafruit FONA Library" version: 1.3.3
   * Search for and Install "SoftwareSerial" version: 1.0
   * Click "Close"
Configure Arduino to use the Adafruit Feather 32u4.
 * Tools Menu -> Board: "Adafruit Feather 32u4"
 * Tools Menu -> Programmer: "AVR ISP"
 * Tools Menu -> Port: Depends on the OS you are using.
Download The Irwin code from Github, (https://github.com/TheIrwin/2G_3G)
Open the Sting_Ray_Detector_v0.1.ino file in Arduino.
Make sure the code compiles correctly with no errors.  If so, then upload it.
 * Sketch Menu -> Verify/Compile
 * Sketch Menu -> Upload
After it is done uploading, you can check it with the Serial Monitor.
 * Tools Menu -> Serial Monitor

The code is heavily commented to try and assist you with any changes you make.

===============================================================================

-- Operation Mode: Scan --
This is the mode you want to use to establish a baseline of the towers near you.
It is recommended that you run this while moving around to different locations
in your home or place of interest.  If you have more than one floor in your home
make sure you check every floor.  Go outside your home and walk to help build a
better baseline.  Change the orientation of the antenna from horizontal to 
vertical. Move the orientation of the device itself from horizontal to vertical.
The more you move the device around, the more towers you will detect over time.

It is recommended that you take several sample over the course of 24 hours in
order to establish a good baseline.  As you get results, look them up online to
see if they are near your location or if they even exist at all. A good site to
use is ( https://cellidfinder.com/ ).  This site may not be 100% accurate, but
it does seem to work well 99% of the time.


-- Operation Mode: Adaptive --
This is the same as "Scan" mode, but it has a built in timer (default 12 hours).
When the timer has been reached any of the towers it has detected during the 
baseline scan become the whitelist and the operating mode is changed to 
"Whitelist".  This was designed for mobile user's that want to have an easy use
for detection while traveling, assuming you are not being followed that closely.


-- Operation Mode: Whitelist --
After you have established a good baseline for your location and have built your
Whitelist and WhitelistLite arrays, you would then switch your device to 
"Whitelist" mode.  Once you are in this mode, if any new towers are detected that
are not in the Whitelist array you are notified via the console output and the 
blinking red LED on the board.  The LED will blink 3 times for each rogue tower
that is detected and 1 blink of a rogue tower was seen but is no longer active.
Also, you *MUST* uncomment the "#define LMode" line while using this mode.


-- Operation Mode: Blacklist --
This is the opposite of Whitelist mode. It is used to detect if a rogue tower
is following you. Say you have a good baseline for your home, but then you go
to a different place several miles (10+) away that would not have the same
towers you have near your home.  Cell Towers do not move locations quickly, if
ever, so if you detect a tower from your home when you are not home, then you
know that someone is following your mobile phone with a portable cell tower.
If you get an alert while using this mode, you will quickly be able to see
which tower it is on the console.  The alert is the same for the LED as it is
in "Whitelist" mode.


-- Operation Flag: LMode --
THIS MUST BE UNCOMMENTED WHEN USING EITHER "Whitelist" or "Blacklist" mode.
Comment this line when using "Scan" or "Adaptive" mode.  Failing to do this
will cause problems.  

===============================================================================
-- User Option: AdapCount --
The number of ScanSleep times the loop has ran before a "baseline" is 
established. (Default 12 hours)

-- User Option: ScanSleep --
The number of milliseconds between scanning for new towers. (Default 1 second)

-- User Option: ScanResetCount --
The number of ScanSleep times without seeing a tower before the FONA reboots. 
Less than 10 seconds is not good because you might not get a weak signal before
the 10 seconds is up.  (Default 30 seconds)

-- User Option: ShowList --
ShowList prints the list of the towers seen so far.  In Whitelist or Blacklist 
mode, this list is the rogue towers.  In Scan or Adaptive modes, this list is
the towers seen so far.

-- User Option: CodeOut --
IF ShowList == true and CodeOut == true, then output of the tower list is in 
a friendly copy/paste format ready to be used for code.  This mode will not
show the RX and COUNT columns in the output.

-- User Option: ShowRaw --
ShowRaw will show the raw output from the SIM800 modem engineering mode.

-- User Option: ShowCount --
ShowCount will show the number of tower scans the program has done and the 
operating mode.

-- User Option: ShowBatt --
ShowBatt will show the battery voltage and charge percentage left.

-- User Option: ShowBand --
ShowBand will show the Cellular Network Band that the modem is set to use.

-- User Option: MaxBuffer --
Uncomment this line to not log or use the MCC and MNC in tower detection. 
Comment this line to disable it.  DO NOT use "true" or "false" with this.

-- Tower List: Whitelist --
This is the array that stores the list of towers to be used in Whitelist
or Blacklist modes.  It uses the MCC, MNC, LAC, and CID.  Each entry is 
16 bytes worth of RAM.

-- Tower List: WhitelistLite --
This is the array that stores the list of towers to be used in Whitelist
or Blacklist modes.  It only uses the LAC and CID.  Each entry is 8 bytes 
worth of RAM.

-- Future development --
  - Python code for detailed historical logging.
  - Tracker Mode to hunt them down!
  - TFT Screen for a portable console interface.
  - SD Card for storage of scan results.
  - 4G LTE

-- License --
This software is Licensed under GNU General Public License v3.
