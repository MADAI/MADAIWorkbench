#ifndef __pqVRPNClientServerStarter_h
#define __pqVRPNClientServerStarter_h

#include <QObject>

class vtkVRPNClient;
class vtkVRPNServer;


class pqVRPNClientServerStarter : public QObject
{
  Q_OBJECT
  typedef QObject Superclass;

public:
  pqVRPNClientServerStarter(QObject* p=0);
  ~pqVRPNClientServerStarter();

  // Start up a VRPN server
  void onStartup();

  // Stop the VRPN server
  void onShutdown();

private:
  Q_DISABLE_COPY(pqVRPNClientServerStarter);

  vtkVRPNClient* Client;

  vtkVRPNServer* Server;
};

#endif // __pqVRPNClientServerStarter_h
