#include "vtkVRPNClient.h"

#include <iostream>

#include <vrpn_Connection.h>
#include <vrpn_3DConnexion.h>

#include <vtkCamera.h>
#include <vtkMath.h>
#include <vtkSMRenderViewProxy.h>

#include <pqActiveObjects.h>
#include <pqView.h>

#include "vtkVRPNServer.h"


//----------------------------------------------------------------------------
vtkVRPNClient::vtkVRPNClient()
{
  this->Stopped = true;

  this->EventSinceLastRender = false;

  this->Timer.setSingleShot( true );
  this->Timer.setInterval( 1 );

  connect( &this->Timer, SIGNAL( timeout() ), this, SLOT( render() ) );

  this->Server = NULL;

  this->Navigator = NULL;
}

//----------------------------------------------------------------------------
vtkVRPNClient::~vtkVRPNClient()
{
  this->Server->Lock();
  //delete this->Navigator;
  this->Server->Unlock();
}

//----------------------------------------------------------------------------
void vtkVRPNClient::SetServer( vtkVRPNServer * server )
{
  this->Server = server;
}

//----------------------------------------------------------------------------
void vtkVRPNClient::Start()
{
  if ( !this->Server )
    {
    std::cerr << "Server not set! Cannot start client." << std::endl;
    return;
    }

  if ( !this->Navigator )
    {
    this->Server->Lock();
    this->Navigator = new vrpn_Analog_Remote( "spaceNavigator@localhost" );
    this->Navigator->register_change_handler( this, AnalogChangeHandler );
    this->Server->Unlock();
    }

  this->Stopped = false;
  this->Timer.start();
}

//----------------------------------------------------------------------------
void vtkVRPNClient::Stop()
{
  this->Stopped = true;
  this->Timer.stop();
}

//----------------------------------------------------------------------------
void vtkVRPNClient::render()
{
  this->Server->Lock();
  this->Navigator->mainloop();
  this->Server->Unlock();

  if ( !this->EventSinceLastRender )
    {
    this->Timer.start();
    return;
    }

  pqView * view = pqActiveObjects::instance().activeView();
  if ( !view )
    {
    this->Timer.start();
    return;
    }

  vtkSMRenderViewProxy * viewProxy =
    vtkSMRenderViewProxy::SafeDownCast( view->getViewProxy() );
  if ( viewProxy )
    {
    viewProxy->StillRender();
    }

  this->EventSinceLastRender = false;

  // Restart the one-shot timer
  this->Timer.start();
}

//----------------------------------------------------------------------------
void VRPN_CALLBACK vtkVRPNClient
::AnalogChangeHandler( void * userData, const vrpn_ANALOGCB a )
{
  // Values for Space Navigator
  if ( a.num_channel != 6 )
    {
    return;
    }

  pqView * view = pqActiveObjects::instance().activeView();
  if ( !view )
    {
    return;
    }

  vtkSMRenderViewProxy * viewProxy =
    vtkSMRenderViewProxy::SafeDownCast( view->getViewProxy() );

  if ( viewProxy )
    {
    double pos[3], fp[3], up[3], dir[3];
    double orient[3];
    const double *channel = a.channel;

    vtkCamera * camera = viewProxy->GetActiveCamera();

    camera->GetPosition( pos );
    camera->GetFocalPoint( fp );
    camera->GetDirectionOfProjection( dir );
    camera->OrthogonalizeViewUp();
    camera->GetViewUp( up );

    double motionScale = 0.025;

    // Apply up-down motion
    for (int i = 0; i < 3; i++)
      {
      double dx = motionScale * channel[2]*up[i];
      pos[i] += dx;
      fp[i]  += dx;
      }

    // Apply right-left motion
    double r[3];
    vtkMath::Cross(dir, up, r);

    for (int i = 0; i < 3; i++)
      {
      double dx = -motionScale * channel[0]*r[i];
      pos[i] += dx;
      fp[i]  += dx;
      }

    camera->SetPosition(pos);
    camera->SetFocalPoint(fp);

    camera->Dolly(pow(1.01, channel[1]));
    camera->Elevation(  1.0*channel[3]);
    camera->Azimuth(    1.0*channel[5]);
    camera->Roll(       1.0*channel[4]);
    camera->OrthogonalizeViewUp();

    vtkVRPNClient * client = reinterpret_cast< vtkVRPNClient * >( userData );
    client->EventSinceLastRender = true;
    }
}
