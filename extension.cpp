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

#include "extension.h"

#include <libgearman-1.0/gearman.h>
#include <string.h>

/**
 * @file extension.cpp
 * @brief Implement extension code here.
 */

Gearman g_Gearman;		/**< Global singleton for extension's main interface */

SMEXT_LINK(&g_Gearman);

void Gearman::OnHandleDestroy(HandleType_t type, void *object) {
	g_pSM->LogMessage(myself, "Destroying handle...");
	if(object != NULL) {
		if(type == gearmanClientHandleType) {
			g_pSM->LogMessage(myself, "Destroy handle type client");
			gearman_client_free(((gearman_client_ctx *) object)->client);
		} else if(type == gearmanWorkerHandleType) {
			g_pSM->LogMessage(myself, "Destroy handle type worker");
			gearman_worker_free((gearman_worker_st *) object);
		} else if(type == gearmanJobHandleType) {
			g_pSM->LogMessage(myself, "Destroy handle type job");
			gearman_job_free((gearman_job_st *) object);
		} else if(type == gearmanTaskHandleType) {
			gearman_task_ctx *ctx = (gearman_task_ctx *) object;
			g_pSM->LogMessage(myself, "Destroy handle type task");
			gearman_task_free(ctx->task);
		}
	}
}
 
bool Gearman::SDK_OnLoad(char *error, size_t err_max, bool late) {
	sharesys->AddNatives(myself, GearmanNatives);
	sharesys->RegisterLibrary(myself, "gearman");
	
	gearmanClientHandleType = g_pHandleSys->CreateType("GearmanClient", this, 0, NULL, NULL, myself->GetIdentity(), NULL);
	gearmanWorkerHandleType = g_pHandleSys->CreateType("GearmanWorker", this, 0, NULL, NULL, myself->GetIdentity(), NULL);
	gearmanJobHandleType = g_pHandleSys->CreateType("GearmanJob", this, 0, NULL, NULL, myself->GetIdentity(), NULL);
	gearmanTaskHandleType = g_pHandleSys->CreateType("GearmanTask", this, 0, NULL, NULL, myself->GetIdentity(), NULL);
	return true;
}

void Gearman::SDK_OnUnload() {
	g_pHandleSys->RemoveType(g_Gearman.gearmanClientHandleType, NULL);
	g_pHandleSys->RemoveType(g_Gearman.gearmanWorkerHandleType, NULL);
	g_pHandleSys->RemoveType(g_Gearman.gearmanJobHandleType, NULL);
	g_pHandleSys->RemoveType(g_Gearman.gearmanTaskHandleType, NULL);
}

// Getters for handles

gearman_client_ctx* Gearman::GetGearmanClientInstanceByHandle(Handle_t handle) {
	HandleSecurity sec;
	sec.pOwner = NULL;
	sec.pIdentity = myself->GetIdentity();
	
	gearman_client_ctx *client;

	if (g_pHandleSys->ReadHandle(handle, g_Gearman.gearmanClientHandleType, &sec, (void**)&client) != HandleError_None)
		return NULL;

	return client;
}

gearman_worker_st* Gearman::GetGearmanWorkerInstanceByHandle(Handle_t handle) {
	HandleSecurity sec;
	sec.pOwner = NULL;
	sec.pIdentity = myself->GetIdentity();
	
	gearman_worker_st *worker;

	if (g_pHandleSys->ReadHandle(handle, g_Gearman.gearmanWorkerHandleType, &sec, (void**) &worker) != HandleError_None)
		return NULL;

	return worker;
}

gearman_job_st* Gearman::GetGearmanJobInstanceByHandle(Handle_t handle) {
	HandleSecurity sec;
	sec.pOwner = NULL;
	sec.pIdentity = myself->GetIdentity();
	
	gearman_job_st *job;

	if (g_pHandleSys->ReadHandle(handle, g_Gearman.gearmanJobHandleType, &sec, (void**)&job) != HandleError_None)
		return NULL;

	return job;
}

gearman_task_ctx* Gearman::GetGearmanTaskCtxInstanceByHandle(Handle_t handle) {
	HandleSecurity sec;
	sec.pOwner = NULL;
	sec.pIdentity = myself->GetIdentity();
	
	gearman_task_ctx *task;

	if (g_pHandleSys->ReadHandle(handle, g_Gearman.gearmanJobHandleType, &sec, (void**) &task) != HandleError_None)
		return NULL;

	return task;
}

// Parsing of tasks

static gearman_return_t Gearman_TaskCreatedFn(gearman_task_st *task) {
	gearman_task_ctx *ctx = (gearman_task_ctx *) gearman_task_context(task);

	if(ctx == NULL)
		return GEARMAN_FAIL;

	if(ctx->createdfunc != 0) {
		// functag GearmanCreateCallback public(Handle:task);
		IPluginFunction *pFunction = ctx->pContext->GetFunctionById(ctx->createdfunc);
		if(pFunction == NULL)
			return GEARMAN_SUCCESS;

		pFunction->PushCell(ctx->hndl);
	
		cell_t result = 0;
		pFunction->Execute(&result);
		return GEARMAN_SUCCESS;
	}

	if(ctx->cContext != NULL && ctx->cContext->createdFunc != 0) {
		// functag GearmanCreateCallback public(Handle:task);
		IPluginFunction *pFunction = ctx->pContext->GetFunctionById(ctx->cContext->createdFunc);
		if(pFunction == NULL)
			return GEARMAN_SUCCESS;

		pFunction->PushCell(ctx->hndl);
	
		cell_t result = 0;
		pFunction->Execute(&result);
		return GEARMAN_SUCCESS;
	}

	return GEARMAN_SUCCESS;
}

static gearman_return_t Gearman_TaskStatusFn(gearman_task_st *task) {
	gearman_task_ctx *ctx = (gearman_task_ctx *) gearman_task_context(task);

	if(ctx == NULL)
		return GEARMAN_FAIL;

	if(ctx->completefunc == 0)
		return GEARMAN_SUCCESS;

	// functag GearmanStatusCallback public(Handle:task, numerator, denominator);
	IPluginFunction *pFunction = ctx->pContext->GetFunctionById(ctx->completefunc);
	if(pFunction == NULL)
		return GEARMAN_SUCCESS;

	pFunction->PushCell(ctx->hndl);
	pFunction->PushCell(gearman_task_numerator(task));
	pFunction->PushCell(gearman_task_denominator(task));

	cell_t result = 0;
	pFunction->Execute(&result);

	return GEARMAN_SUCCESS;
}

static gearman_return_t Gearman_TaskWarningFn(gearman_task_st *task) {
	gearman_task_ctx *ctx = (gearman_task_ctx *) gearman_task_context(task);

	if(ctx == NULL)
		return GEARMAN_FAIL;
	
	if(ctx->warningfunc == 0)
		return GEARMAN_SUCCESS;

	// functag GearmanWarningCallback public(Handle:task);
	IPluginFunction *pFunction = ctx->pContext->GetFunctionById(ctx->completefunc);
	if(pFunction == NULL)
		return GEARMAN_SUCCESS;

	pFunction->PushCell(ctx->hndl);

	cell_t result = 0;
	pFunction->Execute(&result);

	return GEARMAN_SUCCESS;
}

static gearman_return_t Gearman_TaskCompleteFn(gearman_task_st *task) {
	gearman_task_ctx *ctx = (gearman_task_ctx *) gearman_task_context(task);

	if(ctx == NULL)
		return GEARMAN_FAIL;

	if(ctx->completefunc == 0)
		return GEARMAN_SUCCESS;

	// functag GearmanCompleteCallback public(Handle:task, const String:data[], const dataSize);
	IPluginFunction *pFunction = ctx->pContext->GetFunctionById(ctx->completefunc);
	if(pFunction == NULL)
		return GEARMAN_SUCCESS;

	pFunction->PushCell(ctx->hndl);
	pFunction->PushString((char *) gearman_task_data(task));
	pFunction->PushCell(gearman_task_data_size(task));

	cell_t result = 0;
	pFunction->Execute(&result);

	g_pHandleSys->FreeHandle(ctx->hndl, NULL);

	return GEARMAN_SUCCESS;
}

static gearman_return_t Gearman_TaskFailFn(gearman_task_st *task) {
	gearman_task_ctx *ctx = (gearman_task_ctx *) gearman_task_context(task);

	if(ctx == NULL)
		return GEARMAN_FAIL;

	if(ctx->failfunc == 0)
		return GEARMAN_SUCCESS;

	// functag GearmanFailCallback public(Handle:task, const String:error[]);
	IPluginFunction *pFunction = ctx->pContext->GetFunctionById(ctx->completefunc);
	if(pFunction == NULL)
		return GEARMAN_SUCCESS;

	pFunction->PushCell(ctx->hndl);
	pFunction->PushString(gearman_task_error(task));

	cell_t result = 0;
	pFunction->Execute(&result);

	g_pHandleSys->FreeHandle(ctx->hndl, NULL);

	return GEARMAN_SUCCESS;
}
 
// native GearmanClient_Create()
cell_t GearmanClient_Create(IPluginContext *pContext, const cell_t *params) {
	gearman_client_st *client = gearman_client_create(NULL);

	if(client == NULL)
		return BAD_HANDLE;
	
	gearman_client_set_created_fn(client, Gearman_TaskCreatedFn);
	gearman_client_set_fail_fn(client, Gearman_TaskFailFn);
	gearman_client_set_status_fn(client, Gearman_TaskStatusFn);
	gearman_client_set_warning_fn(client, Gearman_TaskWarningFn);
	gearman_client_set_complete_fn(client, Gearman_TaskCompleteFn);

	//gearman_client_add_options(client, GEARMAN_CLIENT_NON_BLOCKING);
	
	gearman_client_ctx *cContext = new gearman_client_ctx;
	cContext->client = client;
	cContext->pContext = pContext;
	cContext->createdFunc = 0;
	// Return the handle
	return g_pHandleSys->CreateHandle(g_Gearman.gearmanClientHandleType, cContext, pContext->GetIdentity(), myself->GetIdentity(), NULL);
}

// native GearmanClient_AddServer(Handle:gearman, const String:address[], port);
cell_t GearmanClient_AddServer(IPluginContext *pContext, const cell_t *params) {
	gearman_client_ctx *client = g_Gearman.GetGearmanClientInstanceByHandle(static_cast<Handle_t>(params[1]));
	if(client == NULL)
		return pContext->ThrowNativeError("Invalid gearman handle: %i", params[1]);
	if (params[3] < 0 || params[3] > 65535)
		return pContext->ThrowNativeError("Invalid port specified");
	
	char *hostname = NULL;
	pContext->LocalToString(params[2], &hostname);
	
	return gearman_client_add_server(client->client, hostname, params[3]);
}

// native GearmanClient_AddTask(Handle:gearman, const String:function[], const String:workload[], GearmanCompletedCallback:callback, GearmanPriority:priority=Gearman_PriorityNormal);
cell_t GearmanClient_AddTask(IPluginContext *pContext, const cell_t *params) {
	gearman_client_ctx *client = g_Gearman.GetGearmanClientInstanceByHandle(static_cast<Handle_t>(params[1]));

	if(client == NULL)
		return pContext->ThrowNativeError("Invalid gearman handle: %i", params[1]);
	
	char *functionName = NULL;
	char *argument = NULL;
	
	pContext->LocalToString(params[2], &functionName);
	pContext->LocalToString(params[3], &argument);
	
	GearmanPriority prio = (GearmanPriority) params[5];
	
	gearman_return_t ret;

	gearman_task_ctx *task = new gearman_task_ctx;
	task->pContext = pContext;
	task->cContext = client;
	task->completefunc = static_cast<funcid_t>(params[4]);

	task->createdfunc = NULL;
	task->failfunc = NULL;
	task->warningfunc = NULL;
	task->statusfunc = NULL;

	task->task = NULL;

	switch(prio) {
	case GearmanPriority_Low:
		task->task = gearman_client_add_task_low(client->client, task->task, task, functionName, "", argument, strlen(argument), &ret);
		break;
	case GearmanPriority_Normal:
		task->task = gearman_client_add_task(client->client, task->task, task, functionName, "", argument, strlen(argument), &ret);
		break;
	case GearmanPriority_High:
		task->task = gearman_client_add_task_high(client->client, task->task, task, functionName, "", argument, strlen(argument), &ret);
		break;
	}

	task->hndl = g_pHandleSys->CreateHandle(g_Gearman.gearmanTaskHandleType, task, pContext->GetIdentity(), myself->GetIdentity(), NULL);

	// MAJOR TODO! This MIGHT need to be threaded
	gearman_client_run_tasks(client->client);
	
	return task->hndl;
}

// native GearmanClient_SetCreatedCallback(Handle:gearman, GearmanCreatedCallback:callback);
cell_t GearmanClient_SetCreatedCallback(IPluginContext *pContext, const cell_t *params) {
	gearman_task_ctx *ctx = g_Gearman.GetGearmanTaskCtxInstanceByHandle(static_cast<Handle_t>(params[1]));
	if(ctx == NULL)
		return pContext->ThrowNativeError("Invalid task handle: %i", params[1]);

	ctx->createdfunc = static_cast<funcid_t>(params[2]);

	return true;
}


// native GearmanWorker_Create()
cell_t GearmanWorker_Create(IPluginContext *pContext, const cell_t *params) {
	gearman_worker_st *worker = gearman_worker_create(NULL);
	
	if(worker == NULL)
		return BAD_HANDLE;

	// Return the handle
	return g_pHandleSys->CreateHandle(g_Gearman.gearmanWorkerHandleType, worker, pContext->GetIdentity(), myself->GetIdentity(), NULL);
}

// native GearmanWorker_AddServer(Handle:gearman, const String:address[], port);
cell_t GearmanWorker_AddServer(IPluginContext *pContext, const cell_t *params) {
	gearman_worker_st *worker = g_Gearman.GetGearmanWorkerInstanceByHandle(static_cast<Handle_t>(params[1]));
	if(worker == NULL)
		return pContext->ThrowNativeError("Invalid gearman handle: %i", params[1]);
	if (params[3] < 0 || params[3] > 65535)
		return pContext->ThrowNativeError("Invalid port specified");
	
	char *hostname = NULL;
	pContext->LocalToString(params[2], &hostname);
	
	return gearman_worker_add_server(worker, hostname, params[3]);
}

// native GearmanWorker_AddFunction(Handle:gearman, const String:functionName[], GearmanWorker:worker);
cell_t GearmanWorker_AddFunction(IPluginContext *pContext, const cell_t *params) {
	gearman_worker_st *worker = g_Gearman.GetGearmanWorkerInstanceByHandle(static_cast<Handle_t>(params[1]));

	if(worker == NULL)
		return pContext->ThrowNativeError("Invalid worker handle: %i", params[1]);
	
	char *funcName = NULL;
	pContext->LocalToString(params[2], &funcName);
	
	gearman_worker_cb *context = new gearman_worker_cb;
	context->pContext = pContext;
	context->funcid = params[3];

	g_pSM->LogMessage(myself, "Worker registered for %s", funcName);
	return gearman_worker_add_function(worker, funcName, 60000, Gearman_CallWorker, context);
}

void* Gearman_CallWorker(gearman_job_st *job, void *context, size_t *result_size, gearman_return_t *ret_ptr) {
	g_pSM->LogMessage(myself, "Worker called");

	gearman_worker_cb * worker_cb = (gearman_worker_cb *) context;

	if(worker_cb == NULL)
		return 0;
	
	Handle_t job_hndl = g_pHandleSys->CreateHandle(g_Gearman.gearmanJobHandleType, job, worker_cb->pContext->GetIdentity(), myself->GetIdentity(), NULL);
	
	IPluginFunction *pFunction = worker_cb->pContext->GetFunctionById(worker_cb->funcid);

	if(pFunction == NULL)
		return 0;
		
		
	char *workload = NULL;
	workload = (char *) gearman_job_workload(job);

	// GearmanWorker(Handle:job, const String:workload[], const workloadSize)
	// Push the job handle
	pFunction->PushCell(job_hndl);
	pFunction->PushString(workload);
	pFunction->PushCell(gearman_job_workload_size(job));

	cell_t result = 0;
	pFunction->Execute(&result);
	
	HandleSecurity sec;
	sec.pOwner = NULL;
	sec.pIdentity = myself->GetIdentity();
	g_pHandleSys->FreeHandle(job_hndl, &sec);
	
	gearman_job_free(job);

	return 0;
}

cell_t GearmanWorker_SetIdentifier(IPluginContext *pContext, const cell_t *params) {
	gearman_worker_st *worker = g_Gearman.GetGearmanWorkerInstanceByHandle(static_cast<Handle_t>(params[1]));

	if(worker == NULL)
		return pContext->ThrowNativeError("Invalid worker handle: %i", params[1]);
	
	char *identifier = NULL;
	pContext->LocalToString(params[2], &identifier);
	
	return gearman_worker_set_identifier(worker, identifier, strlen(identifier));
}

/* Gearman Job Functions */

// native GearmanJob_Send(Handle:job, const String:data[], const dataLength, GearmanResp:type=GearmanResp_Data)
cell_t GearmanJob_Send(IPluginContext *pContext, const cell_t *params) {
	gearman_job_st *job = g_Gearman.GetGearmanJobInstanceByHandle(static_cast<Handle_t>(params[1]));
	if(job == NULL)
		return pContext->ThrowNativeError("Invalid job handle: %i", params[1]);
	
	char *data = NULL;
	pContext->LocalToString(params[2], &data);
	
	GearmanResp type = (GearmanResp) params[4];
	
	gearman_return_t ret;
	
	switch(type) {
		case GearmanResp_Data:
			ret = gearman_job_send_data(job, data, params[3]);
			break;
		case GearmanResp_Warning:
			ret = gearman_job_send_warning(job, data, params[3]);
			break;
		case GearmanResp_Complete:
			ret = gearman_job_send_complete(job, data, params[3]);
			break;
		case GearmanResp_Exception:
			ret = gearman_job_send_exception(job, data, params[3]);
			break;
	}
	return ret;
}

// native GearmanJob_SendFail(Handle:job);
cell_t GearmanJob_SendFail(IPluginContext *pContext, const cell_t *params) {
	gearman_job_st *job = g_Gearman.GetGearmanJobInstanceByHandle(static_cast<Handle_t>(params[1]));
	if(job == NULL)
		return pContext->ThrowNativeError("Invalid job handle: %i", params[1]);
	
	return gearman_job_send_fail(job);
}

// native GearmanJob_SendStatus(Handle:job, numerator, denominator);
cell_t GearmanJob_SendStatus(IPluginContext *pContext, const cell_t *params) {
	gearman_job_st *job = g_Gearman.GetGearmanJobInstanceByHandle(static_cast<Handle_t>(params[1]));
	if(job == NULL)
		return pContext->ThrowNativeError("Invalid job handle: %i", params[1]);
	
	return gearman_job_send_status(job, params[2], params[3]);
}

// native GearmanJob_FunctionName(Handle:job, String:buffer[], maxlen);
cell_t GearmanJob_FunctionName(IPluginContext *pContext, const cell_t *params) {
	gearman_job_st *job = g_Gearman.GetGearmanJobInstanceByHandle(static_cast<Handle_t>(params[1]));
	if(job == NULL)
		return pContext->ThrowNativeError("Invalid job handle: %i", params[1]);

	// Return
	const char *result = gearman_job_function_name(job);
	if(result != NULL) {
		pContext->StringToLocalUTF8(params[2], params[3], result, NULL);
		return strlen(result);
	}

	return -1;
}

// native GearmanJob_Unique(Handle:job, String:buffer[], maxlen);
cell_t GearmanJob_Unique(IPluginContext *pContext, const cell_t *params) {
	gearman_job_st *job = g_Gearman.GetGearmanJobInstanceByHandle(static_cast<Handle_t>(params[1]));
	if(job == NULL)
		return pContext->ThrowNativeError("Invalid job handle: %i", params[1]);

	// Return
	const char *result = gearman_job_unique(job);
	if(result != NULL) {
		pContext->StringToLocalUTF8(params[2], params[3], result, NULL);
		return strlen(result);
	}

	return -1;
}

// native GearmanJob_Workload(Handle:job, String:buffer[], maxlen);
cell_t GearmanJob_Workload(IPluginContext *pContext, const cell_t *params) {
	gearman_job_st *job = g_Gearman.GetGearmanJobInstanceByHandle(static_cast<Handle_t>(params[1]));
	if(job == NULL)
		return pContext->ThrowNativeError("Invalid job handle: %i", params[1]);

	// Return
	const char *result = (char*) gearman_job_workload(job);
	if(result != NULL) {
		pContext->StringToLocalUTF8(params[2], params[3], result, NULL);
		return strlen(result);
	}

	return -1;
}

// native GearmanJob_WorkloadSize(Handle:job);
cell_t GearmanJob_WorkloadSize(IPluginContext *pContext, const cell_t *params) {
	gearman_job_st *job = g_Gearman.GetGearmanJobInstanceByHandle(static_cast<Handle_t>(params[1]));
	if(job == NULL)
		return pContext->ThrowNativeError("Invalid job handle: %i", params[1]);
	
	return gearman_job_workload_size(job);
}

// Gearman task functions

// native GearmanTask_SetCreatedCallback(Handle:task, GearmanCreatedCallback:cb);
cell_t GearmanTask_SetCreatedCallback(IPluginContext *pContext, const cell_t *params) {
	gearman_task_ctx *ctx = g_Gearman.GetGearmanTaskCtxInstanceByHandle(static_cast<Handle_t>(params[1]));
	if(ctx == NULL)
		return pContext->ThrowNativeError("Invalid task handle: %i", params[1]);

	ctx->createdfunc = static_cast<funcid_t>(params[2]);

	return true;
}

// native GearmanTask_SetStatusCallback(Handle:task, GearmanStatusCallback:cb);
cell_t GearmanTask_SetStatusCallback(IPluginContext *pContext, const cell_t *params) {
	gearman_task_ctx *ctx = g_Gearman.GetGearmanTaskCtxInstanceByHandle(static_cast<Handle_t>(params[1]));
	if(ctx == NULL)
		return pContext->ThrowNativeError("Invalid task handle: %i", params[1]);
	
	ctx->statusfunc = static_cast<funcid_t>(params[2]);

	return true;
}

// native GearmanTask_SetFailCallback(Handle:task, GearmanStatusCallback:cb);
cell_t GearmanTask_SetFailCallback(IPluginContext *pContext, const cell_t *params) {
	gearman_task_ctx *ctx = g_Gearman.GetGearmanTaskCtxInstanceByHandle(static_cast<Handle_t>(params[1]));
	if(ctx == NULL)

	ctx->failfunc = static_cast<funcid_t>(params[2]);

	return true;
}

// native GearmanTask_SetWarningCallback(Handle:task, GearmanWarningCallback:cb);
cell_t GearmanTask_SetWarningCallback(IPluginContext *pContext, const cell_t *params) {
	gearman_task_ctx *ctx = g_Gearman.GetGearmanTaskCtxInstanceByHandle(static_cast<Handle_t>(params[1]));
	if(ctx == NULL)
		return pContext->ThrowNativeError("Invalid task handle: %i", params[1]);

	ctx->warningfunc = static_cast<funcid_t>(params[2]);

	return true;
}

const sp_nativeinfo_t GearmanNatives[] = {
	{"GearmanClient_Create", GearmanClient_Create},
	{"GearmanClient_AddServer", GearmanClient_AddServer},
	{"GearmanClient_AddTask", GearmanClient_AddTask},
	{"GearmanClient_SetCreatedCallback", GearmanClient_SetCreatedCallback},
	
	{"GearmanWorker_Create", GearmanWorker_Create},
	{"GearmanWorker_AddServer", GearmanWorker_AddServer},
	{"GearmanWorker_AddFunction", GearmanWorker_AddFunction},
	{"GearmanWorker_SetIdentifier", GearmanWorker_SetIdentifier},
	
	{"GearmanJob_Send", GearmanJob_Send},
	{"GearmanJob_SendFail", GearmanJob_SendFail},
	{"GearmanJob_SendStatus", GearmanJob_SendStatus},
	{"GearmanJob_FunctionName", GearmanJob_FunctionName},
	{"GearmanJob_Unique", GearmanJob_Unique},
	{"GearmanJob_Workload", GearmanJob_Workload},
	{"GearmanJob_WorkloadSize", GearmanJob_WorkloadSize},

	{"GearmanTask_SetCreatedCallback", GearmanTask_SetCreatedCallback},
	{"GearmanTask_SetStatusCallback", GearmanTask_SetStatusCallback},
	{"GearmanTask_SetFailCallback", GearmanTask_SetFailCallback},
	{"GearmanTask_SetWarningCallback", GearmanTask_SetWarningCallback},
	{NULL, NULL}
};
