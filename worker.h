#include "extension.h"

class GearmanWorkerThread : public IThread
{
private:
	gearman_worker_ctx *ctx;
public: //IThread
	void RunThread(IThreadHandle *pThread);
	void OnTerminate(IThreadHandle *pThread, bool cancel);
public:
	GearmanWorkerThread(gearman_worker_ctx *ctx);
	~GearmanWorkerThread();
};