#!/bin/sh
# Dependency installation script for buildroots (server daemon start).
# Author: Abdullah Alabd (abdullah.alabd.else@gmail.com)


case "$1" in 
    start)
        echo "Starting daemon!"
        start-stop-daemon -S -n aesdsocket -a "/usr/bin/aesdsocket" -- -d
        ;;
    stop)
        echo "Stopping daemon!"
        start-stop-daemon -K -n aesdsocket
        ;;
    *)
        echo "USAGE $0 {start|stop}"
        exit 1
        ;;
esac

exit 0
