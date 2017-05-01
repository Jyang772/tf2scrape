CC = g++
CFLAGS = -Wall -std=c++11
OBJECTS = main.o least_squares.o prices_model.o
LDLIBS = -lmysqlcppconn

tf2cpp: $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) -o tf2cpp $(LDLIBS)

%.o : %.cpp
	$(CC) $(CFLAGS) -c $<

clean:
	rm *.o tf2cpp
