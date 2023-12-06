#ifndef DrawingLayerH
#define DrawingLayerH

#include "Object.h"

#include <qcolor.h>

#include <tinyxml.h>


/*! Layer struct with relevant attributes */
class DrawingLayer : public Object {
public:

	DrawingLayer() {}

	const char * typeinfo() const override {
		return "DrawingLayer";
	}

	void readXML(const TiXmlElement * element);

	TiXmlElement * writeXML(TiXmlElement * parent) const;

	/*! Color of layer if defined */
	QColor			m_color = QColor();			// XML:A
	/*! Line weight of layer if defined */
	int				m_lineWeight;				// XML:A
	/*! ID of block. */
	unsigned int	m_idBlock = INVALID_ID;		// XML:A
};


#endif // DrawingLayerH
