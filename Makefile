build-exe:
	cmake -B build -S . -DCMAKE_TOOLCHAIN_FILE=$(HOME)/cpp/pkg/vcpkg/scripts/buildsystems/vcpkg.cmake
	cmake --build build

clean:
	rm -rf build