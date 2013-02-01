#include "vtkVRPNServer.h"

#include <iostream>

#include <vrpn_Connection.h>
#include <vrpn_3DConnexion.h>


//----------------------------------------------------------------------------
vtkVRPNServer::vtkVRPNServer()
{
}

//----------------------------------------------------------------------------
vtkVRPNServer::~vtkVRPNServer()
{
}

//----------------------------------------------------------------------------
void vtkVRPNServer::Start()
{
  this->Process.start( "/Users/quammen/dev/madai/installers/MADAIWorkbench-1.5.0-Release/bin/MADAIWorkbench-build/bin/vrpn_server" );
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
      break;
    }
}

//----------------------------------------------------------------------------
void vtkVRPNServer::Stop()
{
  this->Process.terminate();
  this->Process.waitForFinished( 5000 );
}
