car: car_example.cpp
	g++ $< -o $@ -lpistache -lcrypto -lssl -lpthread
