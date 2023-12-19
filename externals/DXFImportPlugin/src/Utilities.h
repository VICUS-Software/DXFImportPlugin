#ifndef UTILITIES_H
#define UTILITIES_H

#include "Constants.h"

#include <tinyxml.h>

#include <IBK_StringUtils.h>

#include <QString>

/*! Generates a new unique name in format "basename" or "basename (<nr>)" with increasing numbers until
	the name no longer exists in set existingNames.
	NOTE: we cannot use [<nr>] because when generating output variable names, this interferes with the
		unit specification.

	NOTE: basename is always trimmed.

	\param baseName Contains the original name, which may include already "(<nr>)"
*/
QString uniqueName(const QString & baseName, const std::set<QString> & existingNames);



template <typename T>
T readPODAttributeValue(const TiXmlElement * element, const TiXmlAttribute * attrib) {
	FUNCID(NANDRAD::readPODAttributeValue);
	try {
		return IBK::string2val<T>(attrib->ValueStr());
	} catch (IBK::Exception & ex) {
		throw IBK::Exception( ex, IBK::FormatString(XML_READ_ERROR).arg(element->Row()).arg(
			IBK::FormatString("Error reading '"+attrib->NameStr()+"' attribute.") ), FUNC_ID);
	}
}

template <typename T>
T readPODElement(const TiXmlElement * element, const std::string & eName) {
	FUNCID(NANDRAD::readPODElement);
	try {
		return IBK::string2val<T>(element->GetText());
	} catch (IBK::Exception & ex) {
		throw IBK::Exception( ex, IBK::FormatString(XML_READ_ERROR).arg(element->Row()).arg(
			IBK::FormatString("Error reading '"+eName+"' tag.") ), FUNC_ID);
	}
}


#endif // UTILITIES_H
