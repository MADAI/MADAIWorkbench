#include "pqLogoStarter.h"

#include "vtkProcessModule.h"

#include "pqActiveObjects.h"
#include "pqApplicationCore.h"
#include "pqDisplayPolicy.h"
#include "pqObjectBuilder.h"
#include "pqPipelineSource.h"
#include "pqServerManagerModel.h"
#include "pqServerManagerObserver.h"

#include <QtDebug>

//-----------------------------------------------------------------------------
pqLogoStarter::pqLogoStarter(QObject * p)
  : QObject(p)
{
}

//-----------------------------------------------------------------------------
pqLogoStarter::~pqLogoStarter()
{
}

//-----------------------------------------------------------------------------
void pqLogoStarter::onStartup()
{
  qWarning() << "Message from pqMyApplicationStarter: Application Started";

  pqApplicationCore* core = pqApplicationCore::instance();
  QObject::connect(core->getServerManagerModel(), SIGNAL(preServerAdded(pqServer*)),
                   this, SLOT(newServerAdded()));
}

//-----------------------------------------------------------------------------
void pqLogoStarter::newServerAdded()
{
  QObject::connect(&pqActiveObjects::instance(), SIGNAL(viewChanged(pqView*)),
                   this, SLOT(createSource()), Qt::QueuedConnection);
}

//-----------------------------------------------------------------------------
void pqLogoStarter::createSource()
{
  if (pqActiveObjects::instance().activeView())
    {
    QObject::disconnect(&pqActiveObjects::instance(), SIGNAL(viewChanged(pqView*)),
                        this, SLOT(createSource()));
    pqApplicationCore* app = pqApplicationCore::instance();
    pqObjectBuilder* builder = app->getObjectBuilder();
    pqPipelineSource *text = builder->createSource("sources",
                                                   "MADAILogoTextSource",
                                                   pqActiveObjects::instance().activeServer());
    text->rename(tr("MADAI Logo"));
    text->setModifiedState(pqProxy::UNMODIFIED);
    pqDisplayPolicy* displayPolicy = pqApplicationCore::instance()->getDisplayPolicy();
    displayPolicy->setRepresentationVisibility(
      text->getOutputPort(0), pqActiveObjects::instance().activeView(), true);
    }
}

