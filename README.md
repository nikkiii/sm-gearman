Currently unfinished.

Things to do:

* Each Worker object needs it's own thread for gearman_worker_work
* MIGHT need to use the threading setup which database connections use to call gearman_client_run_tasks on task add (It won't run them otherwise)