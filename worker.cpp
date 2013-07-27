#include "worker.h"

GearmanWorkerThread::~GearmanWorkerThread() {
}

GearmanWorkerThread::GearmanWorkerThread(gearman_worker_ctx* worker):IThread() {	
	ctx = worker;
}

void GearmanWorkerThread::RunThread(IThreadHandle* pHandle) {
	gearman_return_t ret;
	do {
		ret = gearman_worker_work(ctx->worker);
	} while(ctx != NULL && ctx->worker != NULL && ret == GEARMAN_SUCCESS);
}

void GearmanWorkerThread::OnTerminate(IThreadHandle* pHandle, bool cancel) {
}