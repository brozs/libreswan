../../guestbin/ipsec-look.sh
: ==== cut ====
ipsec auto --status
cat /tmp/xl2tpd.log
: ==== tuc ====
grep 'Result using RFC 3947' /tmp/pluto.log
