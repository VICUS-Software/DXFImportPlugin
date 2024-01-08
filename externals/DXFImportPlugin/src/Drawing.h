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



class PlaneGeometry;

/*!
	Drawing class is data structure for all primitive drawing objects
	such as Layer, Lines, Circles, ... .
 */
class Drawing : public Object {

public:

	/*! Type-info string. */
	const char * typeinfo() const override {
		return "Drawing";
	}

	// *** PUBLIC MEMBER FUNCTIONS ***

	Drawing();

	void updateParents() {
		m_children.clear();
		for (DrawingLayer & dl : m_drawingLayers) {
			m_children.push_back(&dl);
			dl.m_parent = dynamic_cast<Object *>(this);
		}
		updatePointer();
	}

	TiXmlElement * writeXML(TiXmlElement * parent) const;


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

		unsigned int			m_id;					// unsigned int
		QString					m_currentBlockName;		// name of block
		QString					m_parentBlockName;		// name of block

		/* Block Entities that belongs to name. */
		Block					*m_currentBlock = nullptr;
		Block					*m_parentBlock = nullptr;

		double					m_xScale = 1.0;			// scale-factor in x direction
		double					m_yScale = 1.0;			// scale-factor in y direction
		double					m_zScale = 1.0;			// scale-factor in z direction

		double					m_angle = 0.0;			// angle of rotation

		IBKMK::Vector2D			m_insertionPoint;		// insertion point
	};

	/* Abstract class for all directly drawable dxf entities */
	struct AbstractDrawingObject {
		/*! Standard C'tor. */
		AbstractDrawingObject() = default;

		/*! Copy constructor */
		AbstractDrawingObject(const AbstractDrawingObject &other):
			m_layerName(other.m_layerName),
			m_parentLayer(other.m_parentLayer),
			m_color(other.m_color),
			m_lineWeight(other.m_lineWeight),
			m_zPosition(other.m_zPosition),
			m_blockName(other.m_blockName),
			m_block(other.m_block),
			m_id(other.m_id),
			m_trans(other.m_trans),
			m_isInsertObject(other.m_isInsertObject),
			m_simpleTranslation(other.m_simpleTranslation)
			{}

		/*! D'tor. */
		virtual ~AbstractDrawingObject() {}

		virtual void readXML(const TiXmlElement * element) = 0;
		/*! Abstract writeXML function: The actual object shall only be writen if this is not a block
		 *  */
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

		virtual TiXmlElement * writeXMLPrivate(TiXmlElement * parent) const = 0;

		/*! Function to update points, needed for easier
			handling of objects in scene3D to construct 3D
			Objects and for picking operations.

			Point will be recalculated when m_dirty is true;
		*/
		virtual const std::vector<IBKMK::Vector2D>& points2D() const = 0 ;

		/*! name of Entity */
		QString										m_layerName;
		/*! Layer of Entity */
		const DrawingLayer							*m_parentLayer = nullptr;
		/*! Color of Entity if defined, use getter color() instead */
		QColor										m_color = QColor();
		/*! Line weight of Entity, use getter lineWeight() instead */
		double										m_lineWeight = 0;
		/*! integer to create a drawing hierarchy in a dxf file to avoid overlapping of entities */
		unsigned int								m_zPosition;
		/*! Name of block. */
		QString										m_blockName;
		/*! Block Entity belongs to, if nullptr, no block is used */
		Block										*m_block = nullptr;
		/*! ID of object. */
		unsigned int								m_id;
		/*! Transformation matrix. */
		QMatrix4x4									m_trans = QMatrix4x4();
		/*! Defines wether this is a run-time only generated object defined
			by a block and according insert. If true the object shall not be written.
		*/
		bool										m_isInsertObject = false;

		IBKMK::Vector2D								m_simpleTranslation = IBKMK::Vector2D(0,0);

	protected:
		/*! Flag to indictate recalculation of points. */
		mutable bool								m_dirtyPoints = true;
		/*! Flag to indictate recalculation triangulation. */
		mutable bool								m_dirtyTriangulation = true;
		/*! Points of objects. */
		mutable std::vector<IBKMK::Vector2D>		m_pickPoints;

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

		/*! Extension line length. */
		double				m_extensionLineLength = 0.0;

		/*! Point 2 of Line. */
		double				m_textHeight = 0.0;

		/*! Global scaling factor. */
		double				m_globalScalingFactor = 1.0;

		/*! Text scaling factor. */
		double				m_textScalingFactor = 1.0;

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
		void readXML(const TiXmlElement * element) override;

		/*! Calculate points. */
		const std::vector<IBKMK::Vector2D>& points2D() const override;

		/*! Point coordinate */
		IBKMK::Vector2D					m_point;

	};

	/*! Stores attributes of line */
	struct Line : public AbstractDrawingObject {

		TiXmlElement * writeXMLPrivate(TiXmlElement * element) const override;
		void readXML(const TiXmlElement * element) override;

		/*! Calculate points. */
		const std::vector<IBKMK::Vector2D>& points2D() const override;

		/*! Point coordinate */
		IBKMK::Vector2D					m_point1;
		/*! Point coordinate */
		IBKMK::Vector2D					m_point2;
	};

	/*! Stores both LW and normal polyline */
	struct PolyLine : public AbstractDrawingObject {

		TiXmlElement * writeXMLPrivate(TiXmlElement * element) const override;
		void readXML(const TiXmlElement * element) override;

		/*! Calculate points. */
		const std::vector<IBKMK::Vector2D>& points2D() const override;

		/*! polyline coordinates */
		std::vector<IBKMK::Vector2D>    m_polyline;
		/*! When end point is connected to start */
		bool							m_endConnected = false;
	};

	/* Stores attributes of circle */
	struct Circle : public AbstractDrawingObject {

		TiXmlElement * writeXMLPrivate(TiXmlElement * element) const override;
		void readXML(const TiXmlElement * element) override;

		/*! Calculate points. */
		const std::vector<IBKMK::Vector2D>& points2D() const override;

		/*! Circle center */
		IBKMK::Vector2D					m_center;
		/*! Circle radius */
		double							m_radius;
	};

	/* Stores attributes of ellipse */
	struct Ellipse : public AbstractDrawingObject {

		TiXmlElement * writeXMLPrivate(TiXmlElement * element) const override;
		void readXML(const TiXmlElement * element) override;

		const std::vector<IBKMK::Vector2D> &points2D() const override;

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
		void readXML(const TiXmlElement * element) override;

		const std::vector<IBKMK::Vector2D> &points2D() const override;

		/*! Arc center */
		IBKMK::Vector2D			m_center;
		/*! Arc radius */
		double					m_radius;
		/*! Arc start angle */
		double					m_startAngle;
		/*! Arc end angle */
		double					m_endAngle;
	};

	/* Stores attributes of solid, dummy struct */
	struct Solid : public AbstractDrawingObject {

		TiXmlElement * writeXMLPrivate(TiXmlElement * element) const override;
		void readXML(const TiXmlElement * element) override;

		const std::vector<IBKMK::Vector2D> &points2D() const override;

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
		void readXML(const TiXmlElement * element) override;

		const std::vector<IBKMK::Vector2D> &points2D() const override;

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
		void readXML(const TiXmlElement * element) override;

		const std::vector<IBKMK::Vector2D> &points2D() const override;

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
		double						m_angle;

		/*! Name of Dim style. */
		QString						m_styleName;

		/*! Measurement text. */
		QString						m_measurement = "";

		/*! Pointer to style object. Updated in
			updatePointers();
		*/
		DimStyle					*m_style = nullptr;
	};


	// *** PUBLIC MEMBER FUNCTIONS ***

	/*! Returns the drawing object based on the ID. */
	const AbstractDrawingObject* objectByID(unsigned int id) const;

	void sortLayersAlphabetical();

	/*! used to assign the correct layer to an entity */
	void updatePointer();

	/*! Generates all inserting geometries. */
	void generateInsertGeometries(unsigned int &nextId);

	/*! Returns 3D Pick points of drawing. */
	const std::map<unsigned int, std::vector<IBKMK::Vector3D>> &pickPoints() const;

	/*! Generates 3D Points from 2D points by applying transformation from drawing. */
	const std::vector<IBKMK::Vector3D> points3D(const std::vector<IBKMK::Vector2D> &verts, unsigned int zPosition, const QMatrix4x4 &trans = QMatrix4x4()) const;

	/*! Returns the normal vector of the drawing. */
	const IBKMK::Vector3D normal() const;

	/*! Returns the normal vector of the drawing. */
	const IBKMK::Vector3D localX() const;

	/*! Returns the normal vector of the drawing. */
	const IBKMK::Vector3D localY() const;

	/*! Calculates the center coordinates, defined as mean value of all coordinates
		Pointers must be updated before calling this function!
	*/
	IBKMK::Vector3D weightedCenter(unsigned int nextId);

	IBKMK::Vector3D weightedCenterMedian(unsigned int nextId);

	/*! Subtracts the origin from all coordinates */
	void moveToOrigin();

	/*! updates insert Points to reduce offset */
	void compensateCoordinates();

	/*! point of origin */
	IBKMK::Vector3D															m_origin			= IBKMK::Vector3D(0,0,0);						// XML:E
	/*! rotation matrix */
	RotationMatrix															m_rotationMatrix	= RotationMatrix(QQuaternion(1.0,0.0,0.0,0.0)); // XML:E
	/*! scale factor */
	double																	m_scalingFactor		= 1.0;											// XML:E

	/*! list of blocks */
	std::vector<Block>														m_blocks;			// XML:E
	/*! list of layers */
	std::vector<DrawingLayer>												m_drawingLayers;	// XML:E
	/*! list of points */
	std::vector<Point>														m_points;			// XML:E
	/*! list of lines */
	std::vector<Line>														m_lines;			// XML:E
	/*! list of polylines */
	std::vector<PolyLine>													m_polylines;		// XML:E
	/*! list of circles */
	std::vector<Circle>														m_circles;			// XML:E
	/*! list of ellipses */
	std::vector<Ellipse>													m_ellipses;			// XML:E
	/*! list of arcs */
	std::vector<Arc>														m_arcs;				// XML:E
	/*! list of solids, dummy implementation */
	std::vector<Solid>														m_solids;			// XML:E
	/*! list of texts */
	std::vector<Text>														m_texts;			// XML:E
	/*! list of texts */
	std::vector<LinearDimension>											m_linearDimensions;	// XML:E
	/*! list of Dim Styles */
	std::vector<DimStyle>													m_dimensionStyles;	// XML:E
	/*! list of inserts. */
	std::vector<Insert>														m_inserts;

	/*! Counter of entities, used to create a drawing hierarchy
		in a dxf file to avoid overlapping of entities */
	unsigned int															m_zCounter = 0;	// XML:E

	/*! Is the default color when no other color was specified */
	QColor																	m_defaultColor = QColor(); // XML:E

private:

	const Block *findParentBlock(const Insert &i) const;

	const Block *findParentBlockByBlock(Block & block) const;

	/*! Helper function to assign the correct layer to an entity */
	DrawingLayer *findLayerPointer(const QString &layername);

	/*! Helper function to assign the correct block to an entity */
	Block* findBlockPointer(const QString &name, const std::map<QString, Block*> &blockRefs);

	/*! Transforms all inserts. */
	void transformInsert(IBKMK::Vector2D trans, const Drawing::Insert &insert, unsigned int &nextId);

	/*! Cached unique-ID -> object ptr map. Greatly speeds up objectByID() and any other lookup functions.
		This map is updated in updatePointers().
	*/
	std::map<unsigned int, Drawing::AbstractDrawingObject*>	m_objectPtr;

	/*! Cached pick points of drawing.
		\param Key is ID of drawing object, to get better referencing in picking.
		\param Value is vector with 3D pick points
	*/
	mutable std::map<unsigned int, std::vector<IBKMK::Vector3D>>	m_pickPoints;

	/*! Mark if pick points have to be recalculated. */
	mutable bool													m_dirtyPickPoints = true;
};



#endif // DRAWING_H
