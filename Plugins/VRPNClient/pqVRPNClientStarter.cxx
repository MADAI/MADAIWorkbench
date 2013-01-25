#include "pqVRPNClientStarter.h"

#include <QDebug>

#include "vtkVRPNClient.h"

//-----------------------------------------------------------------------------
pqVRPNClientStarter::pqVRPNClientStarter(QObject* p/*=0*/)
  : QObject(p)
{
  this->Client = new vtkVRPNClient();
}

//-----------------------------------------------------------------------------
pqVRPNClientStarter::~pqVRPNClientStarter()
{
  if ( this->Client )
    {
    delete this->Client;
    }
}

//-----------------------------------------------------------------------------
void pqVRPNClientStarter::onStartup()
{
  this->Client->Start();
}

//-----------------------------------------------------------------------------
void pqVRPNClientStarter::onShutdown()
{
  this->Client->Stop();
}
