Currently unfinished.

It sort of works. The client part works, the worker part semi-works (It works for 3 times and on the fourth it times out, then works again for another 3).

It may leak memory (If you see an obvious leak, let me know in an issue), it is NOT stable (It probably will crash randomly!), dobackground is not tested and needs to be redone to not return a job handle which could be bad if left unclosed.