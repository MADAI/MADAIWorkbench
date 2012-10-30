#include "vtkVRPNServer.h"

#include <iostream>

#include <vrpn_Connection.h>
#include <vrpn_3DConnexion.h>

//----------------------------------------------------------------------------
vtkVRPNServer::vtkVRPNServer()
{
  this->Stopped = true;
  this->SpaceNavigator = NULL;
}

//----------------------------------------------------------------------------
vtkVRPNServer::~vtkVRPNServer()
{
  if ( this->SpaceNavigator )
    {
    delete this->SpaceNavigator;
    this->SpaceNavigator = NULL;
    }
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
  vrpn_Connection * connection = vrpn_create_server_connection( 7120 );
  this->SpaceNavigator = new vrpn_3DConnexion_Navigator( "device0", connection );

  while ( !this->Stopped )
    {
    std::cout << "running" << std::endl;

    this->SpaceNavigator->mainloop();
    
    connection->mainloop();

    QThread::msleep( 60 );
    }
}
