bonus :
	make all
	
all :
	docker compose -f ./srcs/docker-compose.yml up --build -d

down :
	-docker compose -f ./srcs/docker-compose.yml down

renginx :
	docker compose -f ./srcs/docker-compose.yml up --build -d nginx

rewordpress :
	docker compose -f ./srcs/docker-compose.yml up --build -d wordpress

remariadb :
	docker compose -f ./srcs/docker-compose.yml --build -d mariadb

re :
	make clean
	make all

clean :
	-docker compose -f ./srcs/docker-compose.yml down
	-docker container ps --all | awk '{print $$1}' | grep -v CONTAINER | xargs docker container rm -f
	-docker images | awk '{print $$3}' | grep -v "REPOSITORY" | tr '\n' ' ' | xargs docker image rm
	-docker volume ls | awk '{print $$2}' | grep -v "VOLUME" | tr '\n' ' ' | xargs docker volume rm