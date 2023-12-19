#include <DrawingLayer.h>

#include <IBK_messages.h>
#include <IBK_Exception.h>
#include <IBK_StringUtils.h>
#include <Constants.h>
#include <Utilities.h>

#include <tinyxml.h>


void DrawingLayer::readXML(const TiXmlElement * element) {
	FUNCID(DrawingLayer::readXML);

	try {
		// search for mandatory attributes
		if (!TiXmlAttribute::attributeByName(element, "id"))
			throw IBK::Exception( IBK::FormatString(XML_READ_ERROR).arg(element->Row()).arg(
									 IBK::FormatString("Missing required 'id' attribute.") ), FUNC_ID);

		// reading attributes
		const TiXmlAttribute * attrib = element->FirstAttribute();
		while (attrib) {
			const std::string & attribName = attrib->NameStr();
			if (attribName == "id")
				m_id = readPODAttributeValue<unsigned int>(element, attrib);
			else if (attribName == "displayName")
				m_displayName = QString::fromStdString(attrib->ValueStr());
			else if (attribName == "visible")
				m_visible = readPODAttributeValue<bool>(element, attrib);
			else if (attribName == "color")
				m_color.setNamedColor(QString::fromStdString(attrib->ValueStr()));
			else if (attribName == "lineWeight")
				m_lineWeight = readPODAttributeValue<int>(element, attrib);
			else if (attribName == "idBlock")
				m_idBlock = readPODAttributeValue<unsigned int>(element, attrib);
			else {
				IBK::IBK_Message(IBK::FormatString(XML_READ_UNKNOWN_ATTRIBUTE).arg(attribName).arg(element->Row()), IBK::MSG_WARNING, FUNC_ID, IBK::VL_STANDARD);
			}
			attrib = attrib->Next();
		}
	}
	catch (IBK::Exception & ex) {
		throw IBK::Exception( ex, IBK::FormatString("Error reading 'DrawingLayer' element."), FUNC_ID);
	}
	catch (std::exception & ex2) {
		throw IBK::Exception( IBK::FormatString("%1\nError reading 'DrawingLayer' element.").arg(ex2.what()), FUNC_ID);
	}
}

TiXmlElement * DrawingLayer::writeXML(TiXmlElement * parent) const {
	if (m_id == INVALID_ID)  return nullptr;
	TiXmlElement * e = new TiXmlElement("DrawingLayer");
	parent->LinkEndChild(e);

	if (m_id != INVALID_ID)
		e->SetAttribute("id", IBK::val2string<unsigned int>(m_id));
	if (!m_displayName.isEmpty())
		e->SetAttribute("displayName", m_displayName.toStdString());
	if (m_visible != DrawingLayer().m_visible)
		e->SetAttribute("visible", IBK::val2string<bool>(m_visible));
	if (m_color.isValid())
		e->SetAttribute("color", m_color.name().toStdString());
	e->SetAttribute("lineWeight", IBK::val2string<int>(m_lineWeight));
	if (m_idBlock != INVALID_ID)
		e->SetAttribute("idBlock", IBK::val2string<unsigned int>(m_idBlock));
	return e;
}
