#include "arPrecompiled.h"

#include "icecubeGeometryInput.h"

GeometryInput::GeometryInput(){
	
}

GeometryInput::~GeometryInput(){

}

void GeometryInput::getText(){
	string line;

    ifstream myfile("..\\..\\src\\neutrinos\\data\\icecube\\geometry\\Icecube_Geometry_Data.txt");

    if(!myfile) //Always test if the is file open.
    {
		cout<<"Error opening output file [File not found]"<< endl;
        return;
    }
    while (getline(myfile, line))
    {
		int str, mod;
		float x, y, z;
		istringstream stm(line) ;
		stm >> str >> mod >> x >> y >> z;
		icecubeGeometry.strings.push_back(str);
		icecubeGeometry.modules.push_back(mod);
		icecubeGeometry.xCoord.push_back(x);
		icecubeGeometry.yCoord.push_back(y);
		icecubeGeometry.zCoord.push_back(z);
		//Here line has the string representation of the current line we're on.
    }
	//for(int i=0; i < icecubeGeometry.strings.size(); i++){
	//	cout<< "String : " << icecubeGeometry.strings[i]<< " Module: " << icecubeGeometry.modules[i] << " x: " << icecubeGeometry.xCoord[i] << " y: " << icecubeGeometry.yCoord[i] << " z: " << icecubeGeometry.zCoord[i] << endl;
	//}
	cout << "initialized icecube geometry" << endl;
}

