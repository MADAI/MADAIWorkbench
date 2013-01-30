#include "vtkVRPNServer.h"

#include <iostream>

#include <vrpn_Connection.h>
#include <vrpn_3DConnexion.h>


//----------------------------------------------------------------------------
vtkVRPNServer::vtkVRPNServer()
{
  this->Timer.setSingleShot( true );
  this->Timer.setInterval( 2 );

  connect( &this->Timer, SIGNAL( timeout() ), this, SLOT( Process() ) );

  this->Connection = NULL;
  this->Navigator = NULL;
}

//----------------------------------------------------------------------------
vtkVRPNServer::~vtkVRPNServer()
{
  if ( this->Navigator )
    {
    delete this->Navigator;
    }
}

//----------------------------------------------------------------------------
void vtkVRPNServer::Start()
{
  if ( !this->Connection )
    {
    this->Connection = vrpn_create_server_connection();
    }
  if ( !this->Navigator )
    {
    this->Navigator = new vrpn_3DConnexion_Navigator( "spaceNavigator",
                                                      this->Connection );
    }
  this->Timer.start();
}

//----------------------------------------------------------------------------
void vtkVRPNServer::Stop()
{
  this->Timer.stop();
}

//----------------------------------------------------------------------------
void vtkVRPNServer::Process()
{
  if ( this->Navigator )
    {
    this->Navigator->mainloop();  
    }

  if ( this->Connection )
    {
    this->Connection->mainloop();
    }

  this->Timer.start();
}
