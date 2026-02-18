#
# Makefile for eyesScan daemon
#
# This file is part of the sim-ctl distribution.
#
# Copyright (c) 2024 VetSim, Cornell University College of Veterinary Medicine Ithaca, NY
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, version 3.

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
