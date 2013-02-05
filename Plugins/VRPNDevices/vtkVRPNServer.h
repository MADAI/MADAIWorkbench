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

protected slots:
  void TryToStartServer();

private:
  vtkVRPNServer(const vtkVRPNServer&); // Not implemented
  void operator=(const vtkVRPNServer&); // Not implemented

  QProcess Process;

  // Timer for repeated attempts to start the server
  QTimer Timer;
};

#endif // __vtkVRPNServer_h
