
.PHONY : all
all : out 

CFLAGS = -std=c99 -g -O -Wall -Wextra -Wno-unused-parameter
LDFLAGS =

SRC = $(wildcard *.c)
OBJ = $(SRC:.c=.o)

LD_OBJ = $(OBJ) $(patsubst %, %/out, $(LIBSUBDIRS))
out : $(LD_OBJ)
	$(CC) -o $@ $(CFLAGS) $(LDFLAGS) $(LD_OBJ)

ifeq ($(MAKECMDGOALS), )
 
%.d: %.c
	@set -e; rm -f $@; \
	 $(CC) -MM $(CFLAGS) $< > $@.$$$$; \
	 sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
	 $(RM) $@.$$$$

-include $(SRC:.c=.d)

endif

.PHONY : clean
clean:
	$(RM) out $(OBJ) $(OBJ:.o=.d)

