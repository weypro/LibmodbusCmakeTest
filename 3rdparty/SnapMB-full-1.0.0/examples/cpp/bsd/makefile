##
## BSD barebone makefile for c++ examples : good for all platforms
##
## Simply run gmake or gmake clean
##
## Note: change Compiler in accord of the release installed in your system
##       or create a symbolic link g++ --> g++<your version> 
##

Libs     := -lsnapmb 
Wrapper  :=snapmb.cpp

CXX      := g++11
CC       := gcc
CXXFLAGS :=-O3
CFLAGS   :=

.PHONY: all clean

all: 
	$(CXX) $(CXXFLAGS) -o client ../cpp_client.cpp ../$(Wrapper) $(Libs) 
	$(CXX) $(CXXFLAGS) -o device ../cpp_device.cpp ../$(Wrapper) $(Libs) 

clean:
	$(RM) client
	$(RM) device

