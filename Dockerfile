FROM gcc:13 AS build
WORKDIR /app
COPY main.c .
RUN gcc -O3 -fopenmp -o matmul main.c -lm
 
FROM debian:bookworm-slim
RUN apt-get update \
    && apt-get install -y --no-install-recommends libgomp1 \
    && rm -rf /var/lib/apt/lists/*
COPY --from=build /app/matmul /usr/local/bin/matmul
ENTRYPOINT ["matmul"]
