#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QRadioButton>
#include <QButtonGroup>
#include <QPushButton>

class SettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SettingsDialog(QWidget *parent = nullptr);

private slots:
    void loadConfig();
    void saveConfig();
    void onFieldChanged();

private:
    QLineEdit *snapRootEdit;
    QLineEdit *includePathsEdit;
    QLineEdit *excludePathsEdit;
    
    QRadioButton *encryptGpgRadio;
    QRadioButton *encryptNoneRadio;
    QButtonGroup *encryptGroup;
    
    QLineEdit *gpgRecipientEdit;
    
    QPushButton *applyButton;
    QPushButton *cancelButton;

    const QString configPath = "/etc/alpsnap.conf";

    QString parseValue(const QString &line) const;
};

#endif
