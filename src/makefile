# compiler flags
check_error		= -Wall 
check_memory	= -fsanitize=address 
build_object	= -c 
optimize 		= -O2 

# source file name
sources			= main.c bline.c 
objects			= main.o bline.o 

# target file name
filename		= -o bline

# temporary file name
temp_files		= `echo *~`

debug:
	@echo -n "compile: $(sources)"
	@gcc $(build_object) $(sources) $(check_error) $(check_memory)
	@echo '... okay'

	@echo -n "linking: $(objects)"
	@gcc $(objects) $(filename) $(check_error) $(check_memory)
	@echo '... okay'

build:
	@echo -n "compile: $(sources)"
	@gcc $(build_object) $(sources) $(check_error)
	@echo '... okay'

	@echo -n "linking: $(objects)"
	@gcc $(objects) $(optimize) $(filename) $(check_error)
	@echo '... okay'

clean:
	@echo -n "cleaning: $(objects)"
	@rm -f *.o
	@echo '... okay'

	@echo -n "cleaning: $(temp_files)"
	@rm -f *~
	@echo '... okay'

