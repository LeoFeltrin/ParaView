/*=========================================================================

   Program: ParaView
   Module:    pqServer.h

   Copyright (c) 2005-2008 Sandia Corporation, Kitware Inc.
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

=========================================================================*/
#ifndef _pqServer_h
#define _pqServer_h

class vtkObject;
class pqTimeKeeper;
class vtkProcessModule;
class vtkPVOptions;
class vtkPVServerInformation;
class vtkPVXMLElement;
class vtkSMApplication;
class vtkSMProxy;
class vtkSMProxyManager;
class vtkSMRenderViewProxy;
class vtkSMSession;

#include "pqCoreExport.h"
#include "pqServerManagerModelItem.h"
#include "pqServerResource.h"
#include "vtkSmartPointer.h"
#include "vtkSMMessageMinimal.h"
#include "vtkWeakPointer.h"
#include <QPointer>
#include <QTimer>

/// pqServer (should be renamed to pqSession) is a pqServerManagerModelItem
/// subclass that represents a vtkSMSession. Besides providing API to access
/// vtkSMSession, it also performs some initialization of session-related
/// proxies such as time-keeper and global-mapper-properties proxies.
class PQCORE_EXPORT pqServer : public pqServerManagerModelItem 
{
  Q_OBJECT
public:  
  pqServer(vtkIdType connectionId, vtkPVOptions*, QObject* parent = NULL);
  virtual ~pqServer();

  const pqServerResource& getResource();
  void setResource(const pqServerResource &server_resource);

  /// Returns the session instance which the pqServer represents.
  vtkSMSession* session() const;

  /// Returns the connection id for the server connection.
  vtkIdType GetConnectionID() const;

  /// Returns the proxy manager for this session.
  vtkSMProxyManager* proxyManager() const;

  /// Return the number of data server partitions on this 
  /// server connection. A convenience method.
  int getNumberOfPartitions();

  /// Returns is this connection is a connection to a remote
  /// server or a built-in server.
  bool isRemote() const;

  /// Returns true is this connection has a separate render-server and
  /// data-server.
  bool isRenderServerSeparate();

  /// Returns the time keeper for this connection.
  pqTimeKeeper* getTimeKeeper() const;

  /// Initializes the pqServer, must be called as soon as pqServer 
  /// is created.
  void initialize();

  /// Returns the PVOptions for this connection. These are client side options.
  vtkPVOptions* getOptions() const;

  /// Returns the vtkPVServerInformation object which contains information about
  /// the command line options specified on the remote server, if any.
 vtkPVServerInformation* getServerInformation() const;

  /// Get/Set the application wide heart beat timeout setting.
  /// Heartbeats are used in case of remote server connections to avoid the
  /// connection timing out due to inactivity. When set, the client send a
  /// heartbeat message to all servers every \c msec milliseconds.
  static void setHeartBeatTimeoutSetting(int msec);
  static int getHeartBeatTimeoutSetting();

  /// Get/Set the application wide coincident topology resolution settings.
  static void setCoincidentTopologyResolutionModeSetting(int mode);
  static int coincidentTopologyResolutionModeSetting();
  static void setPolygonOffsetParametersSetting(double factor, double value);
  static void polygonOffsetParametersSetting(double &factor, double &value);
  static void setPolygonOffsetFacesSetting(bool);
  static bool polygonOffsetFacesSetting();
  static void setZShiftSetting(double shift);
  static double zShiftSetting();

  /// Get/Set global immediate mode rendering.
  static void setGlobalImmediateModeRenderingSetting(bool val);
  static bool globalImmediateModeRenderingSetting();


signals:
  /// Fired when the name of the proxy is changed.
  void nameChanged(pqServerManagerModelItem*);

  /// Fired about 5 minutes before the server timesout. This signal will not be
  /// fired at all if server timeout < 5 minutes. The server timeout is
  /// specified by --timeout option on the server process.
  /// This is not fired if timeout is not specified on the server process.
  void fiveMinuteTimeoutWarning();

  /// Fired about 1 minute before the server timesout. 
  /// The server timeout is specified by --timeout option on the server process.
  /// This is not fired if timeout is not specified on the server process.
  void finalTimeoutWarning();

protected:
  // Creates the TimeKeeper proxy for this connection.
  void createTimeKeeper();

  /// Returns the string key used for the heart beat time interval.
  static const char* HEARBEAT_TIME_SETTING_KEY();

  /// Set the heartbeat timeout for this instance of pqServer.
  void setHeartBeatTimeout(int msec);
  void setCoincidentTopologyResolutionMode(int);
  void setPolygonOffsetParameters(double factor, double units);
  void setPolygonOffsetFaces(bool offset_faces);
  void setZShift(double shift);
  void setGlobalImmediateModeRendering(bool);

  // updates all servers with the current settings.
  static void updateGlobalMapperProperties();

  // ---- Collaboration client-to-clients communication mechanisme ----

signals:
  /// Allow user to listen messages from other clients.
  /// But if you plan to push some state by for example calling
  /// the sendToOtherClients(vtkSMMessage*) slot, you MUST queued your slot.
  /// Otherwise your communication will not be sent to the server.
  /// Here is a code sample on how to connect to that signal:
  ///
  ///    QObject::connect( server, SIGNAL(sentFromOtherClient(vtkSMMessage*)),
  ///                      this,   SLOT(onClientMessage(vtkSMMessage*)),
  ///                      Qt::QueuedConnection);
  void sentFromOtherClient(vtkSMMessage* msg);

  /// Signal triggered when user information get updated
  void triggeredMasterUser(int);
  void triggeredUserName(int, QString&);
  void triggeredUserListChanged();

public slots:
  /// Allow user to broadcast to other client a given message
  void sendToOtherClients(vtkSMMessage* msg);

  // ---- Collaboration client-to-clients communication mechanisme ----

protected slots:
  /// Called to send a heartbeat to the server.
  void heartBeat();

  /// Called when idle to look for server notification for collaboration purpose
  void processServerNotification();

  /// Called by vtkSMCollaborationManager when associated message happen.
  /// This will convert the given parameter into vtkSMMessage and
  /// emit sentFromOtherClient(vtkSMMessage*) signal.
  void onCollaborationCommunication(vtkObject*, unsigned long, void*, void*);

private:
  pqServer(const pqServer&);  // Not implemented.
  pqServer& operator=(const pqServer&); // Not implemented.

  pqServerResource Resource;
  vtkIdType ConnectionID;
  vtkWeakPointer<vtkSMProxy> GlobalMapperPropertiesProxy;
  vtkWeakPointer<vtkSMSession> Session;

  // TODO:
  // Each connection will eventually have a PVOptions object. 
  // For now, this is same as the vtkProcessModule::Options.
  vtkSmartPointer<vtkPVOptions> Options;

  QTimer IdleCollaborationTimer;

  class pqInternals;
  pqInternals* Internals;
};

#endif // !_pqServer_h
