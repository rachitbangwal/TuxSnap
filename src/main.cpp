#include "alpsnapgui.h"
#include <QApplication>
#include <QMessageBox>
#include <QProcess>
#include <unistd.h>
#include <QDir>
#include <QStandardPaths>

#ifndef ALPSNAP_SCRIPT_PATH
#define ALPSNAP_SCRIPT_PATH "./alpsnap.sh"
#endif

const char* const POLICY_FILE_NAME = "com.alpsnap.gui.policy";

bool isRoot() {
    return getuid() == 0;
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    
    if (!isRoot()) {
        
        QString appPath = QCoreApplication::applicationFilePath();
        
        QProcess process;
                
        QStringList sudoArgs;
        
        sudoArgs << "-E"; 
        
        sudoArgs << appPath; 
        
        for (int i = 1; i < argc; ++i) {
            sudoArgs << QString::fromLocal8Bit(argv[i]);
        }
        
        int result = process.execute("/usr/bin/sudo", sudoArgs);
        
        return result; 
    }
    
    AlpSnapGui w;
    w.show();

    return a.exec();
}
