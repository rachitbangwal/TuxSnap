#ifndef ALPSNAPGUI_H
#define ALPSNAPGUI_H

#include <QMainWindow>
#include <QListWidget>
#include <QPushButton>
#include <QStatusBar>
#include <QProcess>

class AlpSnapGui : public QMainWindow
{
    Q_OBJECT

public:
    explicit AlpSnapGui(QWidget *parent = nullptr);
    void checkAndPromptForSudo();

private slots:
    void createSnapshot();
    void restoreSnapshot();
    void removeSnapshot();
    void showSettings();
    void loadSnapshots();
    void handleScriptOutput(int exitCode, QProcess::ExitStatus exitStatus);
    void handleRestoreButtonState(QListWidgetItem *current);

private:
    QListWidget *snapshotList;
    QPushButton *createButton;
    QPushButton *restoreButton;
    QPushButton *removeButton;
    QPushButton *refreshButton;
    QPushButton *settingsButton;
    QStatusBar *statusBar;

    bool isRootUser();
    void runScriptCommand(const QStringList &arguments, const QString &successMessage);
    QString getAbsoluteScriptPath() const;
};

#endif
