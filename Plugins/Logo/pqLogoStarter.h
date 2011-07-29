#ifndef __pqLogoStarter_h
#define __pqLogoStarter_h

#include "pqServer.h"

#include <QObject>

class pqLogoStarter : public QObject
{
  Q_OBJECT
  typedef QObject Superclass;

public:
  pqLogoStarter(QObject* p=0);
  ~pqLogoStarter();

  /** Callback for when a server is added. This should happen only
   * once, so when it does, we add a logo. Doing so before the server
   * exists results in an error. */

public slots:
  void onStartup();
  void onShutdown() {};

protected slots:
  void createSource();
  void newServerAdded();


private:
  pqLogoStarter(const pqLogoStarter&); // Not implemented.
  void operator=(const pqLogoStarter&); // Not implemented.
};

#endif
