/testing/guestbin/swan-prep
ipsec start
../../guestbin/wait-until-pluto-started
ipsec auto --add westnet-eastnet-aggr
ipsec whack --impair timeout-on-retransmit
echo "initdone"
