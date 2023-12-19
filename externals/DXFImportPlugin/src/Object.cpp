#include "Object.h"

Object::~Object() {
}


Object * Object::findChild(unsigned int ID) {
	// recycle const-variant of function implementation
	const Object * obj = const_cast<const Object*>(this)->findChild(ID);
	return const_cast<Object*>(obj);
}


const Object * Object::findChild(unsigned int ID) const {
	if (m_id == ID)
		return this;
	// search all children
	for (Object * o : m_children) {
		const Object * ob = o->findChild(ID);
		if (ob != nullptr)
			return ob;
	}
	return nullptr; // not found
}


void Object::collectChildIDs(std::set<unsigned int> & nodeContainer) const {
	for (Object * o : m_children) {
		nodeContainer.insert(o->m_id);
		o->collectChildIDs(nodeContainer);
	}
}


QString Object::info() const {
	QString infoStr = QString("%1 #%2").arg(typeinfo()).arg(m_id);
	if (!m_displayName.isEmpty())
		infoStr +=" '" + m_displayName + "'";
	return infoStr;
}
