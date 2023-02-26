#include "g19pluginsettings.hpp"
#include "g19daemon.hpp"

#include <QDialogButtonBox>
#include <QVBoxLayout>
#include <QPushButton>

G19PluginSettings::G19PluginSettings(G19daemon *parent)
        : QDialog(parent) {
    
    g19daemon = parent;
    setWindowTitle("Configure Plugins");

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    setLayout(mainLayout);

    tabWidget = new QTabWidget(this);
    mainLayout->addWidget(tabWidget);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel);
    QPushButton *okButton = buttonBox->button(QDialogButtonBox::Ok);
    okButton->setDefault(true);
    okButton->setShortcut(Qt::CTRL | Qt::Key_Return);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &G19PluginSettings::ok);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &G19PluginSettings::cancel);
    okButton->setDefault(true);

    mainLayout->addWidget(buttonBox);
}

G19PluginSettings::~G19PluginSettings() {
}

void G19PluginSettings::addPlugin(PluginInterface *plugin) {
    QWidget *w = plugin->getSettingsPane();

    if (w == nullptr) { return; }

    tabWidget->addTab(w, plugin->getName());

}

QSize G19PluginSettings::sizeHint() const {
    qDebug() << "sizehint";
    return QSize(640, 480);

}
void G19PluginSettings::ok() {
    g19daemon->savePluginSettings();
    close();
}

void G19PluginSettings::cancel() {
    close();
}
