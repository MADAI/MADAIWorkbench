#include "vtkVRPNServer.h"

#include <iostream>

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
  while ( !this->Stopped )
    {
    std::cout << "running" << std::endl;
    }
}
