#include "vtkVRPNServer.h"

#include <iostream>

#include <vrpn_Connection.h>
#include <vrpn_3DConnexion.h>


//----------------------------------------------------------------------------
vtkVRPNServer::vtkVRPNServer()
{
  this->Stopped = true;
  this->Mutex = new vtkSimpleMutexLock();
}

//----------------------------------------------------------------------------
vtkVRPNServer::~vtkVRPNServer()
{
  if ( this->Mutex )
    {
    delete this->Mutex;
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
void vtkVRPNServer::Lock()
{
  this->Mutex->Lock();
}

//----------------------------------------------------------------------------
void vtkVRPNServer::Unlock()
{
  this->Mutex->Unlock();
}

//----------------------------------------------------------------------------
void vtkVRPNServer::run()
{
  this->Lock();
  vrpn_Connection * connection = vrpn_create_server_connection();
  vrpn_3DConnexion_Navigator *navigator =
    new vrpn_3DConnexion_Navigator( "spaceNavigator", connection );
  this->Unlock();

  while ( !this->Stopped )
    {
    this->Lock();

    navigator->mainloop();
    
    connection->mainloop();

    this->Unlock();

    //QThread::msleep( 10 );
    }

  this->Lock();
  delete navigator;
  delete connection;
  this->Unlock();
}
