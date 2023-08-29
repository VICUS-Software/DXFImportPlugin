#ifndef IFCImportPlugin_H
#define IFCImportPlugin_H

#include <SVImportPluginInterface.h>

#define DXFImportPlugin_iid "de.dresden-tu.arch.ibk.DXFImportPlugin/1.0"

class IFCImportPlugin : public QObject, public SVImportPluginInterface
{
	Q_OBJECT
	Q_PLUGIN_METADATA(IID DXFImportPlugin_iid FILE "metadata.json")
	Q_INTERFACES(SVImportPluginInterface)

public:
	explicit IFCImportPlugin(QObject *parent = nullptr);
	virtual bool import(QWidget * parent, QString& projectText) override;
	virtual QString title() const override;
	virtual QString importMenuCaption() const override;
	virtual void setLanguage(QString langId, QString appname) override;
	QString IFCFileName() const;

private:
	QString					m_ifcFileName;
};

#endif // IFCImportPlugin_H
