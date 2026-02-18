/*
 * simData.h
 *
 * This file is part of the sim-ctl distribution (https://github.com/OpenVetSimDevelopers/sim-ctl).
 * 
 * Copyright (c) 2019 VetSim, Cornell University College of Veterinary Medicine Ithaca, NY
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

#ifndef SIMDATA_H_
#define SIMDATA_H_

#include <semaphore.h>
#include "simCtlComm.h"

#define SHM_NAME	"shmData"
#define SHM_CREATE	1
#define SHM_OPEN	0

#define SIMMGR_VERSION		1
#define STR_SIZE			64
#define COMMENT_SIZE		1024

#define LUB_DELAY (120*1000*1000) // Delay 120ms (in ns)
#define DUB_DELAY (200*1000*1000) // Delay 200ms (in ns)
#define PULSE_DELAY (120*1000*1000) // Delay 120ms (in ns)

struct cardiac
{
	char rhythm[STR_SIZE];
	char vpc[STR_SIZE];
	int vpc_freq;		// 0-100% - Frequency of VPC insertions (when vpc is not set to "none")
	char vfib_amplitude[STR_SIZE];	// low, med, high
	int pea;			// Pulse-less Electrical Activity
	int rate;			// Heart Rate in Beats per Minute
	char pwave[STR_SIZE];
	int pr_interval;	// PR interval in msec
	int qrs_interval;		// QRS in msec
	int bps_sys;
	int bps_dia;
	int nibp_rate;
	int nibp_read;
	int nibp_freq;
	int right_dorsal_pulse_strength; 	// 0 - None, 1 - Weak, 2 - Normal, 3 - Strong
	int right_femoral_pulse_strength;
	int left_dorsal_pulse_strength;
	int left_femoral_pulse_strength;
	
	char heart_sound[STR_SIZE];
	int heart_sound_volume;
	int heart_sound_mute;
	
};

struct respiration
{
	// Sounds for Inhalation, Exhalation and Background
	char left_lung_sound[STR_SIZE];		// Base Sound 
	int left_lung_sound_volume;
	int left_lung_sound_mute;
	
	char right_lung_sound[STR_SIZE];		// Base Sound
	int right_lung_sound_volume;
	int right_lung_sound_mute;
	
	//char left_lung_sound[STR_SIZE];
	//char right_lung_sound[STR_SIZE];
	
	// Duration of in/ex
	int inhalation_duration;	// in msec
	int exhalation_duration;	// in msec

// Current respiration rate
	int awRR;	// Computed rate
	int rate;	// defined rate
	
	int chest_movement;
	int manual_breath;
	int active;
	
	int riseState;
	int fallState;
};

struct auscultation
{
	int side;	// 0 - None, 1 - Left, 2 - Right
	int row;	// Row 0 is closest to spine
	int col;	// Col 0 is closets to head
	int heartStrength;
	int leftLungStrength;
	int rightLungStrength;
	char tag[STR_SIZE];
	int heartTrim;
	int lungTrim;
};

#define PULSE_NOT_ACTIVE					0
#define PULSE_RIGHT_DORSAL					1
#define PULSE_RIGHT_FEMORAL					2
#define PULSE_LEFT_DORSAL					3
#define PULSE_LEFT_FEMORAL					4
#define PULSE_POINTS_MAX					5	// Actually, one more than max, as 0 is not used

#define PULSE_TOUCH_NONE					0
#define PULSE_TOUCH_LIGHT					1
#define PULSE_TOUCH_NORMAL					2
#define PULSE_TOUCH_HEAVY					3
#define PULSE_TOUCH_EXCESSIVE				4

struct pulse
{
	int right_dorsal;	// Touch Pressure
	int left_dorsal;	// Touch Pressure
	int right_femoral;	// Touch Pressure
	int left_femoral;	// Touch Pressure
	
	int ain[PULSE_POINTS_MAX];
	int touch[PULSE_POINTS_MAX];
	int base[PULSE_POINTS_MAX];
	int volume[PULSE_POINTS_MAX];
};
struct cpr
{
	int last;			// msec time of last compression
	int	compression;	// 0 to 100%
	int release;		// 0 to 100%
	int duration;
	int x;
	int y;
	int z;
	int tof_present;	// Set if tof sensor is found
	int distance;		// distance in mm, used for 
	int maxDistance;	// Fully extended distance
};

struct defibrillation
{
	int last;			// msec time of last shock
	int energy;			// Energy in Joules of last shock
};

// Eye state values
#define EYE_STATE_NORMAL    0
#define EYE_STATE_OBTUNDED  1
#define EYE_STATE_MIOTIC    2
#define EYE_STATE_DILATED   3

#define EYE_LID_OPEN        0
#define EYE_LID_CLOSED      1
#define EYE_LID_PARTIAL     2

#define EYE_MOVE_NORMAL     0
#define EYE_MOVE_INFREQ_SLOW 1
#define EYE_MOVE_NONE       2

#define EYE_POS_CENTER      0
#define EYE_POS_RIGHT       1
#define EYE_POS_LEFT        2
#define EYE_POS_UP          3
#define EYE_POS_DOWN        4
#define EYE_POS_UP_RIGHT    5
#define EYE_POS_UP_LEFT     6
#define EYE_POS_DOWN_RIGHT  7
#define EYE_POS_DOWN_LEFT   8

#define EYE_BLINK_NORMAL         0
#define EYE_BLINK_INFREQ_SLOW    1
#define EYE_BLINK_PARTIAL_INFREQ 2
#define EYE_BLINK_NONE           3

struct eyes
{
	int connected;        // 1 if eyes device responds on I2C at 0x42

	// Right eye state
	int right_state;      // EYE_STATE_*
	int right_lid;        // EYE_LID_*
	int right_move;       // EYE_MOVE_*
	int right_position;   // EYE_POS_*
	int right_blink;      // EYE_BLINK_*
	int right_pupil;      // 5-90 (percent of max size)

	// Left eye state
	int left_state;
	int left_lid;
	int left_move;
	int left_position;
	int left_blink;
	int left_pupil;

	// Command flags - set to 1 to send command, cleared after send
	int send_command;
};

struct shmData 
{
	sem_t	i2c_sema;	// Mutex lock - Lock for I2C bus access
	char simMgrIPAddr[32];
	int simMgrStatusPort;
	
	// This data is from the sim-mgr, it controls our outputs
	struct cardiac cardiac;
	struct respiration respiration;
	
	// This data is internal to the sim-ctl and is sent to the sim-mgr
	struct auscultation auscultation;
	struct pulse pulse;
	struct cpr cpr;
	struct defibrillation defibrillation;
	struct eyes eyes;
	int manual_breath_ain;
	int manual_breath_baseline;
	int manual_breath_threashold;
	int manual_breath_count;
	int manual_breath_invert;
};

int cardiac_parse(const char *elem,  const char *value, struct cardiac *card );
int respiration_parse(const char *elem,  const char *value, struct respiration *resp );

#endif /* SIMDATA_H_ */