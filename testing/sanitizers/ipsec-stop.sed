# match: ipsec stop

/^ ipsec stop/,/^[a-z][a-z]* #$/ {

  # FreeBSD: Waiting for PIDS: 1192.
  /Waiting for PIDS: / s/[0-9][0-9]*/PID/

  s/Redirecting to:.*$/Redirecting to: [initsystem]/

}
