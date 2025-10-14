#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>
#include <QLineEdit>

class SettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SettingsDialog(QWidget *parent = nullptr);

private slots:
    void loadConfig();
    void saveConfig();

private:
    QLineEdit *snapRootEdit;
    QLineEdit *includePathsEdit;
    QLineEdit *compressModeEdit;
    const QString configPath = "/etc/alpsnap.conf";
};

#endif
