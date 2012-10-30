#include "vtkVRPNServer.h"

#include <iostream>

#include <vrpn_Connection.h>
#include <vrpn_3DConnexion.h>

//----------------------------------------------------------------------------
vtkVRPNServer::vtkVRPNServer()
{
  this->Stopped = true;
}

//----------------------------------------------------------------------------
vtkVRPNServer::~vtkVRPNServer()
{
}

//----------------------------------------------------------------------------
void vtkVRPNServer::Start()
{
  this->Stopped = false;
  this->start();
}

//----------------------------------------------------------------------------
void vtkVRPNServer::Stop()
{
  this->Stopped = true;
  QThread::wait();
}

//----------------------------------------------------------------------------
void vtkVRPNServer::run()
{
  vrpn_Connection * connection = vrpn_create_server_connection();
  vrpn_3DConnexion_Navigator *navigator =
    new vrpn_3DConnexion_Navigator( "device0", connection );

  while ( !this->Stopped )
    {
    navigator->mainloop();
    
    connection->mainloop();

    QThread::msleep( 10 );
    }

  delete navigator;
  delete connection;
}
