#ifndef __vtkVRPNServer_h
#define __vtkVRPNServer_h

#include <QThread>

#include <vtkObject.h>
#include <vtkMutexLock.h>


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

  // Acquire the lock around VRPN calls
  void Lock();

  // Release lock around VRPN calls
  void Unlock();

protected slots:
  void run();

private:
  vtkVRPNServer(const vtkVRPNServer&); // Not implemented
  void operator=(const vtkVRPNServer&); // Not implemented

  bool Stopped;

  vtkSimpleMutexLock * Mutex;
};

#endif // __vtkVRPNServer_h
