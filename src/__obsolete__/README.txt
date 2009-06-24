This folder contains files and sources for things that are obsoleted.

The files will go to nirvana soon.

This can be that they have benn superceeded by any other implementation, or that the implementation had some severe limitations or problems

-- as an example, the CConnectTCP is there, as I couldn't conbinve libcommoncpp to send two pacakges in a row...
-- CWorkScheduler was thrown out to elimnate libcommoncpp, as I had some problems with it too and also -- due the dual licensing with GPL and LPGPL
   i am not sure if the license is compatible with my intents.
   
List of classes / reasoning
 
CConnectTCP	 -- failed to perform: Could not convince libcommoncpp to send two packages over one connection. The second one did not go on wire...

CMutexHelper -- reworked too slimline and get rid of libcommoncpp (now uses BOOST)


