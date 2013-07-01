#include "arPrecompiled.h"

#include "icecubeDataInput.h"

DataInput::DataInput(){
	
}

DataInput::~DataInput(){

}

void DataInput::getText(string filename){
	string line;

    ifstream myfile(filename);   ///"..\\..\\src\\neutrinos\\data\\icecube\\geometry\\Icecube_Geometry_Data.txt");   ///Need to change the name of this file/////////////////////////////////////////////////////

    if(!myfile) //Always test if the file is open.
    {
		cout<<"Error opening output file [File not found]"<< endl;
        return;
    }
    while (getline(myfile, line))
    {
		int dom, str;
		float x, y, z, charge, time;
		istringstream stm(line) ;
		stm >> dom >> str >> x >> y >> z >> charge >> time;
		icecubeData.doms.push_back(dom);
		icecubeData.strings.push_back(str);
		icecubeData.xCoord.push_back(x);
		icecubeData.yCoord.push_back(y);
		icecubeData.zCoord.push_back(z);
		icecubeData.charge.push_back(charge);
		icecubeData.time.push_back(time);
		//Here line has the string representation of the current line we're on.
    }
	icecubeData.doms.pop_back();
	icecubeData.strings.pop_back();
	icecubeData.xCoord.pop_back();
	icecubeData.yCoord.pop_back();
	icecubeData.zCoord.pop_back();
	icecubeData.charge.pop_back();
	icecubeData.time.pop_back();
	//for(int i=0; i < icecubeData.strings.size(); i++){
	//	cout<< " DOMS: " << icecubeData.doms[i]<< "String : " << icecubeData.strings[i] << " x: " << icecubeData.xCoord[i] << " y: " << icecubeData.yCoord[i] << " z: " << icecubeData.zCoord[i] <<  " charge: " << icecubeData.charge[i] <<  " time: " << icecubeData.time[i] << endl;
	//}
	cout << "initialized icecube event data" <<endl;

	sortByTime();
	
}

void DataInput::getExtremeTimes(){

}

void DataInput::sortByTime(){

	eventData temp;

	for(unsigned int i=0; i<icecubeData.time.size(); i++){
		if(i==0){
			temp.time.push_back(icecubeData.time.at(i));
			temp.doms.push_back(icecubeData.doms.at(i));
			temp.strings.push_back(icecubeData.strings.at(i));
			temp.xCoord.push_back(icecubeData.xCoord.at(i));
			temp.yCoord.push_back(icecubeData.yCoord.at(i));
			temp.zCoord.push_back(icecubeData.zCoord.at(i));
			temp.charge.push_back(icecubeData.charge.at(i));
		}
		else{
			for(unsigned int j=0; j<temp.time.size(); j++){
				if(temp.time.at(j) > icecubeData.time.at(i)){
					temp.time.insert(temp.time.begin() + j, icecubeData.time.at(i));
					temp.doms.insert(temp.doms.begin() + j, icecubeData.doms.at(i));
					temp.strings.insert(temp.strings.begin() + j, icecubeData.strings.at(i));
					temp.xCoord.insert(temp.xCoord.begin() + j, icecubeData.xCoord.at(i));
					temp.yCoord.insert(temp.yCoord.begin() + j, icecubeData.yCoord.at(i));
					temp.zCoord.insert(temp.zCoord.begin() + j, icecubeData.zCoord.at(i));
					temp.charge.insert(temp.charge.begin() + j, icecubeData.charge.at(i));

					j = temp.time.size() + 50;
				}
			}
		}
	}

	icecubeData = temp;

	for(int i=0; i<icecubeData.time.size(); i++){
		cout << "time[" << i << "] = " << icecubeData.time.at(i) << "; " << endl;
	}

}