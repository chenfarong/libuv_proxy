CC=g++
FLAGS=-I./ -I../LibXs  -std=c++0x -D__LINUX__=1 -lpthread -lssl -lcrypto -ldl  -I/usr/include/mysql -I../libuv/include 
LIBS=-ldl -lpthread -lmysqlclient -lz -lm -lssl -lcrypto

all:

#	`svn info | tail -2 |head -1 | awk '{print "#define IDS_VERSION \"" $0 "\""  }' > version.h`
#	sh ../genver.sh
#	$(CC) -c $(FLAGS)
	
	$(CC) -c $(FLAGS) ../LibXs/XLog.cpp
	$(CC) -c $(FLAGS) ../LibXs/xnet.cpp
#	$(CC) -c $(FLAGS) ../LibXs/XTcpClient.cpp
	$(CC) -c $(FLAGS) ../LibXs/XServer/PosixDaemon.cpp
#H:\MSGx3\Gx3\LibXs\XServer\ServiceWin32.cpp

	$(CC) -c $(FLAGS) ../LibXs/XString/StrUtil.cpp
	$(CC) -c $(FLAGS) ../LibXs/XString/utf8util.cpp
	$(CC) -c $(FLAGS) ../LibXs/XString/XStringEx.cpp
	
	$(CC) -c $(FLAGS) GxProxy.cpp
	$(CC) -c $(FLAGS) main.cpp
	$(CC) -c $(FLAGS) myClient.cpp
	$(CC) -c $(FLAGS) myCmd.cpp
	$(CC) -c $(FLAGS) myConfig.cpp
	$(CC) -c $(FLAGS) myMQ.cpp
	$(CC) -c $(FLAGS) myProxy.cpp
	$(CC) -c $(FLAGS) myServer.cpp

#	$(CC) -c $(FLAGS)
#	$(CC) -c main.cpp $(FLAGS)

	$(CC) -o GxProxy  *.o   -L/usr/lib64/mysql ../libuv/lib/Linux64Release/libuv.a $(LIBS)
   
#	cp ./Game ../bin/Game
	
clean:
	rm *.o core.*
	rm Game

install:
	cp ./Game ../bin/linux/centos7/Game
	
