#ifndef g19pluginsettings_H
#define g19pluginsettings_H


#include "plugininterface.hpp"
#include "g19daemon.hpp"
#include <QDialog>
#include <QTabWidget>


namespace Ui {
class G19PluginSettings;
}

class G19PluginSettings : public QDialog {
Q_OBJECT
public:
  explicit G19PluginSettings(G19daemon *parent = 0);
  ~G19PluginSettings();

  void addPlugin(PluginInterface *plugin);

  QSize sizeHint() const override;

public slots:
  void ok();
  void cancel();
private:
  QTabWidget *tabWidget;
  G19daemon *g19daemon;

};

#endif