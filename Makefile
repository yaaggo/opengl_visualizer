cxx_compiler = g++
compiler_flags = -Wall -Wextra -std=c++17
linker_flags = -lfreeglut -lopengl32 -lglu32

src_dir = src
obj_dir = obj
bin_dir = bin

target_bin = $(bin_dir)/visualizer.exe

source_files = $(wildcard $(src_dir)/*.cpp)
object_files = $(patsubst $(src_dir)/%.cpp, $(obj_dir)/%.o, $(source_files))

all: $(target_bin)

$(target_bin): $(object_files) | $(bin_dir)
	$(cxx_compiler) $(object_files) -o $(target_bin) $(linker_flags)

$(obj_dir)/%.o: $(src_dir)/%.cpp | $(obj_dir)
	$(cxx_compiler) $(compiler_flags) -c $< -o $@

$(bin_dir) $(obj_dir):
	mkdir $@

run: all
	./$(target_bin)

clean:
	-cmd /c "rmdir /s /q obj bin"

.PHONY: all run clean
