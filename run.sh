#!/bin/bash

PIDFILE=/var/run/lonely.pid

case $1 in
   start)
       /home/pi/lonelyghosts/main.py 2>/dev/null &
       echo $! > ${PIDFILE} 
   ;;
   stop)
      kill `cat ${PIDFILE}`
      rm ${PIDFILE}
   ;;
   *)
      echo "usage: {start|stop}" ;;
esac
exit 0
