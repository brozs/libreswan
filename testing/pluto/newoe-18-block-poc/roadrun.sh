# blocked
../../guestbin/ping-once.sh --down -I 192.1.3.209 192.1.2.23
# (pointlessly) wait on OE retransmits and rekeying
sleep 5
# no tunnel and no bare shunts expected
ipsec whack --trafficstatus
ipsec whack --shuntstatus
../../guestbin/ipsec-look.sh
killall ip > /dev/null 2> /dev/null
cp /tmp/xfrm-monitor.out OUTPUT/road.xfrm-monitor.txt
# ping should fail on outgoing block rule
../../guestbin/ping-once.sh --down -I 192.1.3.209 192.1.2.23
echo done
