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
  this->Server->Start();
}

//-----------------------------------------------------------------------------
void pqVRPNServerStarter::onShutdown()
{
  this->Server->Stop();
}
