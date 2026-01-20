/*
 * Nvs.cpp
 *
 *  Created on: 20 dic 2023
 *      Author: maurizio
 */

#include "Nvs.h"
#include "esp_log.h"
#include <string>
#include <fstream>
#include <iostream>
#include <streambuf>
#include <dirent.h>
static const char *TAG = "NVS";

Nvs::Nvs(const string ns,int i):
	ns(ns)
{
}


bool Nvs::setString(string key, string value)
{
	std::ofstream file(ns+"_"+key);
	file << value;
    return true;
}

bool Nvs::getString(string key, string &value)
{
	std::ifstream file(ns+"_"+key);
	// confirm file opening
	if (!file.is_open()) {
	        // print error message and return
	        cerr << "Failed to open file: " << ns+"_"+key << endl;
	}
	getline(file, value);
    printf("********* %s=%s ********\n",key.c_str(),value.c_str());
    return true;
}

vector <string> Nvs::getKeys()
{
	vector <string> ret;
	DIR *dir;
	struct dirent *ent;
	if ((dir = opendir (".")) != NULL) {
	  /* print all the files and directories within directory */
	  while ((ent = readdir (dir)) != NULL) {

	    string name = string(ent->d_name);
	    if (name.substr(0,4)== ns+"_")
	    {
	    	printf ("%s\n", ent->d_name);
	    	ret.push_back(string(ent->d_name).substr(4));
	    }
	  }
	  closedir (dir);
	} else {
	  /* could not open directory */
	  perror ("");
	}
	return ret;
}

Nvs::~Nvs() {
}

