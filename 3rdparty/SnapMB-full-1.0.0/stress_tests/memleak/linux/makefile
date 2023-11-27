##
## LINUX barebone makefile for c++ examples : good for all platforms
##
## Simply run make or make clean
##
## Intend this makefile only as a "batch examples updater" after library modification.
##

Libs     := -lsnapmb 
Wrapper  :=snapmb.cpp

CXX      := g++
CC       := gcc
CXXFLAGS :=-O3
CFLAGS   :=

.PHONY: all clean

all: 
	$(CXX) $(CXXFLAGS) -o client ../client.cpp ../$(Wrapper) $(Libs) 
	$(CXX) $(CXXFLAGS) -o device ../device.cpp ../$(Wrapper) $(Libs) 

clean:
	$(RM) client
	$(RM) device

