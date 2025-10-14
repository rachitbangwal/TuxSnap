#include "settingsdialog.h"
#include <QVBoxLayout>
#include <QFormLayout>
#include <QPushButton>
#include <QMessageBox>
#include <QFile>
#include <QTextStream>
#include <QDebug>

SettingsDialog::SettingsDialog(QWidget *parent) : QDialog(parent)
{
    setWindowTitle("AlpSnap Settings");
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    QFormLayout *formLayout = new QFormLayout;

    snapRootEdit = new QLineEdit;
    includePathsEdit = new QLineEdit;
    compressModeEdit = new QLineEdit;

    formLayout->addRow("SNAP_ROOT:", snapRootEdit);
    formLayout->addRow("INCLUDE_PATHS:", includePathsEdit);
    formLayout->addRow("COMPRESS_MODE:", compressModeEdit);

    mainLayout->addLayout(formLayout);

    QPushButton *saveButton = new QPushButton("Save");
    connect(saveButton, &QPushButton::clicked, this, &SettingsDialog::saveConfig);
    mainLayout->addWidget(saveButton);

    loadConfig();
}

void SettingsDialog::loadConfig()
{
    QFile file(configPath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) return;

    QTextStream in(&file);
    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        if (line.startsWith("#") || line.isEmpty()) continue;

        if (line.contains("SNAP_ROOT=")) snapRootEdit->setText(line.section('=', 1).trimmed().remove('"'));
        else if (line.contains("INCLUDE_PATHS=")) includePathsEdit->setText(line.section('=', 1).trimmed().remove('"'));
        else if (line.contains("COMPRESS_MODE=")) compressModeEdit->setText(line.section('=', 1).trimmed().remove('"'));
    }
    file.close();
}

void SettingsDialog::saveConfig()
{
    QFile file(configPath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
        QMessageBox::critical(this, "Error", "Could not open config for writing. Requires root privileges.");
        return;
    }

    QTextStream out(&file);
    out << "# alpsnap configuration\n\n";
    out << "SNAP_ROOT=\"" << snapRootEdit->text() << "\"\n";
    out << "SNAP_BACKEND=\"rsync\"\n\n";
    out << "INCLUDE_PATHS=\"" << includePathsEdit->text() << "\"\n\n";
    out << "COMPRESS_MODE=\"" << compressModeEdit->text() << "\"\n";
    
    file.close();
    
    QMessageBox::information(this, "Success", "Configuration saved.");
    accept(); 
}
