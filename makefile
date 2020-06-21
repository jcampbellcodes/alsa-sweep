src = $(wildcard *.cpp)
obj = $(src:.cpp=.o)

LDFLAGS = -lasound

booper: $(obj)
	$(CXX) -o $@ $^ $(LDFLAGS)

.PHONY: clean
clean:
	rm -f $(obj) booper

