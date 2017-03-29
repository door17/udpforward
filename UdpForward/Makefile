CXXFLAGS =	-O2 -g -Wall -fmessage-length=0

OBJS =		UdpForward.o

LIBS =

TARGET =	UdpForward.exe

$(TARGET):	$(OBJS)
	$(CXX) -o $(TARGET) $(OBJS) $(LIBS)

all:	$(TARGET)

clean:
	rm -f $(OBJS) $(TARGET)
