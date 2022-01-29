
INCFLAGS =

CXX = clang++ -std=c++20 -fno-rtti
# ricing intensifies
#CFLAGS = $(INCFLAGS) -Ofast -march=native -flto -ffast-math -funroll-loops
CFLAGS = $(INCFLAGS) -Og -g3 -ggdb3
CXXFLAGS = $(CFLAGS) -Wall -Wextra
LDFLAGS = -flto -ldl -lm  -lreadline
target = run

src = \
	$(wildcard *.cpp) \

obj = $(src:.cpp=.o)
dep = $(obj:.o=.d)


$(target): $(obj)
	$(CXX) -o $@ $^ $(LDFLAGS)

-include $(dep)

# rule to generate a dep file by using the C preprocessor
# (see man cpp for details on the -MM and -MT options)
%.d: %.cpp
	$(CXX) $(CFLAGS) $< -MM -MT $(@:.d=.o) -MF $@

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $(DBGFLAGS) -o $@ $<

.PHONY: clean
clean:
	rm -f $(obj) $(target)

.PHONY: cleandep
cleandep:
	rm -f $(dep)

.PHONY: rebuild
rebuild: clean cleandep $(target)

.PHONY: sanity
sanity:
	./run sanity.msl