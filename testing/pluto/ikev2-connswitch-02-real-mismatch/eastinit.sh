/testing/guestbin/swan-prep
certutil -d sql:/etc/ipsec.d -D -n west
ipsec start
../../guestbin/wait-until-pluto-started
ipsec auto --add westnet-eastnet-foo
ipsec auto --add westnet-eastnet-bar
echo "initdone"
