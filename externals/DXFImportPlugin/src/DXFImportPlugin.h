#ifndef DXFImportPlugin_H
#define DXFImportPlugin_H

#include <SVImportPluginInterface.h>

#include <IBK_MessageHandler.h>

#define DXFImportPlugin_iid "de.dresden-tu.arch.ibk.DXFImportPlugin/1.0"

class DXFImportPlugin : public QObject, public SVImportPluginInterface
{
	Q_OBJECT
	Q_PLUGIN_METADATA(IID DXFImportPlugin_iid FILE "metadata.json")
	Q_INTERFACES(SVImportPluginInterface)

public:
	explicit DXFImportPlugin(QObject *parent = nullptr);
	virtual bool import(QWidget * parent, QString& projectText) override;
	virtual QString title() const override;
	virtual QString importMenuCaption() const override;
	virtual void setLanguage(QString langId, QString appname) override;
	QString DXFFileName() const;

private:
	QString					m_dxfFileName;
	IBK::MessageHandler		m_messageHandler;
};

#endif // DXFImportPlugin_H
