#
# Makefile for eyesScan daemon
#
# This file is part of the sim-ctl distribution (https://github.com/openvetsim/sim-ctl).
# 
# This program is free software: you can redistribute it and/or modify  
# it under the terms of the GNU General Public License as published by  
# the Free Software Foundation, version 3.
#
# This program is distributed in the hope that it will be useful, but 
# WITHOUT ANY WARRANTY; without even the implied warranty of 
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU 
# General Public License for more details.
#
# You should have received a copy of the GNU General Public License 
# along with this program. If not, see <http://www.gnu.org/licenses/>.

installTargets=eyesScan
targets=$(installTargets)
CFLAGS=-pthread -Wall -g -ggdb
LDFLAGS=-lrt

default:	$(targets)

all: $(targets)

eyesScan: eyesScan.cpp eyesI2C.o eyesI2C.h ../comm/simUtil.o ../comm/simUtil.h ../comm/shmData.h
	g++ eyesScan.cpp $(CFLAGS) eyesI2C.o ../comm/simUtil.o -o eyesScan $(LDFLAGS)

eyesI2C.o: eyesI2C.cpp eyesI2C.h ../comm/simUtil.h ../comm/shmData.h
	g++ $(CFLAGS) -c -o eyesI2C.o eyesI2C.cpp

install: $(installTargets) .FORCE
	sudo cp -u $(installTargets) /usr/local/bin

factory: $(installTargets) .FORCE
	sudo cp $(installTargets) /usr/local/bin

clean: .FORCE
	rm -f $(targets) *.o

.FORCE:
