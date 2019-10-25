
main: http_downloader.o getip.o getrequest.o
	gcc -o http_downloader http_downloader.o getip.o getrequest.o -lcurl -lpthread

main.o: http_downloader.c getip.h getrequest.o
	gcc -c http_downloader.c

getip.o: getip.c getip.h
	gcc -c getip.c

getrequest.o: getrequest.c getreq.h
	gcc -c getrequest.c


clean:
	rm -rf http_downloader http_downloader.o getip.o getrequest.o


