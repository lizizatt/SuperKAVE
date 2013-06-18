#include <iostream>
#include <vector>
#include <fstream>
#include <string>
#include <sstream>

using namespace std;

class GeometryInput{

public:
	
	GeometryInput();
	~GeometryInput();

	struct geometryData{
		vector<int> strings;
	vector<int> modules;
	vector<float> xCoord;
	vector<float> yCoord;
	vector<float> zCoord;} icecubeGeometry;

	
	void getText();
};