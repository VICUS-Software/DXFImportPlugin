#include "ImportDXFDialog.h"
#include "ui_ImportDXFDialog.h"

#include <QMessageBox>

#include <regex>

#include <IBK_physics.h>
#include <IBK_messages.h>

#include <IBKMK_3DCalculations.h>

#include <libdxfrw.h>



ImportDXFDialog::ImportDXFDialog(QWidget *parent) :
	QDialog(parent),
	m_ui(new Ui::ImportDXFDialog)
{
	m_ui->setupUi(this);

	resize(1500, 800);

	QString defaultName = tr("Drawing");

	m_ui->lineEditDrawingName->setText(defaultName);

	m_ui->comboBoxUnit->addItem("Auto", SU_Auto);
	m_ui->comboBoxUnit->addItem("Meter", SU_Meter);
	m_ui->comboBoxUnit->addItem("Decimeter", SU_Decimeter);
	m_ui->comboBoxUnit->addItem("Centimeter", SU_Centimeter);
	m_ui->comboBoxUnit->addItem("Millimeter", SU_Millimeter);

	m_ui->checkBoxFixFonts->setHidden(true);

	m_ui->progressBar->setValue(0);
	m_ui->progressBar->update();
	m_ui->progressBar->setEnabled(false);
}

ImportDXFDialog::~ImportDXFDialog() {
	delete m_ui;
}


ImportDXFDialog::ImportResults ImportDXFDialog::importFile(const QString &fname) {

	if (m_ui->lineEditDrawingName->text().trimmed().isEmpty()) {
		QMessageBox::critical(this, QString(), tr("Please enter a descriptive name!"));
		m_ui->lineEditDrawingName->selectAll();
		m_ui->lineEditDrawingName->setFocus();
	}

	m_ui->pushButtonImport->setEnabled(false);
	m_filePath = fname;

	m_ui->lineEditDrawingName->setText(fname);

	int res = exec();
	if (res == QDialog::Rejected)
		return ImportCancelled;

	if (m_ui->checkBoxMove->isChecked())
		moveDrawings();

	if (m_ui->checkBoxFixFonts->isChecked())
		fixFonts();

	m_ui->plainTextEditLogWindow->clear();

	return m_returnCode;
}


void ImportDXFDialog::on_pushButtonConvert_clicked() {
	FUNCID(ImportDXFDialog::on_pushButtonConvert_clicked);

	m_ui->progressBar->setEnabled(true);
	m_ui->progressBar->setRange(0,4);
	m_ui->progressBar->setValue(1);
	m_ui->progressBar->setTextVisible(true);
	m_ui->progressBar->setFormat("Reading file %p%");
	m_ui->progressBar->update();

	QString log;
	QFile fileName(m_filePath);
	if (!fileName.exists()) {
		throw QMessageBox::warning(this, tr("DXF Conversion"), tr("File %1 does not exist.").arg(fileName.fileName()));
		log += "File " + fileName.fileName() + " does not exist! Aborting Conversion.\n";
	}

	bool success;
	try {
		// we clear the drawing
		m_drawing = Drawing();
		m_drawing.m_id = 1;

		success = readDxfFile(m_drawing, fileName.fileName());

		m_ui->progressBar->setValue(2);
		m_ui->progressBar->setFormat("Update References %p%");

		// we need to generate inserted geometries here only in order to find the correct drawing center!
		m_nextId = 2;
//		m_drawing.generateInsertGeometries(m_nextId);
		m_drawing.updateParents();

		if (!m_ui->checkBoxImportText->isChecked()) {
			m_drawing.m_texts.clear();
			m_drawing.m_linearDimensions.clear();
		}

		m_ui->pushButtonImport->setEnabled(success);

		if (!success)
			throw IBK::Exception(IBK::FormatString("Import of DXF-File was not successful!"), FUNC_ID);

		for (Drawing::Arc &arc: m_drawing.m_arcs) {
			const DrawingLayer *dl = arc.m_parentLayer;
			if (dl == nullptr) {
				throw IBK::Exception("arcs", FUNC_ID);
			}
		}

		// set name for drawing from lineEdit
		m_drawing.m_displayName = m_ui->lineEditDrawingName->text();

		log += "Import successful!\nThe following objects were imported:\n";
		log += QString("---------------------------------------------------------\n");
		log += QString("Layers:\t\t%1\n").arg(m_drawing.m_drawingLayers.size());
		log += QString("Lines:\t\t%1\n").arg(m_drawing.m_lines.size());
		log += QString("Polylines:\t\t%1\n").arg(m_drawing.m_polylines.size());
		log += QString("Arcs:\t\t%1\n").arg(m_drawing.m_arcs.size());
		log += QString("Circles:\t\t%1\n").arg(m_drawing.m_circles.size());
		log += QString("Ellipses:\t\t%1\n").arg(m_drawing.m_ellipses.size());
		log += QString("Points:\t\t%1\n").arg(m_drawing.m_points.size());
		log += QString("Linear Dimensions:\t%1\n").arg(m_drawing.m_linearDimensions.size());
		log += QString("Dimension Styles:\t%1\n").arg(m_drawing.m_dimensionStyles.size());
		log += QString("Inserts:\t\t%1\n").arg(m_drawing.m_inserts.size());
		log += QString("Solids:\t\t%1\n").arg(m_drawing.m_solids.size());
		log += QString("---------------------------------------------------------\n");

		ScaleUnit su = (ScaleUnit)m_ui->comboBoxUnit->currentData().toInt();

		double scalingFactor[NUM_SU] = {0.001, 1.0, 0.1, 0.01, 0.001};

		m_ui->progressBar->setValue(3);
		m_ui->progressBar->setFormat("Calculate bounding box and center %p%");

		IBKMK::Vector3D dummy;
		m_drawing.updatePointer();
		IBKMK::Vector3D bounding = boundingBox(&m_drawing, dummy, false);

		// calculate center
		m_drawing.m_origin = m_drawing.weightedCenter();

		// Drawing should be at least bigger than 150 m
		double AUTO_SCALING_THRESHOLD = 1000;
		if (su == SU_Auto) {
			bool foundAutoScaling = false;
			for (unsigned int i=1; i<NUM_SU; ++i) {

				if (scalingFactor[i] * bounding.m_x > AUTO_SCALING_THRESHOLD)
					continue;

				if (scalingFactor[i] * bounding.m_y > AUTO_SCALING_THRESHOLD)
					continue;

				scalingFactor[SU_Auto] = scalingFactor[i];
				foundAutoScaling = true;
				break;
			}
			if (foundAutoScaling)
				log += QString("Found auto scaling factor: %1\n").arg(scalingFactor[SU_Auto]);
		}
		log += QString("Current dimensions - X: %1 Y: %2 Z: %3\n").arg(scalingFactor[su] * bounding.m_x)
				   .arg(scalingFactor[su] * bounding.m_y)
				   .arg(scalingFactor[su] * bounding.m_z);

		log += QString("Current center - X: %1 Y: %2 Z: %3\n").arg(scalingFactor[su] * m_drawing.m_origin.m_x)
				   .arg(scalingFactor[su] * m_drawing.m_origin.m_y)
				   .arg(scalingFactor[su] * m_drawing.m_origin.m_z);
		log += QString("---------------------------------------------------------\n");
		log += QString("\nPLEASE MIND: Currently are no hatchings supported.\n");

		m_drawing.m_scalingFactor = scalingFactor[su];
		m_drawing.m_origin *= scalingFactor[su];

	} catch (IBK::Exception &ex) {
		log += "Error in converting DXF-File. See Error below\n";
		log += ex.what();
	}

	m_ui->plainTextEditLogWindow->setPlainText(log);

	m_ui->progressBar->setValue(4);
}


void ImportDXFDialog::on_pushButtonImport_clicked() {
	m_returnCode = AddDrawings;
	accept();
}


bool ImportDXFDialog::readDxfFile(Drawing &drawing, const QString &fname) {
	DRW_InterfaceImpl drwIntImpl(&drawing, m_nextId);
	dxfRW dxf(fname.toStdString().c_str());

	bool success = dxf.read(&drwIntImpl, false);
	return success;
}

template <typename t>
void movePoints(const IBKMK::Vector2D &center, std::vector<t> &objects) {
	for (Drawing::AbstractDrawingObject &obj: objects) {
		for (const IBKMK::Vector2D &v2D : obj.points2D()) {
			const_cast<IBKMK::Vector2D &>(v2D) -= center;
		}
	}
}

void ImportDXFDialog::moveDrawings() {
	m_drawing.moveToOrigin();
}

void ImportDXFDialog::fixFonts() {
	double HEIGHT = 15;

	for (Drawing::Text &text : m_drawing.m_texts) {
		if (text.m_height > HEIGHT)
			text.m_height *= 0.1;
	}
	for (Drawing::LinearDimension &linDem : m_drawing.m_linearDimensions) {
		if (linDem.m_style->m_textHeight > HEIGHT)
			linDem.m_style->m_textHeight *= 0.1;
	}
}


const Drawing& ImportDXFDialog::drawing() const {
	return m_drawing;
}


//template <typename t>
//void drawingObjectsWeightedCenter(const Drawing &d, const std::vector<t> &drawingObjects, IBKMK::Vector3D &center, unsigned int &numPoints) {

//	for (const t &drawObj : drawingObjects) {
//		const DrawingLayer *dl = dynamic_cast<const DrawingLayer *>(drawObj.m_parentLayer);
//		Q_ASSERT(dl != nullptr);
//		if (!dl->m_visible)
//			continue;

//		const std::vector<IBKMK::Vector3D> &points = d.points3D(drawObj.points2D(), drawObj.m_zPosition, drawObj.m_trans);
//		for (const IBKMK::Vector3D &v : points) {
//			center += v;
//			numPoints++;
//		}
//	}
//}


//void drawingWeightedCenter(const Drawing &d,
//							IBKMK::Vector3D &center) {

//	unsigned int numPoints = 0;
//	center = IBKMK::Vector3D();
//	drawingObjectsWeightedCenter(d, d.m_arcs, center, numPoints);
//	drawingObjectsWeightedCenter(d, d.m_circles, center, numPoints);
//	drawingObjectsWeightedCenter(d, d.m_ellipses, center, numPoints);
//	drawingObjectsWeightedCenter(d, d.m_lines, center, numPoints);
//	drawingObjectsWeightedCenter(d, d.m_polylines, center, numPoints);
//	drawingObjectsWeightedCenter(d, d.m_points, center, numPoints);
//	drawingObjectsWeightedCenter(d, d.m_solids, center, numPoints);
//	drawingObjectsWeightedCenter(d, d.m_texts, center, numPoints);
//	drawingObjectsWeightedCenter(d, d.m_linearDimensions, center, numPoints);

//	center /= numPoints;
//}


template <typename t>
void drawingBoundingBox(const Drawing &d,
						const std::vector<t> &drawingObjects,
						IBKMK::Vector3D &upperValues,
						IBKMK::Vector3D &lowerValues,
						const IBKMK::Vector3D &offset = IBKMK::Vector3D(0,0,0),
						const IBKMK::Vector3D &xAxis = IBKMK::Vector3D(1,0,0),
						const IBKMK::Vector3D &yAxis = IBKMK::Vector3D(0,1,0),
						const IBKMK::Vector3D &zAxis = IBKMK::Vector3D(0,0,1)) {

	// store selected surfaces
	if (drawingObjects.empty())
		return;

	// process all drawings
	for (const t &drawObj : drawingObjects) {
		const DrawingLayer *dl = dynamic_cast<const DrawingLayer *>(drawObj.m_parentLayer);

		Q_ASSERT(dl != nullptr);

		if (!dl->m_visible)
			continue;

		const std::vector<IBKMK::Vector3D> &points = d.points3D(drawObj.points2D(), drawObj.m_zPosition, drawObj.m_trans);

		for (const IBKMK::Vector3D &v : points) {

			IBKMK::Vector3D vLocal, point;

			IBKMK::lineToPointDistance(offset, xAxis, v, vLocal.m_x, point);
			IBKMK::lineToPointDistance(offset, yAxis, v, vLocal.m_y, point);
			IBKMK::lineToPointDistance(offset, zAxis, v, vLocal.m_z, point);

			upperValues.m_x = std::max(upperValues.m_x, (double)vLocal.m_x);
			upperValues.m_y = std::max(upperValues.m_y, (double)vLocal.m_y);
			upperValues.m_z = std::max(upperValues.m_z, (double)vLocal.m_z);

			lowerValues.m_x = std::min(lowerValues.m_x, (double)vLocal.m_x);
			lowerValues.m_y = std::min(lowerValues.m_y, (double)vLocal.m_y);
			lowerValues.m_z = std::min(lowerValues.m_z, (double)vLocal.m_z);
		}
	}
}


IBKMK::Vector3D ImportDXFDialog::boundingBox(const Drawing *drawing, IBKMK::Vector3D & center, bool transformPoints) {

	IBKMK::Vector3D lowerValues(std::numeric_limits<double>::max(),
								std::numeric_limits<double>::max(),
								std::numeric_limits<double>::max());
	IBKMK::Vector3D upperValues(std::numeric_limits<double>::lowest(),
								std::numeric_limits<double>::lowest(),
								std::numeric_limits<double>::lowest());

	drawingBoundingBox<Drawing::Arc>(*drawing, drawing->m_arcs, upperValues, lowerValues);
	drawingBoundingBox<Drawing::Circle>(*drawing, drawing->m_circles, upperValues, lowerValues);
	drawingBoundingBox<Drawing::Ellipse>(*drawing, drawing->m_ellipses, upperValues, lowerValues);
	drawingBoundingBox<Drawing::Line>(*drawing, drawing->m_lines, upperValues, lowerValues);
	drawingBoundingBox<Drawing::PolyLine>(*drawing, drawing->m_polylines, upperValues, lowerValues);
	drawingBoundingBox<Drawing::Point>(*drawing, drawing->m_points, upperValues, lowerValues);
	drawingBoundingBox<Drawing::Solid>(*drawing, drawing->m_solids, upperValues, lowerValues);
	drawingBoundingBox<Drawing::Text>(*drawing, drawing->m_texts, upperValues, lowerValues);
	drawingBoundingBox<Drawing::LinearDimension>(*drawing, drawing->m_linearDimensions, upperValues, lowerValues);

	// center point of bounding box
	center = 0.5*(lowerValues+upperValues);
	// difference between upper and lower values gives bounding box (dimensions of selected geometry)
	return (upperValues-lowerValues);
}


void ImportDXFDialog::on_comboBoxUnit_activated(int index) {
	m_ui->comboBoxUnit->setCurrentIndex(index);
}


DRW_InterfaceImpl::DRW_InterfaceImpl(Drawing *drawing, unsigned int &nextId) :
	m_drawing(drawing),
	m_nextId(&nextId)
{}


void DRW_InterfaceImpl::addHeader(const DRW_Header* /*data*/){}
void DRW_InterfaceImpl::addLType(const DRW_LType& /*data*/){}
void DRW_InterfaceImpl::addLayer(const DRW_Layer& data){

	// If nullptr return
	if(m_activeBlock != nullptr)
		return;

	// initialise struct Layer and populate the attributes
	DrawingLayer newLayer;

	// Set ID
	newLayer.m_id = (*m_nextId)++;

	// name of layer
	newLayer.m_displayName = QString::fromStdString(data.name);

	// read linewidth from dxf file, convert to double using lineWidth2dxfInt from libdxfrw
	newLayer.m_lineWeight = DRW_LW_Conv::lineWidth2dxfInt(data.lWeight);

	/* value 256 means use defaultColor, value 7 is black */
	if (data.color != 256 && data.color != 7)
		newLayer.m_color = QColor(DRW::dxfColors[data.color][0], DRW::dxfColors[data.color][1], DRW::dxfColors[data.color][2]);
//	else
//		newLayer.m_color = SVStyle::instance().m_defaultDrawingColor;

	// Push new layer into vector<Layer*> m_layer
	m_drawing->m_drawingLayers.push_back(newLayer);
}


void DRW_InterfaceImpl::addDimStyle(const DRW_Dimstyle& data) {
	Drawing::DimStyle dimStyle;
	dimStyle.m_name = QString::fromStdString(data.name);
	dimStyle.m_upperLineDistance = data.dimexe;
	dimStyle.m_extensionLineLowerDistance = data.dimexo;

	dimStyle.m_fixedExtensionLength = data.dimfxlon == 1;
	dimStyle.m_extensionLineLength = data.dimfxl;
	dimStyle.m_textHeight = data.dimtxt;
	dimStyle.m_globalScalingFactor = data.dimscale;
	dimStyle.m_textScalingFactor = data.dimtfac;
	dimStyle.m_textLinearFactor = data.dimlfac;
	dimStyle.m_textDecimalPlaces = data.dimdec;

	qDebug() << data.dimlfac;

	dimStyle.m_id = (*m_nextId)++;

	// Add object
	m_drawing->m_dimensionStyles.push_back(dimStyle);
}


void DRW_InterfaceImpl::addVport(const DRW_Vport& /*data*/){}
void DRW_InterfaceImpl::addTextStyle(const DRW_Textstyle& /*data*/){}
void DRW_InterfaceImpl::addAppId(const DRW_AppId& /*data*/){}
void DRW_InterfaceImpl::addBlock(const DRW_Block& data){

	if (data.name.empty())
		return;

	// New Block
	Drawing::Block newBlock;

	// Set name
	newBlock.m_name = QString::fromStdString(data.name);

	// Set color
	newBlock.m_color = QColor();

	// Set line weight
	newBlock.m_lineWeight = 0;

	// ID
	newBlock.m_id = (*m_nextId)++;

	// Set base
	newBlock.m_basePoint = IBKMK::Vector2D(data.basePoint.x, data.basePoint.y);

	m_drawing->m_blocks.push_back(newBlock);

	// Set actove block
	m_activeBlock = &m_drawing->m_blocks.back();
}


void DRW_InterfaceImpl::setBlock(const int /*handle*/){}
void DRW_InterfaceImpl::endBlock(){
	// Active block not existing
	m_activeBlock = nullptr;

}


void DRW_InterfaceImpl::addPoint(const DRW_Point& data){

	Drawing::Point newPoint;
	newPoint.m_zPosition = m_drawing->m_zCounter;
	m_drawing->m_zCounter++;

	//create new point, insert into vector m_points from drawing
	newPoint.m_point = IBKMK::Vector2D(data.basePoint.x, data.basePoint.y);
	newPoint.m_lineWeight = DRW_LW_Conv::lineWidth2dxfInt(data.lWeight);
	newPoint.m_layerName = QString::fromStdString(data.layer);

	newPoint.m_id = (*m_nextId)++;
	if (m_activeBlock != nullptr)
		newPoint.m_blockName = m_activeBlock->m_name;

	/* value 256 means use defaultColor, value 7 is black */
	if(!(data.color == 256 || data.color == 7))
		newPoint.m_color = QColor(DRW::dxfColors[data.color][0], DRW::dxfColors[data.color][1], DRW::dxfColors[data.color][2]);
	else
		newPoint.m_color = QColor();

	m_drawing->m_points.push_back(newPoint);

}


void DRW_InterfaceImpl::addLine(const DRW_Line& data){

	Drawing::Line newLine;
	newLine.m_zPosition = m_drawing->m_zCounter;
	m_drawing->m_zCounter++;

	//create new line, insert into vector m_lines from drawing
	newLine.m_point1 = IBKMK::Vector2D(data.basePoint.x, data.basePoint.y);
	newLine.m_point2 = IBKMK::Vector2D(data.secPoint.x, data.secPoint.y);
	newLine.m_lineWeight = DRW_LW_Conv::lineWidth2dxfInt(data.lWeight);
	newLine.m_layerName = QString::fromStdString(data.layer);

	newLine.m_id = (*m_nextId)++;
	if (m_activeBlock != nullptr)
		newLine.m_blockName = m_activeBlock->m_name;

	/* value 256 means use defaultColor, value 7 is black */
	/* value 256 means use defaultColor, value 7 is black */
	if(!(data.color == 256 || data.color == 7))
		newLine.m_color = QColor(DRW::dxfColors[data.color][0], DRW::dxfColors[data.color][1], DRW::dxfColors[data.color][2]);
	else
		newLine.m_color = QColor();

	m_drawing->m_lines.push_back(newLine);
}

void DRW_InterfaceImpl::addRay(const DRW_Ray& /*data*/){}
void DRW_InterfaceImpl::addXline(const DRW_Xline& /*data*/){}
void DRW_InterfaceImpl::addArc(const DRW_Arc& data){

	Drawing::Arc newArc;
	newArc.m_zPosition = m_drawing->m_zCounter;
	m_drawing->m_zCounter++;

	// create new arc, insert into vector m_arcs from drawing
	newArc.m_radius = data.radious;
	newArc.m_startAngle = data.staangle;
	newArc.m_endAngle = data.endangle;
	newArc.m_center = IBKMK::Vector2D(data.basePoint.x, data.basePoint.y);
	newArc.m_lineWeight = DRW_LW_Conv::lineWidth2dxfInt(data.lWeight);
	newArc.m_layerName = QString::fromStdString(data.layer);

	newArc.m_id = (*m_nextId)++;
	if (m_activeBlock != nullptr)
		newArc.m_blockName = m_activeBlock->m_name;

	/* value 256 means use defaultColor, value 7 is black */
	if(!(data.color == 256 || data.color == 7))
		newArc.m_color = QColor(DRW::dxfColors[data.color][0], DRW::dxfColors[data.color][1], DRW::dxfColors[data.color][2]);
	else
		newArc.m_color = QColor();

	m_drawing->m_arcs.push_back(newArc);
}


void DRW_InterfaceImpl::addCircle(const DRW_Circle& data){



	Drawing::Circle newCircle;
	newCircle.m_zPosition = m_drawing->m_zCounter;
	m_drawing->m_zCounter++;

	newCircle.m_center = IBKMK::Vector2D(data.basePoint.x, data.basePoint.y);

	newCircle.m_radius = data.radious;
	newCircle.m_lineWeight = DRW_LW_Conv::lineWidth2dxfInt(data.lWeight);
	newCircle.m_layerName = QString::fromStdString(data.layer);

	newCircle.m_id = (*m_nextId)++;
	if (m_activeBlock != nullptr)
		newCircle.m_blockName = m_activeBlock->m_name;

	/* value 256 means use defaultColor, value 7 is black */
	if(!(data.color == 256 || data.color == 7))
		newCircle.m_color = QColor(DRW::dxfColors[data.color][0], DRW::dxfColors[data.color][1], DRW::dxfColors[data.color][2]);
	else
		newCircle.m_color = QColor();

	m_drawing->m_circles.push_back(newCircle);
}


void DRW_InterfaceImpl::addEllipse(const DRW_Ellipse& data){

	Drawing::Ellipse newEllipse;
	newEllipse.m_zPosition = m_drawing->m_zCounter;
	m_drawing->m_zCounter++;

	newEllipse.m_center = IBKMK::Vector2D(data.basePoint.x, data.basePoint.y);
	newEllipse.m_majorAxis = IBKMK::Vector2D(data.secPoint.x, data.secPoint.y);
	newEllipse.m_ratio = data.ratio;
	newEllipse.m_startAngle = data.staparam;
	newEllipse.m_endAngle = data.endparam;
	newEllipse.m_lineWeight = DRW_LW_Conv::lineWidth2dxfInt(data.lWeight);
	newEllipse.m_layerName = QString::fromStdString(data.layer);

	newEllipse.m_id = (*m_nextId)++;
	if (m_activeBlock != nullptr)
		newEllipse.m_blockName = m_activeBlock->m_name;

	/* value 256 means use defaultColor, value 7 is black */
	if(!(data.color == 256 || data.color == 7))
		newEllipse.m_color = QColor(DRW::dxfColors[data.color][0], DRW::dxfColors[data.color][1], DRW::dxfColors[data.color][2]);
	else
		newEllipse.m_color = QColor();

	m_drawing->m_ellipses.push_back(newEllipse);
}


void DRW_InterfaceImpl::addLWPolyline(const DRW_LWPolyline& data){

	Drawing::PolyLine newPolyline;
	newPolyline.m_zPosition = m_drawing->m_zCounter;
	m_drawing->m_zCounter++;

	newPolyline.m_polyline = std::vector<IBKMK::Vector2D>();
	newPolyline.m_lineWeight = DRW_LW_Conv::lineWidth2dxfInt(data.lWeight);

	newPolyline.m_id = (*m_nextId)++;
	if (m_activeBlock != nullptr)
		newPolyline.m_blockName = m_activeBlock->m_name;

	// iterate over data.vertlist, insert all vertices of Polyline into vector
	for(size_t i = 0; i < data.vertlist.size(); i++){
		IBKMK::Vector2D point(data.vertlist[i]->x, data.vertlist[i]->y);
		newPolyline.m_polyline.push_back(point);
	}

	newPolyline.m_layerName = QString::fromStdString(data.layer);

	/* value 256 means use defaultColor, value 7 is black */
	if(!(data.color == 256 || data.color == 7))
		newPolyline.m_color = QColor(DRW::dxfColors[data.color][0], DRW::dxfColors[data.color][1], DRW::dxfColors[data.color][2]);
	else
		newPolyline.m_color = QColor();

	newPolyline.m_endConnected = data.flags == 129 || data.flags == 1 ;

	// insert vector into m_lines[data.layer] vector
	m_drawing->m_polylines.push_back(newPolyline);
}


void DRW_InterfaceImpl::addPolyline(const DRW_Polyline& data){

	Drawing::PolyLine newPolyline;
	newPolyline.m_zPosition = m_drawing->m_zCounter;
	m_drawing->m_zCounter++;
	newPolyline.m_polyline = std::vector<IBKMK::Vector2D>();

	// iterateover data.vertlist, insert all vertices of Polyline into vector
	for(size_t i = 0; i < data.vertlist.size(); i++){
		IBKMK::Vector2D point(data.vertlist[i]->basePoint.x, data.vertlist[i]->basePoint.y);
		newPolyline.m_polyline.push_back(point);
	}

	newPolyline.m_id = (*m_nextId)++;
	if (m_activeBlock != nullptr)
		newPolyline.m_blockName = m_activeBlock->m_name;

	newPolyline.m_layerName = QString::fromStdString(data.layer);
	newPolyline.m_lineWeight = DRW_LW_Conv::lineWidth2dxfInt(data.lWeight);

	/* value 256 means use defaultColor, value 7 is black */
	if(!(data.color == 256 || data.color == 7))
		newPolyline.m_color = QColor(DRW::dxfColors[data.color][0], DRW::dxfColors[data.color][1], DRW::dxfColors[data.color][2]);
	else
		newPolyline.m_color = QColor();

	newPolyline.m_endConnected = data.flags == 129 || data.flags == 1 ;

	// insert vector into m_lines[data.layer] vector
	m_drawing->m_polylines.push_back(newPolyline);
}
void DRW_InterfaceImpl::addSpline(const DRW_Spline* /*data*/){}
void DRW_InterfaceImpl::addKnot(const DRW_Entity & /*data*/){}

void DRW_InterfaceImpl::addInsert(const DRW_Insert& data){
	if (data.name.empty())
		return;

	Drawing::Insert newInsert;
	newInsert.m_currentBlockName = QString::fromStdString(data.name);
	newInsert.m_id = (*m_nextId)++;

	newInsert.m_angle = data.angle;

	newInsert.m_xScale = data.xscale;
	newInsert.m_yScale = data.yscale;
	newInsert.m_zScale = data.zscale;

	newInsert.m_insertionPoint = IBKMK::Vector2D(data.basePoint.x, data.basePoint.y);
	if (m_activeBlock != nullptr)
		newInsert.m_parentBlockName = m_activeBlock->m_name;

	m_drawing->m_inserts.push_back(newInsert);
}

void DRW_InterfaceImpl::addTrace(const DRW_Trace& /*data*/){}
void DRW_InterfaceImpl::add3dFace(const DRW_3Dface& /*data*/){}
void DRW_InterfaceImpl::addSolid(const DRW_Solid& data){

	Drawing::Solid newSolid;
	newSolid.m_zPosition = m_drawing->m_zCounter;
	m_drawing->m_zCounter++;

	newSolid.m_point1 = IBKMK::Vector2D(data.basePoint.x, data.basePoint.y);
	newSolid.m_point2 = IBKMK::Vector2D(data.secPoint.x, data.secPoint.y);
	newSolid.m_point3 = IBKMK::Vector2D(data.fourPoint.x, data.fourPoint.y);
	newSolid.m_point4 = IBKMK::Vector2D(data.thirdPoint.x, data.thirdPoint.y);

	newSolid.m_lineWeight = DRW_LW_Conv::lineWidth2dxfInt(data.lWeight);
	newSolid.m_layerName = QString::fromStdString(data.layer);

	newSolid.m_id = (*m_nextId)++;
	if (m_activeBlock != nullptr)
		newSolid.m_blockName = m_activeBlock->m_name;

	/* value 256 means use defaultColor, value 7 is black */
	if(!(data.color == 256 || data.color == 7))
		newSolid.m_color = QColor(DRW::dxfColors[data.color][0], DRW::dxfColors[data.color][1], DRW::dxfColors[data.color][2]);
	else
		newSolid.m_color = QColor();

	m_drawing->m_solids.push_back(newSolid);

}

std::string replaceFormatting(const std::string &str) {
	std::string replaced = str;
	try {
		replaced = std::regex_replace(replaced, std::regex("\\\\\\\\"), "\032");
		replaced = std::regex_replace(replaced, std::regex("\\\\P|\\n|\\t"), " ");
		replaced = std::regex_replace(replaced, std::regex("\\\\(\\\\[ACcFfHLlOopQTW])|\\\\[ACcFfHLlOopQTW][^\\\\;]*;|\\\\[ACcFfHLlOopQTW]"), "$1");
		replaced = std::regex_replace(replaced, std::regex("([^\\\\])\\\\S([^;]*)[/#\\^]([^;]*);"), "$1$2/$3");
		replaced = std::regex_replace(replaced, std::regex("\\\\(\\\\S)|[\\\\](})|}"), "$1$2");
		replaced = std::regex_replace(replaced, std::regex("\\{/?"), "");
	}
	catch (std::exception &ex) {
		IBK::IBK_Message(IBK::FormatString("Could not replace all regex expressions.\n%1").arg(ex.what()),
						 IBK::MSG_WARNING);
	}

	return replaced;
}

void DRW_InterfaceImpl::addMText(const DRW_MText& data){
	Drawing::Text newText;
	newText.m_text = QString::fromStdString(replaceFormatting(data.text));
	newText.m_basePoint = IBKMK::Vector2D(data.basePoint.x, data.basePoint.y);
	newText.m_zPosition = m_drawing->m_zCounter;
	m_drawing->m_zCounter++;
	newText.m_id = (*m_nextId)++;
	if (m_activeBlock != nullptr)
		newText.m_blockName = m_activeBlock->m_name;

	newText.m_lineWeight = DRW_LW_Conv::lineWidth2dxfInt(data.lWeight);
	newText.m_layerName = QString::fromStdString(data.layer);
	newText.m_height = data.height;
	newText.m_alignment = data.alignH == DRW_Text::HCenter ? Qt::AlignHCenter : Qt::AlignLeft;
	newText.m_rotationAngle = data.angle;

	/* value 256 means use defaultColor, value 7 is black */
	if(!(data.color == 256 || data.color == 7))
		newText.m_color = QColor(DRW::dxfColors[data.color][0], DRW::dxfColors[data.color][1], DRW::dxfColors[data.color][2]);
	else
		newText.m_color = QColor();

	m_drawing->m_texts.push_back(newText);
}

void DRW_InterfaceImpl::addText(const DRW_Text& data){

	Drawing::Text newText;
	newText.m_text = QString::fromStdString(data.text);
	newText.m_basePoint = IBKMK::Vector2D(data.basePoint.x, data.basePoint.y);
	newText.m_zPosition = m_drawing->m_zCounter;
	m_drawing->m_zCounter++;
	newText.m_id = (*m_nextId)++;
	if (m_activeBlock != nullptr)
		newText.m_blockName = m_activeBlock->m_name;

	newText.m_lineWeight = DRW_LW_Conv::lineWidth2dxfInt(data.lWeight);
	newText.m_layerName = QString::fromStdString(data.layer);
	newText.m_height = data.height;
	newText.m_alignment = data.alignH == DRW_Text::HCenter ? Qt::AlignHCenter : Qt::AlignLeft;
	newText.m_rotationAngle = data.angle;

	/* value 256 means use defaultColor, value 7 is black */
	if(!(data.color == 256 || data.color == 7))
		newText.m_color = QColor(DRW::dxfColors[data.color][0],
								 DRW::dxfColors[data.color][1],
								 DRW::dxfColors[data.color][2]);
	else
		newText.m_color = QColor();

	m_drawing->m_texts.push_back(newText);
}
void DRW_InterfaceImpl::addDimAlign(const DRW_DimAligned */*data*/){}
void DRW_InterfaceImpl::addDimLinear(const DRW_DimLinear *data){

	// Line points
	const DRW_Coord &def1Point = data->getDef1Point();
	const DRW_Coord &def2Point = data->getDef2Point();
	const DRW_Coord &defPoint = data->getDefPoint();
	const DRW_Coord &textPoint = data->getTextPoint();

	// Definitions
	IBKMK::Vector2D def (defPoint.x, defPoint.y);
	IBKMK::Vector2D def1 (def1Point.x, def1Point.y);
	IBKMK::Vector2D def2 (def2Point.x, def2Point.y);
	IBKMK::Vector2D text (textPoint.x, textPoint.y);

	Drawing::LinearDimension newLinearDimension;

	newLinearDimension.m_zPosition = m_drawing->m_zCounter;
	m_drawing->m_zCounter++;
	newLinearDimension.m_id = (*m_nextId)++;
	if (m_activeBlock != nullptr)
		newLinearDimension.m_blockName = m_activeBlock->m_name;

	newLinearDimension.m_lineWeight = DRW_LW_Conv::lineWidth2dxfInt(data->lWeight);
	newLinearDimension.m_layerName = QString::fromStdString(data->layer);
	newLinearDimension.m_point1 = def1;
	newLinearDimension.m_point2 = def2;
	newLinearDimension.m_dimensionPoint = def;
	newLinearDimension.m_textPoint = text;
	newLinearDimension.m_angle = data->getAngle();
	newLinearDimension.m_measurement = data->getText().c_str();
	newLinearDimension.m_styleName = QString::fromStdString(data->getStyle());

	// value 256 means use defaultColor, value 7 is black
	if(!(data->color == 256 || data->color == 7))
		newLinearDimension.m_color = QColor(DRW::dxfColors[data->color][0],
											DRW::dxfColors[data->color][1],
											DRW::dxfColors[data->color][2]);
	else
		newLinearDimension.m_color = QColor();


	/// Linear dimension needs to be fully constructed.
	/// Dimension point can belongs to point 1 or 2 and has also an angle
	/// For more information look here:
	/// https://ezdxf.readthedocs.io/en/stable/tutorials/linear_dimension.html

	IBKMK::Vector2D xAxis(1, 0);
	IBKMK::Vector2D lineVec (std::cos(newLinearDimension.m_angle * IBK::DEG2RAD),
							 std::sin(newLinearDimension.m_angle * IBK::DEG2RAD));

	IBKMK::Vector2D lineVec2 (lineVec.m_y, -lineVec.m_x);

	const unsigned int SCALING_FACTOR = 1E6;

	IBKMK::Vector2D measurePoint1 = newLinearDimension.m_point1 + SCALING_FACTOR * lineVec2;
	IBKMK::Vector2D measurePoint2 = newLinearDimension.m_point2 + SCALING_FACTOR * lineVec2;

	IBK::Line lineMeasure (newLinearDimension.m_dimensionPoint -   SCALING_FACTOR * lineVec,
						   newLinearDimension.m_dimensionPoint + 2*SCALING_FACTOR * lineVec);

	IBK::Line lineLeft  (newLinearDimension.m_point1 - SCALING_FACTOR * lineVec2, measurePoint1);
	IBK::Line lineRight (newLinearDimension.m_point2 - SCALING_FACTOR * lineVec2, measurePoint2);

	IBKMK::Vector2D intersection1Left, intersection2Left;
	IBKMK::Vector2D intersection1Right, intersection2Right;
	bool intersect1 = lineMeasure.intersects(lineLeft, intersection1Left, intersection2Left) == 1;
	bool intersect2 = lineMeasure.intersects(lineRight, intersection1Right, intersection2Right) == 1;

	if (!intersect1 && !intersect2) {
		IBK::IBK_Message(IBK::FormatString("Linear dimension seems broken. Skipping."), IBK::MSG_WARNING);
		return;
	}

	IBKMK::Vector2D leftPoint, rightPoint;
	IBKMK::Vector2D point1, point2;
	bool left = false, right = false;
	if (intersect1 && (newLinearDimension.m_dimensionPoint - intersection1Left).magnitudeSquared() > 1E-3 ) {
		newLinearDimension.m_leftPoint = intersection1Left;
		newLinearDimension.m_rightPoint = newLinearDimension.m_dimensionPoint;
		left = true;
	}
	if (intersect2 && (newLinearDimension.m_dimensionPoint - intersection1Right).magnitudeSquared() > 1E-3 ) {
		newLinearDimension.m_leftPoint = newLinearDimension.m_dimensionPoint;
		newLinearDimension.m_rightPoint = intersection1Right;
		right = true;
	}

	if (!left && !right) {
		IBK::IBK_Message(IBK::FormatString("Linear dimension seems broken. Skipping."), IBK::MSG_WARNING);
		return;
	}

	m_drawing->m_linearDimensions.push_back(newLinearDimension);

}

void DRW_InterfaceImpl::addDimRadial(const DRW_DimRadial */*data*/){}
void DRW_InterfaceImpl::addDimDiametric(const DRW_DimDiametric */*data*/){}
void DRW_InterfaceImpl::addDimAngular(const DRW_DimAngular */*data*/){}
void DRW_InterfaceImpl::addDimAngular3P(const DRW_DimAngular3p */*data*/){}
void DRW_InterfaceImpl::addDimOrdinate(const DRW_DimOrdinate */*data*/){}
void DRW_InterfaceImpl::addLeader(const DRW_Leader */*data*/){}
void DRW_InterfaceImpl::addHatch(const DRW_Hatch */*data*/){}
void DRW_InterfaceImpl::addViewport(const DRW_Viewport& /*data*/){}
void DRW_InterfaceImpl::addImage(const DRW_Image */*data*/){}
void DRW_InterfaceImpl::linkImage(const DRW_ImageDef */*data*/){}
void DRW_InterfaceImpl::addComment(const char* /*comment*/){}

// no need to implement
void DRW_InterfaceImpl::writeHeader(DRW_Header& /*data*/){}
void DRW_InterfaceImpl::writeBlocks(){}
void DRW_InterfaceImpl::writeBlockRecords(){}
void DRW_InterfaceImpl::writeEntities(){}
void DRW_InterfaceImpl::writeLTypes(){}
void DRW_InterfaceImpl::writeLayers(){}
void DRW_InterfaceImpl::writeTextstyles(){}
void DRW_InterfaceImpl::writeVports(){}
void DRW_InterfaceImpl::writeDimstyles(){}
void DRW_InterfaceImpl::writeAppId(){}

