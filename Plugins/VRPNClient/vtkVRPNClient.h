#ifndef __vtkVRPNClient_h
#define __vtkVRPNClient_h

#include <QThread>

#include "vtkObject.h"

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
};

#endif // __vtkVRPNClient_h
