/testing/guestbin/swan-prep
../../guestbin/ip.sh route del default
ip addr del 192.1.33.222/24 dev eth0 2>/dev/null
sleep 2
ip addr add 192.1.3.209/24 dev eth0 2>/dev/null
../../guestbin/ip.sh route add default via 192.1.3.254
ipsec start
../../guestbin/wait-until-pluto-started
ipsec auto --add road-eastnet
echo "initdone"
