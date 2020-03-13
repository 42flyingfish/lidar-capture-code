#ifndef DECODER_H
#define DECODER_H

#include<pcap.h>
#include<fstream>




int getData();

void callback(u_char *pcap_handle, const struct pcap_pkthdr *pkthdr, const u_char *packet);

double getAzimuth(const u_char *packet, const int offset);

double getDistance(const u_char *packet, const int offset);

unsigned int getTimestamp(const u_char *packet);

void signalHandler(int signum);

double degreeToRadian(const double degree);

void writeLine(std::ofstream & myfile, double x, double y, double z, double a, double d, int w);

#endif
