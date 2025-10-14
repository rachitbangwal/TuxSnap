#include "alpsnapgui.h"
#include "settingsdialog.h"
#include <QApplication>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QWidget>
#include <QMessageBox>
#include <QProcess>
#include <QDebug>
#include <QFile>
#include <QTextStream>
#include <QDialog>
#include <QFormLayout>
#include <QLineEdit>
#include <QTimer>
#include <unistd.h>
#include <QDir>

#ifndef ALPSNAP_SCRIPT_PATH
#define ALPSNAP_SCRIPT_PATH "./alpsnap.sh"
#endif

// --- Main Window Implementation ---

AlpSnapGui::AlpSnapGui(QWidget *parent) : QMainWindow(parent)
{
    setWindowTitle("AlpSnap GUI");
    setMinimumSize(600, 400);

    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
    QHBoxLayout *topBarLayout = new QHBoxLayout;

    // 1. Buttons
    createButton = new QPushButton("Create Snapshot");
    restoreButton = new QPushButton("Restore Snapshot");
    settingsButton = new QPushButton("Settings");

    topBarLayout->addWidget(createButton);
    topBarLayout->addWidget(restoreButton);
    topBarLayout->addStretch(1);
    topBarLayout->addWidget(settingsButton);

    restoreButton->setEnabled(false);

    // 2. Snapshot List
    snapshotList = new QListWidget;
    snapshotList->setToolTip("List of available snapshots (Format: YYYYMMDD-HHMMSS)");

    mainLayout->addLayout(topBarLayout);
    mainLayout->addWidget(snapshotList);
    
    // Status Bar
    statusBar = new QStatusBar(this);
    setStatusBar(statusBar);
    statusBar->showMessage("Awaiting user action...");

    // Connects
    connect(createButton, &QPushButton::clicked, this, &AlpSnapGui::createSnapshot);
    connect(restoreButton, &QPushButton::clicked, this, &AlpSnapGui::restoreSnapshot);
    connect(settingsButton, &QPushButton::clicked, this, &AlpSnapGui::showSettings);
    connect(snapshotList, &QListWidget::currentItemChanged, this, &AlpSnapGui::handleRestoreButtonState);
    
    // Load snapshots on start
    QTimer::singleShot(100, this, &AlpSnapGui::loadSnapshots);
}

QString AlpSnapGui::getAbsoluteScriptPath() const
{
    QDir appDir(QCoreApplication::applicationDirPath());
    
    return appDir.filePath(ALPSNAP_SCRIPT_PATH);
}

bool AlpSnapGui::isRootUser()
{
    return getuid() == 0;
}

// Security: Functions for running script with elevated privileges 

void AlpSnapGui::runScriptCommand(const QStringList &arguments, const QString &successMessage)
{
    if (!isRootUser()) {
        QMessageBox::critical(this, "Privilege Error", "The application must be run as root (via sudo) to perform this action.");
        return;
    }

    QString absoluteScriptPath = getAbsoluteScriptPath(); 

    statusBar->showMessage(QString("Executing: %1 %2...").arg(absoluteScriptPath).arg(arguments.join(" ")));
    createButton->setEnabled(false);
    restoreButton->setEnabled(false);

    QProcess *process = new QProcess(this);
    process->start("bash", QStringList() << absoluteScriptPath << arguments);

    connect(process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), 
        [=, this](int exitCode, QProcess::ExitStatus exitStatus) {
        handleScriptOutput(exitCode, exitStatus);
        
        if (exitStatus == QProcess::NormalExit && exitCode == 0) {
            QMessageBox::information(this, "Success", successMessage);
            loadSnapshots();
        } else {
            QString errorOutput = QString::fromUtf8(process->readAllStandardError());
            QMessageBox::critical(this, "Operation Failed", 
                                  QString("Script failed with exit code %1.\nError: %2").arg(exitCode).arg(errorOutput.trimmed()));
        }

        createButton->setEnabled(true);
        handleRestoreButtonState(snapshotList->currentItem());
        process->deleteLater();
    });
}

void AlpSnapGui::handleScriptOutput(int exitCode, QProcess::ExitStatus exitStatus)
{
    if (exitStatus == QProcess::NormalExit && exitCode == 0) {
        statusBar->showMessage("Operation successful.");
    } else {
        statusBar->showMessage(QString("Operation failed (Exit Code: %1)").arg(exitCode), 5000);
    }
}


void AlpSnapGui::loadSnapshots()
{
    if (!isRootUser()) {
        statusBar->showMessage("App not running as root. Cannot load snapshots.", 5000);
        return;
    }
    
    statusBar->showMessage("Loading snapshots...");
    snapshotList->clear();
    
    QString absoluteScriptPath = getAbsoluteScriptPath(); 
    
    QProcess *process = new QProcess(this);
    process->start("bash", QStringList() << absoluteScriptPath << "list");

    connect(process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), 
        [=, this](int exitCode, QProcess::ExitStatus exitStatus) {
        if (exitStatus == QProcess::NormalExit && exitCode == 0) {
            QString output = QString::fromUtf8(process->readAllStandardOutput()).trimmed();
            QStringList snapshots = output.split('\n', Qt::SkipEmptyParts);
            
            if (snapshots.isEmpty()) {
                 QListWidgetItem *item = new QListWidgetItem("No snapshots found.", snapshotList);
                 item->setFlags(item->flags() & ~Qt::ItemIsSelectable); // Make it non-selectable
                 statusBar->showMessage("No snapshots found.", 3000);
            } else {
                snapshotList->addItems(snapshots);
                statusBar->showMessage(QString("Loaded %1 snapshots.").arg(snapshots.count()), 3000);
            }

        } else {
            QString errorOutput = QString::fromUtf8(process->readAllStandardError()).trimmed();
            QMessageBox::critical(this, "List Error", 
                                 QString("Failed to list snapshots. Check script logs.\nError: %1").arg(errorOutput));
            statusBar->showMessage("Failed to load snapshots.", 5000);
        }
        process->deleteLater();
    });
}

void AlpSnapGui::createSnapshot()
{
    if (QMessageBox::question(this, "Confirm Creation", "Are you sure you want to create a new snapshot?", 
                              QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes) {
        runScriptCommand(QStringList() << "snapshot", "Snapshot creation started. Check logs for progress.");
    }
}

void AlpSnapGui::restoreSnapshot()
{
    QListWidgetItem *selected = snapshotList->currentItem();
    if (!selected) return;

    QString sid = selected->text();

    if (QMessageBox::warning(this, "Confirm Restoration", 
                             QString("WARNING: This will overwrite your system files with snapshot '%1'. Continue?").arg(sid), 
                             QMessageBox::Yes | QMessageBox::No, QMessageBox::No) == QMessageBox::Yes) {
        runScriptCommand(QStringList() << "restore" << sid, QString("Restoration of %1 initiated. System files are being overwritten. Reboot might be required after completion.").arg(sid));
    }
}

void AlpSnapGui::showSettings()
{
    SettingsDialog dialog(this);
    dialog.exec();
}

void AlpSnapGui::handleRestoreButtonState(QListWidgetItem *current)
{
    restoreButton->setEnabled(current != nullptr && (current->flags() & Qt::ItemIsSelectable));
}
