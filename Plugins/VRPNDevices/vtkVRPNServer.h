#ifndef __vtkVRPNServer_h
#define __vtkVRPNServer_h

#include <QProcess>
#include <QTimer>

class vtkVRPNServer : public QObject
{
  Q_OBJECT

public:
  vtkVRPNServer();
  virtual ~vtkVRPNServer();

  void Start();
  void Stop();

private:
  vtkVRPNServer(const vtkVRPNServer&); // Not implemented
  void operator=(const vtkVRPNServer&); // Not implemented

  QProcess Process;
};

#endif // __vtkVRPNServer_h
