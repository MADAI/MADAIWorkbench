#ifndef __vtkVRPNServer_h
#define __vtkVRPNServer_h

#include <QThread>

#include "vtkObject.h"

class vrpn_3DConnexion_Navigator;
class vrpn_Connection;

class vtkVRPNServer : public QThread
{
  Q_OBJECT
  typedef QThread Superclass;

public:
  vtkVRPNServer();
  virtual ~vtkVRPNServer();

  // Start the thread
  void Start();

  // Stop the thread
  void Stop();

protected slots:
  void run();

private:
  vtkVRPNServer(const vtkVRPNServer&); // Not implemented
  void operator=(const vtkVRPNServer&); // Not implemented

  bool Stopped;

  vrpn_3DConnexion_Navigator* SpaceNavigator;
};

#endif // __vtkVRPNServer_h
