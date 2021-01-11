/*
  Nano33BLESensorExample_magnetic.ino
  Copyright (c) 2020 Dale Giancono. All rights reserved..
  This program is an example program showing some of the cababilities of the 
  Nano33BLESensor Library. In this case it outputs magnetic data from the 
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
    Magnetometers 
    Magnetometers measure a magnetic field such as the Earth's magnetic field. 
    They can be packaged in combination with an accelerometer to allow tilt 
    compensation in the application. Devices integrating both, a magnetometer 
    and an accelerometer in one package are called e-Compasses. The output of 
    ST's magnetometers corresponds to [gauss] (usually abbreviated as [G] or [Gs]. 
    1 [G] = 100 [µT]
*/

/* Required Libraries
    Arduino_LSM9DS1
    Nano33BLESensor
*/

/* ST IMU LSM9DS1
    data sheet - https://content.arduino.cc/assets/Nano_BLE_Sense_lsm9ds1.pdf
        3D Magnetic Sensor
        +-2/+-4/+-8/+-16 gauss magnetic full scale
        Sensitivity
            +-2 gauss   0.14 mgauss/LSB
          * +-4 gauss   0.29 mgauss/LSB
            +-8 gauss   0.43 mgauss/LSB
           +-16 gauss   0.58 mgauss/LSB

          * configured setting
*/

/*****************************************************************************/
/*INCLUDES                                                                   */
/*****************************************************************************/
#include "Arduino.h"
#include <CircularBuffer.h>
/* For the bluetooth funcionality */
#include <ArduinoBLE.h>
/* For the use of the IMU sensor */
#include <Nano33BLEMagnetic.h>

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
#define BLE_LOCAL_NAME                "Magnetic BLE"

/*****************************************************************************/
/*GLOBAL Data                                                                */
/*****************************************************************************/
/* 
 * Nano33BLEMagneticData object which we will store data in each time we read
 * the magnetic data. 
 */ 
Nano33BLEMagneticData magneticData;

/* 
 * Declares the BLEService and characteristics we will need for the BLE 
 * transfer. The UUID was randomly generated using one of the many online 
 * tools that exist. It was chosen to use BLECharacteristic instead of 
 * BLEFloatCharacteristic was it is hard to view float data in most BLE 
 * scanning software. Strings can be viewed easiler enough. In an actual
 * application you might want to transfer floats directly.
 */
BLEService BLEMagnetic("590d65c7-3a0a-4023-a05a-6aaf2f22441c");
BLECharacteristic magneticXBLE("0001", BLERead | BLENotify | BLEBroadcast, BLE_BUFFER_SIZES);
BLECharacteristic magneticYBLE("0002", BLERead | BLENotify | BLEBroadcast, BLE_BUFFER_SIZES);
BLECharacteristic magneticZBLE("0003", BLERead | BLENotify | BLEBroadcast, BLE_BUFFER_SIZES);

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
        BLE.setAdvertisedService(BLEMagnetic);
        /* A seperate characteristic is used for each X, Y, and Z axis. */
        BLEMagnetic.addCharacteristic(magneticXBLE);
        BLEMagnetic.addCharacteristic(magneticYBLE);
        BLEMagnetic.addCharacteristic(magneticZBLE);

        BLE.addService(BLEMagnetic);
        BLE.advertise();

        /* 
         * Initialises the IMU sensor, and starts the periodic reading of the 
         * sensor using a Mbed OS thread. The data is placed in a circular 
         * buffer and can be read whenever.
         */
        Magnetic.begin();

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

    // jls - only use serial output, ignore BLE
    
    //jls orig - if(central)
    if(true)
    {
        int writeLength;
        /* 
         * If a BLE device is connected, magnetic data will start being read, 
         * and the data will be written to each BLE characteristic. The same 
         * data will also be output through serial so it can be plotted using 
         * Serial Plotter. 
         */
        //jls orig - while(central.connected())
        while(true)
        {            
            if(Magnetic.pop(magneticData))
            {
                /* 
                 * sprintf is used to convert the read float value to a string 
                 * which is stored in bleBuffer. This string is then written to 
                 * the BLE characteristic. 
                 */

                /*
                  jls - scale magnetic data
                  Note- scaling appears wrong in LSM9DS1.cpp library
                  Display data in mg
                */ 
                magneticData.x *= 10.0;
                magneticData.y *= 10.0;
                magneticData.z *= 10.0;

                // set reading output to match magnetometer percision - Max 4.000 Min 0.00012

                writeLength = sprintf(bleBuffer, "%4.3f", magneticData.x);
                magneticXBLE.writeValue(bleBuffer, writeLength); 
                writeLength = sprintf(bleBuffer, "%4.3f", magneticData.y);
                magneticYBLE.writeValue(bleBuffer, writeLength);      
                writeLength = sprintf(bleBuffer, "%4.3f", magneticData.z);
                magneticZBLE.writeValue(bleBuffer, writeLength);      

                Serial.printf("%4.3f,%4.3f,%4.3f\r\n", magneticData.x, magneticData.y, magneticData.z);
            }

            //jls - added 
            //delay(1000);   // print output every second
        }
    }
}
