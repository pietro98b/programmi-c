CC = gcc

FLAGS = -Wall -pedantic -I ./intestazioni/ -g

EXE = eseguibile/farm

OGGETTI = oggetti/main.o oggetti/master.o oggetti/worker.o oggetti/conn.o oggetti/collector.o

$(EXE) : $(OGGETTI)
	$(CC) $(FLAGS) $^ -o $(EXE)
	
oggetti/master.o : sorgenti/master.c intestazioni/master.h intestazioni/worker.h intestazioni/collector.h
	$(CC) $(FLAGS) -c $< -o oggetti/master.o 
	
oggetti/worker.o : sorgenti/worker.c intestazioni/worker.h intestazioni/conn.h
	$(CC) $(FLAGS) -c $< -o oggetti/worker.o
	
oggetti/conn.o: sorgenti/conn.c intestazioni/conn.h
	$(CC) $(FLAGS) -c $< -o oggetti/conn.o

oggetti/collector.o : sorgenti/collector.c intestazioni/collector.h
	$(CC) $(FLAGS) -c $< -o oggetti/collector.o
	
oggetti/main.o : sorgenti/main.c sorgenti/master.c
	$(CC) $(FLAGS) -c $< -o oggetti/main.o
