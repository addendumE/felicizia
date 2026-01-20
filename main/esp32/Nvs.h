/*
 * Nvs.h
 *
 *  Created on: 20 dic 2023
 *      Author: maurizio
 */

#ifndef MAIN_UTILITY_NVS_H_
#define MAIN_UTILITY_NVS_H_

#include "nvs_flash.h"
#include "Lock.h"
#include <string>
#include <vector>
using namespace std;

class Nvs : public Lock {
public:
	Nvs(const string ns,nvs_open_mode_t open_mode = NVS_READWRITE );
	virtual ~Nvs();
	bool getString(const string key, string &value);
	bool setString(const string key, const string value);
	bool getNumber(const string key, uint32_t &value);
	vector <string> getKeys();
private:
	const string ns;
	bool opened;
	nvs_handle_t handle;
};

#endif /* MAIN_UTILITY_NVS_H_ */
