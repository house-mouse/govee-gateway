
#ifndef __GoveeData_H
#define __GoveeData_H

#include <map>
#include <queue>

#include "blePacket.h"

class  GoveeData {
public:
	const char *model;
	float temperatureC;			//!< temperature in C
	float temperatureF;			//!< temperature in F
	float humidity;					//!< humidity in %
	int battery;						//!< battery level in %
	signed char rssi;				//!< rssi in dBm
	signed char ma;					//!< mysterious attribute

	int decodeData(const BLEPacket::t_adStructure* p);
	void calcAverage(std::queue<GoveeData>* it);													//!< calculate average from iterator
	// constructor
	GoveeData() : temperatureC(0), temperatureF(0), humidity(0), battery(0), rssi(0), ma(0){ };
};

#endif //__GoveeData_H

