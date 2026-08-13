#!/bin/bash
# Build the matmul image on this host and import it into containerd
# (k8s.io namespace) on every cluster node, so no registry is needed.
#
# Usage:  ./build-import.sh
#         NODES="compute01 compute02" ./build-import.sh   # subset
set -euo pipefail
 
IMAGE="matmul-openmp:latest"
NODES=${NODES:-"m1.hpc.com m2.hpc.com m3.hpc.com compute01.hpc.com compute02.hpc.com"}
 
docker build -t "$IMAGE" -f Dockerfile .
 
docker save "$IMAGE" -o /tmp/matmul-openmp.tar
 
for h in $NODES; do
  echo ">> importing image on ${h}"
  scp /tmp/matmul-openmp.tar "$h:/tmp/matmul-openmp.tar"
  ssh "$h" "sudo ctr -n k8s.io images import /tmp/matmul-openmp.tar"
done
 
echo ">> image '${IMAGE}' imported on all nodes"
