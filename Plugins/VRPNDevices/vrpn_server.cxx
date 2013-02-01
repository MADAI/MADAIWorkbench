#include <vrpn_Connection.h>
#include <vrpn_3DConnexion.h>

#include <unistd.h>


int main( int argc, char * argv[] )
{
  vrpn_Connection * connection = vrpn_create_server_connection();

  vrpn_3DConnexion_Navigator * navigator =
    new vrpn_3DConnexion_Navigator( "spaceNavigator", connection );

  while ( true ) {
    navigator->mainloop();
    connection->mainloop();

    // Sleep for a millisecond
    usleep( 1000 );
  }

  return 0;
}
