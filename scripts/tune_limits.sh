#!/bin/bash
set -e

echo "=== File Descriptor Limits ==="
echo "Current soft limit (ulimit -n): $(ulimit -Sn)"
echo "Current hard limit (ulimit -Hn)"

ulimit -n 65536
echo "Tuned soft limit for this shell: $(ulimit -Sn)"

echo ""
echo "=== Kernel TCP Tuning (read-only here; applied at runtime) ==="
# net.core.somaxconn        — max completed connections queued for accept()
#   Default on Ubuntu: 4096 (older kernels: 128). Raise to match your
#   listen() backlog so the kernel does not silently cap it for you.
echo "net.core.somaxconn    (current): $(sysctl -n net.core.somaxconn)"

# net.core.netdev_max_backlog — max packets queued for the kernel to process.
#   Raise under high packet rates so the NIC driver does not drop SYN/ACKs.
echo "net.core.netdev_max_backlog (current): $(sysctl -n net.core.netdev_max_backlog 2>/dev/null || echo 'n/a')"

# net.ipv4.tcp_max_syn_backlog — SYN flood / backlog queue for incomplete
#   handshakes (before accept()). Raise when expecting connection bursts.
echo "net.ipv4.tcp_max_syn_backlog (current): $(sysctl -n net.ipv4.tcp_max_syn_backlog)"

# net.ipv4.ip_local_port_range — ephemeral port pool for outbound connections.
#   Widen this so the LOAD-TEST client (wrk, wrk2) does not run out of ports
#   before the server does (see Week 2 "ephemeral port exhaustion").
echo "net.ipv4.ip_local_port_range (current): $(sysctl -n net.ipv4.ip_local_port_range)"

echo ""
echo "=== Recommended runtime values ==="
cat <<'EOF'
# Apply these before Week 2's load test (needs root / privileged container):
#   sysctl -w net.core.somaxconn=65535
#   sysctl -w net.core.netdev_max_backlog=500000
#   sysctl -w net.ipv4.tcp_max_syn_backlog=8192
#   sysctl -w net.ipv4.ip_local_port_range="1024 65535"
#
# For docker run:
#   --ulimit nofile=200000:200000
# For docker compose (ulimits section, see docker/docker-compose.loadtest.yml).
EOF
