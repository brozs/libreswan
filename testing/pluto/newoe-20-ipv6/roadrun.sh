../../guestbin/ping-once.sh --forget -6 2001:db8:1:2::23
ping6 -n -q -c 8 2001:db8:1:2::23
ipsec whack --trafficstatus
ipsec whack --shuntstatus
killall ip > /dev/null 2> /dev/null
cp /tmp/xfrm-monitor.out OUTPUT/road.xfrm-monitor.txt
echo done
