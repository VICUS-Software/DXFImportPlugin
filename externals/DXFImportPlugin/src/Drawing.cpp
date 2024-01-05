#include "Drawing.h"

#include "Utilities.h"
#include "Constants.h"

#include <IBK_MessageHandler.h>
#include <IBK_messages.h>
#include <IBK_physics.h>

#include <IBKMK_Vector2D.h>

#include <tinyxml.h>

static int PRECISION = 15;  // precision of floating point values for output writing

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

void Drawing::sortLayersAlphabetical() {
	std::sort(m_drawingLayers.begin(), m_drawingLayers.end(), [](const DrawingLayer& a, const DrawingLayer& b) {
		return a.m_displayName < b.m_displayName;
	});
}


void Drawing::updatePointer() {
	FUNCID(Drawing::updatePointer);
	m_objectPtr.clear();

	// map layer name to reference initially for improved performance
	std::map<QString, const DrawingLayer*> layerRefs;
	for (const DrawingLayer &dl: m_drawingLayers) {
		layerRefs[dl.m_displayName] = &dl;
	}

	// map block name to reference initially for improved performance
	std::map<QString, Block*> blockRefs;
	blockRefs[""] = nullptr; // This is just in case. But actually blocks without name should not exist
	for (Block &b: m_blocks) {
		blockRefs[b.m_name] = &b;
	}

	/* Note: Layer references must always be valid. Hence, when "layerRefs.at()" throws an exception, this is due to an invalid DXF.
	 * Block references are optional, therefore we use the access function which returns a nullptr if there is no block ref
	*/
	try {

		for (unsigned int i=0; i < m_points.size(); ++i){
			m_points[i].m_parentLayer = layerRefs.at(m_points[i].m_layerName);
			m_points[i].m_block = findBlockPointer(m_points[i].m_blockName, blockRefs);
			m_objectPtr[m_points[i].m_id] = &m_points[i];
		}
		for (unsigned int i=0; i < m_lines.size(); ++i){
			m_lines[i].m_parentLayer = layerRefs.at(m_lines[i].m_layerName);
			m_lines[i].m_block = findBlockPointer(m_lines[i].m_blockName, blockRefs);
			m_objectPtr[m_lines[i].m_id] = &m_lines[i];
		}
		for (unsigned int i=0; i < m_polylines.size(); ++i){
			m_polylines[i].m_parentLayer = layerRefs.at(m_polylines[i].m_layerName);
			m_polylines[i].m_block = findBlockPointer(m_polylines[i].m_blockName, blockRefs);
			m_objectPtr[m_polylines[i].m_id] = &m_polylines[i];
		}
		for (unsigned int i=0; i < m_circles.size(); ++i){
			m_circles[i].m_parentLayer = layerRefs.at(m_circles[i].m_layerName);
			m_circles[i].m_block = findBlockPointer(m_circles[i].m_blockName, blockRefs);
			m_objectPtr[m_circles[i].m_id] = &m_circles[i];
		}
		for (unsigned int i=0; i < m_arcs.size(); ++i){
			m_arcs[i].m_parentLayer = layerRefs.at(m_arcs[i].m_layerName);
			m_arcs[i].m_block = findBlockPointer(m_arcs[i].m_blockName, blockRefs);
			m_objectPtr[m_arcs[i].m_id] = &m_arcs[i];
		}
		for (unsigned int i=0; i < m_ellipses.size(); ++i){
			m_ellipses[i].m_parentLayer = layerRefs.at(m_ellipses[i].m_layerName);
			m_ellipses[i].m_block = findBlockPointer(m_ellipses[i].m_blockName, blockRefs);
			m_objectPtr[m_ellipses[i].m_id] = &m_ellipses[i];
		}
		for (unsigned int i=0; i < m_solids.size(); ++i){
			m_solids[i].m_parentLayer = layerRefs.at(m_solids[i].m_layerName);
			m_solids[i].m_block = findBlockPointer(m_solids[i].m_blockName, blockRefs);
			m_objectPtr[m_solids[i].m_id] = &m_solids[i];
		}
		for (unsigned int i=0; i < m_texts.size(); ++i){
			m_texts[i].m_parentLayer = layerRefs.at(m_texts[i].m_layerName);
			m_texts[i].m_block = findBlockPointer(m_texts[i].m_blockName, blockRefs);
			m_objectPtr[m_texts[i].m_id] = &m_texts[i];
		}

		// For inserts there must be a valid currentBlock reference!
		for (unsigned int i=0; i < m_inserts.size(); ++i){
			m_inserts[i].m_currentBlock = findBlockPointer(m_inserts[i].m_currentBlockName, blockRefs);
			Q_ASSERT(m_inserts[i].m_currentBlock);
			m_inserts[i].m_parentBlock = findBlockPointer(m_inserts[i].m_parentBlockName, blockRefs);
		}
		for (unsigned int i=0; i < m_linearDimensions.size(); ++i){
			m_linearDimensions[i].m_parentLayer = layerRefs.at(m_linearDimensions[i].m_layerName);
			m_texts[i].m_block = findBlockPointer(m_linearDimensions[i].m_blockName, blockRefs);
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
	}
	catch (std::exception &ex) {
		throw IBK::Exception(IBK::FormatString("Error during initialization of DXF file. "
											   "Might be du to invalid layer references.\n%1").arg(ex.what()), FUNC_ID);
	}
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


IBKMK::Vector3D Drawing::weightedCenter(unsigned int nextId) {

	std::set<Block *> listAllBlocksWithNoInsertPoint;
	listAllBlocksWithNoInsertPoint.insert(nullptr);
	for(Block &b : m_blocks){
		bool hasInsertBlock = false;
		for(Insert &i : m_inserts){
			if(i.m_currentBlock == &b){
				hasInsertBlock = true;
			}
		}
		if(!hasInsertBlock){
			listAllBlocksWithNoInsertPoint.insert(&b);
		}
	}

	generateInsertGeometries(nextId);

	IBKMK::Vector2D averageAccumulation(0,0);
	unsigned int cnt = 0;

	// iterate over all elements, accumulate the coordinates
	for (const Point &p : m_points) {
		if(listAllBlocksWithNoInsertPoint.find(p.m_block) != listAllBlocksWithNoInsertPoint.end()){
		averageAccumulation += p.m_point;
		qDebug() << "Point: x" << p.m_point.m_x << " y " << p.m_point.m_y;
		++cnt;
		}
	}

	for (const Line &l: m_lines) {
		if(listAllBlocksWithNoInsertPoint.find(l.m_block) != listAllBlocksWithNoInsertPoint.end()){
		if(l.m_point1.m_x < 500000) {
			qDebug() << "Line: x" << l.m_point1.m_x << " y " << l.m_point1.m_y;
		}
		IBKMK::Vector2D averageLineAccumulation = l.m_point1 + l.m_point2;
		averageLineAccumulation /= 2;
		averageAccumulation += averageLineAccumulation;
		++cnt;
		}
	}

	for (const PolyLine &pl: m_polylines) {
		if(listAllBlocksWithNoInsertPoint.find(pl.m_block) != listAllBlocksWithNoInsertPoint.end()){
		unsigned int polyLineCounter = 0;
		IBKMK::Vector2D polyLineAccumulation(0,0);
		for (const IBKMK::Vector2D &v: pl.m_polyline) {
			polyLineAccumulation += v;
			++polyLineCounter;
		}
		polyLineAccumulation /= polyLineCounter;
		averageAccumulation += polyLineAccumulation;
		++cnt;
		}
	}

	for (const Circle &c: m_circles) {
		if(listAllBlocksWithNoInsertPoint.find(c.m_block) != listAllBlocksWithNoInsertPoint.end()){
		averageAccumulation += c.m_center;
		++cnt;
		}
	}

	for (const Ellipse &e: m_ellipses) {
		if(listAllBlocksWithNoInsertPoint.find(e.m_block) != listAllBlocksWithNoInsertPoint.end()){
		averageAccumulation += e.m_center;
		++cnt;
		}
	}

	for (const Arc &a: m_arcs) {
		if(listAllBlocksWithNoInsertPoint.find(a.m_block) != listAllBlocksWithNoInsertPoint.end()){
		averageAccumulation += a.m_center;
		++cnt;
		}
	}

	for (const Solid &s: m_solids) {
		if(listAllBlocksWithNoInsertPoint.find(s.m_block) != listAllBlocksWithNoInsertPoint.end()){
		IBKMK::Vector2D averageSolidAccumulation = s.m_point1 + s.m_point2 + s.m_point3 + s.m_point4;
		averageSolidAccumulation /= 4;
		averageAccumulation += averageSolidAccumulation;
		++cnt;
		}
	}

	for (const Text &t: m_texts) {
		if(listAllBlocksWithNoInsertPoint.find(t.m_block) != listAllBlocksWithNoInsertPoint.end()){
		averageAccumulation += t.m_basePoint;
		++cnt;
		}
	}

	for (const LinearDimension &ld: m_linearDimensions) {
		if(listAllBlocksWithNoInsertPoint.find(ld.m_block) != listAllBlocksWithNoInsertPoint.end()){
		IBKMK::Vector2D averageLinearDimensionAccumulation = ld.m_dimensionPoint + ld.m_leftPoint
															 + ld.m_rightPoint + ld.m_point1 + ld.m_point2 + ld.m_textPoint;
		averageLinearDimensionAccumulation /= 6;
		averageAccumulation += averageLinearDimensionAccumulation;
		++cnt;
		}
	}

	if (cnt > 0)
		averageAccumulation /= cnt;

	return IBKMK::Vector3D(averageAccumulation.m_x, averageAccumulation.m_y, 0);
}


const Drawing::Block *Drawing::findParentBlock(const Insert &i) const {
	if (i.m_parentBlock == nullptr)
		return i.m_currentBlock;
	for (const Insert &pIn: m_inserts) {
		if (i.m_parentBlock == pIn.m_currentBlock) {
			return findParentBlock(pIn);
		}
	}
	return i.m_parentBlock;
}

const Drawing::Block *Drawing::findParentBlockByBlock(Block &block) const {
	for (const Insert &pIn: m_inserts) {
		if (&block == pIn.m_currentBlock) {
			return findParentBlock(pIn);
		}
	}
	return &block;
}


void Drawing::moveToOrigin() {

	IBKMK::Vector2D origin(m_origin.m_x, m_origin.m_y);

	std::vector<Block *> listAllBlocksWithNoInsertPoint;
	listAllBlocksWithNoInsertPoint.push_back(nullptr);
	for(Block &b : m_blocks){
		bool hasInsertBlock = false;
		for(Insert &i : m_inserts){
			if(i.m_currentBlock == &b){
				hasInsertBlock = true;
			}
		}
		if(!hasInsertBlock){
			listAllBlocksWithNoInsertPoint.push_back(&b);
		}
	}

	for (Point &p: m_points) {
		if (std::find(listAllBlocksWithNoInsertPoint.begin(),
					  listAllBlocksWithNoInsertPoint.end(), p.m_block) != listAllBlocksWithNoInsertPoint.end()) {
			p.m_point -= origin;
		}
	}

	for (Line &l: m_lines) {
		if (std::find(listAllBlocksWithNoInsertPoint.begin(),
					  listAllBlocksWithNoInsertPoint.end(), l.m_block) != listAllBlocksWithNoInsertPoint.end()) {
			l.m_point1 -= origin;
			l.m_point2 -= origin;
		}
	}

	for (PolyLine &pl: m_polylines) {
		if (std::find(listAllBlocksWithNoInsertPoint.begin(),
					  listAllBlocksWithNoInsertPoint.end(), pl.m_block) != listAllBlocksWithNoInsertPoint.end()) {
			for (IBKMK::Vector2D &v: pl.m_polyline) {
				v -= origin;
			}
		}
	}

	for (Circle &c: m_circles) {
		if (std::find(listAllBlocksWithNoInsertPoint.begin(),
					  listAllBlocksWithNoInsertPoint.end(), c.m_block) != listAllBlocksWithNoInsertPoint.end()) {
			c.m_center -= origin;
		}
	}

	for (Ellipse &e: m_ellipses) {
		if (std::find(listAllBlocksWithNoInsertPoint.begin(),
					  listAllBlocksWithNoInsertPoint.end(), e.m_block) != listAllBlocksWithNoInsertPoint.end()) {
			e.m_center -= origin;
		}
	}

	for (Arc &a: m_arcs) {
		if (std::find(listAllBlocksWithNoInsertPoint.begin(),
					  listAllBlocksWithNoInsertPoint.end(), a.m_block) != listAllBlocksWithNoInsertPoint.end()) {
			a.m_center -= origin;
		}
	}

	for (Solid &s: m_solids) {
		if (std::find(listAllBlocksWithNoInsertPoint.begin(),
					  listAllBlocksWithNoInsertPoint.end(), s.m_block) != listAllBlocksWithNoInsertPoint.end()) {
			s.m_point1 -= origin;
			s.m_point2 -= origin;
			s.m_point3 -= origin;
			s.m_point4 -= origin;
		}
	}

	for (Text &t: m_texts) {
		if (std::find(listAllBlocksWithNoInsertPoint.begin(),
					  listAllBlocksWithNoInsertPoint.end(), t.m_block) != listAllBlocksWithNoInsertPoint.end()) {
			t.m_basePoint -= origin;
		}
	}

	for (LinearDimension &ld: m_linearDimensions) {
		if (std::find(listAllBlocksWithNoInsertPoint.begin(),
					  listAllBlocksWithNoInsertPoint.end(), ld.m_block) != listAllBlocksWithNoInsertPoint.end()) {
			ld.m_dimensionPoint -= origin;
			ld.m_leftPoint -= origin;
			ld.m_rightPoint -= origin;
			ld.m_point1 -= origin;
			ld.m_point2 -= origin;
			ld.m_textPoint -= origin;
		}
	}

	for (Insert &ic: m_inserts){
		if(std::find(listAllBlocksWithNoInsertPoint.begin(),
					  listAllBlocksWithNoInsertPoint.end(), ic.m_parentBlock) != listAllBlocksWithNoInsertPoint.end()) {
			ic.m_insertionPoint -= origin;
		}
	}

	// we have shifted to 0,0 now
	m_origin = IBKMK::Vector3D(0,0,0);
}


void Drawing::compensateCoordinates() {

	for (Insert &i: m_inserts) {
		Block *currentBlock = i.m_currentBlock;
		IBKMK::Vector2D	averageAccumulation(0,0);
		unsigned int cnt = 0;

		// iterate over all elements, accumulate the coordinates
		for (Point &p: m_points) {
			if (p.m_block == currentBlock) {
				averageAccumulation += p.m_point;
				++cnt;
			}
		}

		for (Line &l: m_lines) {
			if (l.m_block == currentBlock) {
				IBKMK::Vector2D averageLineAccumulation = l.m_point1 + l.m_point2;
				averageLineAccumulation /= 2;
				averageAccumulation += averageLineAccumulation;
				++cnt;
			}
		}

		for (PolyLine &pl: m_polylines) {
			if (pl.m_block == currentBlock) {
				unsigned int polyLineCounter = 0;
				IBKMK::Vector2D polyLineAccumulation(0,0);
				for (IBKMK::Vector2D &v: pl.m_polyline) {
					polyLineAccumulation += v;
					++polyLineCounter;
				}
				polyLineAccumulation /= polyLineCounter;
				averageAccumulation += polyLineAccumulation;
				++cnt;
			}
		}

		for (Circle &c: m_circles) {
			if (c.m_block == currentBlock) {
				averageAccumulation += c.m_center;
				++cnt;
			}
		}

		for (Ellipse &e: m_ellipses) {
			if (e.m_block == currentBlock) {
				averageAccumulation += e.m_center;
				++cnt;
			}
		}

		for (Arc &a: m_arcs) {
			if (a.m_block == currentBlock) {
				averageAccumulation += a.m_center;
				++cnt;
			}
		}

		for (Solid &s: m_solids) {
			if (s.m_block == currentBlock) {
				IBKMK::Vector2D averageSolidAccumulation = s.m_point1 + s.m_point2 + s.m_point3 + s.m_point4;
				averageSolidAccumulation /= 4;
				averageAccumulation += averageSolidAccumulation;
				++cnt;
			}
		}

		for (Text &t: m_texts) {
			if (t.m_block == currentBlock) {
				averageAccumulation += t.m_basePoint;
				++cnt;
			}
		}

		for (LinearDimension &ld: m_linearDimensions) {
			if (ld.m_block == currentBlock) {
				IBKMK::Vector2D averageLinearDimensionAccumulation = ld.m_dimensionPoint + ld.m_leftPoint
								+ ld.m_rightPoint + ld.m_point1 + ld.m_point2 + ld.m_textPoint;
				averageLinearDimensionAccumulation /= 6;
				averageAccumulation += averageLinearDimensionAccumulation;
				++cnt;
			}
		}

		// calculate the average of all elements attached to current Insert Point
		if (cnt > 0)
			averageAccumulation /= cnt;
		IBKMK::Vector2D newInsertPoint = i.m_insertionPoint + averageAccumulation;


		// update all coordinates of elements attached to insert point. Also update insert points of all blocks attached to current block
		// newElementPoint = oldElementPoint + oldInsertPoint - newInsertPoint

		// calculates delta of oldInsertPoint and newInsertPoint for simplified calculation
		IBKMK::Vector2D deltaInsertPoint = i.m_insertionPoint - newInsertPoint;


		for (Point &p: m_points) {
			if (p.m_block == currentBlock) {
				p.m_point += deltaInsertPoint;
			}
		}

		for (Line &l: m_lines) {
			if (l.m_block == currentBlock) {
				l.m_point1 += deltaInsertPoint;
				l.m_point2 += deltaInsertPoint;
			}
		}

		for (PolyLine &pl: m_polylines) {
			if (pl.m_block == currentBlock) {
				for (IBKMK::Vector2D &v: pl.m_polyline) {
					v += deltaInsertPoint;
				}
			}
		}

		for (Circle &c: m_circles) {
			if (c.m_block == currentBlock) {
				c.m_center += deltaInsertPoint;
			}
		}

		for (Ellipse &e: m_ellipses) {
			if (e.m_block == currentBlock) {
				e.m_center += deltaInsertPoint;
			}
		}

		for (Arc &a: m_arcs) {
			if (a.m_block == currentBlock) {
				a.m_center += deltaInsertPoint;
			}
		}

		for (Solid &s: m_solids) {
			if (s.m_block == currentBlock) {
				s.m_point1 += deltaInsertPoint;
				s.m_point2 += deltaInsertPoint;
				s.m_point3 += deltaInsertPoint;
				s.m_point4 += deltaInsertPoint;
			}
		}

		for (Text &t: m_texts) {
			if (t.m_block == currentBlock) {
				t.m_basePoint += deltaInsertPoint;
			}
		}

		for (LinearDimension &ld: m_linearDimensions) {
			if (ld.m_block == currentBlock) {
				ld.m_dimensionPoint += deltaInsertPoint;
				ld.m_leftPoint += deltaInsertPoint;
				ld.m_rightPoint += deltaInsertPoint;
				ld.m_point1 += deltaInsertPoint;
				ld.m_point2 += deltaInsertPoint;
				ld.m_textPoint += deltaInsertPoint;
			}
		}

		for (Insert &ic: m_inserts){
			if(ic.m_parentBlock == i.m_currentBlock) {
				ic.m_insertionPoint += deltaInsertPoint;
			}
		}


		// now update this insertPoint
		i.m_insertionPoint = newInsertPoint;
	}
}


Drawing::Block *Drawing::findBlockPointer(const QString &name, const std::map<QString, Block*> &blockRefs){
	const auto it = blockRefs.find(name);
	if (it == blockRefs.end())
		return nullptr;
	else
		return it->second;
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
		newObj.m_isInsertObject = true;

		newObjects.push_back(newObj);
	}

	objects.insert(objects.end(), newObjects.begin(), newObjects.end());
}


void Drawing::transformInsert(QMatrix4x4 trans, const Insert & insert, unsigned int & nextId) {

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


TiXmlElement * Drawing::Text::writeXMLPrivate(TiXmlElement * parent) const {
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
	if (m_rotationAngle != 0.0)
		e->SetAttribute("rotationAngle", IBK::val2string<double>(m_rotationAngle));
	if (m_height != 10.0)
		e->SetAttribute("height", IBK::val2string<double>(m_height));
	if (!m_blockName.isEmpty())
		e->SetAttribute("blockName", m_blockName.toStdString());

	TiXmlElement::appendSingleAttributeElement(e, "BasePoint", nullptr, std::string(), m_basePoint.toString(PRECISION));

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


TiXmlElement * Drawing::Solid::writeXMLPrivate(TiXmlElement * parent) const {
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

	TiXmlElement::appendSingleAttributeElement(e, "Point1", nullptr, std::string(), m_point1.toString(PRECISION));
	TiXmlElement::appendSingleAttributeElement(e, "Point2", nullptr, std::string(), m_point2.toString(PRECISION));
	TiXmlElement::appendSingleAttributeElement(e, "Point3", nullptr, std::string(), m_point3.toString(PRECISION));
	TiXmlElement::appendSingleAttributeElement(e, "Point4", nullptr, std::string(), m_point4.toString(PRECISION));

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


TiXmlElement * Drawing::LinearDimension::writeXMLPrivate(TiXmlElement * parent) const {
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
	if (m_measurement != "")
		e->SetAttribute("measurement", m_measurement.toStdString());
	if (!m_layerName.isEmpty())
		e->SetAttribute("layer", m_layerName.toStdString());
	if (!m_styleName.isEmpty())
		e->SetAttribute("styleName", m_styleName.toStdString());

	TiXmlElement::appendSingleAttributeElement(e, "Point1", nullptr, std::string(), m_point1.toString(PRECISION));
	TiXmlElement::appendSingleAttributeElement(e, "Point2", nullptr, std::string(), m_point2.toString(PRECISION));
	TiXmlElement::appendSingleAttributeElement(e, "DimensionPoint", nullptr, std::string(), m_dimensionPoint.toString(PRECISION));
	TiXmlElement::appendSingleAttributeElement(e, "LeftPoint", nullptr, std::string(), m_leftPoint.toString(PRECISION));
	TiXmlElement::appendSingleAttributeElement(e, "RightPoint", nullptr, std::string(), m_rightPoint.toString(PRECISION));
	TiXmlElement::appendSingleAttributeElement(e, "TextPoint", nullptr, std::string(), m_textPoint.toString(PRECISION));

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


TiXmlElement * Drawing::Point::writeXMLPrivate(TiXmlElement * parent) const {
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

	TiXmlElement::appendSingleAttributeElement(e, "Point", nullptr, std::string(), m_point.toString(PRECISION));

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


TiXmlElement * Drawing::Line::writeXMLPrivate(TiXmlElement * parent) const {
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

	TiXmlElement::appendSingleAttributeElement(e, "Point1", nullptr, std::string(), m_point1.toString(PRECISION));
	TiXmlElement::appendSingleAttributeElement(e, "Point2", nullptr, std::string(), m_point2.toString(PRECISION));

	return e;
}


TiXmlElement * Drawing::Circle::writeXMLPrivate(TiXmlElement * parent) const {
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

	TiXmlElement::appendSingleAttributeElement(e, "Center", nullptr, std::string(), m_center.toString(PRECISION));
	TiXmlElement::appendSingleAttributeElement(e, "Radius", nullptr, std::string(), IBK::val2string<double>(m_radius, PRECISION));

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


TiXmlElement * Drawing::PolyLine::writeXMLPrivate(TiXmlElement * parent) const {
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
		const std::vector<IBKMK::Vector2D> & polyVertexes = m_polyline;
		for (unsigned int i=0; i<polyVertexes.size(); ++i) {
			vals << polyVertexes[i].toString(PRECISION);
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


TiXmlElement * Drawing::Arc::writeXMLPrivate(TiXmlElement * parent) const {
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

	TiXmlElement::appendSingleAttributeElement(e, "Center", nullptr, std::string(), m_center.toString(PRECISION));
	TiXmlElement::appendSingleAttributeElement(e, "Radius", nullptr, std::string(), IBK::val2string<double>(m_radius, PRECISION));
	TiXmlElement::appendSingleAttributeElement(e, "StartAngle", nullptr, std::string(), IBK::val2string<double>(m_startAngle, PRECISION));
	TiXmlElement::appendSingleAttributeElement(e, "EndAngle", nullptr, std::string(), IBK::val2string<double>(m_endAngle, PRECISION));

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


TiXmlElement * Drawing::Ellipse::writeXMLPrivate(TiXmlElement * parent) const {
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

	TiXmlElement::appendSingleAttributeElement(e, "Center", nullptr, std::string(), m_center.toString(PRECISION));
	TiXmlElement::appendSingleAttributeElement(e, "MajorAxis", nullptr, std::string(), m_majorAxis.toString(PRECISION));
	TiXmlElement::appendSingleAttributeElement(e, "Ratio", nullptr, std::string(), IBK::val2string<double>(m_ratio, PRECISION));
	TiXmlElement::appendSingleAttributeElement(e, "StartAngle", nullptr, std::string(), IBK::val2string<double>(m_startAngle, PRECISION));
	TiXmlElement::appendSingleAttributeElement(e, "EndAngle", nullptr, std::string(), IBK::val2string<double>(m_endAngle, PRECISION));

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
	if (!m_name.isEmpty())
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
	if (m_globalScalingFactor != 1.0)
		e->SetAttribute("globalScalingFactor", IBK::val2string<double>(m_globalScalingFactor));
	if (m_globalScalingFactor != 1.0)
		e->SetAttribute("textScalingFactor", IBK::val2string<double>(m_textScalingFactor));
	if (m_textLinearFactor != 1.0)
		e->SetAttribute("textLinearFactor", IBK::val2string<double>(m_textLinearFactor));
	if (m_textDecimalPlaces != 1)
		e->SetAttribute("textDecimalPlaces", IBK::val2string<int>(m_textDecimalPlaces));

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
			else if (attribName == "globalScalingFactor")
				m_textHeight = readPODAttributeValue<double>(element, attrib);
			else if (attribName == "textScalingFactor")
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

	TiXmlElement::appendSingleAttributeElement(e, "basePoint", nullptr, std::string(), m_basePoint.toString(PRECISION));

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

	if (!m_currentBlockName.isEmpty())
		e->SetAttribute("blockName", m_currentBlockName.toStdString());
	if (!m_parentBlockName.isEmpty())
		e->SetAttribute("parentBlockName", m_parentBlockName.toStdString());
	if (m_angle != 0.0)
		e->SetAttribute("angle", IBK::val2string<double>(m_angle));
	if (m_xScale != 1.0)
		e->SetAttribute("xScale", IBK::val2string<double>(m_xScale));
	if (m_yScale != 1.0)
		e->SetAttribute("yScale", IBK::val2string<double>(m_yScale));
	if (m_zScale != 1.0)
		e->SetAttribute("zScale", IBK::val2string<double>(m_zScale));

	TiXmlElement::appendSingleAttributeElement(e, "insertionPoint", nullptr, std::string(), m_insertionPoint.toString(PRECISION));

	return e;
}


void Drawing::Insert::readXML(const TiXmlElement *element) {
	FUNCID(Drawing::DimStyle::readXMLPrivate);

	try {

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

	TiXmlElement::appendSingleAttributeElement(e, "Origin", nullptr, std::string(), m_origin.toString(PRECISION));

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


	if (!m_inserts.empty()) {
		TiXmlElement * child = new TiXmlElement("Inserts");
		e->LinkEndChild(child);

		for (std::vector<Insert>::const_iterator it = m_inserts.begin();
			 it != m_inserts.end(); ++it)
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
