#ifndef TXTTHREAD
#define TXTTHREAD

#include <ctime>
#include <stdint.h>
#include <cstdio> // For popen()
#include <thread>
#include <string>

#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <iostream>

#include <QString>
#include <QDir>
#include <QThread>
#include <QFile>
#include <QTextStream>
#include <QVector>

#include <QProcess>
#include <QByteArray>

#include <QDebug>


class FileThread : public QThread
{
  Q_OBJECT;

public:
  FileThread();
  ~FileThread();
  
  void setFile();
  void createFile();
  bool pythonAI=true;


public slots:
  void GetImage();  
  void saveMatrix(QVector<int> tempImage);
  

private slots:
   
signals:
  void readyBBox();

private:
  QString Dir =  QString("/home/Thermal_Camera/AI_Temps.csv");
  QFile file;
  QVector<int> tempsImage;
  
};

#endif
