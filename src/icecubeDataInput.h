#include <iostream>
#include <vector>
#include <fstream>
#include <string>
#include <sstream>

using namespace std;

class DataInput{

public:
	
	DataInput();
	~DataInput();

	struct eventData{
		vector<int> doms;
	vector<int> strings;
	vector<float> xCoord;
	vector<float> yCoord;
	vector<float> zCoord;
	vector<float> charge;
	vector<float> time;} icecubeData, icecubeDataTimeSorted, icecubeDataChargeSorted;
	
	float maxCharge, minCharge;

	void getText(string filename);
	void getExtremeTimes();
	void sortByTime();
};