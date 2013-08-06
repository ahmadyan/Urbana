# This is the makefile used to build daedalus
# tested on Mac OSX Lion using the gcc compiler.


HDRS = -I/usr/local/include -I/usr/include 
CXXFLAGS =	-O2 -g  -fmessage-length=0 -w -Wno-write-strings $(HDRS) -DDEBUG 
# These are the link time flags passed to gcc
LFLAGS = -L/usr/local/lib 

#LIBS = /usr/local/lib/libboost_thread.dylib /usr/local/lib/libboost_system.dylib

# this is the name of the output executable
TARGET = urbanaSAT
#VPATH = core model exec log kernel readers others

# dlib source
#SRCS = ../lib/dlib/all/source.cpp 

OBJS =	config.o	
OBJS += Graph.o				
OBJS += main.o
OBJS += Node.o 
OBJS += object.o
OBJS += Output.o
OBJS += plot.o
OBJS += SAT.o
OBJS += State.o
OBJS += utility.o


TMPS = $(SRCS:.cpp=.o)
OBJS += $(TMPS:.c=.o)


$(TARGET):	$(OBJS)
	@echo Linking $@
	$(CXX) -o $(TARGET) $(OBJS) $(LIBS)  $(HDRS) $(LFLAGS)
	@echo Build Complete	

all:	$(TARGET)

clean:
	rm -f $(OBJS) $(TARGET)
	@echo All object files and binaries removed

# Lion's make system does not need this.
#.cpp.o: $<
#	@echo Compiling $<
#	@$(CC) -c $(CFLAGS) $< -o $@

dep: 
	@echo Running makedepend
	@makedepend -- $(CFLAGS) -- $(SRC) 2> /dev/null 
	@echo Completed makedepend

	
#bison: parser.yy
#	bison -d parser.yy

#flex: tokenizer.l
#	flex tokenizer.l

#sim:
#	$(CC) $(CFLAGS) ScriptGenerator.cpp -o ScriptGenerator.exe -I$(BOOST)

#analysis:
#	$(CC) $(CFLAGS) ResultAnalysis.cpp -o ResultAnalysis.exe -I$(BOOST)


	