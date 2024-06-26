iptables -t nat -F
iptables -F
# NAT
iptables -t nat -A POSTROUTING --source 192.1.3.208/32 --destination 0.0.0.0/0 -j SNAT --to-source 192.1.2.254
# NAT one differently
../../guestbin/ip.sh address add 192.1.2.222/24 dev eth1
iptables -t nat -A POSTROUTING --source 192.1.3.209/32 --destination 0.0.0.0/0 -j SNAT --to-source 192.1.2.222
# make sure that we never acidentially let ESP through.
#
iptables -I FORWARD 1 --proto 50 -j DROP
iptables -I FORWARD 2 --destination 192.0.2.0/24 -j DROP
iptables -I FORWARD 3 --source 192.0.2.0/24 -j DROP
# route
iptables -I INPUT 1 --destination 192.0.2.0/24 -j DROP
# Display the table, so we know it is correct.
iptables -t nat -L -n
iptables -L -n
echo "initdone"
: ==== end ====
