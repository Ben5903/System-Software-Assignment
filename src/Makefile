# Specify the compiler
CC = gcc

# Compiler flags
CFLAGS = -Wall -Wextra

# Source files
SRC = backup_dash.c check_uploads.c collect_reports.c daemon.c \
	lock_directories.c sig_handler.c unlock_directories.c update_timer.c

# Object files (derived from source files)
OBJ = $(SRC:.c=.o)

# Name of the output executable
OUTPUT = mydaemon

# Compile the program
$(OUTPUT): $(OBJ)
		$(CC) $(CFLAGS) $(OBJ) -o $(OUTPUT)


# Compile source files into object files
%.o: %.c
	 $(CC) $(CFLAGS) -c $< -o $@

# Clean up object files and the output executable
clean:
	 rm -f $(OBJ) $(OUTPUT)