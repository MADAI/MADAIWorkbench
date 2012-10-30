#include "pqVRPNServerStarter.h"

#include <QDebug>

#include "vtkVRPNServer.h"

//-----------------------------------------------------------------------------
pqVRPNServerStarter::pqVRPNServerStarter(QObject* p/*=0*/)
  : QObject(p)
{
  this->Server = new vtkVRPNServer();
}

//-----------------------------------------------------------------------------
pqVRPNServerStarter::~pqVRPNServerStarter()
{
  if ( this->Server )
    {
    delete this->Server;
    }
}

//-----------------------------------------------------------------------------
void pqVRPNServerStarter::onStartup()
{
  qWarning() << "Starting VRPN server";

  this->Server->Start();
}

//-----------------------------------------------------------------------------
void pqVRPNServerStarter::onShutdown()
{
  qWarning() << "Stopping VRPN server";

  this->Server->Stop();
}
