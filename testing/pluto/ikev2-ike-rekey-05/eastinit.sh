/testing/guestbin/swan-prep
ip addr show dev eth0 | grep 192.0.200.254 || ip addr add 192.0.200.254/24 dev eth0:1
ip addr show dev eth0 | grep 192.0.201.254 || ip addr add 192.0.201.254/24 dev eth0:1
ip route show scope global | grep 192.0.100.0 || ip route add 192.0.100.0/24 via 192.1.2.45  dev eth1
ip route show scope global | grep 192.0.101.0 || ip route add 192.0.101.0/24 via 192.1.2.45  dev eth1
ipsec start
../../guestbin/wait-until-pluto-started
ipsec auto --add westnet-eastnet-a
ipsec auto --add westnet-eastnet-b
ipsec auto --add westnet-eastnet-c
echo "initdone"
