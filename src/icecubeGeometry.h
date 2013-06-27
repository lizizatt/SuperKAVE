#ifndef _ICECUBE_GEOMETRY_H_
#define _ICECUBE_GEOMETRY_H_

#include "arMath.h"
#include "arInteractableThing.h"

class DOM : public arInteractableThing {

	public:
		DOM() : arInteractableThing(), m_domString(-1), m_domModule(-1), m_pos(0.f, 0.f, 0.f) {}
		DOM(int dString, int dModule, const arVector3 &vPos) : arInteractableThing(), m_domString(dString), m_domModule(dModule), m_pos(vPos) {}
		virtual ~DOM() {}
		
		void				draw(arMasterSlaveFramework *fw=0);
		const arVector3 &	getPos(void) const { return m_pos; }
		int					getDomString(void) const { return m_domString; }
		int					getDomModule(void) const { return m_domModule; }

	private:
		int			m_domString;
		int			m_domModule;
		arVector3	m_pos;
};

class IceCubeGeometry
{
public:
	IceCubeGeometry() {}
	~IceCubeGeometry() {}

	void draw(arMasterSlaveFramework *fw=0, float fDownScale=100.f);
	void loadGeometry(const char *sFilename);

private:
	std::vector<DOM> m_doms;
};

#endif