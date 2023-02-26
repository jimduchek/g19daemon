#ifndef q19pluginsettings_H
#define q19pluginsettings_H


#include "plugininterface.hpp"
#include <QDialog>

namespace Ui {
class G19PluginSettings;
}

class G19PluginSettings : public QDialog {
Q_OBJECT
public:
  explicit G19PluginSettings(QWidget *parent = 0);
  ~G19PluginSettings();


};

#endif