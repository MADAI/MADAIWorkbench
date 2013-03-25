#include "vtkVRPNClient.h"

#include <iostream>

#include <vrpn_Connection.h>
#include <vrpn_3DConnexion.h>

#include <vtkCamera.h>
#include <vtkMath.h>
#include <vtkSMRenderViewProxy.h>

#include <pqActiveObjects.h>
#include <pqView.h>


//----------------------------------------------------------------------------
vtkVRPNClient::vtkVRPNClient()
{
  this->EventSinceLastRender = false;

  this->Timer.setSingleShot( true );
  this->Timer.setInterval( 20 );

  connect( &this->Timer, SIGNAL( timeout() ), this, SLOT( Process() ) );

  this->Navigator = NULL;
  this->HasACallbackBeenCalled = false;
}

//----------------------------------------------------------------------------
vtkVRPNClient::~vtkVRPNClient()
{
}

//----------------------------------------------------------------------------
void vtkVRPNClient::Start()
{
  if ( !this->Navigator )
    {
    this->Navigator = new vrpn_Analog_Remote( "spaceNavigator@localhost" );
    this->Navigator->register_change_handler( this, AnalogChangeHandler );
    }

  this->Timer.start();

  this->StartTime = QTime::currentTime();
}

//----------------------------------------------------------------------------
void vtkVRPNClient::Stop()
{
  this->Timer.stop();

  delete this->Navigator;
  this->Navigator = NULL;
}

//----------------------------------------------------------------------------
void vtkVRPNClient::Process()
{
  QTime currentTime = QTime::currentTime();
  if ( this->StartTime.secsTo( currentTime ) > 10 && !this->HasACallbackBeenCalled )
    {
    // Give up trying to get events from server. The timer is
    // one-shot, so we don't need to do anything to stop it because it
    // is already stopped.
    return;
    }

  this->Navigator->mainloop();

  pqView * view = pqActiveObjects::instance().activeView();
  vtkSMRenderViewProxy * viewProxy = NULL;

  if ( this->EventSinceLastRender && view )
    {
    viewProxy = vtkSMRenderViewProxy::SafeDownCast( view->getViewProxy() );
    if ( viewProxy )
      {
      viewProxy->StillRender();
      }

    this->EventSinceLastRender = false;
    }

  // Restart the one-shot timer
  this->Timer.start();
}

//----------------------------------------------------------------------------
void VRPN_CALLBACK vtkVRPNClient
::AnalogChangeHandler( void * userData, const vrpn_ANALOGCB a )
{
  vtkVRPNClient * client = reinterpret_cast< vtkVRPNClient * >( userData );
  client->HasACallbackBeenCalled = true;

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
