CXX := g++
LDLIBS := -lpcap
CAPLEN := -1
CPPFLAGS := -g -Wall -DCAPLEN=$(CAPLEN)

all: pointgetter 

pointgetter: main.o decoder.o
	$(CXX) $(CPPFLAGS) -o $@ $^ $(LDLIBS)

main.o: main.cpp

decoder.o: decoder.cpp


.PHONY : all
