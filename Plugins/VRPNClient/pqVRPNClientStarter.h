#ifndef __pqVRPNClientStarter_h
#define __pqVRPNClientStarter_h

#include <QObject>

class vtkVRPNClient;

class pqVRPNClientStarter : public QObject
{
  Q_OBJECT
  typedef QObject Superclass;

public:
  pqVRPNClientStarter(QObject* p=0);
  ~pqVRPNClientStarter();

  // Start up a VRPN server
  void onStartup();

  // Stop the VRPN server
  void onShutdown();

private:
  Q_DISABLE_COPY(pqVRPNClientStarter);

  vtkVRPNClient* Client;
};

#endif // __pqVRPNClientStarter_h
