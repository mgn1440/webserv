FROM debian:stable

RUN apt update && apt upgrade && apt install siege vim procps -y 

# siege -b -c 100 -t 20S http://host.docker.internal:8000
# siege -b -c 100 -t 10S http://host.docker.internal:8000
# ENTRYPOINT ["siege"]

ENTRYPOINT ["tail", "-f"]
