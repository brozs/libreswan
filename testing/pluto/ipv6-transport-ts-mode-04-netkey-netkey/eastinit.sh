/testing/guestbin/swan-prep --46
ipsec start
../../guestbin/wait-until-pluto-started
ipsec auto --add v6-transport
nc -l 1701 &
echo "initdone"
