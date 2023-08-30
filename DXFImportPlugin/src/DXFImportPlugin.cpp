#include "DXFImportPlugin.h"

#include "ImportDXFDialog.h"

#include <QFileDialog>
#include <QDir>
#include <QMessageBox>

#include <QtExt_Directories.h>
#include <QtExt_LanguageHandler.h>


const std::string VERSION = "1.0";

DXFImportPlugin::DXFImportPlugin(QObject *parent)
{
}


bool DXFImportPlugin::import(QWidget * parent, QString& projectText) {

	// ask for filename and check


	QString filename = QFileDialog::getOpenFileName(
				parent,
				tr("Select DXF file"),
				m_dxfFileName,
				tr("DXF files (*.dxf);;All files (*.*)"), nullptr );

	if (filename.isEmpty())
		return false;

	QFile f1(filename);
	if (!f1.exists()) {
		QMessageBox::critical(
					parent,
					tr("File not found"),
					tr("The file '%1' does not exist or cannot be accessed.").arg(filename)
					);
		m_dxfFileName.clear();
		return false;
	}

	m_dxfFileName = filename;

	// open dialog
	ImportDXFDialog diag(parent);
	ImportDXFDialog::ImportResults res = diag.importFile(filename);

	if (res == ImportDXFDialog::AddDrawings) {

		TiXmlDocument doc;
		TiXmlDeclaration * decl = new TiXmlDeclaration( "1.0", "UTF-8", "" );
		doc.LinkEndChild( decl );

		TiXmlElement * root = new TiXmlElement( "VicusProject" );
		doc.LinkEndChild(root);

		root->SetAttribute("fileVersion", VERSION);

		TiXmlElement * e = new TiXmlElement("Project");
		root->LinkEndChild(e);

		/* if file was read successfully, add drawing to project */
		Drawing dr = diag.drawing();
		dr.writeXML(e);

		// Declare a printer
		TiXmlPrinter printer;

		// attach it to the document you want to convert in to a std::string
		doc.Accept(&printer);

		// Create a std::string and copy your document data in to the string
		std::string str = printer.CStr();
		projectText = QString::fromStdString(str);

		return true;
	}

	return false;
}


QString DXFImportPlugin::title() const {
	return tr("Import DXF file");
}

QString DXFImportPlugin::importMenuCaption() const {
	return tr("Import DXF file ...");
}

void DXFImportPlugin::setLanguage(QString langId, QString appname) {
	QtExt::Directories::appname = appname;
	QtExt::Directories::devdir = appname;

	// initialize resources in dependent libraries
	Q_INIT_RESOURCE(QtExt);

	// *** Create log file directory and setup message handler ***
	QDir baseDir;
	baseDir.mkpath(QtExt::Directories::userDataDir());

	QtExt::LanguageHandler::instance().installTranslator(langId);
}

QString DXFImportPlugin::DXFFileName() const {
	return m_dxfFileName;
}

