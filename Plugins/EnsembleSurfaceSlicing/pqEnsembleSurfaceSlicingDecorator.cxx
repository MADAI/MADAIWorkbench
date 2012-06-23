/*=========================================================================

   Program: ParaView
   Module:    pqEnsembleSurfaceSlicingDecorator.cxx

   Copyright (c) 2005,2006 Sandia Corporation, Kitware Inc.
   All rights reserved.

   ParaView is a free software; you can redistribute it and/or modify it
   under the terms of the ParaView license version 1.2.

   See License_v1.2.txt for the full ParaView license.
   A copy of this license can be obtained by contacting
   Kitware Inc.
   28 Corporate Drive
   Clifton Park, NY 12065
   USA

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHORS OR
CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

========================================================================*/
#include "pqEnsembleSurfaceSlicingDecorator.h"
#include "ui_pqEnsembleSurfaceSlicingDecorator.h"
// Server Manager Includes.

// Qt Includes.
#include <QVBoxLayout>

// ParaView Includes.
#include "pqDisplayPanel.h"
#include "pqDisplayProxyEditor.h"
#include "pqPipelineRepresentation.h"
#include "pqPropertyLinks.h"
#include "pqSMAdaptor.h"
#include "pqWidgetRangeDomain.h"
#include "vtkSMProxy.h"
#include "vtkSMProperty.h"

// VTK includes
#include <vtkEventQtSlotConnect.h>
#include <vtkSmartPointer.h>


class pqEnsembleSurfaceSlicingDecorator::pqInternals :
  public Ui::pqEnsembleSurfaceSlicingDecorator
{
public:
  pqPropertyLinks                        Links;
  vtkSMProxy*                            RepresentationProxy;
  vtkSmartPointer<vtkEventQtSlotConnect> VTKConnect;
  pqPipelineRepresentation*              PipelineRepresentation;
  pqWidgetRangeDomain*                   SliceWidthRangeDomain;
  pqWidgetRangeDomain*                   SliceDisplacementRangeDomain;

  pqInternals(QWidget* parent)
  {
    this->RepresentationProxy = NULL;
    this->VTKConnect = vtkSmartPointer<vtkEventQtSlotConnect>::New();
    this->SliceWidthRangeDomain = NULL;
    this->SliceDisplacementRangeDomain = NULL;
  }
};

//-----------------------------------------------------------------------------
pqEnsembleSurfaceSlicingDecorator::pqEnsembleSurfaceSlicingDecorator(pqDisplayPanel* displayPanel)
  : Superclass(displayPanel)
{
  this->setTitle( "Ensemble Surface Slicing" );

  pqDisplayProxyEditor* panel =
      qobject_cast<pqDisplayProxyEditor*> (displayPanel);
  pqRepresentation* representation = panel->getRepresentation();
  vtkSMProxy* representationProxy = (representation) ? representation->getProxy() : NULL;

  this->Internals = new pqInternals(this);
  QVBoxLayout* vlayout = qobject_cast<QVBoxLayout*>(panel->layout());
  if (vlayout)
    {
    vlayout->insertWidget(2, this);
    }
  else
    {
    panel->layout()->addWidget(this);
    }
  this->Internals->setupUi(this);
  this->Internals->RepresentationProxy = representationProxy;

  this->setupGUIConnections();

  this->setRepresentation(
    static_cast<pqPipelineRepresentation*>( panel->getRepresentation()));
  QObject::connect(&this->Internals->Links, SIGNAL(smPropertyChanged()), panel,
      SLOT(updateAllViews()), Qt::QueuedConnection);

}

//-----------------------------------------------------------------------------
pqEnsembleSurfaceSlicingDecorator::~pqEnsembleSurfaceSlicingDecorator()
{
  delete this->Internals;
}

//-----------------------------------------------------------------------------
void pqEnsembleSurfaceSlicingDecorator::setupGUIConnections()
{
  this->connect(this->Internals->SliceWidthEdit, SIGNAL(editingFinished()),
                SLOT(onSliceWidthChanged()));
}

//-----------------------------------------------------------------------------
void pqEnsembleSurfaceSlicingDecorator::setRepresentation(
  pqPipelineRepresentation* repr)
{
  if (this->Internals->PipelineRepresentation == repr)
    {
    return;
    }

  vtkSMProperty* prop;
  if (this->Internals->PipelineRepresentation)
    {
    // break all old links.
    this->Internals->Links.removeAllPropertyLinks();
    }

  this->Internals->PipelineRepresentation = repr;

  vtkSMProperty* sliceWidthProperty = this->Internals->RepresentationProxy->GetProperty( "SliceWidth" );
  if ( ! sliceWidthProperty )
    {
    std::cerr << "No SliceWidth property found!" << std::endl;
    return;
    }

  this->LinkWithRange(this->Internals->SliceWidthEdit, SIGNAL(valueChanged(double)),
                      sliceWidthProperty, this->Internals->SliceWidthRangeDomain);

  vtkSMProperty* sliceDisplacementProperty = this->Internals->RepresentationProxy->GetProperty( "SliceDisplacement" );
  if ( ! sliceDisplacementProperty )
    {
    std::cerr << "No SliceDisplacement property found!" << std::endl;
    return;
    }

  this->LinkWithRange(this->Internals->SliceDisplacementEdit, SIGNAL(valueChanged(double)),
                      sliceDisplacementProperty, this->Internals->SliceDisplacementRangeDomain);
}

//-----------------------------------------------------------------------------
void pqEnsembleSurfaceSlicingDecorator::LinkWithRange(QWidget* widget,
                                                      const char* signal,
                                                      vtkSMProperty* prop,
                                                      pqWidgetRangeDomain* & widgetRangeDomain)
{
  if (!prop || !widget)
    return;

  prop->UpdateDependentDomains();

  if (widgetRangeDomain != NULL)
    {
    delete widgetRangeDomain;
    }
  widgetRangeDomain = new pqWidgetRangeDomain(widget, "minimum", "maximum",
      prop);

  this->Internals->Links.addPropertyLink(widget, "value", signal,
      this->Internals->RepresentationProxy, prop);
}


//-----------------------------------------------------------------------------
void pqEnsembleSurfaceSlicingDecorator::onSliceWidthChanged()
{
  pqPipelineRepresentation* repr = this->Internals->PipelineRepresentation;
  vtkSMProxy* reprProxy = (repr) ? repr->getProxy() : NULL;
  if (!reprProxy)
    return;

  double sliceWidth = pqSMAdaptor::getElementProperty(reprProxy->GetProperty(
                                                        "SliceWidth")).toDouble();

  double newSliceWidth = this->Internals->SliceWidthEdit->value();

  reprProxy->UpdateProperty( "SliceWidth" );
  reprProxy->UpdateVTKObjects();

  if (this->Internals->PipelineRepresentation)
    {
    this->Internals->PipelineRepresentation->renderViewEventually();
    }
}
