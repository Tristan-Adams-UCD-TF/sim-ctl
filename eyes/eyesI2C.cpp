/*
 * eyesI2C.cpp
 * Implementation of a class to interface with the OVS Eyes RP2040 controller
 *
 * This file is part of the sim-ctl distribution.
 *
 * Copyright (c) 2024 VetSim, Cornell University College of Veterinary Medicine Ithaca, NY
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3.
 */

#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include "eyesI2C.h"
#include "../comm/simUtil.h"
#include "../comm/shmData.h"

extern int debug;

eyesI2C::eyesI2C()
{
    present = 0;
    I2CAddr = EYES_I2C_ADDR;
    I2Cfile = -1;

    (void)scanForDevice();
}

int eyesI2C::scanForDevice(void)
{
    int sts;
    unsigned char readBuf[4];
    struct i2c_msg i2cMsg[1];
    struct i2c_rdwr_ioctl_data ioctl_data;

    // Scan I2C buses 1 and 2 for the eyes controller
    for (I2CBus = 1; I2CBus < 3; I2CBus++)
    {
        snprintf(I2Cnamebuf, sizeof(I2Cnamebuf), "/dev/i2c-%d", I2CBus);
        if ((I2Cfile = open(I2Cnamebuf, O_RDWR)) < 0)
        {
            continue;
        }

        if (ioctl(I2Cfile, I2C_SLAVE, I2CAddr) >= 0)
        {
            // Try to read a status byte from the device
            i2cMsg[0].addr = I2CAddr;
            i2cMsg[0].flags = I2C_M_RD;
            i2cMsg[0].len = 1;
            i2cMsg[0].buf = readBuf;
            ioctl_data.nmsgs = 1;
            ioctl_data.msgs = &i2cMsg[0];

            sts = getI2CLock();
            if (sts)
            {
                close(I2Cfile);
                I2Cfile = -1;
                continue;
            }

            sts = ioctl(I2Cfile, I2C_RDWR, &ioctl_data);
            releaseI2CLock();

            if (sts >= 0)
            {
                // Device responded - check for expected status byte (0x01 = ready)
                if (readBuf[0] == 0x01)
                {
                    present = 1;
                    if (debug)
                    {
                        printf("Eyes controller found on I2C bus %d at address 0x%02X\n",
                               I2CBus, I2CAddr);
                    }
                    return present;
                }
            }
        }
        close(I2Cfile);
        I2Cfile = -1;
    }

    present = 0;
    return present;
}

int eyesI2C::sendCommand(unsigned char* packet)
{
    int status;
    struct i2c_msg i2cMsg[1];
    struct i2c_rdwr_ioctl_data ioctl_data;
    int sts;

    if (!present || I2Cfile < 0)
    {
        return -1;
    }

    // Calculate checksum: XOR of bytes 1-10
    packet[PKT_CHECKSUM] = 0;
    for (int i = 1; i <= 10; i++)
    {
        packet[PKT_CHECKSUM] ^= packet[i];
    }

    i2cMsg[0].addr = I2CAddr;
    i2cMsg[0].flags = 0;  // Write
    i2cMsg[0].len = EYES_PACKET_SIZE;
    i2cMsg[0].buf = packet;
    ioctl_data.nmsgs = 1;
    ioctl_data.msgs = &i2cMsg[0];

    sts = getI2CLock();
    if (sts)
    {
        printf("eyesI2C::sendCommand: Could not get I2C Lock\n");
        return -1;
    }

    status = ioctl(I2Cfile, I2C_RDWR, &ioctl_data);
    releaseI2CLock();

    if (status < 0)
    {
        if (errno == 121)
        {
            present = 0;
        }
        return -2;
    }

    return 0;
}

unsigned char eyesI2C::encodeStandard(int setR, int valR, int setL, int valL)
{
    unsigned char result = 0;
    if (setR) result |= (1 << SET_R_BIT);
    result |= ((valR & 0x03) << VAL_R_SHIFT);
    if (setL) result |= (1 << SET_L_BIT);
    result |= ((valL & 0x03) << VAL_L_SHIFT);
    return result;
}

unsigned char eyesI2C::encodePosition(int set, int pos)
{
    unsigned char result = 0;
    if (set) result |= (1 << POS_SET_BIT);
    result |= ((pos & 0x0F) << POS_VAL_SHIFT);
    return result;
}

unsigned char eyesI2C::encodePupil(int set, int pupil)
{
    unsigned char result = 0;
    if (set) result |= (1 << PUPIL_SET_BIT);
    // Convert 5-90 range to 0-85 for encoding
    int encoded = pupil - 5;
    if (encoded < 0) encoded = 0;
    if (encoded > 85) encoded = 85;
    result |= (encoded & PUPIL_VAL_MASK);
    return result;
}

int eyesI2C::sendEyeState(int rightState, int leftState)
{
    unsigned char packet[EYES_PACKET_SIZE];

    memset(packet, 0, EYES_PACKET_SIZE);
    packet[PKT_HEADER] = EYES_CMD_HEADER;
    packet[PKT_EYESTATE] = encodeStandard(1, rightState, 1, leftState);

    return sendCommand(packet);
}

int eyesI2C::sendFullCommand(int rState, int lState,
                              int rLid, int lLid,
                              int rMove, int lMove,
                              int rPos, int lPos,
                              int rBlink, int lBlink,
                              int rPupil, int lPupil)
{
    unsigned char packet[EYES_PACKET_SIZE];

    memset(packet, 0, EYES_PACKET_SIZE);
    packet[PKT_HEADER] = EYES_CMD_HEADER;
    packet[PKT_EYESTATE] = encodeStandard(1, rState, 1, lState);
    packet[PKT_LID] = encodeStandard(1, rLid, 1, lLid);
    packet[PKT_MOVE] = encodeStandard(1, rMove, 1, lMove);
    packet[PKT_POS_R] = encodePosition(1, rPos);
    packet[PKT_POS_L] = encodePosition(1, lPos);
    packet[PKT_BLINK] = encodeStandard(1, rBlink, 1, lBlink);
    packet[PKT_PUPIL_R] = encodePupil(1, rPupil);
    packet[PKT_PUPIL_L] = encodePupil(1, lPupil);

    return sendCommand(packet);
}

eyesI2C::~eyesI2C()
{
    if (I2Cfile >= 0)
    {
        close(I2Cfile);
    }
}
