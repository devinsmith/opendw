# Launcher for CMake

export CMAKE_GENERATOR = Ninja
export CMAKE_COLOR_DIAGNOSTICS = ON

all: debug release

clean:
	rm -r build_cmake

clean_cache:
	rm -rf build_cmake/*/CMake*

debug:
	mkdir -p build_cmake/debug
	cd build_cmake/debug && cmake -DCMAKE_BUILD_TYPE=Debug ../..
	cd build_cmake/debug && cmake --build .

release:
	mkdir -p build_cmake/release
	cd build_cmake/release && cmake -DCMAKE_BUILD_TYPE=MinSizeRel ../..
	cd build_cmake/release && cmake --build .
