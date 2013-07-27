#include "worker.h"

GearmanWorkerThread::~GearmanWorkerThread() {
}

GearmanWorkerThread::GearmanWorkerThread(gearman_worker_ctx* worker):IThread() {	
	ctx = worker;
}

void GearmanWorkerThread::RunThread(IThreadHandle* pHandle) {
	g_pSM->LogMessage(myself, "Worker thread running");
	gearman_return_t ret;
	do {
		g_pSM->LogMessage(myself, "Looking for work...");
		ret = gearman_worker_work(ctx->worker);
	} while(ctx != NULL && ctx->worker != NULL && ret == GEARMAN_SUCCESS);
	g_pSM->LogMessage(myself, "Worker thread ended");
}

void GearmanWorkerThread::OnTerminate(IThreadHandle* pHandle, bool cancel) {
}