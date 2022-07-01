#include <sys/types.h>
#include <sstream>
#include "govee-data.h"

#include <iostream>

using namespace std;


int GoveeData::decodeData(const BLEPacket::t_adStructure* p)
{
  // GVH5075_xxxx
  if ((p->length == 9) && (p->data[0] == 0x88) && (p->data[1] == 0xEC)) {
    model = "GVH5075";

    int iTemp = int(p->data[3]) << 16 | int(p->data[4]) << 8 | int(p->data[5]);
    bool bNegative = iTemp & 0x800000;      // check sign bit
    iTemp = iTemp & 0x7ffff;                        // mask off sign bit
    temperatureC = float(iTemp) / 10000.0;
    if (bNegative) {                                          // apply sign bit
      temperatureC = -1.0 * temperatureC;
    }
    humidity = float(iTemp % 1000) / 10.0;
    battery = int(p->data[6]);

    ma = int(p->data[8]); // what's this?
    temperatureF = temperatureC*9/5+32;
    return 1;	// success on GVH5075
  }

  // Govee_H5074_xxxx
  if ((p->length == 10) && (p->data[0] == 0x88) && (p->data[1] == 0xEC)) {
    model = "5074";
    // This data came from https://github.com/neilsheps/GoveeTemperatureAndHumidity
    // 88EC00 0902 CD15 64 02 (Temp) 41.378°F (Humidity) 55.81% (Battery) 100%
    // 2 3 4  5 6  7 8  9
    short iTemp = short(p->data[4]) << 8 | short(p->data[3]);
    int iHumidity = int(p->data[6]) << 8 | int(p->data[5]);
    temperatureC = float(iTemp) / 100.0;
    humidity = float(iHumidity) / 100.0;
    battery = int(p->data[7]);

    temperatureF = temperatureC*9/5+32;
    return 1;
    }

    // GVH5177_xxxx
  if ((p->length == 9) && (p->data[0] == 0x01) && (p->data[1] == 0x00)) {
    model="5177";
    // This is a guess based on the H5075 3 byte encoding
    // 01000101 029D1B 64 (Temp) 62.8324°F (Humidity) 29.1% (Battery) 100%
    // 2 3 4 5  6 7 8  9
    int iTemp = int(p->data[4]) << 16 | int(p->data[5]) << 8 | int(p->data[6]);
    bool bNegative = iTemp & 0x800000;      // check sign bit
    iTemp = iTemp & 0x7ffff;                        // mask off sign bit
    temperatureC = float(iTemp) / 10000.0;
    humidity = float(iTemp % 1000) / 10.0;
    if (bNegative) {                                          // apply sign bit
      temperatureC = -1.0 * temperatureC;
    }
    battery = int(p->data[7]);
    temperatureF = temperatureC*9/5+32;
    return 1;
  }

  // GVH5183 (UUID) 5183 B5183011
  if (p->length== 17 && (p->data[3] == 0x01) && (p->data[4] == 0x00) && (p->data[5] == 0x01) && (p->data[6] == 0x01)) {
    model="5183";
    // Govee Bluetooth Wireless Meat Thermometer, Digital Grill Thermometer with 1 Probe, 230ft Remote Temperature Monitor, Smart Kitchen Cooking Thermometer, Alert Notifications for BBQ, Oven, Smoker, Cakes
    // https://www.amazon.com/gp/product/B092ZTD96V
    // The probe measuring range is 0° to 300°C /32° to 572°F.
    // 5DA1B4 01000101 E4 01 80 0708 13 24 00 00
    // 2 3 4  5 6 7 8  9  0  1  2 3  4  5  6  7
    // (Manu) 5DA1B4 01000101 81 0180 07D0 1324 0000 (Temp) 20°C (Temp) 49°C (Battery) 1% (Other: 00)  (Other: 00)  (Other: 00)  (Other: 00)  (Other: 00)  (Other: BF)
    // the first three bytes are the last three bytes of the bluetooth address.
    // then next four bytes appear to be a signature for the device type.
    short iTemp = short(p->data[10]) << 8 | short(p->data[11]);
    temperatureC = float(iTemp) / 100.0;
    iTemp = short(p->data[12]) << 8 | short(p->data[13]);
    temperatureC = float(iTemp) / 100.0; // This appears to be the alarm temperature.
    humidity = 0;
    battery = int(p->data[7] & 0x7F);
    temperatureF = temperatureC*9/5+32;
    return 1;
  }

// GVH5182 (UUID) 5182 (Manu) 30132701000101E4018606A413F78606A41318
  if (p->length== 20 && (p->data[3] == 0x01) && (p->data[4] == 0x00) && (p->data[5] == 0x01) && (p->data[6] == 0x01)) {
    model = "5182";
    // Govee Bluetooth Meat Thermometer, 230ft Range Wireless Grill Thermometer Remote Monitor with Temperature Probe Digital Grilling Thermometer with Smart Alerts for Smoker , Cooking, BBQ, Kitchen, Oven
    // https://www.amazon.com/gp/product/B094N2FX9P
    // 301327 01000101 64 01 80 05DC 1324 86 06A4 FFFF
    // 2 3 4  5 6 7 8  9  0  1  2 3  4 5  6  7 8  9 0
    // (Manu) 301327 01000101 3A 01 86 076C FFFF 86 076C FFFF (Temp) 19°C (Temp) -0.01°C (Temp) 19°C (Temp) -0.01°C (Battery) 58%
    // If the probe is not connected to the device, the temperature data is set to FFFF.
    // If the alarm is not set for the probe, the data is set to FFFF.
    short iTemp = short(p->data[10]) << 8 | short(p->data[11]);   // Probe 1 Temperature
    temperatureC  = float(iTemp) / 100.0;

    iTemp = short(p->data[12]) << 8 | short(p->data[13]);         // Probe 1 Alarm Temperature
    temperatureC  = float(iTemp) / 100.0;
    iTemp = short(p->data[15]) << 8 | short(p->data[16]);         // Probe 2 Temperature
    temperatureC = float(iTemp) / 100.0;
    iTemp = short(p->data[17]) << 8 | short(p->data[18]);         // Probe 2 Alarm Temperature
    temperatureC = float(iTemp) / 100.0;
    humidity = 0;
    battery = int(p->data[7]);
    temperatureF = temperatureC*9/5+32;
    return 1;
    }

// Govee_H5179_BD14

printf("LENGTH: %d\n", p->length);
/*
  if (p->length== 20 && (p->data[3] == 0x01) && (p->data[4] == 0x00) && (p->data[5] == 0x01) && (p->data[6] == 0x01)) {
    model = "5179";
    // Govee Bluetooth Meat Thermometer, 230ft Range Wireless Grill Thermometer Remote Monitor with Temperature Probe Digital Grilling Thermometer with Smart Alerts for Smoker , Cooking, BBQ, Kitchen, Oven
    // https://www.amazon.com/gp/product/B094N2FX9P
    // 301327 01000101 64 01 80 05DC 1324 86 06A4 FFFF
    // 2 3 4  5 6 7 8  9  0  1  2 3  4 5  6  7 8  9 0
    // (Manu) 301327 01000101 3A 01 86 076C FFFF 86 076C FFFF (Temp) 19°C (Temp) -0.01°C (Temp) 19°C (Temp) -0.01°C (Battery) 58%
    // If the probe is not connected to the device, the temperature data is set to FFFF.
    // If the alarm is not set for the probe, the data is set to FFFF.
    short iTemp = short(p->data[10]) << 8 | short(p->data[11]);   // Probe 1 Temperature
    temperatureC  = float(iTemp) / 100.0;

    iTemp = short(p->data[12]) << 8 | short(p->data[13]);         // Probe 1 Alarm Temperature
    temperatureC  = float(iTemp) / 100.0;
    iTemp = short(p->data[15]) << 8 | short(p->data[16]);         // Probe 2 Temperature
    temperatureC = float(iTemp) / 100.0;
    iTemp = short(p->data[17]) << 8 | short(p->data[18]);         // Probe 2 Alarm Temperature
    temperatureC = float(iTemp) / 100.0;
    humidity = 0;
    battery = int(p->data[7]);
    temperatureF = temperatureC*9/5+32;
    return 1;
    }
*/
  return -1;
}

void GoveeData::calcAverage(std::queue<GoveeData>* it)
{
	signed int rssi_temp=0;             	//!< separate rssi variable to calculate rssi average
	int sz =it->size();							//!< size of iterator
	// read all values in queue and calculate averages
	while (!it->empty())
	{
		temperatureF += it->front().temperatureF;
		temperatureC += it->front().temperatureC;
		humidity += it->front().humidity;
		battery += it->front().battery;
		rssi_temp += (it->front().rssi);
		ma += (it->front().ma);
		it->pop();
	}
	temperatureF = temperatureF / sz;
	temperatureC = temperatureC / sz;
	humidity = humidity / sz;
	battery = battery / sz;
	rssi_temp = int(rssi_temp / sz);
	ma = ma /sz ;
	rssi = static_cast<signed char>(rssi_temp);
}
