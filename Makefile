main: handle.o main.o
	$(CXX) $(CXXFLAGS) $+ -o $@
