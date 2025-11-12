#include "settingsdialog.h"
#include <QVBoxLayout>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <QMap>

SettingsDialog::SettingsDialog(QWidget *parent) : QDialog(parent)
{
    setWindowTitle("AlpSnap Settings");
    setMinimumWidth(500);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    QFormLayout *formLayout = new QFormLayout;

    snapRootEdit = new QLineEdit;
    snapRootEdit->setToolTip("The root directory to store snapshots (e.g., /snapshots)");
    
    includePathsEdit = new QLineEdit;
    includePathsEdit->setToolTip("Space-separated list of paths to back up (e.g., /root /home)");

    excludePathsEdit = new QLineEdit;
    excludePathsEdit->setToolTip("Space-separated list of paths to exclude (e.g., /var/log /tmp)");

    encryptGpgRadio = new QRadioButton("gpg");
    encryptNoneRadio = new QRadioButton("none");
    encryptGroup = new QButtonGroup(this);
    encryptGroup->addButton(encryptGpgRadio);
    encryptGroup->addButton(encryptNoneRadio);

    QHBoxLayout *radioLayout = new QHBoxLayout;
    radioLayout->addWidget(encryptGpgRadio);
    radioLayout->addWidget(encryptNoneRadio);
    radioLayout->addStretch();

    gpgRecipientEdit = new QLineEdit;
    gpgRecipientEdit->setToolTip("The GPG recipient ID for encryption (email, key ID, etc.)");

    formLayout->addRow("SNAP_ROOT:", snapRootEdit);
    formLayout->addRow("INCLUDE_PATHS:", includePathsEdit);
    formLayout->addRow("EXCLUDE_PATHS:", excludePathsEdit);
    formLayout->addRow("ENCRYPT_MODE:", radioLayout);
    formLayout->addRow("GPG_RECIPIENT:", gpgRecipientEdit);

    mainLayout->addLayout(formLayout);

    applyButton = new QPushButton("Apply");
    cancelButton = new QPushButton("Cancel");
    applyButton->setEnabled(false);

    QHBoxLayout *buttonLayout = new QHBoxLayout;
    buttonLayout->addStretch();
    buttonLayout->addWidget(cancelButton);
    buttonLayout->addWidget(applyButton);

    mainLayout->addLayout(buttonLayout);

    connect(cancelButton, &QPushButton::clicked, this, &SettingsDialog::reject);
    connect(applyButton, &QPushButton::clicked, this, &SettingsDialog::saveConfig);

    connect(snapRootEdit, &QLineEdit::textChanged, this, &SettingsDialog::onFieldChanged);
    connect(includePathsEdit, &QLineEdit::textChanged, this, &SettingsDialog::onFieldChanged);
    connect(excludePathsEdit, &QLineEdit::textChanged, this, &SettingsDialog::onFieldChanged);
    connect(gpgRecipientEdit, &QLineEdit::textChanged, this, &SettingsDialog::onFieldChanged);
    connect(encryptGpgRadio, &QRadioButton::toggled, this, &SettingsDialog::onFieldChanged);

    loadConfig();
}

QString SettingsDialog::parseValue(const QString &line) const
{
    return line.section('=', 1).trimmed().remove('"');
}

void SettingsDialog::loadConfig()
{
    QFile file(configPath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "Warning", QString("Could not read config file: %1").arg(configPath));
        return;
    }

    QMap<QString, QString> config;
    QTextStream in(&file);
    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        if (line.startsWith("#") || line.isEmpty() || !line.contains('=')) {
            continue;
        }

        QString key = line.section('=', 0, 0).trimmed();
        QString value = parseValue(line);
        config[key] = value;
    }
    file.close();

    snapRootEdit->setText(config.value("SNAP_ROOT"));
    includePathsEdit->setText(config.value("INCLUDE_PATHS"));
    excludePathsEdit->setText(config.value("EXCLUDE_PATHS"));
    gpgRecipientEdit->setText(config.value("GPG_RECIPIENT"));

    if (config.value("ENCRYPT_MODE") == "gpg") {
        encryptGpgRadio->setChecked(true);
    } else {
        encryptNoneRadio->setChecked(true);
    }

    applyButton->setEnabled(false);
}

void SettingsDialog::saveConfig()
{

    QFile file(configPath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
        QMessageBox::critical(this, "Error", "Could not open config for writing. Requires root privileges.");
        return;
    }

    QString snapRoot = snapRootEdit->text();
    QString includePaths = includePathsEdit->text();
    QString excludePaths = excludePathsEdit->text();
    QString encryptMode = encryptGpgRadio->isChecked() ? "gpg" : "none";
    QString gpgRecipient = gpgRecipientEdit->text();

    QTextStream out(&file);
    
    out << "# alpsnap configuration\n\n";
    out << "SNAP_ROOT=\"" << snapRoot << "\"\n";
    out << "SNAP_BACKEND=\"rsync\"\n\n";
    out << "INCLUDE_PATHS=\"" << includePaths << "\"\n\n";
    out << "EXCLUDE_PATHS=\"" << excludePaths << "\"\n\n";
    out << "LOG_FILE=\"/var/log/alpsnap.log\"\n\n";
    out << "# Snapshot ID format\n";
    out << "TIME_FMT=\"%Y%m%d-%H%M%S\"\n\n";
    out << "# Compression: zstd | tar | none\n";
    out << "COMPRESS_MODE=\"zstd\"\n\n";
    out << "# Encryption: gpg | none\n";
    out << "ENCRYPT_MODE=\"" << encryptMode << "\"\n";
    out << "GPG_RECIPIENT=\"" << gpgRecipient << "\"\n";

    file.close();

    QMessageBox::information(this, "Success", "Configuration saved.");
    accept();
}

void SettingsDialog::onFieldChanged()
{
    applyButton->setEnabled(true);
}
