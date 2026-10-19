Samuel Habib
smh389

To compile, run make.

in one terminal open a controller: ./heartbeat_app <user_id> [options]
in second terminal open a client: ./heartbeat_app <user_id>

Find the logs in the newly created files

options:
-c <interval> start as a controller with an intitial heartbeat with an interval 0-30

-m <mulitcast_ip> override the default group ip

-p <port> override the default port 8000
