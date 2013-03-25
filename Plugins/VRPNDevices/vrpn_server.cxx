#include <vrpn_Connection.h>
#include <vrpn_3DConnexion.h>

#if defined( WIN32)
#include <WinBase.h>
#else
#include <unistd.h>
#endif


int main( int argc, char * argv[] )
{
  vrpn_Connection * connection = vrpn_create_server_connection();

  vrpn_3DConnexion_Navigator * navigator =
    new vrpn_3DConnexion_Navigator( "spaceNavigator", connection );

  while ( true ) {
    navigator->mainloop();
    connection->mainloop();

    // Sleep for a millisecond
#if defined( WIN32 )
    Sleep( 1 );
#else
    usleep( 1000 );
#endif
  }

  return 0;
}
