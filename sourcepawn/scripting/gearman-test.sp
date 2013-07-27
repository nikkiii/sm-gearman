#include <sourcemod>

#define AUTOLOAD_EXTENSIONS
#define REQUIRE_EXTENSIONS
#include <gearman>

public Plugin:myinfo =
{
	name = "Gearman Test",
	author = "Nikki",
	description = "Gearman extension test",
	version = "1.0",
	url = "http://www.sourcemod.net/"
};

public OnPluginStart() {
	TestClient();
	TestWorker();
}

TestWorker() {
	new Handle:hWorker = GearmanWorker_Create();
	GearmanWorker_AddServer(hWorker, "localhost", 4730);
	GearmanWorker_AddFunction(hWorker, "test", Worker_Test);
}

public GearmanReturn:Worker_Test(Handle:job, const String:data[], const dataSize) {
	LogMessage("Got job!");
	GearmanJob_Send(job, "Hello!", GearmanResp_Complete);
	return GEARMAN_SUCCESS;
}

TestClient() {
	new Handle:hClient = GearmanClient_Create();
	GearmanClient_AddServer(hClient, "localhost", 4730);
	
	GearmanClient_AddTask(hClient, "reverse", "Hello world!", Gearman_Executed);
}

public Gearman_Executed(Handle:task, const String:data[], const dataSize) {
	LogMessage("Got response %s", data);
}