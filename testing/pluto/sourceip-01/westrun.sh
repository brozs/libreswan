ipsec auto --up westnet-eastnet-sourceip
# not using -I because sourceip= should add the route
../../guestbin/ping-once.sh --up 192.0.2.254
ipsec whack --trafficstatus
