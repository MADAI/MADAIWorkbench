#ifndef __pqVRPNServerStarter_h
#define __pqVRPNServerStarter_h

#include <QObject>

class pqVRPNServerStarter : public QObject
{
  Q_OBJECT
  typedef QObject Superclass;

public:
  pqVRPNServerStarter(QObject* p=0);
  ~pqVRPNServerStarter();

  // Start up a VRPN server
  void onStartup();

  // Stop the VRPN server
  void onShutdown();

private:
  Q_DISABLE_COPY(pqVRPNServerStarter);

};

#endif // __pqVRPNServerStarter_h
