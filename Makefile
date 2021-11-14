test:test.cc fiber.h
	$(CXX) -g -o $@ $< -std=c++11 -Wno-deprecated-declarations
