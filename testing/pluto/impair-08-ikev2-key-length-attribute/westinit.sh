/testing/guestbin/swan-prep
ipsec start
../../guestbin/wait-until-pluto-started
echo "initdone"
ipsec whack --impair revival
