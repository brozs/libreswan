/testing/guestbin/swan-prep --x509
ipsec certutil -D -n east
ipsec pk12util -W foobar -K '' -i /testing/x509/pkcs12/mainca/usage-both.p12
ipsec start
../../guestbin/wait-until-pluto-started
ipsec auto --add nss-cert
ipsec auto --status |grep nss-cert
ipsec whack --impair suppress-retransmits
echo "initdone"
