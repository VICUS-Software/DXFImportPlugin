#ifndef VICUS_DRAWING_H
#define VICUS_DRAWING_H

#include "tinyxml.h"

#include <IBKMK_Vector2D.h>
#include <IBKMK_Vector3D.h>
#include <IBK_Line.h>

#include "RotationMatrix.h"
#include "Object.h"
#include "DrawingLayer.h"

#include <QQuaternion>
#include <QMatrix4x4>
#include <QColor>
#include <QDebug>

#include <libdxfrw.h>

#include <drw_interface.h>
#include <drw_objects.h>
#include <drw_base.h>

// #include <dl_creationinterface.h>
#include <glm.hpp>
#include <gtc/quaternion.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <gtx/quaternion.hpp>

class PlaneGeometry;

/*!
	Drawing class is data structure for all primitive drawing objects
	such as Layer, Lines, Circles, ... .
 */
class Drawing : public Object {
public:

	/*! Struct for Line Segemts that are drawn by drawing shader.
		Simple Line with 2 3D-points.
	*/
	struct LineSegment {
		LineSegment() {}
		LineSegment(const IBKMK::Vector3D &p1, const IBKMK::Vector3D &p2):
			m_p1(p1),
			m_p2(p2)
		{}

		IBKMK::Vector3D m_p1;
		IBKMK::Vector3D m_p2;
	};

	/*! Drawing geometry is devided into 10x10m fields for speeding up search of snapping points. */
	struct Field {

		Field() {}
		Field(const Drawing &drawing, const IBKMK::Vector3D &p):
			m_x((int)(p.m_x/(drawing.m_fieldSize))),
			m_y((int)(p.m_y/(drawing.m_fieldSize))),
			m_z((int)(p.m_z/(drawing.m_fieldSize)))
		{}

		bool operator==(const Field &other) const {
			return m_x == other.m_x && m_y == other.m_y && m_z == other.m_z;
		}

		bool operator<(const Field &other) const {
			if (m_x != other.m_x) return m_x < other.m_x;
			if (m_y != other.m_y) return m_y < other.m_y;
			return m_z < other.m_z;
		}

		int		m_x;	///< all points between x*10 < p <= (x+1)*10
		int		m_y;	///< all points between y*10 < p <= (y+1)*10
		int		m_z;	///< all points between z*10 < p <= (z+1)*10
	};

	/*! Type-info string. */
	const char * typeinfo() const override {
		return "Drawing";
	}

	// *** PUBLIC MEMBER FUNCTIONS ***

	Drawing();

	/*! Updated parents. */
	void updateParents() {
		m_children.clear();
		for (DrawingLayer & dl : m_drawingLayers) {
			m_children.push_back(&dl);
			dl.m_parent = dynamic_cast<Object *>(this);
		}
		updatePointer();
	}

	/*! Insert structure for Blocks */
	struct Block {

		TiXmlElement * writeXML(TiXmlElement * element) const;
		void readXML(const TiXmlElement * element);

		/*! ID of Block. */
		unsigned int	m_id;
		/*! Name of Block. */
		QString			m_name;
		/*! Block color. */
		QColor			m_color = QColor();
		/*! Line weight. */
		int				m_lineWeight;
		/*! Base point */
		IBKMK::Vector2D	m_basePoint;
	};


	/*! Insert structure for blocks. */
	struct Insert {

		TiXmlElement * writeXML(TiXmlElement * element) const;
		void readXML(const TiXmlElement * element);

		QString					m_currentBlockName;		// name of block
		QString					m_parentBlockName;		// name of block

		/* Block Entities that belongs to name. */
		Block					*m_currentBlock = nullptr;
		Block					*m_parentBlock = nullptr;

		double					m_xScale = 1.0;			// scale-factor in x direction
		double					m_yScale = 1.0;			// scale-factor in y direction
		double					m_zScale = 1.0;			// scale-factor in z direction

		/*! Rotation angle in rad. */
		double					m_angle = 0.0;			// angle of rotation

		IBKMK::Vector2D			m_insertionPoint;		// insertion point
	};


	/* Abstract class for all directly drawable dxf entities */
	struct AbstractDrawingObject {
		/*! Standard C'tor. */
		AbstractDrawingObject() = default;

		/*! D'tor. */
		virtual ~AbstractDrawingObject() {}

		inline void readXML(const TiXmlElement * element){
			const TiXmlAttribute *attrib = TiXmlAttribute::attributeByName(element, "blockName");
			if (attrib != nullptr) {
				m_blockName = QString::fromStdString(attrib->ValueStr());
			}
			TiXmlElement e(*element);
			e.RemoveAttribute("blockName");
			readXMLPrivate(&e);
		}

		/*! To be implemented by inheriting classes */
		virtual void readXMLPrivate(const TiXmlElement * element) = 0;

		/*! Abstract writeXML function: The actual object shall only be written
		 *  if this is not an inserted object. The blockName is written here, all other properties
		 *  are written by the inheriting classes.
		 */
		inline TiXmlElement * writeXML(TiXmlElement * parent) const {
			if (m_isInsertObject)
				return nullptr; // we don't write inserted objects, these are handled by inserts
			TiXmlElement *e = writeXMLPrivate(parent);
			if (e == nullptr)
				return nullptr;
			if (!m_blockName.isEmpty())
				e->SetAttribute("blockName", m_blockName.toStdString());
			return e;
		}

		/*! To be implemented by inheriting classes */
		virtual TiXmlElement * writeXMLPrivate(TiXmlElement * parent) const = 0;

		/*! Function to update points, needed for easier
			handling of objects in scene3D to construct 3D
			Objects and for picking operations.

			Point will be recalculated when m_dirty is true;
		*/
		virtual const std::vector<IBKMK::Vector2D>& points2D() const = 0 ;
		virtual const std::vector<LineSegment>& lineGeometries() const = 0;

		/* used to get correct color of entity */
		const QColor &color() const;
		/* used to get correct lineWeight of entity */
		double lineWeight() const;
		/*! Indicates when triangulation has to be redone. */
		void updatePlaneGeometry() {
			m_dirtyGlobalPoints = true;
		}

		/*! Get parent drawing where object is included. */
		const Drawing *drawing() const {
			Q_ASSERT(m_layerRef != nullptr);
			const Drawing *drawing = dynamic_cast<const Drawing *>(m_layerRef->m_parent);
			Q_ASSERT(drawing != nullptr);
			// Return pointer to drawing
			return drawing;
		}

		/*! Returns the current transformation matrix. */
		const glm::dmat4& transformationMatrix() const;

		/*! Parent drawing. */
		Drawing										*m_parent = nullptr;

		/*! Name of layer */
		QString										m_layerName;
		/*! Pointer to layer */
		const DrawingLayer							*m_layerRef = nullptr;
		/*! Color of Entity if defined, use getter color() instead */
		QColor										m_color = QColor();
		/*! Line weight of Entity, use getter lineWeight() instead */
		double										m_lineWeight = 0;
		/*! Integer to create a drawing hierarchy in a dxf file to avoid overlapping of entities */
		unsigned int								m_zPosition;
		/*! Name of block. */
		QString										m_blockName;
		/*! Block Entity belongs to, if nullptr, no block is used */
		const Block									*m_block = nullptr;
		/*! ID of object. */
		unsigned int								m_id;
		/*! Transformation matrix. */
		QMatrix4x4									m_trans = QMatrix4x4();
		/*! Defines wether this is a run-time only generated object defined
			by a block and according insert. If true the object shall not be written.
		*/
		bool										m_isInsertObject = false;

	protected:
		/*! Flag to indictate recalculation of points. */
		mutable bool								m_dirtyLocalPoints = true;
		/*! Flag to indictate recalculation of global 3D Points. */
		mutable bool								m_dirtyGlobalPoints = true;
		/*! Points of objects. */
		mutable std::vector<IBKMK::Vector2D>		m_pickPoints;
		/*! Plane Geometries with all triangulated data.
		*/
		mutable std::vector<LineSegment>			m_lineGeometries;

		/*! Transformation matrix, needed for calculating 3D-points. */
		mutable glm::dmat4							m_transformationMatrix;
	};


	/*! Struct that holds all data for Dimension styling.
		For now all the basic implementation is just done.
	*/
	struct DimStyle {
		/*!
			https://ezdxf.readthedocs.io/en/stable/tutorials/linear_dimension.html
		*/

		TiXmlElement * writeXML(TiXmlElement * element) const;
		void readXML(const TiXmlElement * element);

		/*! Name of Dim style. */
		QString				m_name;

		/*! ID of object. */
		unsigned int		m_id;

		/*! Fixed length of extension line. */
		bool				m_fixedExtensionLength = false;

		/*! Global scaling factor. */
		double				m_globalScalingFactor = 1.0;

		/*! Text scaling factor. */
		double				m_textScalingFactor = 1.0;

		/*! Extension line length. */
		double				m_extensionLineLength = 0.0;

		/*! Point 2 of Line. */
		double				m_textHeight = 0.0;

		/*! Text scaling linear factor. Is applied before measurement is converted to text. */
		double				m_textLinearFactor = 1.0;

		/*! Text scaling linear factor. Is applied before measurement is converted to text. */
		int					m_textDecimalPlaces = 1;

		/*! Distance between measure line and uper point of
			extension line */
		double				m_upperLineDistance = 0.0;

		/*!	Distance between extension line lower point and object. */
		double				m_extensionLineLowerDistance = 0.0;
	};


	/*! Stores attributes of line */
	struct Point : public AbstractDrawingObject {

		TiXmlElement * writeXMLPrivate(TiXmlElement * element) const override;
		void readXMLPrivate(const TiXmlElement * element) override;

		/*! Calculate points. */
		const std::vector<IBKMK::Vector2D>& points2D() const override;
		/*! Calculate Plane geometries if m_dirtyTriangulation is true and/or returns them.
			Drawing is only needed when m_dirtyTriangulation is true
		*/
		const std::vector<LineSegment>& lineGeometries() const override;

		/*! Point coordinate */
		IBKMK::Vector2D					m_point;

	};


	/*! Stores attributes of line */
	struct Line : public AbstractDrawingObject {

		TiXmlElement * writeXMLPrivate(TiXmlElement * element) const override;
		void readXMLPrivate(const TiXmlElement * element) override;

		/*! Calculate points. */
		const std::vector<IBKMK::Vector2D>& points2D() const override;
		/*! Calculate Plane geometries if m_dirtyTriangulation is true and/or returns them.
			Drawing is only needed when m_dirtyTriangulation is true
		*/
		const std::vector<LineSegment>& lineGeometries() const override;

		/*! Point coordinate */
		IBKMK::Vector2D					m_point1;
		/*! Point coordinate */
		IBKMK::Vector2D					m_point2;
	};


	/*! Stores both LW and normal polyline */
	struct PolyLine : public AbstractDrawingObject {

		TiXmlElement * writeXMLPrivate(TiXmlElement * element) const override;
		void readXMLPrivate(const TiXmlElement * element) override;

		/*! Calculate points. */
		const std::vector<IBKMK::Vector2D>& points2D() const override;
		/*! Calculate Plane geometries if m_dirtyTriangulation is true and/or returns them.
			Drawing is only needed when m_dirtyTriangulation is true
		*/
		const std::vector<LineSegment>& lineGeometries() const override;

		/*! polyline coordinates */
		std::vector<IBKMK::Vector2D>    m_polyline;
		/*! When end point is connected to start */
		bool							m_endConnected = false;
	};


	/* Stores attributes of circle */
	struct Circle : public AbstractDrawingObject {

		TiXmlElement * writeXMLPrivate(TiXmlElement * element) const override;
		void readXMLPrivate(const TiXmlElement * element) override;
		/*! Calculate points. */
		const std::vector<IBKMK::Vector2D> &points2D() const override;
		/*! Calculate Plane geometries if m_dirtyTriangulation is true and/or returns them.
			Drawing is only needed when m_dirtyTriangulation is true
		*/
		const std::vector<LineSegment>& lineGeometries() const override;

		/*! Circle center */
		IBKMK::Vector2D					m_center;
		/*! Circle radius */
		double							m_radius;
	};


	/* Stores attributes of ellipse */
	struct Ellipse : public AbstractDrawingObject {

		TiXmlElement * writeXMLPrivate(TiXmlElement * element) const override;
		void readXMLPrivate(const TiXmlElement * element) override;

		const std::vector<IBKMK::Vector2D> &points2D() const override;
		/*! Calculate Plane geometries if m_dirtyTriangulation is true and/or returns them.
			Drawing is only needed when m_dirtyTriangulation is true
		*/
		const std::vector<LineSegment>& lineGeometries() const override;

		/*! Ellipse center */
		IBKMK::Vector2D			m_center;
		/*! Ellipse major axis */
		IBKMK::Vector2D			m_majorAxis;
		/*! Ellipse minor axis as ratio of majorAxis*/
		double					m_ratio;
		/*! Ellipse start angle */
		double					m_startAngle;
		/*! Ellipse end angle */
		double					m_endAngle;
	};


	/* Stores attributes of arc */
	struct Arc : public AbstractDrawingObject {

		TiXmlElement * writeXMLPrivate(TiXmlElement * element) const override;
		void readXMLPrivate(const TiXmlElement * element) override;

		const std::vector<IBKMK::Vector2D> &points2D() const override;
		/*! Calculate Plane geometries if m_dirtyTriangulation is true and/or returns them.
			Drawing is only needed when m_dirtyTriangulation is true
		*/
		const std::vector<LineSegment>& lineGeometries() const override;

		/*! Arc center */
		IBKMK::Vector2D			m_center;
		/*! Arc radius */
		double					m_radius;
		/*! Arc start angle in rad. */
		double					m_startAngle;
		/*! Arc end angle in rad. */
		double					m_endAngle;
	};


	/* Stores attributes of solid, dummy struct */
	struct Solid : public AbstractDrawingObject {

		TiXmlElement * writeXMLPrivate(TiXmlElement * element) const override;
		void readXMLPrivate(const TiXmlElement * element) override;

		const std::vector<IBKMK::Vector2D> &points2D() const override;
		/*! Calculate Plane geometries if m_dirtyTriangulation is true and/or returns them.
			Drawing is only needed when m_dirtyTriangulation is true
		*/
		const std::vector<LineSegment>& lineGeometries() const override;

		/*! Point 1 */
		IBKMK::Vector2D			m_point1;
		/*! Point 2 */
		IBKMK::Vector2D			m_point2;
		/*! Point 3 */
		IBKMK::Vector2D			m_point3;
		/*! Point 4 */
		IBKMK::Vector2D			m_point4;
	};


	/* Stores attributes of text, dummy struct */
	struct Text : public AbstractDrawingObject {

		TiXmlElement * writeXMLPrivate(TiXmlElement * element) const override;
		void readXMLPrivate(const TiXmlElement * element) override;

		const std::vector<IBKMK::Vector2D> &points2D() const override;
		/*! Calculate Plane geometries if m_dirtyTriangulation is true and/or returns them.
			Drawing is only needed when m_dirtyTriangulation is true
		*/
		const std::vector<LineSegment>& lineGeometries() const override;

		/*! Base point. */
		IBKMK::Vector2D		m_basePoint;
		/*! Height. */
		double				m_rotationAngle = 0;
		/*! Height. */
		double				m_height = 10;
		/*! Alignment. */
		Qt::Alignment		m_alignment;
		/*! Text */
		QString				m_text;
	};


	/* Stores attributes of text, dummy struct */
	struct LinearDimension : public AbstractDrawingObject {

		TiXmlElement * writeXMLPrivate(TiXmlElement * element) const override;
		void readXMLPrivate(const TiXmlElement * element) override;

		const std::vector<IBKMK::Vector2D> &points2D() const override;
		/*! Calculate Plane geometries if m_dirtyTriangulation is true and/or returns them.
			Drawing is only needed when m_dirtyTriangulation is true
		*/
		const std::vector<LineSegment>& lineGeometries() const override;

		/*! Base point. */
		IBKMK::Vector2D				m_dimensionPoint;

		/*! Base point. */
		IBKMK::Vector2D				m_leftPoint;

		/*! Base point. */
		IBKMK::Vector2D				m_rightPoint;

		/*! Point 1 of Line. */
		IBKMK::Vector2D				m_point1;

		/*! Point 2 of Line. */
		IBKMK::Vector2D				m_point2;

		/*! Point 2 of Line. */
		IBKMK::Vector2D				m_textPoint;

		/*! Angle of rotation. */
		double						m_angle = 0.0;

		/*! Measurement text. */
		QString						m_measurement = "";

		/*! Name of Dim style. */
		QString						m_styleName;

		/*! Pointer to style object. Updated in
			updatePointers();
		*/
		DimStyle					*m_style = nullptr;
	};


	// *** PUBLIC MEMBER FUNCTIONS ***

	void readXML(const TiXmlElement * element);
	TiXmlElement * writeXML(TiXmlElement * parent) const;

	/*! Returns the drawing object based on the ID. */
	const AbstractDrawingObject* objectByID(unsigned int id) const;

	/*! Helper function to assign the correct block to an entity
		\returns Pointer of object, when it was found or nullptr if no object has been found
	*/
	Block* findBlockPointer(const QString &name, const std::map<QString, Block*> &blockRefs);

	/*! used to assign the correct layer to an entity */
	void updatePointer();

	/*! Updates all planes, when transformation operations are applied.
		MIND: Always call this function, when the drawing transformation
		(translation, rotation) were changed, since the triangulation is
		redone.
	*/
	void updatePlaneGeometries();

	/*! Generates all inserting geometries. */
	void generateInsertGeometries(unsigned int nextId);

	/*! All drawing geometries are going to be updated. */
	void updateAllGeometries();

	/*! Sorts layers alphabetically. */
	void sortLayersAlphabetical();

	/*! Calculates the center coordinates, defined as median value of all coordinates.
	 *  Median is beneficial here as it is less affected by a little number of extrem outliers.
		Pointers must be updated before calling this function!
	*/
	IBKMK::Vector3D weightedCenterMedian(unsigned int nextId);

	/*! Returns 3D Pick points of drawing. */
	const std::map<Drawing::Field, std::map<unsigned int, std::vector<IBKMK::Vector3D>>> &pickPoints() const;

	/*! Takes the transformation matrix and computes the global 3D-point. */
	const IBKMK::Vector3D point3D(const IBKMK::Vector2D &vert, const AbstractDrawingObject &object) const;

	/*! Generates 3D Points from 2D points by applying transformation from drawing. */
	const std::vector<IBKMK::Vector3D> points3D(const std::vector<IBKMK::Vector2D> &verts, const AbstractDrawingObject &object) const;

	/*! Returns the normal vector of the drawing. */
	const IBKMK::Vector3D normal() const;

	/*! Returns the normal vector of the drawing. */
	const IBKMK::Vector3D localX() const;

	/*! Returns the normal vector of the drawing. */
	const IBKMK::Vector3D localY() const;

	/*! Adds also all intersection points of lines to pickpoints. */
	void addInstersectionPoints() const;

	/*! Template function that removes objects if their layer name is one of the given layerNames. */
	template <typename t>
	void eraseObjectsByLayer(const std::set<QString> &layerNames, std::vector<t> &objects){
		for (unsigned int idx=objects.size(); idx >0; --idx) {
			if (layerNames.find(objects[idx - 1].m_layerName) != layerNames.end())
				objects.erase( objects.begin() + idx-1 );
		}
	}

	/*! Template that is invoked, when all plane geometries need to be updated and retriangulated.
		All 3D points and pick points are recalculated and all triangulated planes are regenerated.
		\param objects Vector with drawing elements
	*/
	template <typename T>
	void updateGeometryForAll(std::vector<T>& objects) {
		for (T& obj : objects) {
			obj.updatePlaneGeometry();
		}
	}

	/*! Adds pick points to the current drawing object. The drawing object contains a map, where key is the id of the object
		where the pick points are part of - for faster access. The value is a vector containing all 3D points, where it is possible
		to pick to,
		\param objects vector with all drawing object, where pick points should be generated and added to verts
	*/
	template <typename t>
	void addPickPoints(const std::vector<t> &objects, bool pickLines = false) const {
		for (const t& obj : objects) {
			// Skip objects, that are part of a block,
			// they have already been generated
			if (obj.m_block != nullptr)	continue;

			// qDebug() << "Obje: " << obj.m_id;

			// Get 3D-points
			std::vector<IBKMK::Vector3D> points(points3D(obj.points2D(), obj));
#define PICK_DRAWING_LINES

#ifdef PICK_DRAWING_LINES
			if (pickLines) {
				// Add pick-points
				std::set<Field> fields;
				for (unsigned int i=0; i<points.size(); ++i) {

					const IBKMK::Vector3D &v1 = points[ i				  ];
					const IBKMK::Vector3D &v2 = points[(i+1)%points.size()];

					IBKMK::Vector3D dir = v2 - v1;
					double length = dir.magnitude();

					unsigned int steps = (int)(length/10.0) + 1;

					for (unsigned int i=0; i < steps; ++i) {
						IBKMK::Vector3D v3D = v1 + (double)i * 10.0 * dir.normalized();
						fields.insert(Field(*this, v3D));
					}
				}

				for (const IBKMK::Vector3D &v3D : points)
					for (const Field &field : fields)
						m_pickPoints[field][obj.m_id].push_back(v3D);

			}
			else {
				// Add pick-points
				for (const IBKMK::Vector3D &v3D : points)
					m_pickPoints[Field(*this, v3D)][obj.m_id].push_back(v3D);
			}
#else
			// Add pick-points
			for (const IBKMK::Vector3D &v3D : points)
				m_pickPoints[Field(v3D)][obj.m_id].push_back(v3D);
#endif
		}
	}

	// *** PUBLIC MEMBER VARIABLES ***

	/*! point of origin */
	IBKMK::Vector3D															m_offset			= IBKMK::Vector3D(0,0,0);
	/*! rotation matrix */
	RotationMatrix															m_rotationMatrix	= RotationMatrix(QQuaternion(1.0,0.0,0.0,0.0));
	/*! scale factor */
	double																	m_scalingFactor		= 1.0;

	/*! list of blocks, dummy implementation */
	std::vector<Block>														m_blocks;
	/*! list of layers */
	std::vector<DrawingLayer>												m_drawingLayers;
	/*! list of points */
	std::vector<Point>														m_points;
	/*! list of lines */
	std::vector<Line>														m_lines;
	/*! list of polylines */
	std::vector<PolyLine>													m_polylines;
	/*! list of circles */
	std::vector<Circle>														m_circles;
	/*! list of ellipses */
	std::vector<Ellipse>													m_ellipses;
	/*! list of arcs */
	std::vector<Arc>														m_arcs;
	/*! list of solids, dummy implementation */
	std::vector<Solid>														m_solids;
	/*! list of texts */
	std::vector<Text>														m_texts;
	/*! list of texts */
	std::vector<LinearDimension>											m_linearDimensions;
	/*! list of Dim Styles */
	std::vector<DimStyle>													m_dimensionStyles;
	/*! list of inserts. */
	std::vector<Insert>														m_inserts;

	/*! Factor to be multiplied with line weight of objects. */
	double																	m_lineWeightScaling = 1;
	/*! Factor to be multiplied with text height. */
	double																	m_textScaling = 1;

	/*! Offset added to line weight of objects. */
	double																	m_lineWeightOffset = 0.001;

	/*! Counter of entities, used to create a drawing hierarchy
		in a dxf file to avoid overlapping of entities */
	unsigned int															m_zCounter = 0;

	/*! Is the default color when no other color was specified */
	QColor																	m_defaultColor = QColor();

	/*! Current Bounding box size. Is updated in Project. */
	mutable IBKMK::Vector3D													m_boundingBox;

	/*! Field size for faster snapping access. Is updated in Project. */
	mutable double															m_fieldSize = 10.0;

	bool dirtyPickPoints() const;

private:
	/*! Helper function to assign the correct layer to an entity */
	const DrawingLayer *layerPointer(const QString &layername);

	/*! Helper function to assign the correct block to an entity */
	const Block *blockPointer(const QString &name);

	/*! Transforms all inserts.
		Mind: Parameter 'trans' is passed by value here on purpose. This allows using the function recursively.
	 */
	void transformInsert(QMatrix4x4 trans, const Drawing::Insert &insert, unsigned int &nextId);

	/*! Function to generate plane geometries from text. Heavy operation. Text is polygonised by QPainterPath with font-size 1
		in order to get a rough letter and less polygons. Some dxfs contain a lot of text and so we would end in having too many
		polygons.
	*/
	void generateLinesFromText(const std::string &text, double textHeight, Qt::Alignment alignment, const double &rotationAngle,
							   const IBKMK::Vector2D &basePoint, const AbstractDrawingObject &object, std::vector<LineSegment> &lineGeometries) const;

	/*! Find layer reference. */
	const DrawingLayer *findLayerReference(const std::map<QString, const DrawingLayer*> &layerRefs, const QString &layerName);

	/*! Cached unique-ID -> object ptr map. Greatly speeds up objectByID() and any other lookup functions.
		This map is updated in updatePointers().
	*/
	std::map<unsigned int, Drawing::AbstractDrawingObject*>							m_objectPtr;

	/*! Cached pick points of drawing.
		\param Key is ID of drawing object, to get better referencing in picking.
		\param Value is vector with 3D pick points
	*/
	mutable std::map<Field, std::map<unsigned int, std::vector<IBKMK::Vector3D>>>	m_pickPoints;

	/*! Mark if pick points have to be recalculated. */
	mutable bool																	m_dirtyPickPoints = true;
};



#endif // DRAWING_H
