#include "vtkVRPNServer.h"

#include <iostream>

#include <QCoreApplication>

#include <vrpn_Connection.h>
#include <vrpn_3DConnexion.h>


//----------------------------------------------------------------------------
vtkVRPNServer::vtkVRPNServer()
{
  this->StartTries = 5;
}

//----------------------------------------------------------------------------
vtkVRPNServer::~vtkVRPNServer()
{
}

//----------------------------------------------------------------------------
void vtkVRPNServer::Start()
{
  this->Timer.start( 1000 );
  this->connect( &this->Timer, SIGNAL( timeout() ),
                 this, SLOT( TryToStartServer() ) );

}

//----------------------------------------------------------------------------
void vtkVRPNServer::TryToStartServer()
{

  if ( this->StartTries == 1 )
    {
    this->Timer.stop();
    }
  else
    {
    this->StartTries--;
    }

  QString appName = QCoreApplication::applicationDirPath();

  // WARNING!!! You must add quotation marks around the process name
  // on Mac OS X (and probably linux as well) to handle spaces in the
  // path correctly. Otherwise QProcess.start() will treat everything
  // after the space as argument(s).
  appName.prepend( "\"" );
  appName.append("/vrpn_server\"");

  this->Process.start( appName );
  this->Process.waitForStarted( 5000 );

  QProcess::ProcessError error = this->Process.error();
  switch ( error )
    {
    case QProcess::FailedToStart:
      std::cerr << "vrpn_server failed to start" << std::endl;
      break;

    case QProcess::Crashed:
      std::cerr << "vrpn_server crashed" << std::endl;
      break;

    case QProcess::Timedout:
      std::cerr << "vrpn_server timed out" << std::endl;
      break;

    case QProcess::WriteError:
      std::cerr << "vrpn_server write error" << std::endl;
      break;

    case QProcess::ReadError:
      std::cerr << "vrpn_server read error" << std::endl;
      break;

    default:
      this->Timer.stop();
      break;
    }
}

//----------------------------------------------------------------------------
void vtkVRPNServer::Stop()
{
  this->Process.terminate();
  this->Process.waitForFinished( 5000 );
}
