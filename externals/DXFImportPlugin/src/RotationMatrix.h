#ifndef RotationMatrixH
#define RotationMatrixH

#include <QQuaternion>

#include <tinyxml.h>

#include <IBK_StringUtils.h>

/*! This is a pretty dumb class and only used for the serialization code generator.
	It stores 4 floats and matches internally the QQuaternion class.
	When we copy back and forth from a QQuaternion, we just treat ourselves as a QQuaternion class
	(low level memory access is can be beautiful :-)
*/
class RotationMatrix {
public:
	RotationMatrix() {}
	RotationMatrix(const QQuaternion & q) {
		setQuaternion(q);
	}

	TiXmlElement * writeXML(TiXmlElement * parent) const {
		TiXmlElement * e = new TiXmlElement("RotationMatrix");
		parent->LinkEndChild(e);

		TiXmlElement::appendSingleAttributeElement(e, "Wp", nullptr, std::string(), IBK::val2string<float>(m_wp));
		TiXmlElement::appendSingleAttributeElement(e, "X", nullptr, std::string(), IBK::val2string<float>(m_x));
		TiXmlElement::appendSingleAttributeElement(e, "Y", nullptr, std::string(), IBK::val2string<float>(m_y));
		TiXmlElement::appendSingleAttributeElement(e, "Z", nullptr, std::string(), IBK::val2string<float>(m_z));
		return e;
	}

	/*! Conversion from QQuaternion */
	void setQuaternion(const QQuaternion & q) {
		*(QQuaternion*)(&m_wp) = q;
	}

	/*! Conversion to QQuaternion */
	QQuaternion toQuaternion() const {
		return QQuaternion(*(const QQuaternion*)(&m_wp));
	}

	float m_wp;	// XML:E:required
	float m_x;	// XML:E:required
	float m_y;	// XML:E:required
	float m_z;	// XML:E:required

};


#endif // RotationMatrixH
