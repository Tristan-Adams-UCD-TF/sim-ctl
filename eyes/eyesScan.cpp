/*
 * eyesScan.cpp
 * Daemon to manage I2C communication with OVS Eyes RP2040 controller
 *
 * This file is part of the sim-ctl distribution.
 *
 * Copyright (c) 2024 VetSim, Cornell University College of Veterinary Medicine Ithaca, NY
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3.
 */

#include <iostream>
#include <string>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <time.h>
#include <errno.h>
#include <sys/mman.h>
#include <semaphore.h>
#include <termios.h>
#include <syslog.h>
#include <signal.h>
#include <string.h>

#include "eyesI2C.h"
#include "../comm/simCtlComm.h"
#include "../comm/simUtil.h"
#include "../comm/shmData.h"

using namespace std;

struct shmData *shmData;
char msgbuf[2048];
int debug = 0;

// Track previous values to detect changes
struct {
    int right_state;
    int right_lid;
    int right_move;
    int right_position;
    int right_blink;
    int right_pupil;
    int left_state;
    int left_lid;
    int left_move;
    int left_position;
    int left_blink;
    int left_pupil;
} prevEyes;

// Initialize previous values from shared memory
void initPrevEyes(void)
{
    prevEyes.right_state = shmData->eyes.right_state;
    prevEyes.right_lid = shmData->eyes.right_lid;
    prevEyes.right_move = shmData->eyes.right_move;
    prevEyes.right_position = shmData->eyes.right_position;
    prevEyes.right_blink = shmData->eyes.right_blink;
    prevEyes.right_pupil = shmData->eyes.right_pupil;
    prevEyes.left_state = shmData->eyes.left_state;
    prevEyes.left_lid = shmData->eyes.left_lid;
    prevEyes.left_move = shmData->eyes.left_move;
    prevEyes.left_position = shmData->eyes.left_position;
    prevEyes.left_blink = shmData->eyes.left_blink;
    prevEyes.left_pupil = shmData->eyes.left_pupil;
}

// Check if any eye values have changed
int eyesChanged(void)
{
    if (shmData->eyes.send_command)
        return 1;
    if (prevEyes.right_state != shmData->eyes.right_state)
        return 1;
    if (prevEyes.right_lid != shmData->eyes.right_lid)
        return 1;
    if (prevEyes.right_move != shmData->eyes.right_move)
        return 1;
    if (prevEyes.right_position != shmData->eyes.right_position)
        return 1;
    if (prevEyes.right_blink != shmData->eyes.right_blink)
        return 1;
    if (prevEyes.right_pupil != shmData->eyes.right_pupil)
        return 1;
    if (prevEyes.left_state != shmData->eyes.left_state)
        return 1;
    if (prevEyes.left_lid != shmData->eyes.left_lid)
        return 1;
    if (prevEyes.left_move != shmData->eyes.left_move)
        return 1;
    if (prevEyes.left_position != shmData->eyes.left_position)
        return 1;
    if (prevEyes.left_blink != shmData->eyes.left_blink)
        return 1;
    if (prevEyes.left_pupil != shmData->eyes.left_pupil)
        return 1;
    return 0;
}

// Update previous values after sending command
void updatePrevEyes(void)
{
    prevEyes.right_state = shmData->eyes.right_state;
    prevEyes.right_lid = shmData->eyes.right_lid;
    prevEyes.right_move = shmData->eyes.right_move;
    prevEyes.right_position = shmData->eyes.right_position;
    prevEyes.right_blink = shmData->eyes.right_blink;
    prevEyes.right_pupil = shmData->eyes.right_pupil;
    prevEyes.left_state = shmData->eyes.left_state;
    prevEyes.left_lid = shmData->eyes.left_lid;
    prevEyes.left_move = shmData->eyes.left_move;
    prevEyes.left_position = shmData->eyes.left_position;
    prevEyes.left_blink = shmData->eyes.left_blink;
    prevEyes.left_pupil = shmData->eyes.left_pupil;
    shmData->eyes.send_command = 0;
}

int main(int argc, char *argv[])
{
    int sts;

    // Check for debug flag
    if (argc > 1 && strcmp(argv[1], "-d") == 0)
    {
        debug = 1;
    }

    if (!debug)
    {
        daemonize();
    }
    else
    {
        catchFaults();
    }

    // Initialize shared memory
    sts = initSHM(0);
    if (sts)
    {
        sprintf(msgbuf, "SHM Failed (%d) - Exiting", sts);
        log_message("", msgbuf);
        exit(-1);
    }

    // Initialize eyes state in shared memory
    shmData->eyes.connected = 0;
    shmData->eyes.right_state = EYE_STATE_NORMAL;
    shmData->eyes.right_lid = EYE_LID_OPEN;
    shmData->eyes.right_move = EYE_MOVE_NORMAL;
    shmData->eyes.right_position = EYE_POS_CENTER;
    shmData->eyes.right_blink = EYE_BLINK_NORMAL;
    shmData->eyes.right_pupil = 70;  // Default 70%
    shmData->eyes.left_state = EYE_STATE_NORMAL;
    shmData->eyes.left_lid = EYE_LID_OPEN;
    shmData->eyes.left_move = EYE_MOVE_NORMAL;
    shmData->eyes.left_position = EYE_POS_CENTER;
    shmData->eyes.left_blink = EYE_BLINK_NORMAL;
    shmData->eyes.left_pupil = 70;
    shmData->eyes.send_command = 0;

    // Scan for eyes controller
    eyesI2C eyesCtl;

    if (eyesCtl.present == 0)
    {
        log_message("", "Eyes controller not found on I2C bus - Waiting");
    }
    else
    {
        log_message("", "Eyes controller found");
        shmData->eyes.connected = 1;

        // Send initial state
        eyesCtl.sendFullCommand(
            shmData->eyes.right_state, shmData->eyes.left_state,
            shmData->eyes.right_lid, shmData->eyes.left_lid,
            shmData->eyes.right_move, shmData->eyes.left_move,
            shmData->eyes.right_position, shmData->eyes.left_position,
            shmData->eyes.right_blink, shmData->eyes.left_blink,
            shmData->eyes.right_pupil, shmData->eyes.left_pupil
        );
    }

    initPrevEyes();

    // Main loop
    while (1)
    {
        if (eyesCtl.present == 0)
        {
            // Device not present - try to reconnect every 10 seconds
            shmData->eyes.connected = 0;
            usleep(10000000);  // 10 seconds
            eyesCtl.scanForDevice();
            if (eyesCtl.present)
            {
                log_message("", "Eyes controller reconnected");
                shmData->eyes.connected = 1;
                shmData->eyes.send_command = 1;  // Force resend of current state
            }
        }
        else
        {
            // Check if any values have changed
            if (eyesChanged())
            {
                if (debug)
                {
                    printf("Eyes changed - sending command\n");
                }

                sts = eyesCtl.sendFullCommand(
                    shmData->eyes.right_state, shmData->eyes.left_state,
                    shmData->eyes.right_lid, shmData->eyes.left_lid,
                    shmData->eyes.right_move, shmData->eyes.left_move,
                    shmData->eyes.right_position, shmData->eyes.left_position,
                    shmData->eyes.right_blink, shmData->eyes.left_blink,
                    shmData->eyes.right_pupil, shmData->eyes.left_pupil
                );

                if (sts < 0)
                {
                    // Send failed - device may have disconnected
                    if (eyesCtl.present == 0)
                    {
                        log_message("", "Eyes controller disconnected");
                        shmData->eyes.connected = 0;
                    }
                }
                else
                {
                    updatePrevEyes();
                }
            }

            usleep(50000);  // 50ms polling interval
        }
    }

    return 0;
}
