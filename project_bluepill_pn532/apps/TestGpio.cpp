/* Copyright (C) 2015  Nils Weiss
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>. */

#include "TestGpio.h"
#include "Gpio.h"
#include "trace.h"
#include "Usb.h"
#include "Tim.h"
#include "Spi.h"
#include <array>
#include "binascii.h"
#include "PN_532.h"
#include <cstring>

static const int __attribute__((unused)) g_DebugZones = ZONE_ERROR | ZONE_WARNING | ZONE_VERBOSE | ZONE_INFO;

char getChar(void)
{
    char ret = 0;
    constexpr const auto& Usb = hal::Factory<hal::Usb>::get();

    if (Usb.isConfigured()) {
        auto len = Usb.receive((uint8_t*)&ret, sizeof(ret), std::chrono::milliseconds(1000));
        if (len != sizeof(ret)) {
            Trace(ZONE_INFO, "Receive Failed\r\n");
            return 0;
        }
    }
    Trace(ZONE_INFO, "Receive Char %c bytesLeft = %d\r\n", ret, 1);

    return ret;
}

const os::TaskEndless gpioTest("Gpio_Test", 2048, os::Task::Priority::MEDIUM, [](const bool&){
                               Trace(ZONE_INFO, "Started\r\n");

                               constexpr const hal::Gpio& out = hal::Factory<hal::Gpio>::get<hal::Gpio::LED>();
                               constexpr const hal::Gpio& spi_cs = hal::Factory<hal::Gpio>::get<hal::Gpio::SPI1_NSS>();
                               constexpr const hal::Gpio& spi_cs2 = hal::Factory<hal::Gpio>::get<hal::Gpio::SPI2_NSS>();

                               constexpr const hal::Spi& spi = hal::Factory<hal::Spi>::get<hal::Spi::PN532SPI>();
                               constexpr const hal::Spi& spi2 = hal::Factory<hal::Spi>::get<hal::Spi::PN532SPI2>();

                               Adafruit_PN532 nfcTarget(spi_cs, spi);

                               nfcTarget.checkAndConfig();

                               //Adafruit_PN532 nfcReader(spi_cs2, spi2);
                               //nfcReader.checkAndConfig();

                               os::ThisTask::sleep(std::chrono::milliseconds(100));
                               uint8_t success;                          // Flag to check if there was an error with the PN532
                               uint8_t uid[] = { 0, 0, 0, 0, 0, 0, 0 };  // Buffer to store the returned UID
                               uint8_t uidLength;                        // Length of the UID (4 or 7 bytes depending on ISO14443A card type)
                               uint8_t currentblock;                     // Counter to keep track of which block we're on
                               bool authenticated = false;               // Flag to indicate if the sector is authenticated
                               uint8_t data[16];                         // Array to store block data during reads

                               // Keyb on NDEF and Mifare Classic should be the same
                               uint8_t keyuniversal[6] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };

                               uint8_t apdubuffer[255] = {}, apdulen;
                               while (true) {
                                   apdulen = 0;
                                   //nfcTarget.SetParameters();
                                   //os::ThisTask::sleep(std::chrono::milliseconds(1));
                                   nfcTarget.AsTarget();
                                   //os::ThisTask::sleep(std::chrono::milliseconds(1));
                                   //nfcTarget.SetParameters();

                                   for (int j = 0; j < 2000; j++) {
                                       //os::ThisTask::sleep(std::chrono::milliseconds(1));
                                       Trace(ZONE_INFO, "Executing getDataTarget\r\n");
                                       success = nfcTarget.getDataTarget(apdubuffer, &apdulen); //Read initial APDU
                                   }
                                   Trace(ZONE_INFO, "getDataLoop ended.\r\n");
                                   //Trace(ZONE_INFO, "getDataTarget success: %d\r\n", success);

                                   /*if (apdulen > 0) {
                                       Trace(ZONE_INFO, "Response len: %02x\r\n", apdulen);
                                       for (uint8_t i = 0; i < apdulen; i++) {
                                           Trace(ZONE_INFO, " 0x%02x\r\n", apdubuffer[i]);
                                       }
                                       Trace(ZONE_INFO, "\r\n");
                                      }*/

                                   // Wait a bit before trying again
                                   //Trace(ZONE_INFO, "\n\nSend a character to run the mem dumper again!\r\n");
                                   os::ThisTask::sleep(std::chrono::milliseconds(1000));
                               }
    });
