#pragma once

#include <string>
#include <vector>
using namespace std;

class Nvs  {
public:
	Nvs(const string ns,int i = 0);
	virtual ~Nvs();
	bool getString(const string key, string &value);
	bool setString(const string key, const string value);
	vector <string> getKeys();
private:
	const string ns;

};
