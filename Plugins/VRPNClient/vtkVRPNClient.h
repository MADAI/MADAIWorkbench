#ifndef __vtkVRPNClient_h
#define __vtkVRPNClient_h

#include <QThread>

#include "vtkObject.h"

#include <vrpn_Analog.h>


class vtkVRPNClient : public QThread
{
  Q_OBJECT
  typedef QThread Superclass;

public:
  vtkVRPNClient();
  virtual ~vtkVRPNClient();

  // Start the thread
  void Start();

  // Stop the thread
  void Stop();

protected slots:
  void run();

private:
  vtkVRPNClient(const vtkVRPNClient&); // Not implemented
  void operator=(const vtkVRPNClient&); // Not implemented

  bool Stopped;

  static void VRPN_CALLBACK AnalogChangeHandler( void * userData, const vrpn_ANALOGCB a );

};

#endif // __vtkVRPNClient_h
