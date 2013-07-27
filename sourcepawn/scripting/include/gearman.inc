#if defined _gearman_included_
	#endinput
#endif
#define _gearman_included_

/**
 * This is used to simplify _low, (normal), and _high for task and background
 */
enum GearmanPriority {
	GearmanPriority_Low,
	GearmanPriority_Normal,
	GearmanPriority_High
};

/**
 * This is used as a wrapper to use send_data, send_warning, send_complete, and send_exception
 */
enum GearmanResp {
	GearmanResp_Data,
	GearmanResp_Warning,
	GearmanResp_Complete,
	GearmanResp_Exception
};

/**
 * This is the same as gearman_return_t in libgearman, use the documentation to find return values of specific functions
 */
enum GearmanReturn {
  GEARMAN_SUCCESS,
  GEARMAN_IO_WAIT,
  GEARMAN_SHUTDOWN,
  GEARMAN_SHUTDOWN_GRACEFUL,
  GEARMAN_ERRNO,
  GEARMAN_EVENT, // DEPRECATED, SERVER ONLY
  GEARMAN_TOO_MANY_ARGS,
  GEARMAN_NO_ACTIVE_FDS, // No servers available
  GEARMAN_INVALID_MAGIC,
  GEARMAN_INVALID_COMMAND,
  GEARMAN_INVALID_PACKET,
  GEARMAN_UNEXPECTED_PACKET,
  GEARMAN_GETADDRINFO,
  GEARMAN_NO_SERVERS,
  GEARMAN_LOST_CONNECTION,
  GEARMAN_MEMORY_ALLOCATION_FAILURE,
  GEARMAN_JOB_EXISTS, // see gearman_client_job_status()
  GEARMAN_JOB_QUEUE_FULL,
  GEARMAN_SERVER_ERROR,
  GEARMAN_WORK_ERROR,
  GEARMAN_WORK_DATA,
  GEARMAN_WORK_WARNING,
  GEARMAN_WORK_STATUS,
  GEARMAN_WORK_EXCEPTION,
  GEARMAN_WORK_FAIL,
  GEARMAN_NOT_CONNECTED,
  GEARMAN_COULD_NOT_CONNECT,
  GEARMAN_SEND_IN_PROGRESS, // DEPRECATED, SERVER ONLY
  GEARMAN_RECV_IN_PROGRESS, // DEPRECATED, SERVER ONLY
  GEARMAN_NOT_FLUSHING,
  GEARMAN_DATA_TOO_LARGE,
  GEARMAN_INVALID_FUNCTION_NAME,
  GEARMAN_INVALID_WORKER_FUNCTION,
  GEARMAN_NO_REGISTERED_FUNCTION,
  GEARMAN_NO_REGISTERED_FUNCTIONS,
  GEARMAN_NO_JOBS,
  GEARMAN_ECHO_DATA_CORRUPTION,
  GEARMAN_NEED_WORKLOAD_FN,
  GEARMAN_PAUSE, // Used only in custom application for client return based on work status, exception, or warning.
  GEARMAN_UNKNOWN_STATE,
  GEARMAN_PTHREAD, // DEPRECATED, SERVER ONLY
  GEARMAN_PIPE_EOF, // DEPRECATED, SERVER ONLY
  GEARMAN_QUEUE_ERROR, // DEPRECATED, SERVER ONLY
  GEARMAN_FLUSH_DATA, // Internal state, should never be seen by either client or worker.
  GEARMAN_SEND_BUFFER_TOO_SMALL,
  GEARMAN_IGNORE_PACKET, // Internal only
  GEARMAN_UNKNOWN_OPTION, // DEPRECATED
  GEARMAN_TIMEOUT,
  GEARMAN_ARGUMENT_TOO_LARGE,
  GEARMAN_INVALID_ARGUMENT,
  GEARMAN_IN_PROGRESS, // See gearman_client_job_status()
  GEARMAN_INVALID_SERVER_OPTION, // Bad server option sent to server
  GEARMAN_MAX_RETURN, /* Always add new error code before */
  GEARMAN_FAIL= GEARMAN_WORK_FAIL,
  GEARMAN_FATAL= GEARMAN_WORK_FAIL,
  GEARMAN_ERROR= GEARMAN_WORK_ERROR
};

// Handlers

/**
 * Called when a worker receives a job
 *
 * @param job		The job handle (See GearmanJob_*)
 * @param data		The job workload
 * @param dataSize	The job workload size
 * @return any of GearmanReturn, or GEARMAN_IN_PROGRESS to handle a response on a timer (Otherwise it auto closes the handle, you will need to close it if GEARMAN_IN_PROGRESS)
 */
functag GearmanWorker GearmanReturn:public(Handle:job, const String:data[], const dataSize);

/**
 * Called when a task is complete
 *
 * @param task		The task handle (See GearmanTask_*)
 * @param data		The task data
 * @param dataSize	The task data size
 */
functag GearmanCompleteCallback public(Handle:task, const String:data[], const dataSize);

/**
 * Called when a task is created
 *
 * @param task		The task handle (See GearmanTask_*)
 */
functag GearmanCreatedCallback public(Handle:task);

/**
 * Called when a task gets a status response
 *
 * @param task		The task handle (See GearmanTask_*)
 * @param numerator	The amount done
 * @param denominator The total
 */
functag GearmanStatusCallback public(Handle:task, numerator, denominator);

/**
 * Called when a task gets a warning (?)
 *
 * @param task		The task handle (See GearmanTask_*)
 */
functag GearmanWarningCallback public(Handle:task);

/**
 * Called when a task fails
 *
 * @param task		The task handle (See GearmanTask_*)
 * @param error		The task error
 */
functag GearmanFailCallback public(Handle:task, const String:error[]);

// Gearman Client natives

/**
 * Create a new client handle
 *
 * @return The client handle
 */
native Handle:GearmanClient_Create();

/**
 * Add a server to a client
 *
 * @param client		The client created with GearmanClient_Create
 *
 * @return GearmanReturn value (SUCCESS or FAIL is common)
 */
native GearmanReturn:GearmanClient_AddServer(Handle:client, const String:address[], port = 4730);

/**
 * Set the callback to use when a task is created
 *
 * @param client		The client created with GearmanClient_Create
 * @param callback		The GearmanCreatedCallback to use
 * @return	true if created, false if not.
 * @error	If the client is invalid
 */
native bool:GearmanClient_SetCreatedCallback(Handle:client, GearmanCreatedCallback:callback);

/**
 * Execute a task with the server
 *
 * @param client		The client created with GearmanClient_Create
 * @param function		The function to execute
 * @param workload		The task workload
 * @param callback		The callback to call when the task is done
 * @param priority		The task priority (See GearmanPriority, takes place of add_task_low, add_task, and add_task_high)
 * @return	The task handle (See GearmanTask_*)
 * @error	If the client is invalid
 */
native Handle:GearmanClient_AddTask(Handle:gearman, const String:function[], const String:workload[], GearmanCompleteCallback:callback, GearmanPriority:priority=GearmanPriority_Normal);

/**
 * Execute a background (no return) task with the server
 *
 * @param client		The client created with GearmanClient_Create
 * @param function		The function to execute
 * @param workload		The task workload
 * @param priority		The task priority (See GearmanPriority, takes place of add_task_low, add_task, and add_task_high)
 * @return	The job handle (See GearmanJob_*)
 * @error	If the client is invalid
 */
native Handle:GearmanClient_DoBackground(Handle:gearman, const String:function[], const String:workload[], GearmanPriority:priority=GearmanPriority_Normal);

// Gearman Worker natives

/**
 * Create a new worker handle
 *
 * @return The worker handle
 */
native Handle:GearmanWorker_Create();

/**
 * Add a server to a client
 *
 * @param client		The client created with GearmanClient_Create
 *
 * @return GearmanReturn value (SUCCESS or FAIL is common)
 */
native GearmanWorker_AddServer(Handle:worker, const String:address[], port = 4730);

/**
 * Add a function to the gearman server
 *
 * @param worker		The worker handle
 * @param functionName	The function name to define
 * @param callback		The worker callback
 * @return GearmanReturn value (SUCCESS or FAIL?)
 */
native GearmanReturn:GearmanWorker_AddFunction(Handle:worker, const String:functionName[], GearmanWorker:callback);

/**
 * Set the worker identifier
 *
 * @param worker		The worker handle
 * @param identifier	The identifier to set
 * @return GearmanReturn value (SUCCESS or FAIL?)
 */
native GearmanWorker_SetIdentifier(Handle:worker, const String:identifier[]);

// Gearman Job natives

/**
 * Send data to a job (Used with Workers)
 *
 * @param job		The job handle
 * @param data		The data to send
 * @param type		The type to respond as (Data, Warning, Complete, Exception)
 * @return GearmanReturn value (SUCCESS or FAIL?)
 */
native GearmanReturn:GearmanJob_Send(Handle:job, const String:data[], GearmanResp:type=GearmanResp_Data);

/**
 * Send a failure to a job
 *
 * @param job		The job handle
 * @return GearmanReturn value (SUCCESS or FAIL?)
 */
native GearmanReturn:GearmanJob_SendFail(Handle:job);

/**
 * Send a status to a job
 *
 * @param job			The job handle
 * @param numerator 	The status numerator
 * @param denominator	The status denominator
 * @return GearmanReturn value (SUCCESS or FAIL?)
 */
native GearmanReturn:GearmanJob_SendStatus(Handle:job, numerator, denominator);

/**
 * Get a job's function name
 *
 * @param job		The job handle
 * @param buffer	The buffer to store the name into
 * @param maxlen	The buffer's size
 * @return	GearmanReturn value
 */
native GearmanReturn:GearmanJob_FunctionName(Handle:job, String:buffer[], maxlen);

/**
 * Get a job's unique value
 *
 * @param job		The job handle
 * @param buffer	The buffer to store the name into
 * @param maxlen	The buffer's size
 * @return	The unique value size
 */
native GearmanJob_Unique(Handle:job, String:buffer[], maxlen);

/**
 * Get a job's workload
 *
 * @param job		The job handle
 * @param buffer	The buffer to store the name into
 * @param maxlen	The buffer's size
 * @return	The workload size
 */
native GearmanJob_Workload(Handle:job, String:buffer[], maxlen);

/**
 * Get a job's workload size
 *
 * @param job		The job handle
 * @return	The workload size
 */
native GearmanJob_WorkloadSize(Handle:job);

// Gearman task natives

/**
 * Sets a task's created callback
 *
 * @param task		The task to set the callback on
 * @param cb		The callback to use
 * @return true or false, true if set successfully, false if otherwise.
 */
native GearmanTask_SetCreatedCallback(Handle:task, GearmanCreatedCallback:cb);

/**
 * Sets a task's status callback
 *
 * @param task		The task to set the callback on
 * @param cb		The callback to use
 * @return true or false, true if set successfully, false if otherwise.
 */
native GearmanTask_SetStatusCallback(Handle:task, GearmanStatusCallback:cb);

/**
 * Sets a task's fail callback
 *
 * @param task		The task to set the callback on
 * @param cb		The callback to use
 * @return true or false, true if set successfully, false if otherwise.
 */
native GearmanTask_SetFailCallback(Handle:task, GearmanFailCallback:cb);

/**
 * Sets a task's warning callback
 *
 * @param task		The task to set the callback on
 * @param cb		The callback to use
 * @return true or false, true if set successfully, false if otherwise.
 */
native GearmanTask_SetWarningCallback(Handle:task, GearmanWarningCallback:cb);

public Extension:__ext_gearman = {
	name = "Gearman",
	file = "gearman.ext",
#if defined AUTOLOAD_EXTENSIONS
	autoload = 1,
#else
	autoload = 0,
#endif
#if defined REQUIRE_EXTENSIONS
	required = 1,
#else
	required = 0,
#endif
};

#if !defined REQUIRE_EXTENSIONS
public __ext_gearman_SetNTVOptional() {
	MarkNativeAsOptional("GearmanClient_Create");
	MarkNativeAsOptional("GearmanClient_AddServer");
	MarkNativeAsOptional("GearmanClient_SetCreatedCallback");
	MarkNativeAsOptional("GearmanClient_AddTask");
	MarkNativeAsOptional("GearmanClient_DoBackground");
	MarkNativeAsOptional("GearmanWorker_Create");
	MarkNativeAsOptional("GearmanWorker_AddServer");
	MarkNativeAsOptional("GearmanWorker_AddFunction");
	MarkNativeAsOptional("GearmanWorker_SetIdentifier");
	MarkNativeAsOptional("GearmanJob_Send");
	MarkNativeAsOptional("GearmanJob_SendFail");
	MarkNativeAsOptional("GearmanJob_SendStatus");
	MarkNativeAsOptional("GearmanJob_FunctionName");
	MarkNativeAsOptional("GearmanJob_Unique");
	MarkNativeAsOptional("GearmanJob_Workload");
	MarkNativeAsOptional("GearmanJob_WorkloadSize");
	MarkNativeAsOptional("GearmanTask_SetCreatedCallback");
	MarkNativeAsOptional("GearmanTask_SetStatusCallback");
	MarkNativeAsOptional("GearmanTask_SetFailCallback");
	MarkNativeAsOptional("GearmanTask_SetWarningCallback");
}
#endif