ipsec auto --up westnet-eastnet-etc-hosts
../../guestbin/ping-once.sh --up -I 192.0.1.254 192.0.2.254
ipsec whack --trafficstatus
echo done
