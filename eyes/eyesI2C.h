/*
 * eyesI2C.h
 * Definition of a class to interface with the OVS Eyes RP2040 controller
 *
 * This file is part of the sim-ctl distribution (https://github.com/openvetsim/sim-ctl).
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef EYESI2C_H_
#define EYESI2C_H_

#define EYES_I2C_BUFFER 0x80
#define MAX_BUS 64

// RP2040 eyes controller I2C address
#define EYES_I2C_ADDR  0x42
#define EYES_MAX_CONSECUTIVE_FAILURES 5

// Command packet structure (matches RP2040 protocol)
#define EYES_PACKET_SIZE       12
#define EYES_CMD_HEADER        0xBB
#define EYES_INPUT_RESP_HEADER 0xCC

// Packet byte indices
#define PKT_HEADER      0
#define PKT_EYESTATE    1
#define PKT_RESET       2
#define PKT_LID         3
#define PKT_MOVE_R      4
#define PKT_POS_R       5
#define PKT_POS_L       6
#define PKT_BLINK       7
#define PKT_PUPIL_R     8
#define PKT_PUPIL_L     9
#define PKT_MOVE_L      10
#define PKT_CHECKSUM    11

// Bit positions for standard command bytes
#define SET_R_BIT       7
#define VAL_R_SHIFT     5
#define SET_L_BIT       3
#define VAL_L_SHIFT     1

// Bit positions for position bytes
#define POS_SET_BIT     7
#define POS_VAL_SHIFT   3

// Bit positions for pupil bytes
#define PUPIL_SET_BIT   7
#define PUPIL_VAL_MASK  0x7F

class eyesI2C {

private:
    int I2CBus;
    char I2CdataBuffer[EYES_I2C_BUFFER];
    char I2Cnamebuf[MAX_BUS];
    int I2Cfile;
    int I2CAddr;

    // Encode standard byte format: [setR(1)|valR(2)|x|setL(1)|valL(2)|x]
    unsigned char encodeStandard(int setR, int valR, int setL, int valL);

    // Encode position byte: [set(1)|pos(4)|x(3)]
    unsigned char encodePosition(int set, int pos);

    // Encode pupil byte: [set(1)|pupil(7)]
    unsigned char encodePupil(int set, int pupil);

public:
    eyesI2C();
    int scanForDevice(void);
    int sendCommand(unsigned char* packet);
    int sendEyeState(int rightState, int leftState);
    int sendFullCommand(int rState, int lState,
                        int rLid, int lLid,
                        int rMove, int lMove,
                        int rPos, int lPos,
                        int rBlink, int lBlink,
                        int rPupil, int lPupil);
    int sendInputResponseCommand(int rPlrExposed, int rPlrConsensual,
                                 int lPlrExposed, int lPlrConsensual,
                                 int rMenace, int lMenace,
                                 int rPalpebral, int lPalpebral,
                                 int rNystagmus, int lNystagmus);
    int present;
    int consecutiveFailures;

    virtual ~eyesI2C();
};

#endif /* EYESI2C_H_ */
