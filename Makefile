OBJECTS = main.cpp

default: ofm2xplane

ofm2xplane: $(OBJECTS)
	g++ $(OBJECTS) -o $@

clean:
	-rm -f ofm2xplane
