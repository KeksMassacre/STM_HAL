// SPDX-License-Identifier: GPL-3.0
/*
 * Copyright (c) 2014-2018 Nils Weiss
 */

/**************************************************************************/
/*!
 * Original File:

    @file     Adafruit_PN532.cpp
    @author   Adafruit Industries
    @license  BSD (see license.txt)

      Driver for NXP's PN532 NFC/13.56MHz RFID Transceiver

      This is a library for the Adafruit PN532 NFC/RFID breakout boards
      This library works with the Adafruit NFC breakout
      ----> https://www.adafruit.com/products/364

      Check out the links above for our tutorials and wiring diagrams
      These chips use SPI or I2C to communicate.

      Adafruit invests time and resources providing this open source code,
      please support Adafruit and open-source hardware by purchasing
      products from Adafruit!

 */
/**************************************************************************/

#include "PN_532.h"
#include "trace.h"
#include <array>
#include "binascii.h"
#include <cstring>
#include <os_Task.h>
#include <cstdint>
#include <chrono>

static const int __attribute__((unused)) g_DebugZones = ZONE_ERROR | ZONE_WARNING | ZONE_VERBOSE | ZONE_INFO;

const uint8_t Adafruit_PN532::pn532ack[6] = {0x00, 0x00, 0xFF, 0x00, 0xFF, 0x00};
const uint8_t Adafruit_PN532::pn532response_firmwarevers[6] = {0x00, 0xFF, 0x06, 0xFA, 0xD5, 0x03};

/**************************************************************************/
/*!
    @brief  Setups the HW
 */
/**************************************************************************/
void Adafruit_PN532::begin()
{
    mSpiCs = false;

    os::ThisTask::sleep(std::chrono::milliseconds(1000));

    // not exactly sure why but we have to send a dummy command to get synced up
    uint8_t cmd = PN532_COMMAND_GETFIRMWAREVERSION;
    auto ret = sendCommandCheckAck(&cmd, 1);

    if (ret == false) {
        Trace(ZONE_INFO, "begin failed");
    }

    mSpiCs = true;
}

/**************************************************************************/
/*!
    @brief  Prints a hexadecimal value in plain characters

    @param  data      Pointer to the uint8_t data
    @param  numBytes  Data length in bytes
 */
/**************************************************************************/
void Adafruit_PN532::PrintHex(const uint8_t* data, const uint32_t numBytes)
{
#if defined(DEBUG)
    if (numBytes > 256) {
        Trace(ZONE_WARNING, "printBuffer not big enough\r\n");
    }
    std::array<uint8_t, 128> dataBuffer;
    std::array<uint8_t, 256> printBuffer;

    const size_t dataLength = std::min(static_cast<size_t>(numBytes), dataBuffer.size() - 1);
    std::memcpy(dataBuffer.data(), data, dataLength);

    hexlify(printBuffer, dataBuffer);
    printBuffer[dataLength * 2] = 0;
    Trace(ZONE_INFO, "%s\r\n", printBuffer.data());
#endif
}

/**************************************************************************/
/*!
    @brief  Checks the firmware version of the PN5xx chip

    @returns  The chip's firmware version and ID
 */
/**************************************************************************/
uint32_t Adafruit_PN532::getFirmwareVersion(void)
{
    uint32_t response;
    std::array<uint8_t, PN532_PACKBUFFSIZ> pn532_packetbuffer;

    pn532_packetbuffer[0] = PN532_COMMAND_GETFIRMWAREVERSION;

    if (!sendCommandCheckAck(pn532_packetbuffer.data(), 1)) {
        return 0;
    }

    // read data packet
    readdata(pn532_packetbuffer.data(), 12);

    // check some basic stuff
    if (0 != strncmp((char*)pn532_packetbuffer.data(), (char*)pn532response_firmwarevers, 6)) {
        Trace(ZONE_INFO, "Firmware doesn't match!");
        return 0;
    }

    int offset = 6;
    response = pn532_packetbuffer[offset++];
    response <<= 8;
    response |= pn532_packetbuffer[offset++];
    response <<= 8;
    response |= pn532_packetbuffer[offset++];
    response <<= 8;
    response |= pn532_packetbuffer[offset++];

    return response;
}

void Adafruit_PN532::checkAndConfig(void)
{
    os::ThisTask::sleep(std::chrono::milliseconds(3000));
    begin();
    uint32_t versiondata = getFirmwareVersion();
    if (!versiondata) {
        Trace(ZONE_INFO, "Didn't find PN53x board");
        while (1) {
            ; // halt
        }
    }
    // Got ok data, print it out!
    Trace(ZONE_INFO, "Found chip PN5%x\r\n", (versiondata >> 24) & 0xFF);
    Trace(ZONE_INFO,
          "Firmware ver. %d.%d\r\n",
          (versiondata >> 16) & 0xFF,
          (versiondata >> 8) & 0xFF);
    // configure board to read RFID tags
    SAMConfig();
}

/**************************************************************************/
/*!
    @brief  Sends a command and waits a specified period for the ACK

    @param  cmd       Pointer to the command buffer
    @param  cmdlen    The size of the command in bytes
    @param  timeout   timeout before giving up

    @returns  1 if everything is OK, 0 if timeout occured before an
              ACK was recieved
 */
/**************************************************************************/
// default timeout of one second
bool Adafruit_PN532::sendCommandCheckAck(uint8_t* cmd, uint8_t cmdlen, uint16_t timeout)
{
    // write the command
    writecommand(cmd, cmdlen);

    // Wait for chip to say its ready!
    if (!waitready(timeout)) {
        return false;
    }

    // read acknowledgement
    if (!readack()) {
        Trace(ZONE_INFO, "No ACK frame received!");
        return false;
    }

    if (!waitready(timeout)) {
        return false;
    }

    return true; // ack'd command
}

/**************************************************************************/
/*!
    Writes an 8-bit value that sets the state of the PN532's GPIO pins

    @warning This function is provided exclusively for board testing and
             is dangerous since it will throw an error if any pin other
             than the ones marked "Can be used as GPIO" are modified!  All
             pins that can not be used as GPIO should ALWAYS be left high
             (value = 1) or the system will become unstable and a HW reset
             will be required to recover the PN532.

             pinState[0]  = P30     Can be used as GPIO
             pinState[1]  = P31     Can be used as GPIO
             pinState[2]  = P32     *** RESERVED (Must be 1!) ***
             pinState[3]  = P33     Can be used as GPIO
             pinState[4]  = P34     *** RESERVED (Must be 1!) ***
             pinState[5]  = P35     Can be used as GPIO

    @returns 1 if everything executed properly, 0 for an error
 */
/**************************************************************************/
bool Adafruit_PN532::writeGPIO(uint8_t pinstate)
{
    std::array<uint8_t, PN532_PACKBUFFSIZ> pn532_packetbuffer;

    // Make sure pinstate does not try to toggle P32 or P34
    pinstate |= (1 << PN532_GPIO_P32) | (1 << PN532_GPIO_P34);

    // Fill command buffer
    pn532_packetbuffer[0] = PN532_COMMAND_WRITEGPIO;
    pn532_packetbuffer[1] = PN532_GPIO_VALIDATIONBIT | pinstate;  // P3 Pins
    pn532_packetbuffer[2] = 0x00;    // P7 GPIO Pins (not used ... taken by SPI)

    Trace(ZONE_INFO, "Writing P3 GPIO: %02x\r\n", pn532_packetbuffer[1]);

    // Send the WRITEGPIO command (0x0E)
    if (!sendCommandCheckAck(pn532_packetbuffer.data(), 3)) {
        return false;
    }

    // Read response packet (00 FF PLEN PLENCHECKSUM D5 CMD+1(0x0F) DATACHECKSUM 00)
    readdata(pn532_packetbuffer.data(), 8);

    Trace(ZONE_INFO, "Received: \r\n");
    PrintHex(pn532_packetbuffer.data(), 8);

    return pn532_packetbuffer[5] == 0x0F;
}

/**************************************************************************/
/*!
    Reads the state of the PN532's GPIO pins

    @returns An 8-bit value containing the pin state where:

             pinState[0]  = P30
             pinState[1]  = P31
             pinState[2]  = P32
             pinState[3]  = P33
             pinState[4]  = P34
             pinState[5]  = P35
 */
/**************************************************************************/
uint8_t Adafruit_PN532::readGPIO(void)
{
    std::array<uint8_t, PN532_PACKBUFFSIZ> pn532_packetbuffer;

    pn532_packetbuffer[0] = PN532_COMMAND_READGPIO;

    // Send the READGPIO command (0x0C)
    if (!sendCommandCheckAck(pn532_packetbuffer.data(), 1)) {
        return 0x0;
    }

    // Read response packet (00 FF PLEN PLENCHECKSUM D5 CMD+1(0x0D) P3 P7 IO1 DATACHECKSUM 00)
    readdata(pn532_packetbuffer.data(), 11);

    /* READGPIO response should be in the following format:

       uint8_t            Description
       -------------   ------------------------------------------
       b0..5           Frame header and preamble (with I2C there is an extra 0x00)
       b6              P3 GPIO Pins
       b7              P7 GPIO Pins (not used ... taken by SPI)
       b8              Interface Mode Pins (not used ... bus select pins)
       b9..10          checksum */

    int p3offset = 6;

#ifdef PN532DEBUG
    Trace(ZONE_VERBOSE, "Received: ");
    PrintHex(pn532_packetbuffer.data(), 11);
    Trace(ZONE_VERBOSE, "\r\n");
    Trace(ZONE_VERBOSE, "P3 GPIO: 0x");
    PN532DEBUGPRINT.println(pn532_packetbuffer[p3offset], HEX);
    Trace(ZONE_VERBOSE, "P7 GPIO: 0x");
    PN532DEBUGPRINT.println(pn532_packetbuffer[p3offset + 1], HEX);
    Trace(ZONE_VERBOSE, "IO GPIO: 0x");
    PN532DEBUGPRINT.println(pn532_packetbuffer[p3offset + 2], HEX);

    // Note: You can use the IO GPIO value to detect the serial bus being used
    switch (pn532_packetbuffer[p3offset + 2]) {
    case 0x00:    // Using UART
        Trace(ZONE_VERBOSE, "Using UART (IO = 0x00)");
        break;

    case 0x01:    // Using I2C
        Trace(ZONE_VERBOSE, "Using I2C (IO = 0x01)");
        break;

    case 0x02:    // Using SPI
        Trace(ZONE_VERBOSE, "Using SPI (IO = 0x02)");
        break;
    }

#endif

    return pn532_packetbuffer[p3offset];
}

/**************************************************************************/
/*!
    @brief  Configures the SAM (Secure Access Module)
 */
/**************************************************************************/
bool Adafruit_PN532::SAMConfig(void)
{
    std::array<uint8_t, PN532_PACKBUFFSIZ> pn532_packetbuffer;

    pn532_packetbuffer[0] = PN532_COMMAND_SAMCONFIGURATION;
    pn532_packetbuffer[1] = 0x01; // normal mode;
    pn532_packetbuffer[2] = 0x14; // timeout 50ms * 20 = 1 second
    pn532_packetbuffer[3] = 0x01; // use IRQ pin!

    if (!sendCommandCheckAck(pn532_packetbuffer.data(), 4)) {
        return false;
    }

    // read data packet
    readdata(pn532_packetbuffer.data(), 8);

    return pn532_packetbuffer[5] == 0x15;
}

/**************************************************************************/
/*!
    @brief  Configures the SAM (Secure Access Module)
 */
/**************************************************************************/
bool Adafruit_PN532::SetParameters(void)
{
    std::array<uint8_t, PN532_PACKBUFFSIZ> pn532_packetbuffer;

    pn532_packetbuffer[0] = PN532_COMMAND_SETPARAMETERS;
    pn532_packetbuffer[1] = 0x36;

    if (!sendCommandCheckAck(pn532_packetbuffer.data(), 2)) {
        return false;
    }

    // read data packet
    readdata(pn532_packetbuffer.data(), 8);

    return pn532_packetbuffer[5] == 0x13;
}

/**************************************************************************/
/*!
    Sets the MxRtyPassiveActivation uint8_t of the RFConfiguration register

    @param  maxRetries    0xFF to wait forever, 0x00..0xFE to timeout
                          after mxRetries

    @returns 1 if everything executed properly, 0 for an error
 */
/**************************************************************************/
bool Adafruit_PN532::setPassiveActivationRetries(uint8_t maxRetries)
{
    std::array<uint8_t, PN532_PACKBUFFSIZ> pn532_packetbuffer;

    pn532_packetbuffer[0] = PN532_COMMAND_RFCONFIGURATION;
    pn532_packetbuffer[1] = 5;    // Config item 5 (MaxRetries)
    pn532_packetbuffer[2] = 0xFF; // MxRtyATR (default = 0xFF)
    pn532_packetbuffer[3] = 0x01; // MxRtyPSL (default = 0x01)
    pn532_packetbuffer[4] = maxRetries;

#ifdef MIFAREDEBUG
    Trace(ZONE_VERBOSE, "Setting MxRtyPassiveActivation to ");
    PN532DEBUGPRINT.print(maxRetries, DEC);
    Trace(ZONE_VERBOSE, " ");
#endif

    if (!sendCommandCheckAck(pn532_packetbuffer.data(), 5)) {
        return 0x0;  // no ACK
    }
    return 1;
}

/***** ISO14443A Commands ******/

/**************************************************************************/
/*!
    Waits for an ISO14443A target to enter the field

    @param  cardBaudRate  Baud rate of the card
    @param  uid           Pointer to the array that will be populated
                          with the card's UID (up to 7 bytes)
    @param  uidLength     Pointer to the variable that will hold the
                          length of the card's UID.

    @returns 1 if everything executed properly, 0 for an error
 */
/**************************************************************************/
bool Adafruit_PN532::readPassiveTargetID(uint8_t cardbaudrate, uint8_t* uid, uint8_t* uidLength, uint16_t timeout)
{
    std::array<uint8_t, PN532_PACKBUFFSIZ> pn532_packetbuffer;

    pn532_packetbuffer[0] = PN532_COMMAND_INLISTPASSIVETARGET;
    pn532_packetbuffer[1] = 1;  // max 1 cards at once (we can set this to 2 later)
    pn532_packetbuffer[2] = cardbaudrate;

    if (!sendCommandCheckAck(pn532_packetbuffer.data(), 3, timeout)) {
        Trace(ZONE_VERBOSE, "No card(s) read");
        return 0x0;  // no cards read
    }

    // wait for a card to enter the field (only possible with I2C)
    Trace(ZONE_VERBOSE, "Waiting for IRQ (indicates card presence)");
    if (!waitready(timeout)) {
        Trace(ZONE_VERBOSE, "IRQ Timeout");
        return 0x0;
    }

    // read data packet
    readdata(pn532_packetbuffer.data(), 20);
    // check some basic stuff

    /* ISO14443A card response should be in the following format:

       uint8_t            Description
       -------------   ------------------------------------------
       b0..6           Frame header and preamble
       b7              Tags Found
       b8              Tag Number (only one used in this example)
       b9..10          SENS_RES
       b11             SEL_RES
       b12             NFCID Length
       b13..NFCIDLen   NFCID                                      */

#ifdef MIFAREDEBUG
    Trace(ZONE_VERBOSE, "Found ");
    PN532DEBUGPRINT.print(pn532_packetbuffer[7], DEC);
    Trace(ZONE_VERBOSE, " tags");
#endif
    if (pn532_packetbuffer[7] != 1) {
        return 0;
    }

    uint16_t sens_res = pn532_packetbuffer[9];
    sens_res <<= 8;
    sens_res |= pn532_packetbuffer[10];
#ifdef MIFAREDEBUG
    Trace(ZONE_VERBOSE, "ATQA: 0x");
    PN532DEBUGPRINT.println(sens_res, HEX);
    Trace(ZONE_VERBOSE, "SAK: 0x");
    PN532DEBUGPRINT.println(pn532_packetbuffer[11], HEX);
#endif

    /* Card appears to be Mifare Classic */
    *uidLength = pn532_packetbuffer[12];
#ifdef MIFAREDEBUG
    Trace(ZONE_VERBOSE, "UID:");
#endif
    for (uint8_t i = 0; i < pn532_packetbuffer[12]; i++) {
        uid[i] = pn532_packetbuffer[13 + i];
#ifdef MIFAREDEBUG
        Trace(ZONE_VERBOSE, " 0x");
        PN532DEBUGPRINT.print(uid[i], HEX);
#endif
    }
#ifdef MIFAREDEBUG
    Trace(ZONE_VERBOSE, "\r\n");
#endif

    return 1;
}

/**************************************************************************/
/*!
    @brief  Exchanges an APDU with the currently inlisted peer

    @param  send            Pointer to data to send
    @param  sendLength      Length of the data to send
    @param  response        Pointer to response data
    @param  responseLength  Pointer to the response data length
 */
/**************************************************************************/
bool Adafruit_PN532::inDataExchange(uint8_t* send, uint8_t sendLength, uint8_t* response, uint8_t* responseLength)
{
    std::array<uint8_t, PN532_PACKBUFFSIZ> pn532_packetbuffer;

    if (sendLength > PN532_PACKBUFFSIZ - 2) {
        Trace(ZONE_VERBOSE, "APDU length too long for packet buffer");
        return false;
    }
    uint8_t i;

    pn532_packetbuffer[0] = 0x40; // PN532_COMMAND_INDATAEXCHANGE;
    pn532_packetbuffer[1] = mInListedTag;
    for (i = 0; i < sendLength; ++i) {
        pn532_packetbuffer[i + 2] = send[i];
    }

    if (!sendCommandCheckAck(pn532_packetbuffer.data(), sendLength + 2, 1000)) {
        Trace(ZONE_VERBOSE, "Could not send APDU");
        return false;
    }

    if (!waitready(1000)) {
        Trace(ZONE_VERBOSE, "Response never received for APDU...");
        return false;
    }

    readdata(pn532_packetbuffer.data(), sizeof(pn532_packetbuffer));

    if ((pn532_packetbuffer[0] == 0) && (pn532_packetbuffer[1] == 0) && (pn532_packetbuffer[2] == 0xff)) {
        uint8_t length = pn532_packetbuffer[3];
        if (pn532_packetbuffer[4] != (uint8_t)(~length + 1)) {
#ifdef PN532DEBUG
            Trace(ZONE_VERBOSE, "Length check invalid");
            PN532DEBUGPRINT.println(length, HEX);
            PN532DEBUGPRINT.println((~length) + 1, HEX);
#endif
            return false;
        }
        if ((pn532_packetbuffer[5] == PN532_PN532TOHOST) && (pn532_packetbuffer[6] == PN532_RESPONSE_INDATAEXCHANGE)) {
            if ((pn532_packetbuffer[7] & 0x3f) != 0) {
                Trace(ZONE_VERBOSE, "Status code indicates an error");
                return false;
            }

            length -= 3;

            if (length > *responseLength) {
                length = *responseLength; // silent truncation...
            }

            for (i = 0; i < length; ++i) {
                response[i] = pn532_packetbuffer[8 + i];
            }
            *responseLength = length;

            return true;
        } else {
#ifdef PN532DEBUG
            Trace(ZONE_VERBOSE, "Don't know how to handle this command: ");
            PN532DEBUGPRINT.println(pn532_packetbuffer[6], HEX);
#endif
            return false;
        }
    } else {
        Trace(ZONE_VERBOSE, "Preamble missing");
        return false;
    }
}

/**************************************************************************/
/*!
    @brief  'InLists' a passive target. PN532 acting as reader/initiator,
            peer acting as card/responder.
 */
/**************************************************************************/
bool Adafruit_PN532::inListPassiveTarget()
{
    std::array<uint8_t, PN532_PACKBUFFSIZ> pn532_packetbuffer;

    pn532_packetbuffer[0] = PN532_COMMAND_INLISTPASSIVETARGET;
    pn532_packetbuffer[1] = 1;
    pn532_packetbuffer[2] = 0;

    Trace(ZONE_VERBOSE, "About to inList passive target");
    if (!sendCommandCheckAck(pn532_packetbuffer.data(), 3, 1000)) {
        Trace(ZONE_VERBOSE, "Could not send inlist message");
        return false;
    }

    if (!waitready(30000)) {
        return false;
    }

    readdata(pn532_packetbuffer.data(), sizeof(pn532_packetbuffer));

    if ((pn532_packetbuffer[0] == 0) && (pn532_packetbuffer[1] == 0) && (pn532_packetbuffer[2] == 0xff)) {
        uint8_t length = pn532_packetbuffer[3];
        if (pn532_packetbuffer[4] != (uint8_t)(~length + 1)) {
#ifdef PN532DEBUG
            Trace(ZONE_VERBOSE, "Length check invalid");
            PN532DEBUGPRINT.println(length, HEX);
            PN532DEBUGPRINT.println((~length) + 1, HEX);
#endif
            return false;
        }
        if ((pn532_packetbuffer[5] == PN532_PN532TOHOST) &&
            (pn532_packetbuffer[6] == PN532_RESPONSE_INLISTPASSIVETARGET))
        {
            if (pn532_packetbuffer[7] != 1) {
#ifdef PN532DEBUG
                Trace(ZONE_VERBOSE, "Unhandled number of targets inlisted");
                Trace(ZONE_VERBOSE, "Number of tags inlisted:");
                PN532DEBUGPRINT.println(pn532_packetbuffer[7]);
#endif

                return false;
            }

            mInListedTag = pn532_packetbuffer[8];
#ifdef PN532DEBUG
            Trace(ZONE_VERBOSE, "Tag number: ");
            PN532DEBUGPRINT.println(mInListedTag);
#endif
            return true;
        } else {
            Trace(ZONE_VERBOSE, "Unexpected response to inlist passive host");
            return false;
        }
    } else {
        Trace(ZONE_VERBOSE, "Preamble missing");
        return false;
    }

    return true;
}

/***** Mifare Classic Functions ******/

/**************************************************************************/
/*!
      Indicates whether the specified block number is the first block
      in the sector (block 0 relative to the current sector)
 */
/**************************************************************************/
bool Adafruit_PN532::mifareclassic_IsFirstBlock(uint32_t uiBlock)
{
    // Test if we are in the small or big sectors
    if (uiBlock < 128) {
        return (uiBlock) % 4 == 0;
    } else {
        return (uiBlock) % 16 == 0;
    }
}

/**************************************************************************/
/*!
      Indicates whether the specified block number is the sector trailer
 */
/**************************************************************************/
bool Adafruit_PN532::mifareclassic_IsTrailerBlock(uint32_t uiBlock)
{
    // Test if we are in the small or big sectors
    if (uiBlock < 128) {
        return (uiBlock + 1) % 4 == 0;
    } else {
        return (uiBlock + 1) % 16 == 0;
    }
}

/**************************************************************************/
/*!
    Tries to authenticate a block of memory on a MIFARE card using the
    INDATAEXCHANGE command.  See section 7.3.8 of the PN532 User Manual
    for more information on sending MIFARE and other commands.

    @param  uid           Pointer to a uint8_t array containing the card UID
    @param  uidLen        The length (in bytes) of the card's UID (Should
                          be 4 for MIFARE Classic)
    @param  blockNumber   The block number to authenticate.  (0..63 for
                          1KB cards, and 0..255 for 4KB cards).
    @param  keyNumber     Which key type to use during authentication
                          (0 = MIFARE_CMD_AUTH_A, 1 = MIFARE_CMD_AUTH_B)
    @param  keyData       Pointer to a uint8_t array containing the 6 uint8_t
                          key value

    @returns 1 if everything executed properly, 0 for an error
 */
/**************************************************************************/
uint8_t Adafruit_PN532::mifareclassic_AuthenticateBlock(uint8_t* uid,
                                                        uint8_t  uidLen,
                                                        uint32_t blockNumber,
                                                        uint8_t  keyNumber,
                                                        uint8_t* keyData)
{
    std::array<uint8_t, PN532_PACKBUFFSIZ> pn532_packetbuffer;

    uint8_t i;
    uint8_t _uid[7];       // ISO14443A uid
    uint8_t _uidLen;       // uid len
    uint8_t _key[6];       // Mifare Classic key

    // Hang on to the key and uid data
    memcpy(_key, keyData, 6);
    memcpy(_uid, uid, uidLen);
    _uidLen = uidLen;

#ifdef MIFAREDEBUG
    Trace(ZONE_VERBOSE, "Trying to authenticate card ");
    Adafruit_PN532::PrintHex(_uid, _uidLen);
    Trace(ZONE_VERBOSE, "Using authentication KEY ");
    PN532DEBUGPRINT.print(keyNumber ? 'B' : 'A');
    Trace(ZONE_VERBOSE, ": ");
    Adafruit_PN532::PrintHex(_key, 6);
#endif

    // Prepare the authentication command //
    pn532_packetbuffer[0] = PN532_COMMAND_INDATAEXCHANGE;   /* Data Exchange Header */
    pn532_packetbuffer[1] = 1;                              /* Max card numbers */
    pn532_packetbuffer[2] = (keyNumber) ? MIFARE_CMD_AUTH_B : MIFARE_CMD_AUTH_A;
    pn532_packetbuffer[3] = blockNumber;                    /* Block Number (1K = 0..63, 4K = 0..255 */
    memcpy(pn532_packetbuffer.data() + 4, _key, 6);
    for (i = 0; i < _uidLen; i++) {
        pn532_packetbuffer[10 + i] = _uid[i];                /* 4 byte card ID */
    }

    if (!sendCommandCheckAck(pn532_packetbuffer.data(), 10 + _uidLen)) {
        return 0;
    }

    // Read the response packet
    readdata(pn532_packetbuffer.data(), 12);

    // check if the response is valid and we are authenticated???
    // for an auth success it should be bytes 5-7: 0xD5 0x41 0x00
    // Mifare auth error is technically byte 7: 0x14 but anything other and 0x00 is not good
    if (pn532_packetbuffer[7] != 0x00) {
        Trace(ZONE_VERBOSE, "Authentification failed: ");
        Adafruit_PN532::PrintHex(pn532_packetbuffer.data(), 12);
        return 0;
    }

    return 1;
}

/**************************************************************************/
/*!
    Tries to read an entire 16-byte data block at the specified block
    address.

    @param  blockNumber   The block number to authenticate.  (0..63 for
                          1KB cards, and 0..255 for 4KB cards).
    @param  data          Pointer to the byte array that will hold the
                          retrieved data (if any)

    @returns 1 if everything executed properly, 0 for an error
 */
/**************************************************************************/
uint8_t Adafruit_PN532::mifareclassic_ReadDataBlock(uint8_t blockNumber, uint8_t* data)
{
    std::array<uint8_t, PN532_PACKBUFFSIZ> pn532_packetbuffer;

#ifdef MIFAREDEBUG
    Trace(ZONE_VERBOSE, "Trying to read 16 bytes from block ");
    PN532DEBUGPRINT.println(blockNumber);
#endif

    /* Prepare the command */
    pn532_packetbuffer[0] = PN532_COMMAND_INDATAEXCHANGE;
    pn532_packetbuffer[1] = 1;                      /* Card number */
    pn532_packetbuffer[2] = MIFARE_CMD_READ;        /* Mifare Read command = 0x30 */
    pn532_packetbuffer[3] = blockNumber;            /* Block Number (0..63 for 1K, 0..255 for 4K) */

    /* Send the command */
    if (!sendCommandCheckAck(pn532_packetbuffer.data(), 4)) {
        Trace(ZONE_VERBOSE, "Failed to receive ACK for read command");
        return 0;
    }

    /* Read the response packet */
    readdata(pn532_packetbuffer.data(), 26);

    /* If byte 8 isn't 0x00 we probably have an error */
    if (pn532_packetbuffer[7] != 0x00) {
        Trace(ZONE_VERBOSE, "Unexpected response");
        Adafruit_PN532::PrintHex(pn532_packetbuffer.data(), 26);
        return 0;
    }

    /* Copy the 16 data bytes to the output buffer        */
    /* Block content starts at byte 9 of a valid response */
    memcpy(data, pn532_packetbuffer.data() + 8, 16);

    /* Display data for debug if requested */
#ifdef MIFAREDEBUG
    Trace(ZONE_VERBOSE, "Block ");
    PN532DEBUGPRINT.println(blockNumber);
    Adafruit_PN532::PrintHex(data, 16);
#endif

    return 1;
}

/**************************************************************************/
/*!
    Tries to write an entire 16-byte data block at the specified block
    address.

    @param  blockNumber   The block number to authenticate.  (0..63 for
                          1KB cards, and 0..255 for 4KB cards).
    @param  data          The byte array that contains the data to write.

    @returns 1 if everything executed properly, 0 for an error
 */
/**************************************************************************/
uint8_t Adafruit_PN532::mifareclassic_WriteDataBlock(uint8_t blockNumber, uint8_t* data)
{
    std::array<uint8_t, PN532_PACKBUFFSIZ> pn532_packetbuffer;

#ifdef MIFAREDEBUG
    Trace(ZONE_VERBOSE, "Trying to write 16 bytes to block ");
    PN532DEBUGPRINT.println(blockNumber);
#endif

    /* Prepare the first command */
    pn532_packetbuffer[0] = PN532_COMMAND_INDATAEXCHANGE;
    pn532_packetbuffer[1] = 1;                      /* Card number */
    pn532_packetbuffer[2] = MIFARE_CMD_WRITE;       /* Mifare Write command = 0xA0 */
    pn532_packetbuffer[3] = blockNumber;            /* Block Number (0..63 for 1K, 0..255 for 4K) */
    memcpy(pn532_packetbuffer.data() + 4, data, 16);          /* Data Payload */

    /* Send the command */
    if (!sendCommandCheckAck(pn532_packetbuffer.data(), 20)) {
        Trace(ZONE_VERBOSE, "Failed to receive ACK for write command");
        return 0;
    }
    os::ThisTask::sleep(std::chrono::milliseconds(10));

    /* Read the response packet */
    readdata(pn532_packetbuffer.data(), 26);

    return 1;
}

/**************************************************************************/
/*!
    Formats a Mifare Classic card to store NDEF Records

    @returns 1 if everything executed properly, 0 for an error
 */
/**************************************************************************/
uint8_t Adafruit_PN532::mifareclassic_FormatNDEF(void)
{
    uint8_t sectorbuffer1[16] =
    {0x14, 0x01, 0x03, 0xE1, 0x03, 0xE1, 0x03, 0xE1, 0x03, 0xE1, 0x03, 0xE1, 0x03, 0xE1, 0x03, 0xE1};
    uint8_t sectorbuffer2[16] =
    {0x03, 0xE1, 0x03, 0xE1, 0x03, 0xE1, 0x03, 0xE1, 0x03, 0xE1, 0x03, 0xE1, 0x03, 0xE1, 0x03, 0xE1};
    uint8_t sectorbuffer3[16] =
    {0xA0, 0xA1, 0xA2, 0xA3, 0xA4, 0xA5, 0x78, 0x77, 0x88, 0xC1, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

    // Note 0xA0 0xA1 0xA2 0xA3 0xA4 0xA5 must be used for key A
    // for the MAD sector in NDEF records (sector 0)

    // Write block 1 and 2 to the card
    if (!(mifareclassic_WriteDataBlock(1, sectorbuffer1))) {
        return 0;
    }
    if (!(mifareclassic_WriteDataBlock(2, sectorbuffer2))) {
        return 0;
    }
    // Write key A and access rights card
    if (!(mifareclassic_WriteDataBlock(3, sectorbuffer3))) {
        return 0;
    }

    // Seems that everything was OK (?!)
    return 1;
}

/**************************************************************************/
/*!
    Writes an NDEF URI Record to the specified sector (1..15)

    Note that this function assumes that the Mifare Classic card is
    already formatted to work as an "NFC Forum Tag" and uses a MAD1
    file system.  You can use the NXP TagWriter app on Android to
    properly format cards for this.

    @param  sectorNumber  The sector that the URI record should be written
                          to (can be 1..15 for a 1K card)
    @param  uriIdentifier The uri identifier code (0 = none, 0x01 =
                          "http://www.", etc.)
    @param  url           The uri text to write (max 38 characters).

    @returns 1 if everything executed properly, 0 for an error
 */
/**************************************************************************/
uint8_t Adafruit_PN532::mifareclassic_WriteNDEFURI(uint8_t sectorNumber, uint8_t uriIdentifier, const char* url)
{
    // Figure out how long the string is
    uint8_t len = strlen(url);

    // Make sure we're within a 1K limit for the sector number
    if ((sectorNumber < 1) || (sectorNumber > 15)) {
        return 0;
    }

    // Make sure the URI payload is between 1 and 38 chars
    if ((len < 1) || (len > 38)) {
        return 0;
    }

    // Note 0xD3 0xF7 0xD3 0xF7 0xD3 0xF7 must be used for key A
    // in NDEF records

    // Setup the sector buffer (w/pre-formatted TLV wrapper and NDEF message)
    uint8_t sectorbuffer1[16] =
    {0x00, 0x00, 0x03, (uint8_t)(len + 5), 0xD1, 0x01, (uint8_t)(len + 1), 0x55, uriIdentifier, 0x00, 0x00, 0x00, 0x00,
     0x00, 0x00, 0x00};
    uint8_t sectorbuffer2[16] =
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    uint8_t sectorbuffer3[16] =
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    uint8_t sectorbuffer4[16] =
    {0xD3, 0xF7, 0xD3, 0xF7, 0xD3, 0xF7, 0x7F, 0x07, 0x88, 0x40, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    if (len <= 6) {
        // Unlikely we'll get a url this short, but why not ...
        memcpy(sectorbuffer1 + 9, url, len);
        sectorbuffer1[len + 9] = 0xFE;
    } else if (len == 7) {
        // 0xFE needs to be wrapped around to next block
        memcpy(sectorbuffer1 + 9, url, len);
        sectorbuffer2[0] = 0xFE;
    } else if ((len > 7) && (len <= 22)) {
        // Url fits in two blocks
        memcpy(sectorbuffer1 + 9, url, 7);
        memcpy(sectorbuffer2, url + 7, len - 7);
        sectorbuffer2[len - 7] = 0xFE;
    } else if (len == 23) {
        // 0xFE needs to be wrapped around to final block
        memcpy(sectorbuffer1 + 9, url, 7);
        memcpy(sectorbuffer2, url + 7, len - 7);
        sectorbuffer3[0] = 0xFE;
    } else {
        // Url fits in three blocks
        memcpy(sectorbuffer1 + 9, url, 7);
        memcpy(sectorbuffer2, url + 7, 16);
        memcpy(sectorbuffer3, url + 23, len - 24);
        sectorbuffer3[len - 22] = 0xFE;
    }

    // Now write all three blocks back to the card
    if (!(mifareclassic_WriteDataBlock(sectorNumber * 4, sectorbuffer1))) {
        return 0;
    }
    if (!(mifareclassic_WriteDataBlock((sectorNumber * 4) + 1, sectorbuffer2))) {
        return 0;
    }
    if (!(mifareclassic_WriteDataBlock((sectorNumber * 4) + 2, sectorbuffer3))) {
        return 0;
    }
    if (!(mifareclassic_WriteDataBlock((sectorNumber * 4) + 3, sectorbuffer4))) {
        return 0;
    }

    // Seems that everything was OK (?!)
    return 1;
}

/***** Mifare Ultralight Functions ******/

/**************************************************************************/
/*!
    Tries to read an entire 4-byte page at the specified address.

    @param  page        The page number (0..63 in most cases)
    @param  buffer      Pointer to the byte array that will hold the
                        retrieved data (if any)
 */
/**************************************************************************/
uint8_t Adafruit_PN532::mifareultralight_ReadPage(uint8_t page, uint8_t* buffer)
{
    std::array<uint8_t, PN532_PACKBUFFSIZ> pn532_packetbuffer;

    if (page >= 64) {
        Trace(ZONE_VERBOSE, "Page value out of range");
        return 0;
    }

    Trace(ZONE_VERBOSE, "Reading page ");
    Trace(ZONE_VERBOSE, "%d", page);

    /* Prepare the command */
    pn532_packetbuffer[0] = PN532_COMMAND_INDATAEXCHANGE;
    pn532_packetbuffer[1] = 1;                   /* Card number */
    pn532_packetbuffer[2] = MIFARE_CMD_READ;     /* Mifare Read command = 0x30 */
    pn532_packetbuffer[3] = page;                /* Page Number (0..63 in most cases) */

    /* Send the command */
    if (!sendCommandCheckAck(pn532_packetbuffer.data(), 4)) {
        Trace(ZONE_VERBOSE, "Failed to receive ACK for write command");
        return 0;
    }

    /* Read the response packet */
    readdata(pn532_packetbuffer.data(), 26);
    Trace(ZONE_VERBOSE, "Received: ");
    Adafruit_PN532::PrintHex(pn532_packetbuffer.data(), 26);

    /* If byte 8 isn't 0x00 we probably have an error */
    if (pn532_packetbuffer[7] == 0x00) {
        /* Copy the 4 data bytes to the output buffer         */
        /* Block content starts at byte 9 of a valid response */
        /* Note that the command actually reads 16 byte or 4  */
        /* pages at a time ... we simply discard the last 12  */
        /* bytes                                              */
        memcpy(buffer, pn532_packetbuffer.data() + 8, 4);
    } else {
#ifdef MIFAREDEBUG
        Trace(ZONE_VERBOSE, "Unexpected response reading block: ");
        Adafruit_PN532::PrintHex(pn532_packetbuffer.data(), 26);
#endif
        return 0;
    }

    /* Display data for debug if requested */
#ifdef MIFAREDEBUG
    Trace(ZONE_VERBOSE, "Page ");
    PN532DEBUGPRINT.print(page);
    Trace(ZONE_VERBOSE, ":");
    Adafruit_PN532::PrintHex(buffer, 4);
#endif

    // Return OK signal
    return 1;
}

/**************************************************************************/
/*!
    Tries to write an entire 4-byte page at the specified block
    address.

    @param  page          The page number to write.  (0..63 for most cases)
    @param  data          The byte array that contains the data to write.
                          Should be exactly 4 bytes long.

    @returns 1 if everything executed properly, 0 for an error
 */
/**************************************************************************/
uint8_t Adafruit_PN532::mifareultralight_WritePage(uint8_t page, uint8_t* data)
{
    std::array<uint8_t, PN532_PACKBUFFSIZ> pn532_packetbuffer;

    if (page >= 64) {
        Trace(ZONE_VERBOSE, "Page value out of range");
        // Return Failed Signal
        return 0;
    }

    Trace(ZONE_VERBOSE, "Trying to write 4 byte page");
    Trace(ZONE_VERBOSE, "%d", page);

    /* Prepare the first command */
    pn532_packetbuffer[0] = PN532_COMMAND_INDATAEXCHANGE;
    pn532_packetbuffer[1] = 1;                      /* Card number */
    pn532_packetbuffer[2] = MIFARE_ULTRALIGHT_CMD_WRITE;       /* Mifare Ultralight Write command = 0xA2 */
    pn532_packetbuffer[3] = page;            /* Page Number (0..63 for most cases) */
    memcpy(pn532_packetbuffer.data() + 4, data, 4);          /* Data Payload */

    /* Send the command */
    if (!sendCommandCheckAck(pn532_packetbuffer.data(), 8)) {
        Trace(ZONE_VERBOSE, "Failed to receive ACK for write command");
        // Return Failed Signal
        return 0;
    }
    os::ThisTask::sleep(std::chrono::milliseconds(10));

    /* Read the response packet */
    readdata(pn532_packetbuffer.data(), 26);

    // Return OK Signal
    return 1;
}

/***** NTAG2xx Functions ******/

/**************************************************************************/
/*!
    Tries to read an entire 4-byte page at the specified address.

    @param  page        The page number (0..63 in most cases)
    @param  buffer      Pointer to the byte array that will hold the
                        retrieved data (if any)
 */
/**************************************************************************/
uint8_t Adafruit_PN532::ntag2xx_ReadPage(uint8_t page, uint8_t* buffer)
{
    std::array<uint8_t, PN532_PACKBUFFSIZ> pn532_packetbuffer;

    // TAG Type       PAGES   USER START    USER STOP
    // --------       -----   ----------    ---------
    // NTAG 203       42      4             39
    // NTAG 213       45      4             39
    // NTAG 215       135     4             129
    // NTAG 216       231     4             225

    if (page >= 231) {
        Trace(ZONE_VERBOSE, "Page value out of range");
        return 0;
    }

    Trace(ZONE_VERBOSE, "Reading page ");
    Trace(ZONE_VERBOSE, "%d", page);

    /* Prepare the command */
    pn532_packetbuffer[0] = PN532_COMMAND_INDATAEXCHANGE;
    pn532_packetbuffer[1] = 1;                   /* Card number */
    pn532_packetbuffer[2] = MIFARE_CMD_READ;     /* Mifare Read command = 0x30 */
    pn532_packetbuffer[3] = page;                /* Page Number (0..63 in most cases) */

    /* Send the command */
    if (!sendCommandCheckAck(pn532_packetbuffer.data(), 4)) {
        Trace(ZONE_VERBOSE, "Failed to receive ACK for write command");
        return 0;
    }

    /* Read the response packet */
    readdata(pn532_packetbuffer.data(), 26);
    Trace(ZONE_VERBOSE, "Received: ");
    Adafruit_PN532::PrintHex(pn532_packetbuffer.data(), 26);

    /* If byte 8 isn't 0x00 we probably have an error */
    if (pn532_packetbuffer[7] == 0x00) {
        /* Copy the 4 data bytes to the output buffer         */
        /* Block content starts at byte 9 of a valid response */
        /* Note that the command actually reads 16 byte or 4  */
        /* pages at a time ... we simply discard the last 12  */
        /* bytes                                              */
        memcpy(buffer, pn532_packetbuffer.data() + 8, 4);
    } else {
        Trace(ZONE_VERBOSE, "Unexpected response reading block: ");
        Adafruit_PN532::PrintHex(pn532_packetbuffer.data(), 26);
        return 0;
    }

    /* Display data for debug if requested */
#ifdef MIFAREDEBUG
    Trace(ZONE_VERBOSE, "Page ");
    PN532DEBUGPRINT.print(page);
    Trace(ZONE_VERBOSE, ":");
    Adafruit_PN532::PrintHex(buffer, 4);
#endif

    // Return OK signal
    return 1;
}

/**************************************************************************/
/*!
    Tries to write an entire 4-byte page at the specified block
    address.

    @param  page          The page number to write.  (0..63 for most cases)
    @param  data          The byte array that contains the data to write.
                          Should be exactly 4 bytes long.

    @returns 1 if everything executed properly, 0 for an error
 */
/**************************************************************************/
uint8_t Adafruit_PN532::ntag2xx_WritePage(uint8_t page, uint8_t* data)
{
    std::array<uint8_t, PN532_PACKBUFFSIZ> pn532_packetbuffer;

    // TAG Type       PAGES   USER START    USER STOP
    // --------       -----   ----------    ---------
    // NTAG 203       42      4             39
    // NTAG 213       45      4             39
    // NTAG 215       135     4             129
    // NTAG 216       231     4             225

    if ((page < 4) || (page > 225)) {
        Trace(ZONE_VERBOSE, "Page value out of range");
        // Return Failed Signal
        return 0;
    }

#ifdef MIFAREDEBUG
    Trace(ZONE_VERBOSE, "Trying to write 4 byte page");
    PN532DEBUGPRINT.println(page);
#endif

    /* Prepare the first command */
    pn532_packetbuffer[0] = PN532_COMMAND_INDATAEXCHANGE;
    pn532_packetbuffer[1] = 1;                              /* Card number */
    pn532_packetbuffer[2] = MIFARE_ULTRALIGHT_CMD_WRITE;    /* Mifare Ultralight Write command = 0xA2 */
    pn532_packetbuffer[3] = page;                           /* Page Number (0..63 for most cases) */
    memcpy(pn532_packetbuffer.data() + 4, data, 4);                 /* Data Payload */

    /* Send the command */
    if (!sendCommandCheckAck(pn532_packetbuffer.data(), 8)) {
        Trace(ZONE_VERBOSE, "Failed to receive ACK for write command");

        // Return Failed Signal
        return 0;
    }
    os::ThisTask::sleep(std::chrono::milliseconds(10));

    /* Read the response packet */
    readdata(pn532_packetbuffer.data(), 26);

    // Return OK Signal
    return 1;
}

/**************************************************************************/
/*!
    Writes an NDEF URI Record starting at the specified page (4..nn)

    Note that this function assumes that the NTAG2xx card is
    already formatted to work as an "NFC Forum Tag".

    @param  uriIdentifier The uri identifier code (0 = none, 0x01 =
                          "http://www.", etc.)
    @param  url           The uri text to write (null-terminated string).
    @param  dataLen       The size of the data area for overflow checks.

    @returns 1 if everything executed properly, 0 for an error
 */
/**************************************************************************/
uint8_t Adafruit_PN532::ntag2xx_WriteNDEFURI(uint8_t uriIdentifier, char* url, uint8_t dataLen)
{
    uint8_t pageBuffer[4] = { 0, 0, 0, 0 };

    // Remove NDEF record overhead from the URI data (pageHeader below)
    uint8_t wrapperSize = 12;

    // Figure out how long the string is
    uint8_t len = strlen(url);

    // Make sure the URI payload will fit in dataLen (include 0xFE trailer)
    if ((len < 1) || (len + 1 > (dataLen - wrapperSize))) {
        return 0;
    }

    // Setup the record header
    // See NFCForum-TS-Type-2-Tag_1.1.pdf for details
    uint8_t pageHeader[12] =
    {
        /* NDEF Lock Control TLV (must be first and always present) */
        0x01,         /* Tag Field (0x01 = Lock Control TLV) */
        0x03,         /* Payload Length (always 3) */
        0xA0,         /* The position inside the tag of the lock bytes (upper 4 = page address, lower 4 = byte offset) */
        0x10,         /* Size in bits of the lock area */
        0x44,         /* Size in bytes of a page and the number of bytes each lock bit can lock (4 bit + 4 bits) */
        /* NDEF Message TLV - URI Record */
        0x03,         /* Tag Field (0x03 = NDEF Message) */
        (uint8_t)(len + 5),        /* Payload Length (not including 0xFE trailer) */
        0xD1,         /* NDEF Record Header (TNF=0x1:Well known record + SR + ME + MB) */
        0x01,         /* Type Length for the record type indicator */
        (uint8_t)(len + 1),        /* Payload len */
        0x55,         /* Record Type Indicator (0x55 or 'U' = URI Record) */
        uriIdentifier /* URI Prefix (ex. 0x01 = "http://www.") */
    };

    // Write 12 byte header (three pages of data starting at page 4)
    memcpy(pageBuffer, pageHeader, 4);
    if (!(ntag2xx_WritePage(4, pageBuffer))) {
        return 0;
    }
    memcpy(pageBuffer, pageHeader + 4, 4);
    if (!(ntag2xx_WritePage(5, pageBuffer))) {
        return 0;
    }
    memcpy(pageBuffer, pageHeader + 8, 4);
    if (!(ntag2xx_WritePage(6, pageBuffer))) {
        return 0;
    }

    // Write URI (starting at page 7)
    uint8_t currentPage = 7;
    char* urlcopy = url;
    while (len) {
        if (len < 4) {
            memset(pageBuffer, 0, 4);
            memcpy(pageBuffer, urlcopy, len);
            pageBuffer[len] = 0xFE; // NDEF record footer
            if (!(ntag2xx_WritePage(currentPage, pageBuffer))) {
                return 0;
            }
            // DONE!
            return 1;
        } else if (len == 4) {
            memcpy(pageBuffer, urlcopy, len);
            if (!(ntag2xx_WritePage(currentPage, pageBuffer))) {
                return 0;
            }
            memset(pageBuffer, 0, 4);
            pageBuffer[0] = 0xFE; // NDEF record footer
            currentPage++;
            if (!(ntag2xx_WritePage(currentPage, pageBuffer))) {
                return 0;
            }
            // DONE!
            return 1;
        } else {
            // More than one page of data left
            memcpy(pageBuffer, urlcopy, 4);
            if (!(ntag2xx_WritePage(currentPage, pageBuffer))) {
                return 0;
            }
            currentPage++;
            urlcopy += 4;
            len -= 4;
        }
    }

    // Seems that everything was OK (?!)
    return 1;
}

/************** high level communication functions (handles both I2C and SPI) */

/**************************************************************************/
/*!
    @brief  Tries to read the SPI or I2C ACK signal
 */
/**************************************************************************/
bool Adafruit_PN532::readack()
{
    uint8_t ackbuff[6];

    readdata(ackbuff, 6);

    return 0 == strncmp((char*)ackbuff, (char*)pn532ack, 6);
}

/**************************************************************************/
/*!
    @brief  Return true if the PN532 is ready with a response.
 */
/**************************************************************************/
bool Adafruit_PN532::isready()
{
    // SPI read status and check if ready.
    mSpiCs = false;
    os::ThisTask::sleep(std::chrono::milliseconds(2));
    uint8_t ready[] = {PN532_SPI_STATREAD};
    mSpi.send(ready, sizeof(ready));
    uint8_t status = 0;
    auto ret = mSpi.receive(&status, 1);
    mSpiCs = true;
    Trace(ZONE_VERBOSE, "read %d byte, status %02d\r\n", ret, status);

    // Check if status is ready.
    return status == PN532_SPI_READY && ret == 1;
}

/**************************************************************************/
/*!
    @brief  Waits until the PN532 is ready.

    @param  timeout   Timeout before giving up
 */
/**************************************************************************/
bool Adafruit_PN532::waitready(uint16_t timeout)
{
    uint16_t timer = 0;
    while (!isready()) {
        if (timeout != 0) {
            timer += 10;
            if (timer > timeout) {
                Trace(ZONE_VERBOSE, "TIMEOUT!");
                return false;
            }
        }
        os::ThisTask::sleep(std::chrono::milliseconds(10));
    }
    return true;
}

/**************************************************************************/
/*!
    @brief  Reads n bytes of data from the PN532 via SPI or I2C.

    @param  buff      Pointer to the buffer where data will be written
    @param  n         Number of bytes to be read
 */
/**************************************************************************/
void Adafruit_PN532::readdata(uint8_t* buff, uint8_t n)
{
    // SPI write.

    mSpiCs = false;
    os::ThisTask::sleep(std::chrono::milliseconds(2));
    const uint8_t cmd = PN532_SPI_DATAREAD;
    mSpi.send(&cmd, 1);

    auto ret = mSpi.receive(buff, n);

    Trace(ZONE_VERBOSE, "Reading: \r\n");
    PrintHex(buff, n);

    mSpiCs = true;

    if (ret != n) {
        Trace(ZONE_WARNING, "Didn't receive exact amount of data\r\n");
    }
}

/**************************************************************************/
/*!
    @brief  Read a response frame from the PN532 of at most length bytes in size.
        Writes the frame identifier and packet data into buffer.
        Returns number of bytes written. Returns 0 if there is an error while parsing.

    @param  buff      Pointer to the buffer where data will be written
    @param  length    Number of bytes to be read
 */
/**************************************************************************/
size_t Adafruit_PN532::read_frame(uint8_t* const buff, const size_t length)
{
    std::array<uint8_t, PN532_PACKBUFFSIZ> pn532_packetbuffer;
    const size_t maxframe = std::min(length + 7, pn532_packetbuffer.size());

    readdata(pn532_packetbuffer.data(), maxframe);

    if (pn532_packetbuffer[0] != PN532_PREAMBLE) {
        Trace(ZONE_WARNING, "Response frame does not start with preamble!");
        return 0;
    }
    if ((pn532_packetbuffer[1] != PN532_STARTCODE1) || (pn532_packetbuffer[2] != PN532_STARTCODE2)) {
        Trace(ZONE_WARNING, "Response frame does not contain start code!");
        return 0;
    }
    size_t datalength = pn532_packetbuffer[3];
    if (((datalength + pn532_packetbuffer[4]) & 0xFF) != 0x00) {
        Trace(ZONE_WARNING, "Response length checksum error!");
        return 0;
    }
    if (maxframe < datalength + 7) {
        Trace(ZONE_WARNING, "Response frame too long. Does not fit buffers.");
        return 0;
    }
    uint8_t datachecksum = 0;
    for (size_t i = 0; i < datalength + 1; i++) {
        datachecksum += pn532_packetbuffer[5 + i];
    }
    if (datachecksum != 0x00) {
        Trace(ZONE_WARNING, "Response data checksum error!");
        return 0;
    }
    if (pn532_packetbuffer[6 + datalength] != PN532_POSTAMBLE) {
        Trace(ZONE_WARNING, "Response frame does not contain postamble!");
        return 0;
    }

    std::memcpy(buff, pn532_packetbuffer.data() + 5, datalength);
    return datalength;
}
/**************************************************************************/
/*!
    @brief  set the PN532 as iso14443a Target behaving as a SmartCard
    @param  None
 #author Salvador Mendoza(salmg.net) new functions:
    -AsTarget
    -getDataTarget
    -setDataTarget
 */
/**************************************************************************/
uint8_t Adafruit_PN532::AsTarget()
{
    std::array<uint8_t, PN532_PACKBUFFSIZ> pn532_packetbuffer;

    pn532_packetbuffer[0] = 0x8C;
    /*uint8_t target[] = {
        0x8C, // INIT AS TARGET
        0x00, // MODE -> BITFIELD
        0x04, 0x00, //SENS_RES - MIFARE PARAMS
        0x54, 0xaa, 0xd3, //NFCID1T
        0x08, //SEL_RES
        0x01, 0xfe, //NFCID2T MUST START WITH 01fe - FELICA PARAMS - POL_RES
        0xa2, 0xa3, 0xa4, 0xa5, 0xa6, 0xa7,
        0xc0, 0xc1, 0xc2, 0xc3, 0xc4, 0xc5, 0xc6, 0xc7,//PAD
        0xff, 0xff, //SYSTEM CODE
        0xaa, 0x99, 0x88, 0x77, 0x66, 0x55, 0x44, 0x33, 0x22, 0x11,
        0x00, //general bytes MAX 47 BYTES ATR_RES
        //0x0E, 0x80, 0x4F, 0x0C, 0xA0, 0x00, 0x00, 0x03, 0x06, 0x03, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00,//HISTORICAL BYTES
        0x0F, 0x80, 0x4F, 0x0C, 0xA0, 0x00, 0x00, 0x03, 0x06, 0x03, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00//HISTORICAL BYTES
       };*/
    // PHONE
    uint8_t target[] = {
        0x8C, // INIT AS TARGET
        0x00, // MODE -> BITFIELD
        0x04, 0x00, //SENS_RES - MIFARE PARAMS
        0x57, 0xda, 0xcd, //NFCID1T
        0x20, //SEL_RES
        0x01, 0xfe, //NFCID2T MUST START WITH 01fe - FELICA PARAMS - POL_RES
        0xa2, 0xa3, 0xa4, 0xa5, 0xa6, 0xa7,
        0xc0, 0xc1, 0xc2, 0xc3, 0xc4, 0xc5, 0xc6, 0xc7,//PAD
        0xff, 0xff, //SYSTEM CODE
        0xaa, 0x99, 0x88, 0x77, 0x66, 0x55, 0x44, 0x33, 0x22, 0x11,
        0x00, //general bytes MAX 47 BYTES ATR_RES
        0x00
    };

    if (!sendCommandCheckAck(target, sizeof(target))) {
        return false;
    }

    // read data packet
    size_t len = read_frame(pn532_packetbuffer.data(), 64);
    Trace(ZONE_INFO, "AsTarget Response: \r\n");
    PrintHex(pn532_packetbuffer.data(), len);

    return pn532_packetbuffer[5] == 0x15;
}
/**************************************************************************/
/*!
    @brief  retrieve response from the emulation mode

    @param  cmd    = data
    @param  cmdlen = data length
 */
/**************************************************************************/
uint8_t Adafruit_PN532::getDataTarget(uint8_t* cmd, uint8_t* cmdlen)
{
    uint8_t length;
    std::array<uint8_t, PN532_PACKBUFFSIZ> pn532_packetbuffer;

    pn532_packetbuffer[0] = PN532_COMMAND_TGGETDATA;
    if (!sendCommandCheckAck(pn532_packetbuffer.data(), 1, 1000)) {
        Trace(ZONE_VERBOSE, "Error en ack\r\n");
        return false;
    }

    // read data packet
    length = read_frame(pn532_packetbuffer.data(), 64);

    //if (length > *responseLength) {// Bug, should avoid it in the reading target data
    //  length = *responseLength; // silent truncation...
    //}

    std::memcpy(cmd, pn532_packetbuffer.data(), length);
    *cmdlen = length;
    return true;
}

/**************************************************************************/
/*!
    @brief  set data in PN532 in the emulation mode

    @param  cmd    = data
    @param  cmdlen = data length
 */
/**************************************************************************/
uint8_t Adafruit_PN532::setDataTarget(uint8_t* cmd, uint8_t cmdlen)
{
    uint8_t length;
    //cmd1[0] = 0x8E; Must!
    std::array<uint8_t, PN532_PACKBUFFSIZ> pn532_packetbuffer;

    if (!sendCommandCheckAck(cmd, cmdlen)) {
        return false;
    }

    // read data packet
    readdata(pn532_packetbuffer.data(), 8);
    length = pn532_packetbuffer[3] - 3;
    for (int i = 0; i < length; ++i) {
        cmd[i] = pn532_packetbuffer[8 + i];
    }
    //cmdl = 0
    cmdlen = length;

    return pn532_packetbuffer[5] == 0x15;
}

/**************************************************************************/
/*!
    @brief  Writes a command to the PN532, automatically inserting the
            preamble and required frame details (checksum, len, etc.)

    @param  cmd       Pointer to the command buffer
    @param  cmdlen    Command length in bytes
 */
/**************************************************************************/
void Adafruit_PN532::writecommand(uint8_t* cmd, uint8_t cmdlen)
{
    std::array<uint8_t, PN532_PACKBUFFSIZ> pn532_packetbuffer;

    // SPI command write.
    uint8_t checksum = PN532_HOSTTOPN532;
    for (uint8_t i = 0; i < cmdlen; i++) {
        checksum += cmd[i];
    }
    // Copy first in case of cmd is pn532_packetbuffer.
    std::memcpy(pn532_packetbuffer.data() + 7, cmd, cmdlen);

    pn532_packetbuffer[0] = PN532_SPI_DATAWRITE;
    pn532_packetbuffer[1] = PN532_PREAMBLE;
    pn532_packetbuffer[2] = PN532_STARTCODE1;
    pn532_packetbuffer[3] = PN532_STARTCODE2;
    pn532_packetbuffer[4] = cmdlen + 1;
    pn532_packetbuffer[5] = ~(cmdlen + 1) + 1;
    pn532_packetbuffer[6] = PN532_HOSTTOPN532;
    pn532_packetbuffer[7 + cmdlen] = ~checksum + 1;
    pn532_packetbuffer[8 + cmdlen] = PN532_POSTAMBLE;

    Trace(ZONE_INFO, "Sending:\r\n");
    PrintHex(pn532_packetbuffer.data(), cmdlen + 9);

    mSpiCs = false;
    mSpi.receive(); // clear RXNE
    mSpi.send(pn532_packetbuffer.data(), cmdlen + 9);
    while (!mSpi.isReadyToReceive()) {
        ;
    }
    mSpiCs = true;
}
