#ifndef __pqMADAILogoTextSourceStarter_h
#define __pqMADAILogoTextSourceStarter_h

#include "pqServer.h"

#include <QObject>

class vtkPVXMLElement;
class vtkSMProxyLocator;
class pqPipelineSource;

class pqMADAILogoTextSourceStarter : public QObject
{
  Q_OBJECT
  typedef QObject Superclass;

public:
  pqMADAILogoTextSourceStarter(QObject* p=0);
  ~pqMADAILogoTextSourceStarter();

  /** Callback for when a server is added. This should happen only
   * once, so when it does, we add a logo. Doing so before the server
   * exists results in an error. */

public slots:
  void onStartup();
  void onShutdown() {};

protected slots:
  void createSource();
  void newServerAdded();
  void removeLogoXMLElement(vtkPVXMLElement*);

private:
  pqMADAILogoTextSourceStarter(const pqMADAILogoTextSourceStarter&); // Not implemented.
  void operator=(const pqMADAILogoTextSourceStarter&); // Not implemented.

  pqPipelineSource* textSourceProxy;

  pqServer* associatedServer;
};

#endif
