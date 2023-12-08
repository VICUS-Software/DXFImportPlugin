#include "Drawing.h"

#include "Utilities.h"
#include "Constants.h"

#include <IBK_MessageHandler.h>
#include <IBK_messages.h>
#include <IBK_physics.h>

#include <IBKMK_Vector2D.h>

#include <tinyxml.h>


/*! IBKMK::Vector3D to QVector3D conversion macro. */
inline QVector3D IBKVector2QVector(const IBKMK::Vector3D & v) {
	return QVector3D((float)v.m_x, (float)v.m_y, (float)v.m_z);
}

/*! QVector3D to IBKMK::Vector3D to conversion macro. */
inline IBKMK::Vector3D QVector2IBKVector(const QVector3D & v) {
	return IBKMK::Vector3D((double)v.x(), (double)v.y(), (double)v.z());
}


Drawing::Drawing() :
	m_blocks(std::vector<Block>()),
	m_drawingLayers(std::vector<DrawingLayer>()),
	m_points(std::vector<Point>()),
	m_lines(std::vector<Line>()),
	m_polylines(std::vector<PolyLine>()),
	m_circles(std::vector<Circle>()),
	m_ellipses(std::vector<Ellipse>()),
	m_arcs(std::vector<Arc>()),
	m_solids(std::vector<Solid>()),
	m_texts(std::vector<Text>())
{}


const Drawing::AbstractDrawingObject * Drawing::objectByID(unsigned int id) const {
	FUNCID(Drawing::objectByID);

	AbstractDrawingObject *obj = m_objectPtr.at(id);
	if (obj == nullptr)
		throw IBK::Exception(IBK::FormatString("Drawing Object with ID #%1 not found").arg(id), FUNC_ID);

	return obj;
}


void Drawing::updatePointer() {
	m_objectPtr.clear();

	for (unsigned int i=0; i < m_points.size(); ++i){
		m_points[i].m_parentLayer = findLayerPointer(m_points[i].m_layerName);
		m_points[i].m_block = findBlockPointer(m_points[i].m_blockName);
		m_objectPtr[m_points[i].m_id] = &m_points[i];
	}
	for (unsigned int i=0; i < m_lines.size(); ++i){
		m_lines[i].m_parentLayer = findLayerPointer(m_lines[i].m_layerName);
		m_lines[i].m_block = findBlockPointer(m_lines[i].m_blockName);
		m_objectPtr[m_lines[i].m_id] = &m_lines[i];
	}
	for (unsigned int i=0; i < m_polylines.size(); ++i){
		m_polylines[i].m_parentLayer = findLayerPointer(m_polylines[i].m_layerName);
		m_polylines[i].m_block = findBlockPointer(m_polylines[i].m_blockName);
		m_objectPtr[m_polylines[i].m_id] = &m_polylines[i];
	}
	for (unsigned int i=0; i < m_circles.size(); ++i){
		m_circles[i].m_parentLayer = findLayerPointer(m_circles[i].m_layerName);
		m_circles[i].m_block = findBlockPointer(m_circles[i].m_blockName);
		m_objectPtr[m_circles[i].m_id] = &m_circles[i];
	}
	for (unsigned int i=0; i < m_arcs.size(); ++i){
		m_arcs[i].m_parentLayer = findLayerPointer(m_arcs[i].m_layerName);
		m_arcs[i].m_block = findBlockPointer(m_arcs[i].m_blockName);
		m_objectPtr[m_arcs[i].m_id] = &m_arcs[i];
	}
	for (unsigned int i=0; i < m_ellipses.size(); ++i){
		m_ellipses[i].m_parentLayer = findLayerPointer(m_ellipses[i].m_layerName);
		m_ellipses[i].m_block = findBlockPointer(m_ellipses[i].m_blockName);
		m_objectPtr[m_ellipses[i].m_id] = &m_ellipses[i];
	}
	for (unsigned int i=0; i < m_solids.size(); ++i){
		m_solids[i].m_parentLayer = findLayerPointer(m_solids[i].m_layerName);
		m_solids[i].m_block = findBlockPointer(m_solids[i].m_blockName);
		m_objectPtr[m_solids[i].m_id] = &m_solids[i];
	}
	for (unsigned int i=0; i < m_texts.size(); ++i){
		m_texts[i].m_parentLayer = findLayerPointer(m_texts[i].m_layerName);
		m_texts[i].m_block = findBlockPointer(m_texts[i].m_blockName);
		m_objectPtr[m_texts[i].m_id] = &m_texts[i];
	}
	for (unsigned int i=0; i < m_inserts.size(); ++i){
		for(unsigned int j = 0; j < m_blocks.size(); ++j) {
			const QString &blockName = m_blocks[j].m_name;
			const QString &insertBlockName = m_inserts[i].m_currentBlockName;
			if (blockName == insertBlockName) {
				m_inserts[i].m_currentBlock = &m_blocks[j];
			}

			const QString &insertParentBlockName = m_inserts[i].m_parentBlockName;
			if (blockName == insertParentBlockName) {
				m_inserts[i].m_parentBlock = &m_blocks[j];
			}
		}
	}
	for (unsigned int i=0; i < m_linearDimensions.size(); ++i){
		m_linearDimensions[i].m_parentLayer = findLayerPointer(m_linearDimensions[i].m_layerName);
		m_texts[i].m_block = findBlockPointer(m_linearDimensions[i].m_blockName);
		m_objectPtr[m_linearDimensions[i].m_id] = &m_linearDimensions[i];

		for(unsigned int j = 0; j < m_dimensionStyles.size(); ++j) {
			const QString &dimStyleName = m_dimensionStyles[j].m_name;
			const QString &styleName = m_linearDimensions[i].m_styleName;
			if (dimStyleName == styleName) {
				m_linearDimensions[i].m_style = &m_dimensionStyles[j];
				break;
			}
		}

		// In order to be safe
		if (m_linearDimensions[i].m_style == nullptr)
			m_linearDimensions[i].m_style = &m_dimensionStyles.front();
	}

	// Update blocks
	//	for (unsigned int i=0; i < m_drawingLayers.size(); ++i) {
	//		if (m_drawingLayers[i].m_idBlock != INVALID_ID ) {
	//			for (unsigned int j=0; j < m_blocks.size(); ++j) {
	//				if (m_blocks[j].m_id == m_drawingLayers[i].m_idBlock) {
	//					m_drawingLayers[i].m_currentBlock = &m_blocks[j];
	//					break;
	//				}
	//			}
	//		}
	//	}
}


void Drawing::generateInsertGeometries(unsigned int &nextId) {
	FUNCID(Drawing::generateInsertGeometries);

	updateParents();

	for (const Drawing::Insert &insert : m_inserts) {

		if (insert.m_parentBlock != nullptr)
			continue;

		if (insert.m_currentBlock == nullptr)
			throw IBK::Exception(IBK::FormatString("Block with name '%1' was not found").arg(insert.m_currentBlockName.toStdString()), FUNC_ID);

		QMatrix4x4 trans;
		transformInsert(trans, insert, nextId);
	}

	updateParents();
}


template <typename t>
void addPickPoints(const std::vector<t> &objects, const Drawing &d, std::map<unsigned int, std::vector<IBKMK::Vector3D>> &verts,
				   const Drawing::Block *block = nullptr) {
	for (const t& obj : objects) {
		bool isBlockDefined = block != nullptr;
		bool hasBlock = obj.m_block != nullptr;

		if ((hasBlock && !isBlockDefined) || (isBlockDefined && !hasBlock))
			continue;

		if (isBlockDefined && hasBlock && block->m_name != obj.m_block->m_name)
			continue;

		const std::vector<IBKMK::Vector3D> &objVerts = d.points3D(obj.points2D(), obj.m_zPosition, obj.m_trans);
		verts[obj.m_id] = objVerts;
	}
}


const std::map<unsigned int, std::vector<IBKMK::Vector3D>> &Drawing::pickPoints() const {
	FUNCID(Drawing::pickPoints);
	try {
		if (m_dirtyPickPoints) {
			addPickPoints(m_points, *this, m_pickPoints);
			addPickPoints(m_arcs, *this, m_pickPoints);
			addPickPoints(m_circles, *this, m_pickPoints);
			addPickPoints(m_ellipses, *this, m_pickPoints);
			addPickPoints(m_linearDimensions, *this, m_pickPoints);
			addPickPoints(m_lines, *this, m_pickPoints);
			addPickPoints(m_polylines, *this, m_pickPoints);
			addPickPoints(m_solids, *this, m_pickPoints);

			m_dirtyPickPoints = false;
		}
		return m_pickPoints;
	}
	catch (IBK::Exception &ex) {
		throw IBK::Exception(IBK::FormatString("Could not generate pick points.\n%1").arg(ex.what()), FUNC_ID);
	}
}


const std::vector<IBKMK::Vector3D> Drawing::points3D(const std::vector<IBKMK::Vector2D> &verts, unsigned int zPosition, const QMatrix4x4 &trans) const {

	std::vector<IBKMK::Vector3D> points3D(verts.size());

	for (unsigned int i=0; i < verts.size(); ++i) {
		const IBKMK::Vector2D &v2D = verts[i];
		double zCoordinate = zPosition * Z_MULTIPLYER;
		IBKMK::Vector3D v3D = IBKMK::Vector3D(v2D.m_x * m_scalingFactor,
											  v2D.m_y * m_scalingFactor,
											  zCoordinate);

		QVector3D qV3D = m_rotationMatrix.toQuaternion() * IBKVector2QVector(v3D);
		qV3D += IBKVector2QVector(m_origin);

		// Apply block transformation from insert
		qV3D = trans * qV3D;
		points3D[i] = QVector2IBKVector(qV3D);
	}

	return points3D;
}

const IBKMK::Vector3D Drawing::normal() const {
	return QVector2IBKVector(m_rotationMatrix.toQuaternion() * QVector3D(0,0,1));
}

const IBKMK::Vector3D Drawing::localX() const {
	return QVector2IBKVector(m_rotationMatrix.toQuaternion() * QVector3D(1,0,0));
}

const IBKMK::Vector3D Drawing::localY() const {
	return QVector2IBKVector(m_rotationMatrix.toQuaternion() * QVector3D(0,1,0));
}


DrawingLayer* Drawing::findLayerPointer(const QString &layername){
	for(unsigned int i = 0; i < m_drawingLayers.size(); ++i) {
		if (m_drawingLayers[i].m_displayName == layername)
			return &m_drawingLayers[i];
	}
	return nullptr;
}

Drawing::Block *Drawing::findBlockPointer(const QString &name){
	for(unsigned int i = 0; i < m_blocks.size(); ++i) {
		if (m_blocks[i].m_name == name)
			return &m_blocks[i];
	}
	return nullptr;
}


template <typename t>
void generateObjectFromInsert(unsigned int &nextId, const Drawing::Block &block,
							  std::vector<t> &objects, const QMatrix4x4 &trans) {
	std::vector<t> newObjects;

	for (const t &obj : objects) {

		if (obj.m_block == nullptr)
			continue;

		if (obj.m_block->m_name != block.m_name)
			continue;

		t newObj(obj);
		newObj.m_id = ++nextId;
		newObj.m_trans = trans;
		newObj.m_blockName = "";
		newObj.m_block = nullptr;

		newObjects.push_back(newObj);
	}

	objects.insert(objects.end(), newObjects.begin(), newObjects.end());
}


void Drawing::transformInsert(QMatrix4x4 & trans, const Insert & insert, unsigned int & nextId) {

	Q_ASSERT(insert.m_currentBlock != nullptr);
	IBKMK::Vector2D insertPoint = insert.m_insertionPoint - insert.m_currentBlock->m_basePoint;

	trans.translate(QVector3D(insertPoint.m_x,
							  insertPoint.m_y,
							  0.0));

	for (const Insert &i : m_inserts) {
		if (i.m_parentBlock == nullptr)
			continue;

		if (insert.m_currentBlockName == i.m_parentBlock->m_name) {
			transformInsert(trans, i, nextId);
		}
	}

	generateObjectFromInsert(nextId, *insert.m_currentBlock, m_points, trans);
	generateObjectFromInsert(nextId, *insert.m_currentBlock, m_arcs, trans);
	generateObjectFromInsert(nextId, *insert.m_currentBlock, m_circles, trans);
	generateObjectFromInsert(nextId, *insert.m_currentBlock, m_ellipses, trans);
	generateObjectFromInsert(nextId, *insert.m_currentBlock, m_lines, trans);
	generateObjectFromInsert(nextId, *insert.m_currentBlock, m_polylines, trans);
	generateObjectFromInsert(nextId, *insert.m_currentBlock, m_solids, trans);
	generateObjectFromInsert(nextId, *insert.m_currentBlock, m_texts, trans);

}


TiXmlElement * Drawing::Text::writeXML(TiXmlElement * parent) const {
	if (m_id == INVALID_ID)  return nullptr;

	TiXmlElement * e = new TiXmlElement("Text");
	parent->LinkEndChild(e);

	if (m_id != INVALID_ID)
		e->SetAttribute("id", IBK::val2string<unsigned int>(m_id));
	if (m_color.isValid())
		e->SetAttribute("color", m_color.name().toStdString());
	if (m_zPosition != 0.0)
		e->SetAttribute("zPosition", IBK::val2string<unsigned int>(m_zPosition));
	if (!m_text.isEmpty())
		e->SetAttribute("text", m_text.toStdString());
	if (!m_layerName.isEmpty())
		e->SetAttribute("layer", m_layerName.toStdString());
	if (m_height != 10.0)
		e->SetAttribute("height", IBK::val2string<double>(m_height));

	TiXmlElement::appendSingleAttributeElement(e, "BasePoint", nullptr, std::string(), m_basePoint.toString());

	return e;
}

void Drawing::Text::readXML(const TiXmlElement *element){
	FUNCID(Drawing::Text::readXMLPrivate);

	try {
		// search for mandatory attributes
		if (!TiXmlAttribute::attributeByName(element, "id")) {
			IBK::IBK_Message( IBK::FormatString(XML_READ_ERROR).arg(element->Row()).arg(
								 IBK::FormatString("Missing required 'id' attribute.") ), IBK::MSG_WARNING, FUNC_ID, IBK::VL_STANDARD);
			return;
		}

		const TiXmlAttribute * attrib = element->FirstAttribute();
		while (attrib) {
			const std::string & attribName = attrib->NameStr();
			if (attribName == "id")
				m_id = readPODAttributeValue<unsigned int>(element, attrib);
			else if (attribName == "color")
				m_color = QColor(QString::fromStdString(attrib->ValueStr()));
			else if (attribName == "zPosition")
				m_zPosition = readPODAttributeValue<unsigned int>(element, attrib);
			else if (attribName == "text")
				m_text = QString::fromStdString(attrib->ValueStr());
			else if (attribName == "layer")
				m_layerName = QString::fromStdString(attrib->ValueStr());
			else if (attribName == "height")
				m_height = readPODAttributeValue<double>(element, attrib);
			else {
				IBK::IBK_Message(IBK::FormatString(XML_READ_UNKNOWN_ATTRIBUTE).arg(attribName).arg(element->Row()), IBK::MSG_WARNING, FUNC_ID, IBK::VL_STANDARD);
			}
			attrib = attrib->Next();
		}

		// reading elements
		const TiXmlElement * c = element->FirstChildElement();
		while (c) {
			const std::string & cName = c->ValueStr();
			if (cName == "BasePoint") {
				try {
					m_basePoint = IBKMK::Vector2D::fromString(c->GetText());
				} catch (IBK::Exception & ex) {
					throw IBK::Exception( ex, IBK::FormatString(XML_READ_ERROR).arg(c->Row())
												 .arg("Invalid vector data."), FUNC_ID);
				}
			}
			else {
				IBK::IBK_Message(IBK::FormatString(XML_READ_UNKNOWN_ELEMENT).arg(cName).arg(c->Row()), IBK::MSG_WARNING, FUNC_ID, IBK::VL_STANDARD);
			}
			c = c->NextSiblingElement();
		}
	}
	catch (IBK::Exception & ex) {
		throw IBK::Exception( ex, IBK::FormatString("Error reading 'Drawing::Text' element."), FUNC_ID);
	}
	catch (std::exception & ex2) {
		throw IBK::Exception( IBK::FormatString("%1\nError reading 'Drawing::Text' element.").arg(ex2.what()), FUNC_ID);
	}
}


const std::vector<IBKMK::Vector2D> &Drawing::Text::points2D() const {
	if (m_dirtyPoints) {
		m_pickPoints.clear();
		m_pickPoints.push_back(m_basePoint);
	}
	return m_pickPoints;
}


TiXmlElement * Drawing::Solid::writeXML(TiXmlElement * parent) const {
	if (m_id == INVALID_ID)  return nullptr;

	TiXmlElement * e = new TiXmlElement("Solid");
	parent->LinkEndChild(e);

	if (m_id != INVALID_ID)
		e->SetAttribute("id", IBK::val2string<unsigned int>(m_id));
	if (m_color.isValid())
		e->SetAttribute("color", m_color.name().toStdString());
	if (m_zPosition != 0.0)
		e->SetAttribute("zPosition", IBK::val2string<unsigned int>(m_zPosition));
	if (!m_layerName.isEmpty())
		e->SetAttribute("layer", m_layerName.toStdString());

	TiXmlElement::appendSingleAttributeElement(e, "Point1", nullptr, std::string(), m_point1.toString());
	TiXmlElement::appendSingleAttributeElement(e, "Point2", nullptr, std::string(), m_point2.toString());
	TiXmlElement::appendSingleAttributeElement(e, "Point3", nullptr, std::string(), m_point3.toString());
	TiXmlElement::appendSingleAttributeElement(e, "Point4", nullptr, std::string(), m_point4.toString());

	return e;
}

void Drawing::Solid::readXML(const TiXmlElement *element){
	FUNCID(Drawing::Solid::readXMLPrivate);

	try {
		// search for mandatory attributes
		if (!TiXmlAttribute::attributeByName(element, "id")) {
			IBK::IBK_Message( IBK::FormatString(XML_READ_ERROR).arg(element->Row()).arg(
								 IBK::FormatString("Missing required 'id' attribute.") ), IBK::MSG_WARNING, FUNC_ID, IBK::VL_STANDARD);
			return;
		}

		const TiXmlAttribute * attrib = element->FirstAttribute();
		while (attrib) {
			const std::string & attribName = attrib->NameStr();
			if (attribName == "id")
				m_id = readPODAttributeValue<unsigned int>(element, attrib);
			else if (attribName == "color")
				m_color = QColor(QString::fromStdString(attrib->ValueStr()));
			else if (attribName == "zPosition")
				m_zPosition = readPODAttributeValue<unsigned int>(element, attrib);
			else if (attribName == "layer")
				m_layerName = QString::fromStdString(attrib->ValueStr());
			else {
				IBK::IBK_Message(IBK::FormatString(XML_READ_UNKNOWN_ATTRIBUTE).arg(attribName).arg(element->Row()), IBK::MSG_WARNING, FUNC_ID, IBK::VL_STANDARD);
			}
			attrib = attrib->Next();
		}

		// reading elements
		const TiXmlElement * c = element->FirstChildElement();
		while (c) {
			const std::string & cName = c->ValueStr();
			if (cName == "Point1") {
				try {
					m_point1 = IBKMK::Vector2D::fromString(c->GetText());
				} catch (IBK::Exception & ex) {
					throw IBK::Exception( ex, IBK::FormatString(XML_READ_ERROR).arg(c->Row())
												 .arg("Invalid vector data."), FUNC_ID);
				}
			}
			else if (cName == "Point2") {
				try {
					m_point2 = IBKMK::Vector2D::fromString(c->GetText());
				} catch (IBK::Exception & ex) {
					throw IBK::Exception( ex, IBK::FormatString(XML_READ_ERROR).arg(c->Row())
												 .arg("Invalid vector data."), FUNC_ID);
				}
			}
			else if (cName == "Point3") {
				try {
					m_point3 = IBKMK::Vector2D::fromString(c->GetText());
				} catch (IBK::Exception & ex) {
					throw IBK::Exception( ex, IBK::FormatString(XML_READ_ERROR).arg(c->Row())
												 .arg("Invalid vector data."), FUNC_ID);
				}
			}
			else if (cName == "Point4") {
				try {
					m_point4 = IBKMK::Vector2D::fromString(c->GetText());
				} catch (IBK::Exception & ex) {
					throw IBK::Exception( ex, IBK::FormatString(XML_READ_ERROR).arg(c->Row())
												 .arg("Invalid vector data."), FUNC_ID);
				}
			}
			else {
				IBK::IBK_Message(IBK::FormatString(XML_READ_UNKNOWN_ELEMENT).arg(cName).arg(c->Row()), IBK::MSG_WARNING, FUNC_ID, IBK::VL_STANDARD);
			}
			c = c->NextSiblingElement();
		}
	}
	catch (IBK::Exception & ex) {
		throw IBK::Exception( ex, IBK::FormatString("Error reading 'Drawing::Solid' element."), FUNC_ID);
	}
	catch (std::exception & ex2) {
		throw IBK::Exception( IBK::FormatString("%1\nError reading 'Drawing::Solid' element.").arg(ex2.what()), FUNC_ID);
	}
}


const std::vector<IBKMK::Vector2D> &Drawing::Solid::points2D() const {
	return m_pickPoints;
}


TiXmlElement * Drawing::LinearDimension::writeXML(TiXmlElement * parent) const {
	if (m_id == INVALID_ID)  return nullptr;

	TiXmlElement * e = new TiXmlElement("LinearDimension");
	parent->LinkEndChild(e);

	if (m_id != INVALID_ID)
		e->SetAttribute("id", IBK::val2string<unsigned int>(m_id));
	if (m_color.isValid())
		e->SetAttribute("color", m_color.name().toStdString());
	if (m_zPosition != 0.0)
		e->SetAttribute("zPosition", IBK::val2string<unsigned int>(m_zPosition));
	if (m_angle != 0.0)
		e->SetAttribute("angle", IBK::val2string<double>(m_angle));
	if (!m_layerName.isEmpty())
		e->SetAttribute("layer", m_layerName.toStdString());
	if (!m_styleName.isEmpty())
		e->SetAttribute("styleName", m_styleName.toStdString());

	TiXmlElement::appendSingleAttributeElement(e, "Point1", nullptr, std::string(), m_point1.toString());
	TiXmlElement::appendSingleAttributeElement(e, "Point2", nullptr, std::string(), m_point2.toString());
	TiXmlElement::appendSingleAttributeElement(e, "DimensionPoint", nullptr, std::string(), m_dimensionPoint.toString());
	TiXmlElement::appendSingleAttributeElement(e, "LeftPoint", nullptr, std::string(), m_leftPoint.toString());
	TiXmlElement::appendSingleAttributeElement(e, "RightPoint", nullptr, std::string(), m_rightPoint.toString());
	TiXmlElement::appendSingleAttributeElement(e, "TextPoint", nullptr, std::string(), m_textPoint.toString());

	return e;
}

void Drawing::LinearDimension::readXML(const TiXmlElement *element){
	FUNCID(Drawing::LinearDimension::readXMLPrivate);

	try {
		// search for mandatory attributes
		if (!TiXmlAttribute::attributeByName(element, "id")) {
			IBK::IBK_Message( IBK::FormatString(XML_READ_ERROR).arg(element->Row()).arg(
								 IBK::FormatString("Missing required 'id' attribute.") ), IBK::MSG_WARNING, FUNC_ID, IBK::VL_STANDARD);
			return;
		}

		const TiXmlAttribute * attrib = element->FirstAttribute();
		while (attrib) {
			const std::string & attribName = attrib->NameStr();
			if (attribName == "id")
				m_id = readPODAttributeValue<unsigned int>(element, attrib);
			else if (attribName == "color")
				m_color = QColor(QString::fromStdString(attrib->ValueStr()));
			else if (attribName == "zPosition")
				m_zPosition = readPODAttributeValue<unsigned int>(element, attrib);
			else if (attribName == "angle")
				m_zPosition = readPODAttributeValue<double>(element, attrib);
			else if (attribName == "layer")
				m_layerName = QString::fromStdString(attrib->ValueStr());
			else if (attribName == "styleName")
				m_styleName = QString::fromStdString(attrib->ValueStr());
			else {
				IBK::IBK_Message(IBK::FormatString(XML_READ_UNKNOWN_ATTRIBUTE).arg(attribName).arg(element->Row()), IBK::MSG_WARNING, FUNC_ID, IBK::VL_STANDARD);
			}
			attrib = attrib->Next();
		}

		// reading elements
		const TiXmlElement * c = element->FirstChildElement();
		while (c) {
			const std::string & cName = c->ValueStr();
			if (cName == "Point1") {
				try {
					m_point1 = IBKMK::Vector2D::fromString(c->GetText());
				} catch (IBK::Exception & ex) {
					throw IBK::Exception( ex, IBK::FormatString(XML_READ_ERROR).arg(c->Row())
												 .arg("Invalid vector data."), FUNC_ID);
				}
			}
			else if (cName == "Point2") {
				try {
					m_point2 = IBKMK::Vector2D::fromString(c->GetText());
				} catch (IBK::Exception & ex) {
					throw IBK::Exception( ex, IBK::FormatString(XML_READ_ERROR).arg(c->Row())
												 .arg("Invalid vector data."), FUNC_ID);
				}
			}
			else if (cName == "DimensionPoint") {
				try {
					m_dimensionPoint = IBKMK::Vector2D::fromString(c->GetText());
				} catch (IBK::Exception & ex) {
					throw IBK::Exception( ex, IBK::FormatString(XML_READ_ERROR).arg(c->Row())
												 .arg("Invalid vector data."), FUNC_ID);
				}
			}
			else if (cName == "LeftPoint") {
				try {
					m_leftPoint = IBKMK::Vector2D::fromString(c->GetText());
				} catch (IBK::Exception & ex) {
					throw IBK::Exception( ex, IBK::FormatString(XML_READ_ERROR).arg(c->Row())
												 .arg("Invalid vector data."), FUNC_ID);
				}
			}
			else if (cName == "RightPoint") {
				try {
					m_rightPoint = IBKMK::Vector2D::fromString(c->GetText());
				} catch (IBK::Exception & ex) {
					throw IBK::Exception( ex, IBK::FormatString(XML_READ_ERROR).arg(c->Row())
												 .arg("Invalid vector data."), FUNC_ID);
				}
			}

			else if (cName == "TextPoint") {
				try {
					m_textPoint = IBKMK::Vector2D::fromString(c->GetText());
				} catch (IBK::Exception & ex) {
					throw IBK::Exception( ex, IBK::FormatString(XML_READ_ERROR).arg(c->Row())
												 .arg("Invalid vector data."), FUNC_ID);
				}
			}
			else {
				IBK::IBK_Message(IBK::FormatString(XML_READ_UNKNOWN_ELEMENT).arg(cName).arg(c->Row()), IBK::MSG_WARNING, FUNC_ID, IBK::VL_STANDARD);
			}
			c = c->NextSiblingElement();
		}
	}
	catch (IBK::Exception & ex) {
		throw IBK::Exception( ex, IBK::FormatString("Error reading 'Drawing::LinearDimension' element."), FUNC_ID);
	}
	catch (std::exception & ex2) {
		throw IBK::Exception( IBK::FormatString("%1\nError reading 'Drawing::LinearDimension' element.").arg(ex2.what()), FUNC_ID);
	}
}


const std::vector<IBKMK::Vector2D> &Drawing::LinearDimension::points2D() const {
	return m_pickPoints;
}


TiXmlElement * Drawing::Point::writeXML(TiXmlElement * parent) const {
	if (m_id == INVALID_ID)  return nullptr;

	TiXmlElement * e = new TiXmlElement("Point");
	parent->LinkEndChild(e);

	if (m_id != INVALID_ID)
		e->SetAttribute("id", IBK::val2string<unsigned int>(m_id));
	if (m_color.isValid())
		e->SetAttribute("color", m_color.name().toStdString());
	if (m_zPosition != 0.0)
		e->SetAttribute("zPosition", IBK::val2string<unsigned int>(m_zPosition));
	if (!m_layerName.isEmpty())
		e->SetAttribute("layer", m_layerName.toStdString());

	TiXmlElement::appendSingleAttributeElement(e, "Point", nullptr, std::string(), m_point.toString());

	return e;
}


void Drawing::Point::readXML(const TiXmlElement *element){
	FUNCID(Drawing::Circle::readXMLPrivate);

	try {
		// search for mandatory attributes
		if (!TiXmlAttribute::attributeByName(element, "id")) {
			IBK::IBK_Message( IBK::FormatString(XML_READ_ERROR).arg(element->Row()).arg(
								 IBK::FormatString("Missing required 'id' attribute.") ), IBK::MSG_WARNING, FUNC_ID, IBK::VL_STANDARD);
			return;
		}


		const TiXmlAttribute * attrib = element->FirstAttribute();
		while (attrib) {
			const std::string & attribName = attrib->NameStr();
			if (attribName == "id")
				m_id = readPODAttributeValue<unsigned int>(element, attrib);
			else if (attribName == "color")
				m_color = QColor(QString::fromStdString(attrib->ValueStr()));
			else if (attribName == "zPosition")
				m_zPosition = readPODAttributeValue<unsigned int>(element, attrib);
			else if (attribName == "layer")
				m_layerName = QString::fromStdString(attrib->ValueStr());
			else {
				IBK::IBK_Message(IBK::FormatString(XML_READ_UNKNOWN_ATTRIBUTE).arg(attribName).arg(element->Row()), IBK::MSG_WARNING, FUNC_ID, IBK::VL_STANDARD);
			}
			attrib = attrib->Next();
		}

		// reading elements
		const TiXmlElement * c = element->FirstChildElement();
		while (c) {
			const std::string & cName = c->ValueStr();
			if (cName == "Point") {
				try {
					m_point = IBKMK::Vector2D::fromString(c->GetText());
				} catch (IBK::Exception & ex) {
					throw IBK::Exception( ex, IBK::FormatString(XML_READ_ERROR).arg(c->Row())
												 .arg("Invalid vector data."), FUNC_ID);
				}
			}
			else {
				IBK::IBK_Message(IBK::FormatString(XML_READ_UNKNOWN_ELEMENT).arg(cName).arg(c->Row()), IBK::MSG_WARNING, FUNC_ID, IBK::VL_STANDARD);
			}
			c = c->NextSiblingElement();
		}
	}
	catch (IBK::Exception & ex) {
		throw IBK::Exception( ex, IBK::FormatString("Error reading 'Drawing::Point' element."), FUNC_ID);
	}
	catch (std::exception & ex2) {
		throw IBK::Exception( IBK::FormatString("%1\nError reading 'Drawing::Point' element.").arg(ex2.what()), FUNC_ID);
	}
}

const std::vector<IBKMK::Vector2D> &Drawing::Point::points2D() const {
	if (m_dirtyPoints) {
		m_pickPoints.clear();
		m_pickPoints.push_back(m_point);

		m_dirtyPoints = false;
	}

	return m_pickPoints;
}


void Drawing::Line::readXML(const TiXmlElement *element){
	FUNCID(Drawing::Circle::readXMLPrivate);

	try {
		// search for mandatory attributes
		if (!TiXmlAttribute::attributeByName(element, "id")) {
			IBK::IBK_Message( IBK::FormatString(XML_READ_ERROR).arg(element->Row()).arg(
								 IBK::FormatString("Missing required 'id' attribute.") ), IBK::MSG_WARNING, FUNC_ID, IBK::VL_STANDARD);
			return;
		}

		const TiXmlAttribute * attrib = element->FirstAttribute();
		while (attrib) {
			const std::string & attribName = attrib->NameStr();
			if (attribName == "id")
				m_id = readPODAttributeValue<unsigned int>(element, attrib);
			else if (attribName == "color")
				m_color = QColor(QString::fromStdString(attrib->ValueStr()));
			else if (attribName == "zPosition")
				m_zPosition = readPODAttributeValue<unsigned int>(element, attrib);
			else if (attribName == "layer")
				m_layerName = QString::fromStdString(attrib->ValueStr());
			else {
				IBK::IBK_Message(IBK::FormatString(XML_READ_UNKNOWN_ATTRIBUTE).arg(attribName).arg(element->Row()), IBK::MSG_WARNING, FUNC_ID, IBK::VL_STANDARD);
			}
			attrib = attrib->Next();
		}

		// reading elements
		const TiXmlElement * c = element->FirstChildElement();
		while (c) {
			const std::string & cName = c->ValueStr();
			if (cName == "Point1") {
				try {
					m_point1 = IBKMK::Vector2D::fromString(c->GetText());
				} catch (IBK::Exception & ex) {
					throw IBK::Exception( ex, IBK::FormatString(XML_READ_ERROR).arg(c->Row())
												 .arg("Invalid vector data."), FUNC_ID);
				}
			}
			else if (cName == "Point2") {
				try {
					m_point2 = IBKMK::Vector2D::fromString(c->GetText());
				} catch (IBK::Exception & ex) {
					throw IBK::Exception( ex, IBK::FormatString(XML_READ_ERROR).arg(c->Row())
												 .arg("Invalid vector data."), FUNC_ID);
				}
			}
			else {
				IBK::IBK_Message(IBK::FormatString(XML_READ_UNKNOWN_ELEMENT).arg(cName).arg(c->Row()), IBK::MSG_WARNING, FUNC_ID, IBK::VL_STANDARD);
			}
			c = c->NextSiblingElement();
		}
	}
	catch (IBK::Exception & ex) {
		throw IBK::Exception( ex, IBK::FormatString("Error reading 'Drawing::Line' element."), FUNC_ID);
	}
	catch (std::exception & ex2) {
		throw IBK::Exception( IBK::FormatString("%1\nError reading 'Drawing::Line' element.").arg(ex2.what()), FUNC_ID);
	}
}


const std::vector<IBKMK::Vector2D> &Drawing::Line::points2D() const {
	if (m_dirtyPoints) {
		m_pickPoints.clear();

		m_pickPoints.push_back(m_point1);
		m_pickPoints.push_back(m_point2);

		m_dirtyPoints = false;
	}

	return m_pickPoints;
}


TiXmlElement * Drawing::Line::writeXML(TiXmlElement * parent) const {
	if (m_id == INVALID_ID)  return nullptr;

	TiXmlElement * e = new TiXmlElement("Line");
	parent->LinkEndChild(e);

	if (m_id != INVALID_ID)
		e->SetAttribute("id", IBK::val2string<unsigned int>(m_id));
	if (m_zPosition != 0.0)
		e->SetAttribute("zPosition", IBK::val2string<unsigned int>(m_zPosition));
	if (m_color.isValid())
		e->SetAttribute("color", m_color.name().toStdString());
	if (!m_layerName.isEmpty())
		e->SetAttribute("layer", m_layerName.toStdString());

	TiXmlElement::appendSingleAttributeElement(e, "Point1", nullptr, std::string(), m_point1.toString());
	TiXmlElement::appendSingleAttributeElement(e, "Point2", nullptr, std::string(), m_point2.toString());

	return e;
}


TiXmlElement * Drawing::Circle::writeXML(TiXmlElement * parent) const {
	if (m_id == INVALID_ID)  return nullptr;

	TiXmlElement * e = new TiXmlElement("Circle");
	parent->LinkEndChild(e);

	if (m_id != INVALID_ID)
		e->SetAttribute("id", IBK::val2string<unsigned int>(m_id));
	if (m_zPosition != 0.0)
		e->SetAttribute("zPosition", IBK::val2string<double>(m_id));
	if (m_color.isValid())
		e->SetAttribute("color", m_color.name().toStdString());
	if (!m_layerName.isEmpty())
		e->SetAttribute("layer", m_layerName.toStdString());

	TiXmlElement::appendSingleAttributeElement(e, "Center", nullptr, std::string(), m_center.toString());
	TiXmlElement::appendSingleAttributeElement(e, "Radius", nullptr, std::string(), IBK::val2string<double>(m_radius));

	return e;
}

void Drawing::Circle::readXML(const TiXmlElement *element){
	FUNCID(Drawing::Circle::readXMLPrivate);

	try {
		// search for mandatory attributes
		if (!TiXmlAttribute::attributeByName(element, "id")) {
			IBK::IBK_Message( IBK::FormatString(XML_READ_ERROR).arg(element->Row()).arg(
								 IBK::FormatString("Missing required 'id' attribute.") ), IBK::MSG_WARNING, FUNC_ID, IBK::VL_STANDARD);
			return;
		}

		const TiXmlAttribute * attrib = element->FirstAttribute();
		while (attrib) {
			const std::string & attribName = attrib->NameStr();
			if (attribName == "id")
				m_id = readPODAttributeValue<unsigned int>(element, attrib);
			else if (attribName == "color")
				m_color = QColor(QString::fromStdString(attrib->ValueStr()));
			else if (attribName == "zPosition")
				m_zPosition = readPODAttributeValue<unsigned int>(element, attrib);
			else if (attribName == "layer")
				m_layerName = QString::fromStdString(attrib->ValueStr());
			else {
				IBK::IBK_Message(IBK::FormatString(XML_READ_UNKNOWN_ATTRIBUTE).arg(attribName).arg(element->Row()), IBK::MSG_WARNING, FUNC_ID, IBK::VL_STANDARD);
			}
			attrib = attrib->Next();
		}

		// reading elements
		const TiXmlElement * c = element->FirstChildElement();
		while (c) {
			const std::string & cName = c->ValueStr();
			if (cName == "Radius")
				m_radius = readPODElement<double>(c, cName);
			else if (cName == "Center") {
				try {
					m_center = IBKMK::Vector2D::fromString(c->GetText());
				} catch (IBK::Exception & ex) {
					throw IBK::Exception( ex, IBK::FormatString(XML_READ_ERROR).arg(c->Row())
												 .arg("Invalid vector data."), FUNC_ID);
				}
			}
			else {
				IBK::IBK_Message(IBK::FormatString(XML_READ_UNKNOWN_ELEMENT).arg(cName).arg(c->Row()), IBK::MSG_WARNING, FUNC_ID, IBK::VL_STANDARD);
			}
			c = c->NextSiblingElement();
		}
	}
	catch (IBK::Exception & ex) {
		throw IBK::Exception( ex, IBK::FormatString("Error reading 'Drawing::Circle' element."), FUNC_ID);
	}
}


const std::vector<IBKMK::Vector2D> &Drawing::Circle::points2D() const {
	if (m_dirtyPoints) {
		m_pickPoints.clear();
		m_pickPoints.resize(SEGMENT_COUNT_CIRCLE);

		for(unsigned int i = 0; i < SEGMENT_COUNT_CIRCLE; ++i){
			m_pickPoints[i] =IBKMK::Vector2D(m_center.m_x + m_radius * cos(2 * IBK::PI * i / SEGMENT_COUNT_CIRCLE),
											  m_center.m_y + m_radius * sin(2 * IBK::PI * i / SEGMENT_COUNT_CIRCLE));
		}

		m_dirtyPoints = false;
	}

	return m_pickPoints;
}


TiXmlElement * Drawing::PolyLine::writeXML(TiXmlElement * parent) const {
	if (m_id == INVALID_ID)
		return nullptr;

	TiXmlElement * e = new TiXmlElement("PolyLine");
	parent->LinkEndChild(e);

	if (!m_polyline.empty()) {
		if (m_id != INVALID_ID)
			e->SetAttribute("id", IBK::val2string<unsigned int>(m_id));
		if (m_color.isValid())
			e->SetAttribute("color", m_color.name().toStdString());
		if (m_zPosition != 0.0)
			e->SetAttribute("zPosition", IBK::val2string<unsigned int>(m_zPosition));
		if (m_endConnected)
			e->SetAttribute("connected", IBK::val2string<bool>(m_endConnected));
		if (!m_layerName.isEmpty())
			e->SetAttribute("layer", m_layerName.toStdString());

		std::stringstream vals;
		vals << std::setprecision(4);
		const std::vector<IBKMK::Vector2D> & polyVertexes = m_polyline;
		for (unsigned int i=0; i<polyVertexes.size(); ++i) {
			vals << polyVertexes[i].m_x << " " << polyVertexes[i].m_y;
			if (i<polyVertexes.size()-1)  vals << ", ";
		}
		TiXmlText * text = new TiXmlText( vals.str() );
		e->LinkEndChild( text );
	}

	return e;
}


void Drawing::PolyLine::readXML(const TiXmlElement *element){
	FUNCID(Drawing::Arc::readXMLPrivate);

	try {
		// search for mandatory attributes
		if (!TiXmlAttribute::attributeByName(element, "id")) {
			IBK::IBK_Message( IBK::FormatString(XML_READ_ERROR).arg(element->Row()).arg(
								 IBK::FormatString("Missing required 'id' attribute.") ), IBK::MSG_WARNING, FUNC_ID, IBK::VL_STANDARD);
			return;
		}

		const TiXmlAttribute * attrib = element->FirstAttribute();
		while (attrib) {
			const std::string & attribName = attrib->NameStr();
			if (attribName == "id")
				m_id = readPODAttributeValue<unsigned int>(element, attrib);
			else if (attribName == "color")
				m_color = QColor(QString::fromStdString(attrib->ValueStr()));
			else if (attribName == "zPosition")
				m_zPosition = readPODAttributeValue<unsigned int>(element, attrib);
			else if (attribName == "connected")
				m_endConnected = readPODAttributeValue<bool>(element, attrib);
			else if (attribName == "layer")
				m_layerName = QString::fromStdString(attrib->ValueStr());
			else {
				IBK::IBK_Message(IBK::FormatString(XML_READ_UNKNOWN_ATTRIBUTE).arg(attribName).arg(element->Row()), IBK::MSG_WARNING, FUNC_ID, IBK::VL_STANDARD);
			}
			attrib = attrib->Next();
		}

		// reading elements
		// read vertexes
		std::string text = element->GetText();
		text = IBK::replace_string(text, ",", " ");
		std::vector<IBKMK::Vector2D> verts;
		try {
			std::vector<double> vals;
			IBK::string2valueVector(text, vals);
			// must have n*2 elements
			if (vals.size() % 2 != 0)
				throw IBK::Exception("Mismatching number of values.", FUNC_ID);
			if (vals.empty())
				throw IBK::Exception("Missing values.", FUNC_ID);
			verts.resize(vals.size() / 2);
			for (unsigned int i=0; i<verts.size(); ++i){
				verts[i].m_x = vals[i*2];
				verts[i].m_y = vals[i*2+1];
			}

			m_polyline = verts;

		} catch (IBK::Exception & ex) {
			throw IBK::Exception( ex, IBK::FormatString(XML_READ_ERROR).arg(element->Row())
										 .arg("Error reading element 'PolyLine'." ), FUNC_ID);
		}
	}
	catch (IBK::Exception & ex) {
		throw IBK::Exception( ex, IBK::FormatString("Error reading 'Drawing::PolyLine' element."), FUNC_ID);
	}
	catch (std::exception & ex2) {
		throw IBK::Exception( IBK::FormatString("%1\nError reading 'Drawing::PolyLine' element.").arg(ex2.what()), FUNC_ID);
	}
}


const std::vector<IBKMK::Vector2D> &Drawing::PolyLine::points2D() const {
	if (m_dirtyPoints) {
		m_pickPoints.clear();

		// iterateover data.vertlist, insert all vertices of Polyline into vector
		for(size_t i = 0; i < m_polyline.size(); ++i){
			m_pickPoints.push_back(m_polyline[i]);
		}
		m_dirtyPoints = false;
	}
	return m_pickPoints;
}


TiXmlElement * Drawing::Arc::writeXML(TiXmlElement * parent) const {
	if (m_id == INVALID_ID)  return nullptr;

	TiXmlElement * e = new TiXmlElement("Arc");
	parent->LinkEndChild(e);

	if (m_id != INVALID_ID)
		e->SetAttribute("id", IBK::val2string<unsigned int>(m_id));
	if (m_color.isValid())
		e->SetAttribute("color", m_color.name().toStdString());
	if (m_zPosition != 0.0)
		e->SetAttribute("zPosition", IBK::val2string<unsigned int>(m_zPosition));
	if (!m_layerName.isEmpty())
		e->SetAttribute("layer", m_layerName.toStdString());

	TiXmlElement::appendSingleAttributeElement(e, "Center", nullptr, std::string(), m_center.toString());
	TiXmlElement::appendSingleAttributeElement(e, "Radius", nullptr, std::string(), IBK::val2string<double>(m_radius));
	TiXmlElement::appendSingleAttributeElement(e, "StartAngle", nullptr, std::string(), IBK::val2string<double>(m_startAngle));
	TiXmlElement::appendSingleAttributeElement(e, "EndAngle", nullptr, std::string(), IBK::val2string<double>(m_endAngle));

	return e;
}

void Drawing::Arc::readXML(const TiXmlElement *element){
	FUNCID(Drawing::Arc::readXMLPrivate);

	try {
		// search for mandatory attributes
		if (!TiXmlAttribute::attributeByName(element, "id")) {
			IBK::IBK_Message( IBK::FormatString(XML_READ_ERROR).arg(element->Row()).arg(
								 IBK::FormatString("Missing required 'id' attribute.") ), IBK::MSG_WARNING, FUNC_ID, IBK::VL_STANDARD);
			return;
		}

		const TiXmlAttribute * attrib = element->FirstAttribute();
		while (attrib) {
			const std::string & attribName = attrib->NameStr();
			if (attribName == "id")
				m_id = readPODAttributeValue<unsigned int>(element, attrib);
			else if (attribName == "color")
				m_color = QColor(QString::fromStdString(attrib->ValueStr()));
			else if (attribName == "zPosition")
				m_zPosition = readPODAttributeValue<unsigned int>(element, attrib);
			else if (attribName == "layer")
				m_layerName = QString::fromStdString(attrib->ValueStr());
			else {
				IBK::IBK_Message(IBK::FormatString(XML_READ_UNKNOWN_ATTRIBUTE).arg(attribName).arg(element->Row()), IBK::MSG_WARNING, FUNC_ID, IBK::VL_STANDARD);
			}
			attrib = attrib->Next();
		}

		// reading elements
		const TiXmlElement * c = element->FirstChildElement();
		while (c) {
			const std::string & cName = c->ValueStr();
			if (cName == "Radius")
				m_radius = readPODElement<double>(c, cName);
			else if (cName == "StartAngle")
				m_startAngle = readPODElement<double>(c, cName);
			else if (cName == "EndAngle")
				m_endAngle = readPODElement<double>(c, cName);
			else if (cName == "Center") {
				try {
					m_center = IBKMK::Vector2D::fromString(c->GetText());
				} catch (IBK::Exception & ex) {
					throw IBK::Exception( ex, IBK::FormatString(XML_READ_ERROR).arg(c->Row())
												 .arg("Invalid vector data."), FUNC_ID);
				}
			}
			else {
				IBK::IBK_Message(IBK::FormatString(XML_READ_UNKNOWN_ELEMENT).arg(cName).arg(c->Row()), IBK::MSG_WARNING, FUNC_ID, IBK::VL_STANDARD);
			}
			c = c->NextSiblingElement();
		}
	}
	catch (IBK::Exception & ex) {
		throw IBK::Exception( ex, IBK::FormatString("Error reading 'Drawing::Arc' element."), FUNC_ID);
	}
	catch (std::exception & ex2) {
		throw IBK::Exception( IBK::FormatString("%1\nError reading 'Drawing::Arc' element.").arg(ex2.what()), FUNC_ID);
	}
}


const std::vector<IBKMK::Vector2D>& Drawing::Arc::points2D() const {

	if (m_dirtyPoints) {
		double startAngle = m_startAngle;
		double endAngle = m_endAngle;

		double angleDifference;

		if (startAngle > endAngle)
			angleDifference = 2 * IBK::PI - startAngle + endAngle;
		else
			angleDifference = endAngle - startAngle;

		unsigned int n = std::ceil(angleDifference / (2 * IBK::PI) * SEGMENT_COUNT_ARC);
		double stepAngle = angleDifference / n;

		m_pickPoints.resize(n + 1);
		for (unsigned int i = 0; i < n+1; ++i){
			m_pickPoints[i] = IBKMK::Vector2D(m_center.m_x + m_radius * cos(startAngle + i * stepAngle),
											  m_center.m_y + m_radius * sin(startAngle + i * stepAngle));
		}

		m_dirtyPoints = false;
	}
	return m_pickPoints;
}


TiXmlElement * Drawing::Ellipse::writeXML(TiXmlElement * parent) const {
	if (m_id == INVALID_ID)  return nullptr;

	TiXmlElement * e = new TiXmlElement("Ellipse");
	parent->LinkEndChild(e);

	if (m_id != INVALID_ID)
		e->SetAttribute("id", IBK::val2string<unsigned int>(m_id));
	if (m_color.isValid())
		e->SetAttribute("color", m_color.name().toStdString());
	if (m_zPosition != 0.0)
		e->SetAttribute("zPosition", IBK::val2string<unsigned int>(m_zPosition));
	if (!m_layerName.isEmpty())
		e->SetAttribute("layer", m_layerName.toStdString());

	TiXmlElement::appendSingleAttributeElement(e, "Center", nullptr, std::string(), m_center.toString());
	TiXmlElement::appendSingleAttributeElement(e, "MajorAxis", nullptr, std::string(), m_majorAxis.toString());
	TiXmlElement::appendSingleAttributeElement(e, "Ratio", nullptr, std::string(), IBK::val2string<double>(m_ratio));
	TiXmlElement::appendSingleAttributeElement(e, "StartAngle", nullptr, std::string(), IBK::val2string<double>(m_startAngle));
	TiXmlElement::appendSingleAttributeElement(e, "EndAngle", nullptr, std::string(), IBK::val2string<double>(m_endAngle));

	return e;
}


void Drawing::Ellipse::readXML(const TiXmlElement *element){
	FUNCID(Drawing::Arc::readXMLPrivate);

	try {
		// search for mandatory attributes
		if (!TiXmlAttribute::attributeByName(element, "id")) {
			IBK::IBK_Message( IBK::FormatString(XML_READ_ERROR).arg(element->Row()).arg(
								 IBK::FormatString("Missing required 'id' attribute.") ), IBK::MSG_WARNING, FUNC_ID, IBK::VL_STANDARD);
			return;
		}

		const TiXmlAttribute * attrib = element->FirstAttribute();
		while (attrib) {
			const std::string & attribName = attrib->NameStr();
			if (attribName == "id")
				m_id = readPODAttributeValue<unsigned int>(element, attrib);
			else if (attribName == "color")
				m_color = QColor(QString::fromStdString(attrib->ValueStr()));
			else if (attribName == "zPosition")
				m_zPosition = readPODAttributeValue<unsigned int>(element, attrib);
			else if (attribName == "layer")
				m_layerName = QString::fromStdString(attrib->ValueStr());
			else {
				IBK::IBK_Message(IBK::FormatString(XML_READ_UNKNOWN_ATTRIBUTE).arg(attribName).arg(element->Row()), IBK::MSG_WARNING, FUNC_ID, IBK::VL_STANDARD);
			}
			attrib = attrib->Next();
		}

		// reading elements
		const TiXmlElement * c = element->FirstChildElement();
		while (c) {
			const std::string & cName = c->ValueStr();
			if (cName == "Ratio")
				m_ratio = readPODElement<double>(c, cName);
			else if (cName == "StartAngle")
				m_startAngle = readPODElement<double>(c, cName);
			else if (cName == "EndAngle")
				m_endAngle = readPODElement<double>(c, cName);
			else if (cName == "Center") {
				try {
					m_center = IBKMK::Vector2D::fromString(c->GetText());
				} catch (IBK::Exception & ex) {
					throw IBK::Exception( ex, IBK::FormatString(XML_READ_ERROR).arg(c->Row())
												 .arg("Invalid vector data."), FUNC_ID);
				}
			}
			else if (cName == "MajorAxis") {
				try {
					m_majorAxis = IBKMK::Vector2D::fromString(c->GetText());
				} catch (IBK::Exception & ex) {
					throw IBK::Exception( ex, IBK::FormatString(XML_READ_ERROR).arg(c->Row())
												 .arg("Invalid vector data."), FUNC_ID);
				}
			}

			else {
				IBK::IBK_Message(IBK::FormatString(XML_READ_UNKNOWN_ELEMENT).arg(cName).arg(c->Row()), IBK::MSG_WARNING, FUNC_ID, IBK::VL_STANDARD);
			}
			c = c->NextSiblingElement();
		}
	}
	catch (IBK::Exception & ex) {
		throw IBK::Exception( ex, IBK::FormatString("Error reading 'Drawing::Arc' element."), FUNC_ID);
	}
	catch (std::exception & ex2) {
		throw IBK::Exception( IBK::FormatString("%1\nError reading 'Drawing::Arc' element.").arg(ex2.what()), FUNC_ID);
	}
}


const std::vector<IBKMK::Vector2D> &Drawing::Ellipse::points2D() const {
	if (m_dirtyPoints) {
		m_pickPoints.clear();

		// iterateover data.vertlist, insert all vertices of Polyline into vector
		double startAngle = m_startAngle;
		double endAngle = m_endAngle;

		double angleStep = (endAngle - startAngle) / (SEGMENT_COUNT_ELLIPSE - 1);

		double majorRadius = sqrt(pow(m_majorAxis.m_x, 2) + pow(m_majorAxis.m_y, 2));
		double minorRadius = majorRadius * m_ratio;
		double rotationAngle = atan2(m_majorAxis.m_y, m_majorAxis.m_x);

		double x, y, rotated_x, rotated_y;

		m_pickPoints.resize(SEGMENT_COUNT_ELLIPSE);

		for (unsigned int i = 0; i < SEGMENT_COUNT_ELLIPSE; ++i) {

			double currentAngle = startAngle + i * angleStep;

			x = majorRadius * cos(currentAngle);
			y = minorRadius * sin(currentAngle);

			rotated_x = x * cos(rotationAngle) - y * sin(rotationAngle);
			rotated_y = x * sin(rotationAngle) + y * cos(rotationAngle);

			m_pickPoints[i] = IBKMK::Vector2D(rotated_x + m_center.m_x,
											  rotated_y + m_center.m_y);

		}
		m_dirtyPoints = false;
	}
	return m_pickPoints;
}


TiXmlElement *Drawing::DimStyle::writeXML(TiXmlElement *parent) const {
	if (m_id == INVALID_ID)  return nullptr;

	TiXmlElement * e = new TiXmlElement("DimStyle");
	parent->LinkEndChild(e);

	if (m_id != INVALID_ID)
		e->SetAttribute("id", IBK::val2string<unsigned int>(m_id));
	if (m_name != QString())
		e->SetAttribute("name", m_name.toStdString());
	if (m_upperLineDistance > 0.0)
		e->SetAttribute("upperLineDistance", IBK::val2string<double>(m_upperLineDistance));
	if (m_extensionLineLowerDistance > 0.0)
		e->SetAttribute("extensionLineLowerDistance", IBK::val2string<double>(m_extensionLineLowerDistance));
	if (m_extensionLineLength > 0.0)
		e->SetAttribute("extensionLineLength", IBK::val2string<double>(m_extensionLineLength));
	if (!m_fixedExtensionLength)
		e->SetAttribute("fixedExtensionLength", IBK::val2string<bool>(m_fixedExtensionLength));
	if (m_textHeight > 0.0)
		e->SetAttribute("textHeight", IBK::val2string<double>(m_textHeight));

	return e;
}

void Drawing::DimStyle::readXML(const TiXmlElement *element) {
	FUNCID(Drawing::DimStyle::readXMLPrivate);

	try {
		// search for mandatory attributes
		if (!TiXmlAttribute::attributeByName(element, "id")) {
			IBK::IBK_Message( IBK::FormatString(XML_READ_ERROR).arg(element->Row()).arg(
								 IBK::FormatString("Missing required 'id' attribute.") ), IBK::MSG_WARNING, FUNC_ID, IBK::VL_STANDARD);
			return;
		}


		const TiXmlAttribute * attrib = element->FirstAttribute();
		while (attrib) {
			const std::string & attribName = attrib->NameStr();
			if (attribName == "id")
				m_id = readPODAttributeValue<unsigned int>(element, attrib);
			else if (attribName == "name")
				m_name = QString::fromStdString(attrib->ValueStr());
			else if (attribName == "upperLineDistance")
				m_upperLineDistance = readPODAttributeValue<double>(element, attrib);
			else if (attribName == "extensionLineLowerDistance")
				m_extensionLineLowerDistance = readPODAttributeValue<double>(element, attrib);
			else if (attribName == "extensionLineLength")
				m_extensionLineLength = readPODAttributeValue<double>(element, attrib);
			else if (attribName == "fixedExtensionLength")
				m_fixedExtensionLength = readPODAttributeValue<bool>(element, attrib);
			else if (attribName == "textHeight")
				m_textHeight = readPODAttributeValue<double>(element, attrib);

			attrib = attrib->Next();
		}

		// reading elements

	}
	catch (IBK::Exception & ex) {
		throw IBK::Exception( ex, IBK::FormatString("Error reading 'Drawing::DimStyle' element."), FUNC_ID);
	}
	catch (std::exception & ex2) {
		throw IBK::Exception( IBK::FormatString("%1\nError reading 'Drawing::DimStyle' element.").arg(ex2.what()), FUNC_ID);
	}
}


TiXmlElement * Drawing::Block::writeXML(TiXmlElement * parent) const {
	if (m_id == INVALID_ID)  return nullptr;

	TiXmlElement * e = new TiXmlElement("Block");
	parent->LinkEndChild(e);

	if (m_id != INVALID_ID)
		e->SetAttribute("id", IBK::val2string<unsigned int>(m_id));
	if (m_color.isValid())
		e->SetAttribute("color", m_color.name().toStdString());
	if (!m_name.isEmpty())
		e->SetAttribute("name", m_name.toStdString());
	if (m_lineWeight > 0)
		e->SetAttribute("lineWeight", IBK::val2string<int>(m_lineWeight));

	TiXmlElement::appendSingleAttributeElement(e, "basePoint", nullptr, std::string(), m_basePoint.toString());

	return e;
}


void Drawing::Block::readXML(const TiXmlElement *element){
	FUNCID(Drawing::Circle::readXMLPrivate);

	try {
		// search for mandatory attributes
		if (!TiXmlAttribute::attributeByName(element, "id")) {
			IBK::IBK_Message( IBK::FormatString(XML_READ_ERROR).arg(element->Row()).arg(
									 IBK::FormatString("Missing required 'id' attribute.") ), IBK::MSG_WARNING, FUNC_ID, IBK::VL_STANDARD);
			return;
		}

		// reading attributes
		const TiXmlAttribute * attrib = element->FirstAttribute();
		while (attrib) {
			const std::string & attribName = attrib->NameStr();
			if (attribName == "id")
				m_id = readPODAttributeValue<unsigned int>(element, attrib);
			else if (attribName == "lineWeight")
				m_lineWeight = readPODAttributeValue<int>(element, attrib);
			else if (attribName == "name")
				m_name = QString::fromStdString(attrib->ValueStr());
			else if (attribName == "color")
				m_color.setNamedColor(QString::fromStdString(attrib->ValueStr()));
			else {
				IBK::IBK_Message(IBK::FormatString(XML_READ_UNKNOWN_ATTRIBUTE).arg(attribName).arg(element->Row()), IBK::MSG_WARNING, FUNC_ID, IBK::VL_STANDARD);
			}
			attrib = attrib->Next();
		}

		// reading elements
		const TiXmlElement * c = element->FirstChildElement();
		while (c) {
			const std::string & cName = c->ValueStr();
			if (cName == "basePoint") {
				try {
					m_basePoint = IBKMK::Vector2D::fromString(c->GetText());
				} catch (IBK::Exception & ex) {
					throw IBK::Exception( ex, IBK::FormatString(XML_READ_ERROR).arg(c->Row())
												 .arg("Invalid vector data."), FUNC_ID);
				}
			}
			else {
				IBK::IBK_Message(IBK::FormatString(XML_READ_UNKNOWN_ELEMENT).arg(cName).arg(c->Row()), IBK::MSG_WARNING, FUNC_ID, IBK::VL_STANDARD);
			}
			c = c->NextSiblingElement();
		}
	}
	catch (IBK::Exception & ex) {
		throw IBK::Exception( ex, IBK::FormatString("Error reading 'ZoneTemplate' element."), FUNC_ID);
	}
	catch (std::exception & ex2) {
		throw IBK::Exception( IBK::FormatString("%1\nError reading 'ZoneTemplate' element.").arg(ex2.what()), FUNC_ID);
	}
}


TiXmlElement *Drawing::Insert::writeXML(TiXmlElement *parent) const {
	TiXmlElement * e = new TiXmlElement("Insert");
	parent->LinkEndChild(e);

	if (m_currentBlockName != QString())
		e->SetAttribute("blockName", m_currentBlockName.toStdString());
	if (m_currentBlockName != QString())
		e->SetAttribute("parentBlockName", m_parentBlockName.toStdString());
	if (m_angle != 0.0)
		e->SetAttribute("angle", IBK::val2string<double>(m_angle));
	if (m_xScale != 1.0)
		e->SetAttribute("xScale", IBK::val2string<double>(m_xScale));
	if (m_yScale != 1.0)
		e->SetAttribute("yScale", IBK::val2string<double>(m_yScale));
	if (m_zScale != 1.0)
		e->SetAttribute("zScale", IBK::val2string<double>(m_zScale));

	TiXmlElement::appendSingleAttributeElement(e, "insertionPoint", nullptr, std::string(), m_insertionPoint.toString());

	return e;
}


void Drawing::Insert::readXML(const TiXmlElement *element) {
	FUNCID(Drawing::DimStyle::readXMLPrivate);

	try {
		// search for mandatory attributes
		if (!TiXmlAttribute::attributeByName(element, "id")) {
			IBK::IBK_Message( IBK::FormatString(XML_READ_ERROR).arg(element->Row()).arg(
								 IBK::FormatString("Missing required 'id' attribute.") ), IBK::MSG_WARNING, FUNC_ID, IBK::VL_STANDARD);
			return;
		}

		const TiXmlAttribute * attrib = element->FirstAttribute();
		while (attrib) {
			const std::string & attribName = attrib->NameStr();
			if (attribName == "blockName")
				m_currentBlockName = QString::fromStdString(attrib->ValueStr());
			else if (attribName == "parentBlockName")
				m_parentBlockName = QString::fromStdString(attrib->ValueStr());
			else if (attribName == "angle")
				m_angle= readPODAttributeValue<double>(element, attrib);
			else if (attribName == "xScale")
				m_xScale = readPODAttributeValue<double>(element, attrib);
			else if (attribName == "yScale")
				m_yScale = readPODAttributeValue<double>(element, attrib);
			else if (attribName == "zScale")
				m_zScale = readPODAttributeValue<bool>(element, attrib);

			attrib = attrib->Next();
		}

		// reading elements
		const TiXmlElement * c = element->FirstChildElement();
		while (c) {
			const std::string & cName = c->ValueStr();
			if (cName == "Point") {
				try {
					m_insertionPoint = IBKMK::Vector2D::fromString(c->GetText());
				} catch (IBK::Exception & ex) {
					throw IBK::Exception( ex, IBK::FormatString(XML_READ_ERROR).arg(c->Row())
												 .arg("Invalid vector data."), FUNC_ID);
				}
			}
			else {
				IBK::IBK_Message(IBK::FormatString(XML_READ_UNKNOWN_ELEMENT).arg(cName).arg(c->Row()), IBK::MSG_WARNING, FUNC_ID, IBK::VL_STANDARD);
			}
			c = c->NextSiblingElement();
		}

	}
	catch (IBK::Exception & ex) {
		throw IBK::Exception( ex, IBK::FormatString("Error reading 'Drawing::Insert' element."), FUNC_ID);
	}
	catch (std::exception & ex2) {
		throw IBK::Exception( IBK::FormatString("%1\nError reading 'Drawing::Insert' element.").arg(ex2.what()), FUNC_ID);
	}
}


TiXmlElement * Drawing::writeXML(TiXmlElement * parent) const {

	if (m_id == INVALID_ID)  return nullptr;
	TiXmlElement * e = new TiXmlElement("Drawing");
	parent->LinkEndChild(e);

	if (m_id != INVALID_ID)
		e->SetAttribute("id", IBK::val2string<unsigned int>(m_id));
	if (!m_displayName.isEmpty())
		e->SetAttribute("displayName", m_displayName.toStdString());
	if (m_visible != Drawing().m_visible)
		e->SetAttribute("visible", IBK::val2string<bool>(m_visible));

	TiXmlElement::appendSingleAttributeElement(e, "Origin", nullptr, std::string(), m_origin.toString());

	m_rotationMatrix.writeXML(e);

	TiXmlElement::appendSingleAttributeElement(e, "ScalingFactor", nullptr, std::string(), IBK::val2string<double>(m_scalingFactor));

	if (!m_blocks.empty()) {
		TiXmlElement * child = new TiXmlElement("Blocks");
		e->LinkEndChild(child);

		for (std::vector<Drawing::Block>::const_iterator it = m_blocks.begin();
			 it != m_blocks.end(); ++it)
		{
			it->writeXML(child);
		}
	}


	if (!m_drawingLayers.empty()) {
		TiXmlElement * child = new TiXmlElement("DrawingLayers");
		e->LinkEndChild(child);

		for (std::vector<DrawingLayer>::const_iterator it = m_drawingLayers.begin();
			 it != m_drawingLayers.end(); ++it)
		{
			it->writeXML(child);
		}
	}


	if (!m_points.empty()) {
		TiXmlElement * child = new TiXmlElement("Points");
		e->LinkEndChild(child);

		for (std::vector<Point>::const_iterator it = m_points.begin();
			 it != m_points.end(); ++it)
		{
			it->writeXML(child);
		}
	}


	if (!m_lines.empty()) {
		TiXmlElement * child = new TiXmlElement("Lines");
		e->LinkEndChild(child);

		for (std::vector<Line>::const_iterator it = m_lines.begin();
			 it != m_lines.end(); ++it)
		{
			it->writeXML(child);
		}
	}


	if (!m_polylines.empty()) {
		TiXmlElement * child = new TiXmlElement("Polylines");
		e->LinkEndChild(child);

		for (std::vector<PolyLine>::const_iterator it = m_polylines.begin();
			 it != m_polylines.end(); ++it)
		{
			it->writeXML(child);
		}
	}


	if (!m_circles.empty()) {
		TiXmlElement * child = new TiXmlElement("Circles");
		e->LinkEndChild(child);

		for (std::vector<Circle>::const_iterator it = m_circles.begin();
			 it != m_circles.end(); ++it)
		{
			it->writeXML(child);
		}
	}


	if (!m_ellipses.empty()) {
		TiXmlElement * child = new TiXmlElement("Ellipses");
		e->LinkEndChild(child);

		for (std::vector<Ellipse>::const_iterator it = m_ellipses.begin();
			 it != m_ellipses.end(); ++it)
		{
			it->writeXML(child);
		}
	}


	if (!m_arcs.empty()) {
		TiXmlElement * child = new TiXmlElement("Arcs");
		e->LinkEndChild(child);

		for (std::vector<Arc>::const_iterator it = m_arcs.begin();
			 it != m_arcs.end(); ++it)
		{
			it->writeXML(child);
		}
	}


	if (!m_solids.empty()) {
		TiXmlElement * child = new TiXmlElement("Solids");
		e->LinkEndChild(child);

		for (std::vector<Solid>::const_iterator it = m_solids.begin();
			 it != m_solids.end(); ++it)
		{
			it->writeXML(child);
		}
	}


	if (!m_texts.empty()) {
		TiXmlElement * child = new TiXmlElement("Texts");
		e->LinkEndChild(child);

		for (std::vector<Text>::const_iterator it = m_texts.begin();
			 it != m_texts.end(); ++it)
		{
			it->writeXML(child);
		}
	}


	if (!m_linearDimensions.empty()) {
		TiXmlElement * child = new TiXmlElement("LinearDimensions");
		e->LinkEndChild(child);

		for (std::vector<LinearDimension>::const_iterator it = m_linearDimensions.begin();
			 it != m_linearDimensions.end(); ++it)
		{
			it->writeXML(child);
		}
	}


	if (!m_dimensionStyles.empty()) {
		TiXmlElement * child = new TiXmlElement("DimensionStyles");
		e->LinkEndChild(child);

		for (std::vector<DimStyle>::const_iterator it = m_dimensionStyles.begin();
			 it != m_dimensionStyles.end(); ++it)
		{
			it->writeXML(child);
		}
	}

	if (m_zCounter != INVALID_ID)
		TiXmlElement::appendSingleAttributeElement(e, "ZCounter", nullptr, std::string(), IBK::val2string<unsigned int>(m_zCounter));
	if (m_defaultColor.isValid())
		TiXmlElement::appendSingleAttributeElement(e, "DefaultColor", nullptr, std::string(), m_defaultColor.name().toStdString());

	return e;
}
