#ifndef __vtkVRPNClient_h
#define __vtkVRPNClient_h

#include <QThread>
#include <QTimer>

#include "vtkObject.h"

#include <vrpn_Analog.h>

class vtkVRPNServer;


class vtkVRPNClient : public QObject
{
  Q_OBJECT

public:
  vtkVRPNClient();
  virtual ~vtkVRPNClient();

  // Set a reference to the server. This is needed to acquire the
  // server's lock before calling VRPN methods.
  void SetServer( vtkVRPNServer * server );

  // Start the thread
  void Start();

  // Stop the thread
  void Stop();

protected slots:
  void render();

private:
  vtkVRPNClient(const vtkVRPNClient&); // Not implemented
  void operator=(const vtkVRPNClient&); // Not implemented

  bool Stopped;

  bool EventSinceLastRender;

  QTimer Timer;

  vtkVRPNServer * Server;

  vrpn_Analog_Remote * Navigator;

  static void VRPN_CALLBACK AnalogChangeHandler( void * userData, const vrpn_ANALOGCB a );

};

#endif // __vtkVRPNClient_h
