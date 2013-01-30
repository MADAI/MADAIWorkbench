#ifndef __vtkVRPNServer_h
#define __vtkVRPNServer_h

#include <QTimer>

#include <vtkObject.h>
#include <vtkMutexLock.h>

#include <vrpn_3DConnexion.h>


class vtkVRPNServer : public QObject
{
  Q_OBJECT

public:
  vtkVRPNServer();
  virtual ~vtkVRPNServer();

  void Start();
  void Stop();

protected slots:
  void Process();

private:
  vtkVRPNServer(const vtkVRPNServer&); // Not implemented
  void operator=(const vtkVRPNServer&); // Not implemented

  QTimer Timer;

  vrpn_Connection * Connection;

  vrpn_3DConnexion_Navigator * Navigator;
};

#endif // __vtkVRPNServer_h
