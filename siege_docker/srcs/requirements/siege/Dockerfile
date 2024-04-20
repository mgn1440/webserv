FROM debian:stable

RUN apt update && apt upgrade && apt install siege vim procps -y 

RUN apt install nginx -y

# ENTRYPOINT [ "tail", "-f"]
# siege -b -c 100 -t 10S http://host.docker.internal:8000

# COPY ./conf/default /etc/nginx/sites-enabled

# CMD [ "nginx", "-g", "daemon off;" ]

# ENTRYPOINT ["siege"]
ENTRYPOINT ["tail", "-f"]
