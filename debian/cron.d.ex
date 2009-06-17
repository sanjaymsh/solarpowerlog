#
# Regular cron jobs for the solarpowerlog package
#
0 4	* * *	root	[ -x /usr/bin/solarpowerlog_maintenance ] && /usr/bin/solarpowerlog_maintenance
