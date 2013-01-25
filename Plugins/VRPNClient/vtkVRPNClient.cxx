#include "vtkVRPNClient.h"

#include <iostream>

#include <vrpn_Connection.h>
#include <vrpn_3DConnexion.h>

//----------------------------------------------------------------------------
vtkVRPNClient::vtkVRPNClient()
{
  this->Stopped = true;
}

//----------------------------------------------------------------------------
vtkVRPNClient::~vtkVRPNClient()
{
}

//----------------------------------------------------------------------------
void vtkVRPNClient::Start()
{
  this->Stopped = false;
  this->start();
}

//----------------------------------------------------------------------------
void vtkVRPNClient::Stop()
{
  this->Stopped = true;
  QThread::wait();
}

//----------------------------------------------------------------------------
void vtkVRPNClient::run()
{
  vrpn_Analog_Remote * navigator = new vrpn_Analog_Remote( "spaceNavigator@localhost" );

  // TODO - hook up callbacks

  while ( !this->Stopped )
    {
    navigator->mainloop();
    
    //QThread::msleep( 10 );
    }

  delete navigator;
}
