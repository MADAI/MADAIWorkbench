#include "pqVRPNClientServerStarter.h"

#include <QDebug>

#include "vtkVRPNClient.h"
#include "vtkVRPNServer.h"


//-----------------------------------------------------------------------------
pqVRPNClientServerStarter::pqVRPNClientServerStarter(QObject* p/*=0*/)
  : QObject(p)
{
  this->Server = new vtkVRPNServer();
  this->Client = new vtkVRPNClient();
  this->Client->SetServer( this->Server );
}

//-----------------------------------------------------------------------------
pqVRPNClientServerStarter::~pqVRPNClientServerStarter()
{
  if ( this->Client )
    {
    delete this->Client;
    }

  if ( this->Server )
    {
    delete this->Server;
    }
}

//-----------------------------------------------------------------------------
void pqVRPNClientServerStarter::onStartup()
{
  this->Server->Start();
  this->Client->Start();
}

//-----------------------------------------------------------------------------
void pqVRPNClientServerStarter::onShutdown()
{
  this->Client->Stop();
  this->Server->Stop();
}
