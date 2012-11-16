#include "pqMADAILogoTextSourceStarter.h"

#include "vtkProcessModule.h"

#include "pqActiveObjects.h"
#include "pqApplicationCore.h"
#include "pqDataRepresentation.h"
#include "pqDisplayPolicy.h"
#include "pqObjectBuilder.h"
#include "pqPipelineSource.h"
#include "pqServerManagerModel.h"
#include "pqServerManagerObserver.h"

#include "vtkPVXMLElement.h"
#include "vtkSMProperty.h"
#include "vtkSMIntVectorProperty.h"
#include "vtkSMPropertyHelper.h"
#include "vtkSMProxy.h"
#include "vtkSMProxyIterator.h"
#include "vtkSMSessionProxyManager.h"

#include <QtDebug>

//-----------------------------------------------------------------------
pqMADAILogoTextSourceStarter::pqMADAILogoTextSourceStarter(QObject * p)
  : QObject(p), textSourceProxy(NULL), associatedServer(NULL)
{
  QObject::connect(pqApplicationCore::instance(),
                   SIGNAL(aboutToLoadState(vtkPVXMLElement*)),
                   this, SLOT(removeLogoXMLElement(vtkPVXMLElement*)));
  QObject::connect(pqApplicationCore::instance(),
                   SIGNAL(stateLoaded(vtkPVXMLElement*, vtkSMProxyLocator*)),
                   this, SLOT(createSource()), Qt::QueuedConnection);
}

//-----------------------------------------------------------------------
pqMADAILogoTextSourceStarter::~pqMADAILogoTextSourceStarter()
{
}

//-----------------------------------------------------------------------
void pqMADAILogoTextSourceStarter::onStartup()
{
  pqApplicationCore* core = pqApplicationCore::instance();
  QObject::connect(core->getServerManagerModel(),
                   SIGNAL(preServerAdded(pqServer*)),
                   this, SLOT(newServerAdded()));
}

//-----------------------------------------------------------------------
void pqMADAILogoTextSourceStarter::newServerAdded()
{
  QObject::connect(&pqActiveObjects::instance(),
                   SIGNAL(viewChanged(pqView*)),
                   this, SLOT(createSource()), Qt::QueuedConnection);
}

//-----------------------------------------------------------------------
void pqMADAILogoTextSourceStarter::createSource()
{
  pqView* view = pqActiveObjects::instance().activeView();
  if (view)
    {
    std::cout << "Object name: " << view->objectName().toStdString() << std::endl;
    pqApplicationCore* app = pqApplicationCore::instance();
    pqObjectBuilder* builder = app->getObjectBuilder();
    if ( !this->textSourceProxy || pqActiveObjects::instance().activeServer() != associatedServer )
      {
      this->textSourceProxy =
        builder->createSource("sources", "MADAILogoTextSource",
                              pqActiveObjects::instance().activeServer());
      this->associatedServer = pqActiveObjects::instance().activeServer();
      }

    this->textSourceProxy->rename(tr("MADAI Logo"));
    this->textSourceProxy->setModifiedState(pqProxy::UNMODIFIED);
    pqDisplayPolicy* displayPolicy =
      pqApplicationCore::instance()->getDisplayPolicy();
    displayPolicy->setRepresentationVisibility(
      this->textSourceProxy->getOutputPort(0), pqActiveObjects::instance().activeView(),
      true);

    // The text source is now the active object and its representation
    // is active.
    pqDataRepresentation *rep =
      pqActiveObjects::instance().activeRepresentation();
    if (rep)
      {
      // Set the logo properties here. For additional properties, see
      // ParaView/Qt/Components/pqTextDisplayPropertiesWidget.cxx.
      vtkSMProxy *repProxy = rep->getProxy();
      if ( repProxy->GetProperty( "Italic", 0 ) )
        {
        vtkSMPropertyHelper( repProxy, "Italic").Set( 1 );
        repProxy->UpdateVTKObjects();
        }
      }
    }

}

//-----------------------------------------------------------------------
void pqMADAILogoTextSourceStarter::removeLogoXMLElement(vtkPVXMLElement* xml)
{
  if (!xml)
    {
    return;
    }

  vtkPVXMLElement *sms = xml->FindNestedElementByName("ServerManagerState");
  if (sms)
    {
    // Iterate through the Proxies looking for MADAILogoTextSources
      for (unsigned int i = 0; i < sms->GetNumberOfNestedElements(); ++i)
        {
        vtkPVXMLElement *proxy = sms->GetNestedElement( i );
        if ( proxy && strcmp( proxy->GetName(), "Proxy" ) == 0 &&
             strcmp( proxy->GetAttribute( "type" ), "MADAILogoTextSource" ) == 0)
          {
            sms->RemoveNestedElement( proxy );
            i--;
          }
        }
    }
}
