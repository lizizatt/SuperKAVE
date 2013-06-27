// precompiled header include MUST appear as the first non-comment line
#include "arPrecompiled.h"

#include "icecubeGeometry.h"

#include "arNavigationUtilities.h"
#include "arGlut.h"
#include "GL\gl.h"

void DOM::draw(arMasterSlaveFramework *fw)
{


}

void IceCubeGeometry::draw(arMasterSlaveFramework *fw, float fDownScale)
{
	arMatrix4 userPosition = ar_getNavMatrix();  //userPosition[0] is the rotation of the user
	//cout << "userPosition (x,y,z) = (" << userPosition[12] << ", " << userPosition[14] << ", " << userPosition[13] << ")" << endl; //12 (initial x direction), 13 (vertical), 14 (initial z direction) to get user's position
  //fw->getMatrix(0); //12 (some 2D movement), 13 (vertical), 14 to get user's position
	//fw->getMatrix(0);
	//cout << "uP[0] = " << uP[0] << endl;

	//drawsphere(1, 1.0f);

	int numDrawn = 0;

	//glEnable (GL_DEPTH_BUFFER);
	glEnable (GL_DEPTH_TEST);
	glDisable (GL_BLEND);
	//glEnable (GL_DEPTH_BUFFER);

	//cout << "time = " << ct.vars.time->val << endl;

	//float fDownScale = 10.f;//10.f;
	float scaleDownSphere = fDownScale/5;
	//fDownScale = 10.f;

	//draw ground level ice...
	glColor3f(1.0f, 1.0f, 1.0f);
	glTranslatef(0.0f, 0.0f, -1920.f/fDownScale);
	glScalef(1.0f, 1.0f, 0.025f);
	glutSolidCube(2000.0f/fDownScale);
	glScalef(1.0f, 1.0f, 40.0f);
	glTranslatef(0.0f, 0.0f, 1920.f/fDownScale);
	
	float sphereX, sphereY, sphereZ;
	float diffX, diffY, diffZ;

	//Draws the Icecube detector grid
	for(unsigned int i=0; i < m_doms.size(); i++)
	{
		glColor3f(0.5f, 0.5f, 0.5f);

		const arVector3 &sphere = m_doms[i].getPos();

		sphereX = sphere.v[0]/fDownScale;
		sphereY = sphere.v[1]/fDownScale;
		sphereZ = -sphere.v[2]/fDownScale;

		diffX = sphereX - userPosition[12]/3.281;
		diffY = sphereY - userPosition[14]/3.281;
		diffZ = 5 -sphereZ - userPosition[13]/3.281;

		int domString = m_doms[i].getDomString();

		float horizDist = diffX*diffX + diffY*diffY;
		float vertDist = diffZ*diffZ;

		if(horizDist < 160000.f/(fDownScale*fDownScale) &&  vertDist < 200000.f/(fDownScale*fDownScale))
		{	
			if(domString > 78)
			{
				glColor3f(1.0f, 1.0f, 1.0f);
			}

			int domModule = m_doms[i].getDomModule();
			if(domModule > 60)
			{
				glColor3f(1.0f, 0.0f, 0.0f);
				glTranslatef(sphereX, sphereY, sphereZ);
				glutSolidSphere(0.55f/scaleDownSphere, 6, 6);
				
				
				glTranslatef(-sphereX, -sphereY, -sphereZ);
			}
			else
			{
				glTranslatef(sphereX, sphereY, sphereZ);
				if(horizDist + vertDist < 2000.f/(fDownScale*fDownScale)){
					glutSolidSphere(0.25f/scaleDownSphere, 6, 6);
				}
				else if(horizDist + vertDist < 10000.f/(fDownScale*fDownScale)){
					glutSolidSphere(0.25f/scaleDownSphere, 5, 5);
				}
				else if(horizDist + vertDist < 25000.f/(fDownScale*fDownScale)){
					glutSolidSphere(0.25f/scaleDownSphere, 5, 4);
				}
				else if(horizDist + vertDist < 40000.f/(fDownScale*fDownScale)){
					glutSolidSphere(0.25f/scaleDownSphere, 4, 4);
				}
				else if(horizDist + vertDist < 90000.f/(fDownScale*fDownScale)){
					glutSolidSphere(0.25f/scaleDownSphere, 4, 3);
				}
				else if(horizDist + vertDist < 120000.f/(fDownScale*fDownScale)){
					glutSolidSphere(0.25f/scaleDownSphere, 3, 3);
				}
				else{
					glutSolidSphere(0.25f/scaleDownSphere, 3, 2);
				}				
				//glutSolidSphere(0.25f/scaleDownSphere, 4, 4);
				glTranslatef(-sphereX, -sphereY, -sphereZ);
				
				glBegin(GL_LINES);
				if(i < m_doms.size()-1 && domString == m_doms[i+1].getDomString())
				{
					const arVector3 &nextSphere = m_doms[i+1].getPos();
					glColor3f(0.75f, 0.75f, 0.75f);
					if(domModule == 60)
					{
						const arVector3 &lastSphere = m_doms[i-59].getPos();
						glVertex3f(lastSphere.v[0]/fDownScale, lastSphere.v[1]/fDownScale, -lastSphere.v[2]/fDownScale);
						glVertex3f(nextSphere.v[0]/fDownScale, nextSphere.v[1]/fDownScale, -nextSphere.v[2]/fDownScale);
					}
					else
					{
						glVertex3f(sphereX, sphereY, sphereZ);
						glVertex3f(nextSphere.v[0]/fDownScale, nextSphere.v[1]/fDownScale, -nextSphere.v[2]/fDownScale);
					}
				}
				glEnd();
			}
		}
		else
		{
			glBegin(GL_LINES);
			if(i < m_doms.size()-1 && domString == m_doms[i+1].getDomString())
			{
				int domModule = m_doms[i].getDomModule();
				const arVector3 &nextSphere = m_doms[i+1].getPos();
				glColor3f(0.75f, 0.75f, 0.75f);
				if(domModule == 60)
				{
					const arVector3 &lastSphere = m_doms[i-59].getPos();
					glVertex3f(lastSphere.v[0]/fDownScale, lastSphere.v[1]/fDownScale, -lastSphere.v[2]/fDownScale);
					glVertex3f(nextSphere.v[0]/fDownScale, nextSphere.v[1]/fDownScale, -nextSphere.v[2]/fDownScale);
				}
				else
				{
					glVertex3f(sphereX, sphereY, sphereZ);
					glVertex3f(nextSphere.v[0]/fDownScale, nextSphere.v[1]/fDownScale, -nextSphere.v[2]/fDownScale);
				}
			}
			glEnd();
		}
	}
}

void IceCubeGeometry::loadGeometry(const char *sFilename)
{
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
		m_doms.push_back(DOM(str, mod, arVector3(x,y,z)));
		//Here line has the string representation of the current line we're on.
    }
	//for(int i=0; i < icecubeGeometry.strings.size(); i++){
	//	cout<< "String : " << icecubeGeometry.strings[i]<< " Module: " << icecubeGeometry.modules[i] << " x: " << icecubeGeometry.xCoord[i] << " y: " << icecubeGeometry.yCoord[i] << " z: " << icecubeGeometry.zCoord[i] << endl;
	//}
	cout << "initialized icecube geometry" << endl;
}