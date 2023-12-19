#ifndef ObjectH
#define ObjectH

#include <vector>
#include <set>

#include <QString>

#include "Constants.h"

class Object {
public:
	/*! Standard C'tor. */
	Object() = default;
	/*! Default copy constructor. */
	Object(Object const&) = default;
	/*! D'tor. */
	virtual ~Object();

	/*! Recursively searches through data hierarchy and returns pointer to object matching the given ID.
		\return Returns pointer to wanted object or nullptr, if it couldn't be found.
	*/
	Object * findChild(unsigned int ID);
	/*! Same as function above, const version. */
	const Object * findChild(unsigned int ID) const;

	/*! Recursively selected all unique IDs of children, includes the object's ID itself as well.
		\note IDs are _added_ to the container which is not cleared initially!
	*/
	void collectChildIDs(std::set<unsigned int> & nodeContainer) const;

	/*! Returns a short descriptive string usable for error messages that identifies the object type. */
	virtual const char * typeinfo() const = 0;

	/*! Returns a descriptive string with object type, ID and optionally displayname that helps identifying this object. */
	QString info() const;

	/*! Parent pointer, do not modify. */
	Object	*m_parent = nullptr;

	/*! Persistant ID of object (not the unique ID, may not be unique in DB model, must be handled
		appropriately in error handling).
	*/
	unsigned int						m_id = INVALID_ID;

	/*! Stores visibility information for this surface. */
	bool								m_selected = false;
	/*! Stores visibility information for this surface (serialized manually in derived classes). */
	bool								m_visible = true;

	/*! The descriptive name of the object. */
	QString								m_displayName;

	/*! Contains a GUID to a IFC object in case of import from IFC.*/
	std::string							m_ifcGUID;

protected:
	/*! List of all children. */
	std::vector<Object *>				m_children;
};


#endif // ObjectH
