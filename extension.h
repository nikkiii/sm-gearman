/**
 * vim: set ts=4 :
 * =============================================================================
 * SourceMod Sample Extension
 * Copyright (C) 2004-2008 AlliedModders LLC.  All rights reserved.
 * =============================================================================
 *
 * This program is free software; you can redistribute it and/or modify it under
 * the terms of the GNU General Public License, version 3.0, as published by the
 * Free Software Foundation.
 * 
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * As a special exception, AlliedModders LLC gives you permission to link the
 * code of this program (as well as its derivative works) to "Half-Life 2," the
 * "Source Engine," the "SourcePawn JIT," and any Game MODs that run on software
 * by the Valve Corporation.  You must obey the GNU General Public License in
 * all respects for all other code used.  Additionally, AlliedModders LLC grants
 * this exception to all derivative works.  AlliedModders LLC defines further
 * exceptions, found in LICENSE.txt (as of this writing, version JULY-31-2007),
 * or <http://www.sourcemod.net/license.php>.
 *
 * Version: $Id$
 */

#ifndef _INCLUDE_SOURCEMOD_EXTENSION_PROPER_H_
#define _INCLUDE_SOURCEMOD_EXTENSION_PROPER_H_

/**
 * @file extension.h
 * @brief Sample extension code header.
 */

#include "smsdk_ext.h"

#include <IThreader.h>
#include <sm_queue.h>

gearman_return_t Gearman_CallWorker(gearman_job_st *job, void *context);

class GearmanWorkerThread;

enum GearmanPriority {
	GearmanPriority_Low,
	GearmanPriority_Normal,
	GearmanPriority_High
};

enum GearmanResp {
	GearmanResp_Data,
	GearmanResp_Warning,
	GearmanResp_Complete,
	GearmanResp_Exception
};

struct gearman_worker_cb {
	IPluginContext *pContext;
	funcid_t funcid;
};

struct gearman_client_ctx {
	IPluginContext *pContext;
	gearman_client_st *client;
	funcid_t createdFunc;
};

struct gearman_worker_ctx {
	IPluginContext *pContext;
	gearman_worker_st *worker;
	GearmanWorkerThread *thread;
};

struct gearman_task_ctx {
	IPluginContext *pContext;
	gearman_client_ctx *cContext;
	gearman_task_st *task;
	gearman_return_t *ret;

	Handle_t hndl;

	funcid_t createdfunc;
	funcid_t statusfunc;
	funcid_t failfunc;
	funcid_t warningfunc;
	funcid_t completefunc;
};

/**
 * @brief Sample implementation of the SDK Extension.
 * Note: Uncomment one of the pre-defined virtual functions in order to use it.
 */
class Gearman : public SDKExtension, public IHandleTypeDispatch, public IThread, public IThreadWorkerCallbacks {
private:
	Queue<gearman_task_ctx *> m_TaskQueue;
	IMutex *m_pQueueLock;				/* Queue safety lock */
	IThreadWorker *m_pWorker;			/* Worker thread object */
public:
	/**
	 * @brief This is called after the initial loading sequence has been processed.
	 *
	 * @param error		Error message buffer.
	 * @param maxlength	Size of error message buffer.
	 * @param late		Whether or not the module was loaded after map load.
	 * @return			True to succeed loading, false to fail.
	 */
	virtual bool SDK_OnLoad(char *error, size_t maxlength, bool late);
	
	/**
	 * @brief This is called right before the extension is unloaded.
	 */
	virtual void SDK_OnUnload();

	/**
	 * @brief This is called once all known extensions have been loaded.
	 * Note: It is is a good idea to add natives here, if any are provided.
	 */
	virtual void SDK_OnAllLoaded();

	/**
	 * @brief this is called when Core wants to know if your extension is working.
	 *
	 * @param error		Error message buffer.
	 * @param maxlength	Size of error message buffer.
	 * @return			True if working, false otherwise.
	 */
	virtual bool QueryRunning(char *error, size_t maxlength);

public:
	gearman_client_ctx* GetGearmanClientInstanceByHandle(Handle_t);
	gearman_worker_ctx* GetGearmanWorkerInstanceByHandle(Handle_t);

	gearman_job_st* GetGearmanJobInstanceByHandle(Handle_t);
	gearman_task_ctx* GetGearmanTaskCtxInstanceByHandle(Handle_t);
	
	HandleType_t gearmanClientHandleType;
	
	HandleType_t gearmanWorkerHandleType;
	
	HandleType_t gearmanJobHandleType;

	HandleType_t gearmanTaskHandleType;
	
	bool AddToQueue(gearman_task_ctx *ctx);
public:
	void RunFrame();
public:
	void OnHandleDestroy(HandleType_t type, void *object);
public: //IThread
	void RunThread(IThreadHandle *pThread);
	void OnTerminate(IThreadHandle *pThread, bool cancel);
public: //IThreadWorkerCallbacks
	void OnWorkerStart(IThreadWorker *pWorker);
	void OnWorkerStop(IThreadWorker *pWorker);
private:
	void KillWorkerThread();
};

extern const sp_nativeinfo_t GearmanNatives[];

#endif // _INCLUDE_SOURCEMOD_EXTENSION_PROPER_H_
