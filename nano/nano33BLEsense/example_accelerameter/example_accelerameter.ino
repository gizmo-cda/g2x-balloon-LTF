/*
  Nano33BLESensorExample_accelerometer.ino
  Copyright (c) 2020 Dale Giancono. All rights reserved..
  This program is an example program showing some of the cababilities of the 
  Nano33BLESensor Library. In this case it outputs accelerometer data from the 
  Arduino Nano 33 BLE Sense's on board IMU sensor via serial in a format that
  can be displayed on the Arduino IDE serial plotter. It also outputs the 
  data via BLE in a string format that can be viewed using a variety of 
  BLE scanning software.
  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.
  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

/*
    Accelerometers 
    Accelerometers measure linear acceleration. ST's MEMS accelerometers 
    embed several useful features for motion and acceleration detection 
    including free-fall, wakeup, single/double-tap recognition, activity/inactivity 
    detection and 6D/4D orientation. They can be also used to measure inclination 
    or vibration. The output of ST's MEMS accelerometers corresponds to [g], 
    where 1 g is equal to 9.80665 m/s2 (standard gravity).
*/

/* Required Libraries
    Arduino_LSM9DS1
    Nano33BLESensor
*/

/* ST IMU LSM9DS1
    data sheet - https://content.arduino.cc/assets/Nano_BLE_Sense_lsm9ds1.pdf
        3D Accelerometer
        ±2/±4/±8/±16 g linear acceleration full scale
        Sensitivity
            +-2 g   0.061 mg    mg - 9.8 X 10-3 m/s*s
          * +-4 g   0.122 mg
            +-8 g   0.244 mg
           +-16 g   0.732 mg

          * default setting
*/

/*****************************************************************************/
/*INCLUDES                                                                   */
/*****************************************************************************/
#include "Arduino.h"
/* For the bluetooth funcionality */
#include <ArduinoBLE.h>
/* For the use of the IMU sensor */
#include "Nano33BLEAccelerometer.h"

/*****************************************************************************/
/*MACROS                                                                     */
/*****************************************************************************/
/* 
 * We use strings to transmit the data via BLE, and this defines the buffer
 * size used to transmit these strings. Only 20 bytes of data can be 
 * transmitted in one packet with BLE, so a size of 20 is chosen the the data 
 * can be displayed nicely in whatever application we are using to monitor the
 * data.
 */
#define BLE_BUFFER_SIZES             20
/* Device name which can be scene in BLE scanning software. */
#define BLE_DEVICE_NAME                "Arduino Nano 33 BLE Sense"
/* Local name which should pop up when scanning for BLE devices. */
#define BLE_LOCAL_NAME                "Accelerometer BLE"

/*****************************************************************************/
/*GLOBAL Data                                                                */
/*****************************************************************************/
/* 
 * Nano33BLEAccelerometerData object which we will store data in each time we read
 * the accelerometer data. 
 */ 
Nano33BLEAccelerometerData accelerometerData;

/* 
 * Declares the BLEService and characteristics we will need for the BLE 
 * transfer. The UUID was randomly generated using one of the many online 
 * tools that exist. It was chosen to use BLECharacteristic instead of 
 * BLEFloatCharacteristic was it is hard to view float data in most BLE 
 * scanning software. Strings can be viewed easiler enough. In an actual
 * application you might want to transfer floats directly.
 */
BLEService BLEAccelerometer("590d65c7-3a0a-4023-a05a-6aaf2f22441c");
BLECharacteristic accelerometerXBLE("0004", BLERead | BLENotify | BLEBroadcast, BLE_BUFFER_SIZES);
BLECharacteristic accelerometerYBLE("0005", BLERead | BLENotify | BLEBroadcast, BLE_BUFFER_SIZES);
BLECharacteristic accelerometerZBLE("0006", BLERead | BLENotify | BLEBroadcast, BLE_BUFFER_SIZES);

/* Common global buffer will be used to write to the BLE characteristics. */
char bleBuffer[BLE_BUFFER_SIZES];

/*****************************************************************************/
/*SETUP (Initialisation)                                                     */
/*****************************************************************************/
void setup()
{
    /* 
     * Serial setup. This will be used to transmit data for viewing on serial 
     * plotter 
     */
    Serial.begin(115200);
    while(!Serial);


    /* BLE Setup. For information, search for the many ArduinoBLE examples.*/
    if (!BLE.begin()) 
    {
        while (1);    
    }
    else
    {
        BLE.setDeviceName(BLE_DEVICE_NAME);
        BLE.setLocalName(BLE_LOCAL_NAME);
        BLE.setAdvertisedService(BLEAccelerometer);
        /* A seperate characteristic is used for each X, Y, and Z axis. */
        BLEAccelerometer.addCharacteristic(accelerometerXBLE);
        BLEAccelerometer.addCharacteristic(accelerometerYBLE);
        BLEAccelerometer.addCharacteristic(accelerometerZBLE);

        BLE.addService(BLEAccelerometer);
        BLE.advertise();
        /* 
         * Initialises the IMU sensor, and starts the periodic reading of the 
         * sensor using a Mbed OS thread. The data is placed in a circular 
         * buffer and can be read whenever.
         */
        Accelerometer.begin();
        //jls - added
        //Serial.println("--- Starting Accelerometer Example ---");
        //Serial.println("");

        /* Plots the legend on Serial Plotter */
        Serial.println("X, Y, Z");
    }
}

/*****************************************************************************/
/*LOOP (runtime super loop)                                                  */
/*****************************************************************************/
void loop()
{
    BLEDevice central = BLE.central();

  /* jls - debug */
  bool central_flag = false;
  
    /*--- jls ---
     * BLE.central() detects if a BLE device is connected (paired?)
     * This modified example ignores BLE and reports the readings over 
     * the serial port.
     */
     
    // jls - orig: if(central)
    if (true)
    {
        int writeLength;

        /* 
         * If a BLE device is connected, accelerometer data will start being read, 
         * and the data will be written to each BLE characteristic. The same 
         * data will also be output through serial so it can be plotted using 
         * Serial Plotter. 
         */
       
        // jls - orig: while(central.connected())
        while(true)
        {            
            if(Accelerometer.pop(accelerometerData))
            {
                /* 
                 * sprintf is used to convert the read float value to a string 
                 * which is stored in bleBuffer. This string is then written to 
                 * the BLE characteristic. 
                 */

                // scale output to mg - +- 4000 mg full scale
                accelerometerData.x *= 1000.0;
                accelerometerData.y *= 1000.0;
                accelerometerData.z *= 1000.0;

                writeLength = sprintf(bleBuffer, "%4.3f", accelerometerData.x);
                accelerometerXBLE.writeValue(bleBuffer, writeLength); 
                writeLength = sprintf(bleBuffer, "%4.3f", accelerometerData.y);
                accelerometerYBLE.writeValue(bleBuffer, writeLength);      
                writeLength = sprintf(bleBuffer, "%4.3f", accelerometerData.z);
                accelerometerZBLE.writeValue(bleBuffer, writeLength);      
  
                Serial.printf("%4.3f,%4.3f,%4.3f\r\n", accelerometerData.x, accelerometerData.y, accelerometerData.z);

                //jls - added 
                // delay(1000);
            }
        }
    }
}
