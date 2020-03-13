/*
 * simple package capture 
 * currently records packages from an interface
 * and saves it to a pcap file
 * Caution: code must be ran as root to work
 */

#include "decoder.h"
#include <csignal> // for SIGINT
#include <cmath>
#include <iostream>
#include <pcap.h>

// Global, should be removed or refactored into class or struct
// needed for pcap_loop and pcap_breakloop
pcap_t * pcap_handle;

// Using this as the default NETMASK due to the lidar's settings
#define NETMASK 0xffffff

std::ofstream myfile;

bool rotating = false;
bool rollover = false;

int getData() {
	char errbuf[PCAP_ERRBUF_SIZE];
	// set as interface
	//const char * device = "eno1";
	//const char * device = "enp0s25";
	const char * device = "lo";

	pcap_handle = pcap_open_live(device, // name of the device
			BUFSIZ, // portion of the packet to capture
			0, // promisc mode
			-1, // timeout in ms, set negative for infinite
			errbuf);
	if(pcap_handle == NULL) {
		std::cout << "pcap_open_live(): " << errbuf << "\n";
		return 1; 
	}
	// Defines the location to record the file to
	// Warning, will overwrite any pre-existing file
	pcap_dumper_t * saveFile = pcap_dump_open(pcap_handle, "./data.pcap");

	myfile.open("example.csv");

	signal(SIGINT, signalHandler);

    // This is a niave way to filter for only vlp16 data packets
    // This can easily break if the lidar's setting is changed to use a different port
	struct bpf_program fcode;
	const char * packet_filter  = "udp port 2368";


	//compile the filter
	if (pcap_compile(pcap_handle, &fcode, packet_filter, 1, NETMASK) < 0 ) {
		std::cout << "\nUnable to compile the packet filter. Check the syntax.\n" << std::endl;
		myfile.close();
		return -1;
	}


	//set the filter
	if (pcap_setfilter(pcap_handle, &fcode)<0)
	{
		std::cout << "\nError setting the filter.\n" << std::endl;
		myfile.close();
		return -1;
	}

#ifndef CAPLEN
#define CAPLEN -1
#endif

	std::cout << "Capturing packets\nType Control-c to stop\n";
	// pcap_loop takes four parameters
	// 1: the pcap_t
	// 2: the number of packets to process
	//    if negative or zero, it is read as infinity
	// 3: the callback function used to operate on each function
	// 4: Optional user defined parameters to be passed to the loop
	//    must be casted to unsigned char *


	pcap_loop(pcap_handle, CAPLEN, callback, (unsigned char *)saveFile);
	pcap_dump_close(saveFile);
	pcap_close(pcap_handle);
	myfile.flush();
	myfile.close();
	std::cout << "The procedure has ended\n";

	return 0;
}


// callback is a function used by the pcap_loop
// on each packet of data captured
// currently just writes the packet to the save file
void callback(u_char *pcap_handle, const struct pcap_pkthdr *pkthdr, const u_char *packet) {
	pcap_dump(pcap_handle, pkthdr, packet);
	for (int i = 0; i < 12; ++i) {
		double azimuth{getAzimuth(packet, 44+i*100)};
		double distance{getDistance(packet, 44+2+3*14+i*100)};
		double ch14X{distance * cos(degreeToRadian(-1)) * sin(degreeToRadian(azimuth))};
		double ch14Y{distance * cos(degreeToRadian(-1)) * cos(degreeToRadian(azimuth))};
		double ch14Z{distance * sin(degreeToRadian(-1))};
		if (distance > 0)
		writeLine(myfile, ch14X, ch14Y, ch14Z, azimuth, distance, -1);
	}
}

// Grabs two bytes, reverses them, cast as integer and divides them by 100
double getAzimuth(const u_char *packet, const int offset) {
	unsigned int temp = (packet[offset] << 0) | (packet[offset+1] << 8);
	return (double)temp/100;
}

// Grabs two bytes, reverses them, and multiplies by 2.00mm
// Distance is measured every 2.00mm
double getDistance(const u_char *packet, const int offset) {
	unsigned int temp = (packet[offset] << 0) | (packet[offset+1] << 8);
	return 0.002 * temp;
}


// Grabs four bytes, reverses them, cast as unsigned integer
// Measured in microseconds
// According to the manual, the timestamp is located in byte 1242
// of the packet
unsigned int getTimestamp(const u_char *packet) {
	const size_t offset{1242};
	unsigned int temp = (packet[offset] << 0)
		| (packet[offset+1] << 8)
		| (packet[offset+2] << 16)
		| (packet[offset+3] << 24)
		;
	return temp;
}


// function intended for breaking the pcap_loop early
// this is intended for cleanup
// while running try control-c to trigger this
void signalHandler(int signum) {
	std::cout << "Caught signal " << signum << "\n";
	pcap_breakloop(pcap_handle);
}

// Simple utility function
double degreeToRadian(const double degree) {
	return (degree * M_PI) / 180.0;
}

void writeLine(std::ofstream & myfile, double x, double y, double z, double a, double d, int w) {
	myfile << "s,s,s," << x << "," << y << "," << z << ",s,s," << a << "," << d << ",s,s," << w << std::endl;
}
