#include "pqVRPNServerStarter.h"

#include <QDebug>

//-----------------------------------------------------------------------------
pqVRPNServerStarter::pqVRPNServerStarter(QObject* p/*=0*/)
  : QObject(p)
{
}

//-----------------------------------------------------------------------------
pqVRPNServerStarter::~pqVRPNServerStarter()
{
}

//-----------------------------------------------------------------------------
void pqVRPNServerStarter::onStartup()
{
  qWarning() << "Starting VRPN server";
}

//-----------------------------------------------------------------------------
void pqVRPNServerStarter::onShutdown()
{
  qWarning() << "Stopping VRPN server";
}
